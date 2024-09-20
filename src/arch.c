/**
 * @file arch.c
 * @brief The implementation file of the Archivio API.
 *
 * This file contains the implementation of the public Archivio 
 * functions.
 *
 * @copyright Copyright (c) 2023-2024 Osfabias
 * @license Licensed under the Apache License, Version 2.0.
 */
#include <time.h>   // time_t, time()
#include <stdio.h>  // file i/o, snprintf()
#include <stdarg.h> // va_list and va_*() functions
#include <string.h> // memcpy, memmove
#include <stdlib.h> // malloc, free
#include <stdint.h> // int types with constant sizes

#include "arch/arch.h"
#include "internal.h"

#define DEFAULT_PATH_FORMAT "./logs/"

// NOTE: The maximum size of the message is affecting the performance.
#define MAX_MESSAGE_LENGTH 512

typedef struct _entry {
  arch_logger_t   *logger;
  arch_log_level_t level;
  char             message[MAX_MESSAGE_LENGTH];
  time_t           time;
} _entry_t;

struct arch_logger {
  arch_log_level_t level;

  char msg_formats[ARCH_LOG_LEVEL_MAX_ENUM][MAX_MESSAGE_LENGTH];
  char file_msg_formats[ARCH_LOG_LEVEL_MAX_ENUM][MAX_MESSAGE_LENGTH];
  FILE *fd;

  uint8_t entry_count;
  bool    destroy_requested;
};

static struct {
  uint8_t   max_entry_count;
  uint8_t   entry_count;
  _entry_t *entries;

  _mutex_t  *mutex;
  _cond_t   *cond;
  _thread_t *thread;

  bool threadIsAlive;
  bool _thread_killRequested;
} s_arch_state;

ROUTINE_PRFX routine(ROUTINE_PARAMS);

FILE* _file_create(const char *filename_format, const char *path_format);

bool _format_str(const char *format, time_t time, const char *msg,
                  char *out, size_t size);

bool arch_init(const arch_init_info_t *info) {
  s_arch_state.entry_count          = 0;
  s_arch_state.max_entry_count       = info->max_entry_count;

  s_arch_state.entries = malloc(sizeof(_entry_t) * info->max_entry_count);
  if (!s_arch_state.entries) { goto FAIL_ENTRIES; }

  if (!(s_arch_state.cond   = _cond_create()))          { goto FAIL_COND;   }
  if (!(s_arch_state.mutex  = _mutex_create()))         { goto FAIL_MUTEX;  }
  if (!(s_arch_state.thread = _thread_create(routine))) { goto FAIL_THREAD; }

  s_arch_state._thread_killRequested = false;
  s_arch_state.threadIsAlive       = true;

  return true;
FAIL_THREAD:  _thread_destroy(s_arch_state.thread);
FAIL_MUTEX:   _mutex_destroy(s_arch_state.mutex);
FAIL_COND:    _cond_destroy(s_arch_state.cond);
FAIL_ENTRIES: free(s_arch_state.entries);
  return false;
}

void arch_terminate(void) {
  s_arch_state._thread_killRequested = true;

  if (
    s_arch_state.entry_count == 0 &&
    !_cond_signal(s_arch_state.cond)
  ) { return; }

  if (!_thread_wait(s_arch_state.thread)) { return; }

  _cond_destroy(s_arch_state.cond);
  _mutex_destroy(s_arch_state.mutex);
  _thread_destroy(s_arch_state.thread);

  free(s_arch_state.entries);
}

arch_logger_t* arch_logger_create(const arch_logger_create_info_t *info) {
  arch_logger_t *logger;
  if (!(logger = malloc(sizeof(arch_logger_t)))) { return NULL; }

  if (!info->filename_format) { goto SKIP_FILE_CREATION; }

  logger->fd= _file_create(
    info->filename_format,
    info->path_format ? info->path_format : DEFAULT_PATH_FORMAT
  );

  if (!logger->fd) {
    free(logger);
    return NULL;
  }

  for (uint_fast8_t i = 0; i < ARCH_LOG_LEVEL_MAX_ENUM; ++i) {
    memcpy(
      logger->file_msg_formats[i],
      info->file_msg_formats[i],
      MAX_MESSAGE_LENGTH
    );
  }

SKIP_FILE_CREATION:
  for (uint_fast8_t i = 0; i < ARCH_LOG_LEVEL_MAX_ENUM; ++i) {
    memcpy(
      logger->msg_formats[i],
      info->msg_formats[i],
      MAX_MESSAGE_LENGTH
    );
  }

  logger->entry_count       = 0;
  logger->destroy_requested = 0;
  logger->level             = info->level;

  return logger;
}

bool arch_is_alive(void) {
  return s_arch_state.threadIsAlive;
}

void arch_logger_destroy(arch_logger_t *logger) {
  logger->destroy_requested = 1;
}

bool arch_log(arch_logger_t *logger, arch_log_level_t level,
             const char *msg, ...) {
  va_list valist;

  va_start(valist, msg);
  bool res = arch_logvl(logger, level, msg, valist);
  va_end(valist);

  return res;
}

bool arch_logvl(arch_logger_t *logger, arch_log_level_t level,
                const char *msg, va_list valist) {
  if (!_mutex_lock(s_arch_state.mutex)) { return false; }

  if (
    s_arch_state.entry_count == s_arch_state.max_entry_count &&
    !_cond_wait(s_arch_state.cond, s_arch_state.mutex)
  ) { return false; }

  _entry_t *entry  = s_arch_state.entries + s_arch_state.entry_count;
  entry->level  = level;
  entry->logger = logger;

  // TODO: Figure out how to copy data from va_list
  // for future use (not pointers)
  char formattedMessage[MAX_MESSAGE_LENGTH];
  if (vsnprintf(formattedMessage, MAX_MESSAGE_LENGTH, msg, valist) < 0) {
    return false;
  }
  memcpy(entry->message, formattedMessage, MAX_MESSAGE_LENGTH);

  // later destrloyed in logging cycle after processing entry
  entry->time = time(0);

  ++logger->entry_count;
  ++s_arch_state.entry_count;

  if (!_mutex_unlock(s_arch_state.mutex)) { return false; }

  if (s_arch_state.entry_count == 1 && !_cond_signal(s_arch_state.cond)) {
    return false;
  }

  return true;
}

bool processEntry(_entry_t *e) {
  char outMessage[MAX_MESSAGE_LENGTH];
  if (!_format_str(
    e->logger->msg_formats[e->level],
    e->time,
    e->message,
    outMessage,
    MAX_MESSAGE_LENGTH
  )) { return 0; }

  if (fputs(outMessage, stdout) == EOF) { return false; }

  if (e->logger->fd) {
    char outMessage[MAX_MESSAGE_LENGTH];
    if (!_format_str(
      e->logger->file_msg_formats[e->level],
      e->time,
      e->message,
      outMessage,
      MAX_MESSAGE_LENGTH
    )) { return false; }

    if (fputs(outMessage, e->logger->fd) == EOF) { return false; }
  }

  return 1;
}

ROUTINE_PRFX routine(ROUTINE_PARAMS) {
  // suppress "Unused parameter" compiler warning
  { ROUTINE_PARAMS_UNUSED }

  _entry_t entry;
  bool  skipEntry;

  while (1) {
    if (!_mutex_lock(s_arch_state.mutex)) { return NULL; }

    if (s_arch_state.entry_count == 0) {
      if (s_arch_state._thread_killRequested) { break; }

      if (!_cond_wait(s_arch_state.cond, s_arch_state.mutex)) { return NULL; }

      if (
        s_arch_state.entry_count == 0 &&
        s_arch_state._thread_killRequested
      ) { break; }
    }

    entry = s_arch_state.entries[0];

    if (entry.level < entry.logger->level) {
      skipEntry = true;
    }
    else {
      skipEntry = false;
      memcpy(
        entry.message,
        s_arch_state.entries[0].message,
        MAX_MESSAGE_LENGTH
      );
    }

    memmove(
      s_arch_state.entries,
      s_arch_state.entries + 1,
      s_arch_state.entry_count * sizeof(_entry_t)
    );

    --s_arch_state.entry_count;
    --entry.logger->entry_count;

    if (!_mutex_unlock(s_arch_state.mutex)) { return NULL; }

    if (
      s_arch_state.entry_count == s_arch_state.max_entry_count - 1 &&
      !_cond_signal(s_arch_state.cond)
    ) { return NULL; }

    if (!(skipEntry || processEntry(&entry))) { return 0; }

    if (
      entry.logger->destroy_requested &&
      entry.logger->entry_count == 0
    ) {
      fclose(entry.logger->fd); // don't care about return code
      free(entry.logger);
    }
  }

  s_arch_state.threadIsAlive = false;
  return NULL;
}

FILE* _file_create(const char *filename_format, const char *path_format) {
  FILE *file;
  time_t time_stamp = time(0);

  char filename[256];
  if (!_format_str(filename_format, time_stamp, "", filename, 255)) {
    return NULL;
  }
  
  char path[256 + 256];
  if (!_format_str(path_format, time_stamp, "", path, 511)) {
    return NULL;
  }

  const uint16_t pathLen = strlen(path);
  for (uint16_t i = 0; i < pathLen; ++i) {
    if (path[i] != '/') { continue; }

    path[i] = '\0';

    if (!(_dir_is_exists(path) || _mkdir(path))) {
      return NULL;
    }
    path[i] = '/';
  }

  strncat(path, filename, 511);

  if (!(file = fopen(path, "w"))) { return NULL; }
  return file;
}

// Used for snprintf calls in _format_str
#define FORMAT(str, len, fmt, ...) \
  if (snprintf(str, len, fmt, __VA_ARGS__) < 0) { return false; } \

bool _format_str(const char *format, time_t time, const char *msg,
                 char *out, size_t size) {
  memcpy(out, format, size);

  char tmpStr[size];

  struct tm *timeInfo;
  timeInfo = localtime(&time);

  for (uint64_t i = 0; i < (size - 1) && out[i] != '\0'; ++i) {
    if (out[i] != '#') { continue; }

    char specifier = out[i + 1];

    out[i]   = '%';
    out[++i] = 's';
    switch (specifier) {
      case 's': // seconds
        FORMAT(tmpStr, size, out, "%02d")
        FORMAT(out, size, tmpStr, timeInfo->tm_sec);
        break;
      case 'm': // minutes
        FORMAT(tmpStr, size, out, "%02d"); 
        FORMAT(out, size, tmpStr, timeInfo->tm_min);
        break;
      case 'h': // hours
        FORMAT(tmpStr, size, out, "%02d"); 
        FORMAT(out, size, tmpStr, timeInfo->tm_hour);
        break;
      case 'd': // days
        FORMAT(tmpStr, size, out, "%02d"); 
        FORMAT(out, size, tmpStr, timeInfo->tm_mday);
        break;
      case 'M': // month
        FORMAT(tmpStr, size, out, "%02d"); 
        FORMAT(out, size, tmpStr, timeInfo->tm_mon); 
        break;
      case 'y': // year
        FORMAT(tmpStr, size, out, "%d"); 
        // adding 1900 because tmFORMATyear represent a number
        // of years passed since 1900
        FORMAT(out, size, tmpStr, timeInfo->tm_year + 1900);
        break;
      case '}': // reset style
        memcpy(tmpStr, out, size);
        FORMAT(out, size, tmpStr, "\033[0m");
        break;
      case 't':
        memcpy(tmpStr, out, size);
        FORMAT(out, size, tmpStr, msg);
        break;
      default: { out[i] = 'n'; }
    }
  }

  return true;
}

