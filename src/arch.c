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

#include "arch.h"
#include "uthread.h"
#include "internal.h"

// NOTE: The maximum size of the message is affecting the performance.
#define MAX_MESSAGE_LENGTH 1024

#define MAX_ENTRY_COUNT 8

typedef struct _entry {
  struct arch_logger *logger;
  arch_log_level_t    level;
  char                message[MAX_MESSAGE_LENGTH];
  time_t              time;
} _entry;

struct arch_logger {
  arch_log_level_t level;

  char msg_fmts[ARCH_LOG_LEVEL_MAX_ENUM][MAX_MESSAGE_LENGTH];
  char file_msg_fmts[ARCH_LOG_LEVEL_MAX_ENUM][MAX_MESSAGE_LENGTH];
  FILE *fd;

  int entry_count;
  int destroy_requested;
};

static struct {
  int logger_count;

  int    entry_count;
  _entry entries[MAX_ENTRY_COUNT];

  ut_mutex_t  mutex;
  ut_cond_t   cond;
  ut_thread_t thread;

  int thread_is_alive;
  int thread_kill_requested;
} s_arch_state = { .thread_is_alive = 0 };

static UT_ROUTINE_RETURN_TYPE _logging_routine(void *arg);

static FILE* _file_create(
  const char *filename_fmt,
  const char *path_fmt
);

static int _fmt_str(
  const char *fmt,
  time_t      time,
  const char *msg,
  char       *out,
  size_t      size
);

int _init(void) {
  s_arch_state.entry_count = 0;

  if (!ut_cond_create(&s_arch_state.cond))   goto FAIL_COND;
  if (!ut_mutex_create(&s_arch_state.mutex)) goto FAIL_MUTEX;

  if (!ut_thread_create(&s_arch_state.thread, _logging_routine, NULL))
    goto FAIL_THREAD;

  s_arch_state.thread_kill_requested = 0;
  s_arch_state.thread_is_alive       = 1;

  return 1;
FAIL_THREAD: ut_thread_kill(s_arch_state.thread);
FAIL_MUTEX:  ut_mutex_destroy(&s_arch_state.mutex);
FAIL_COND:   ut_cond_destroy(&s_arch_state.cond);
  return 0;
}

void _quit(void) {
  s_arch_state.thread_kill_requested = 1;

  if (
    s_arch_state.entry_count == 0 &&
    !ut_cond_signal(&s_arch_state.cond)
  ) { return; }

  if (!ut_thread_wait(s_arch_state.thread))
    return;

  ut_cond_destroy(&s_arch_state.cond);
  ut_mutex_destroy(&s_arch_state.mutex);
  ut_thread_kill(s_arch_state.thread);

  free(s_arch_state.entries);
}

struct arch_logger* arch_logger_create(
  const arch_logger_create_info_t *info
) {
  struct arch_logger *logger = malloc(sizeof(struct arch_logger));
  if (!logger)
    return NULL;

  /************************ file creation ***********************/
  if (!info->filename_fmt)
    goto SKIP_FILE_CREATION;

  logger->fd= _file_create(info->filename_fmt, info->path_fmt);

  if (!logger->fd) {
    free(logger);
    return NULL;
  }

  for (int i = 0; i < ARCH_LOG_LEVEL_MAX_ENUM; ++i)
    memcpy(
      logger->file_msg_fmts[i], info->file_msg_fmts[i],
      MAX_MESSAGE_LENGTH
    );
  /************************ file creation ***********************/
SKIP_FILE_CREATION:
  for (int i = 0; i < ARCH_LOG_LEVEL_MAX_ENUM; ++i)
    memcpy(logger->msg_fmts[i], info->msg_fmts[i], MAX_MESSAGE_LENGTH);

  logger->entry_count       = 0;
  logger->destroy_requested = 0;
  logger->level             = info->level;

  ++s_arch_state.logger_count;
  if (s_arch_state.logger_count == 1)
    _init();

  return logger;
}

void arch_logger_destroy(struct arch_logger* logger) {
  logger->destroy_requested = 1;
}

int arch_log(
  struct arch_logger *logger, arch_log_level_t level,
  const char *msg, ...
) {
  va_list valist;

  va_start(valist, msg);
  int res = arch_logvl(logger, level, msg, valist);
  va_end(valist);

  return res;
}

int arch_logvl(
  struct arch_logger *logger, arch_log_level_t level,
  const char *msg, va_list valist
) {
  if (!ut_mutex_lock(&s_arch_state.mutex))
    return 0;

  if (
    s_arch_state.entry_count == MAX_ENTRY_COUNT &&
    !ut_cond_wait(&s_arch_state.cond, &s_arch_state.mutex)
  ) return 0;

  _entry *entry  = s_arch_state.entries + s_arch_state.entry_count;
  entry->level   = level;
  entry->logger  = logger;

  // TODO: Figure out how to copy data from va_list
  // for future use (not pointers)
  char out_msg[MAX_MESSAGE_LENGTH];
  if (vsnprintf(out_msg, MAX_MESSAGE_LENGTH, msg, valist) < 0)
    return 0;

  memcpy(entry->message, out_msg, MAX_MESSAGE_LENGTH);

  // later destrloyed in logging cycle after processing entry
  entry->time = time(NULL);

  ++logger->entry_count;
  ++s_arch_state.entry_count;

  if (!ut_mutex_unlock(&s_arch_state.mutex))
    return 0;

  if (
    s_arch_state.entry_count == 1 &&
    !ut_cond_signal(&s_arch_state.cond)
  ) { return 0; }

  return 1;
}

int _process_entry(_entry *e) {
  char out_msg[MAX_MESSAGE_LENGTH];
  if (
    !_fmt_str(
      e->logger->msg_fmts[e->level],
      e->time,
      e->message,
      out_msg,
      MAX_MESSAGE_LENGTH
    )
  ) { return 0; }

  if (fputs(out_msg, stdout) == EOF) { return 0; }

  if (e->logger->fd) {
    char out_msg[MAX_MESSAGE_LENGTH];
    if (!_fmt_str(
      e->logger->file_msg_fmts[e->level],
      e->time,
      e->message,
      out_msg,
      MAX_MESSAGE_LENGTH
    )) { return 0; }

    if (fputs(out_msg, e->logger->fd) == EOF)
      return 0;
  }

  return 1;
}

UT_ROUTINE_RETURN_TYPE _logging_routine(void *arg) {
  // suppress "Unused parameter" compiler warning
  (void)(arg);

  _entry entry;
  int    skip_entry;

  while (1) {
    if (!ut_mutex_lock(&s_arch_state.mutex))
      return NULL;

    if (s_arch_state.entry_count == 0) {
      if (s_arch_state.thread_kill_requested) { break; }

      if (!ut_cond_wait(&s_arch_state.cond, &s_arch_state.mutex))
        return NULL;

      if (
        s_arch_state.entry_count == 0 &&
        s_arch_state.thread_kill_requested
      ) { break; }
    }

    entry = s_arch_state.entries[0];

    if (entry.level < entry.logger->level) {
      skip_entry = 1;
    }
    else {
      skip_entry = 0;
      memcpy(
        entry.message,
        s_arch_state.entries[0].message,
        MAX_MESSAGE_LENGTH
      );
    }

    memmove(
      s_arch_state.entries,
      s_arch_state.entries + 1,
      s_arch_state.entry_count * sizeof(_entry)
    );

    --s_arch_state.entry_count;
    --entry.logger->entry_count;

    if (!ut_mutex_unlock(&s_arch_state.mutex))
      return NULL;

    if (
      s_arch_state.entry_count == MAX_ENTRY_COUNT - 1 &&
      !ut_cond_signal(&s_arch_state.cond)
    ) { return NULL; }

    if (!(skip_entry || _process_entry(&entry)))
      return 0;

    if (
      entry.logger->destroy_requested &&
      entry.logger->entry_count == 0
    ) {
      fclose(entry.logger->fd); // don't care about return code
      free(entry.logger);

      if (--s_arch_state.logger_count == 0)
        break;
    }
  }

  s_arch_state.thread_is_alive = 0;
  return NULL;
}

FILE* _file_create(const char *filename_fmt, const char *path_fmt) {
  FILE *file;
  time_t time_stamp = time(0);

  char filename[256];
  if (!_fmt_str(filename_fmt, time_stamp, "", filename, 255))
    return NULL;
  
  char path[256 + 256];
  if (!_fmt_str(path_fmt, time_stamp, "", path, 511))
    return NULL;

  const uint16_t pathLen = strlen(path);
  for (uint16_t i = 0; i < pathLen; ++i) {
    if (path[i] != '/') { continue; }

    path[i] = '\0';

    if (!(_is_dir_exists(path) || _mkdir(path)))
      return NULL;

    path[i] = '/';
  }

  strncat(path, filename, 511);

  if (!(file = fopen(path, "w"))) { return NULL; }
  return file;
}

// Used for snprintf calls in _fmt_str
#define fmt(str, len, fmt, ...) \
  if (snprintf(str, len, fmt, __VA_ARGS__) < 0) { return 0; }

int _fmt_str(
  const char *fmt,
  time_t time,
  const char *msg,
  char *out,
  size_t size
) {
  memcpy(out, fmt, size);

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
        fmt(tmpStr, size, out, "%02d")
        fmt(out, size, tmpStr, timeInfo->tm_sec);
        break;
      case 'm': // minutes
        fmt(tmpStr, size, out, "%02d"); 
        fmt(out, size, tmpStr, timeInfo->tm_min);
        break;
      case 'h': // hours
        fmt(tmpStr, size, out, "%02d"); 
        fmt(out, size, tmpStr, timeInfo->tm_hour);
        break;
      case 'd': // days
        fmt(tmpStr, size, out, "%02d"); 
        fmt(out, size, tmpStr, timeInfo->tm_mday);
        break;
      case 'M': // month
        fmt(tmpStr, size, out, "%02d"); 
        fmt(out, size, tmpStr, timeInfo->tm_mon); 
        break;
      case 'y': // year
        fmt(tmpStr, size, out, "%d"); 
        // adding 1900 because tmfmtyear represent a number
        // of years passed since 1900
        fmt(out, size, tmpStr, timeInfo->tm_year + 1900);
        break;
      case '}': // reset style
        memcpy(tmpStr, out, size);
        fmt(out, size, tmpStr, "\033[0m");
        break;
      case 't':
        memcpy(tmpStr, out, size);
        fmt(out, size, tmpStr, msg);
        break;
      default: { out[i] = 'n'; }
    }
  }

  return 1;
}

