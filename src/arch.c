#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "arch/arch.h"
#include "internal.h"

#define DEFAULT_PATH              "./logs/"

#ifndef ARCH_MAX_ENTRY_COUNT
  #define ARCH_MAX_ENTRY_COUNT    8
#endif

#ifndef ARCH_MAX_MESSAGE_LENGTH
  #define ARCH_MAX_MESSAGE_LENGTH 1024
#endif

typedef struct Entry {
  ArchLogLevel level;
  char         message[ARCH_MAX_MESSAGE_LENGTH];
} Entry;

struct ArchLogger {
  const char    *name;
  ArchLogLevel   level;

  char           messageFormats[ARCH_LOG_LEVEL_MAX_ENUM]
                               [ARCH_MAX_MESSAGE_LENGTH];

  File          *file;

  Mutex         *mutex;
  Cond          *cond;
  uint8_t        entryCount;
  Entry          entries[ARCH_MAX_ENTRY_COUNT];
  uint8_t        destroyRequested;
  Thread        *thread;
};

void processEntry(ArchLogger *logger, Entry *entry) {
  // Generate format string
  char format[ARCH_MAX_MESSAGE_LENGTH];
  formatString(logger->messageFormats[entry->level], entry->level,
               format, ARCH_MAX_MESSAGE_LENGTH);

  // Insert entry message into format
  char outMessage[ARCH_MAX_MESSAGE_LENGTH];
  snprintf(outMessage, ARCH_MAX_MESSAGE_LENGTH, format, entry->message);

  // Write entry
  fileWrite(logger->file, outMessage); // TODO: remove styles from message
  print(outMessage);                   // when writing it to a file
}

void* loggerCycle(void *arg) {
  ArchLogger *logger = (ArchLogger*)arg;
  Entry entry;
  uint8_t skipEntry;

  while (1) {
    mutexLock(logger->mutex);

    if (logger->entryCount == 0) {
      if (logger->destroyRequested) { break; }
      condWait(logger->cond, logger->mutex);
    }

    if (logger->entries[0].level < logger->level) {
      skipEntry = 1;
    }
    else {
      skipEntry = 0;

      entry.level = logger->entries[0].level;
      memoryCopy(entry.message, logger->entries[0].message,
            ARCH_MAX_MESSAGE_LENGTH);
    }

    memoryMove(logger->entries, logger->entries + 1,
               logger->entryCount * sizeof(Entry));
    --logger->entryCount;

    mutexUnlock(logger->mutex);

    if (logger->entryCount == ARCH_MAX_ENTRY_COUNT - 1) {
      condSignal(logger->cond);
    }

    if (skipEntry) { continue; }
    processEntry(logger, &entry);
  }

  return 0;
}

ArchLogger* archLoggerCreate(const ArchLoggerCreateInfo *info) {
  ArchLoggerCreateInfoEx exInfo = {
    .name = info->name,
    .fileNameFormat = info->fileNameFormat,
    .pathFormat = info->pathFormat,
    .level = info->level,
  };

  for (uint8_t i = 0; i < ARCH_LOG_LEVEL_MAX_ENUM; ++i) {
    exInfo.messageFormats[i] = memoryAlloc(ARCH_MAX_MESSAGE_LENGTH);
    // Casting const char* to void* in order to not
    // generate a compiler warning
    memoryCopy((void*)exInfo.messageFormats[i], info->messageFormat,
          ARCH_MAX_MESSAGE_LENGTH);
  }

  return archLoggerCreateEx(&exInfo);
}

uint8_t validateCreateInfoEx(const ArchLoggerCreateInfoEx *info) {
  if (!info) { return 0; }

  if (info->pathFormat &&
      info->pathFormat[strlen(info->pathFormat) - 1] != '/') {
    return 0;
  }

  // Validate log message formats
  for (uint_fast8_t i = 0; i < ARCH_LOG_LEVEL_MAX_ENUM; ++i) {
    if (!info->messageFormats[i]) { return 0; }
  }

  if (info->level < ARCH_LOG_LEVEL_TRACE ||
      info->level >= ARCH_LOG_LEVEL_MAX_ENUM) {
    return 0;
  }

  return 1;
}

uint8_t createThreadStuff(ArchLogger *logger) {
  logger->mutex = mutexCreate();
  if (!logger->mutex) { return 0; }

  logger->cond = condCreate();
  if (!logger->cond) { return 0; }

  logger->cond = condCreate();
  if (!logger->cond) { return 0; }

  logger->entryCount       = 0;
  logger->destroyRequested = 0;

  logger->thread = threadCreate(loggerCycle, logger);
  if (!logger->thread) { return 0; }

  return 1;
}

ArchLogger* archLoggerCreateEx(const ArchLoggerCreateInfoEx *info) {
  if (!validateCreateInfoEx(info)) { return 0; }

  ArchLogger *logger = memoryAlloc(sizeof(ArchLogger));

  logger->name  = info->name;
  logger->level = info->level;

  for (uint_fast8_t i = 0; i < ARCH_LOG_LEVEL_MAX_ENUM; ++i) {
    uint64_t formatSize = strlen(info->messageFormats[i]) + 1;
    memoryCopy(logger->messageFormats[i], info->messageFormats[i],
          formatSize);
  }

  if (info->fileNameFormat) {
    logger->file = fileCreate(info->fileNameFormat,
                              info->pathFormat);
    if (!logger->file) { memoryFree(logger); return 0; }
  }

  if (!createThreadStuff(logger)) {
    fileClose(logger->file);
    memoryFree(logger);
    return 0;
  }

  return logger;
}

void archLoggerDestroy(ArchLogger *logger) {
  logger->destroyRequested = 1;

  threadWait(logger->thread);

  condDestroy(logger->cond);
  mutexDestroy(logger->mutex);
  threadDestroy(logger->thread);

  fileClose(logger->file);

  memoryFree(logger);
}

void archLog(ArchLogger *logger, ArchLogLevel level,
             const char *message, ...) {

  mutexLock(logger->mutex);

  if (logger->entryCount == ARCH_MAX_ENTRY_COUNT) {
    condWait(logger->cond, logger->mutex);
  }

  Entry *entry = logger->entries + logger->entryCount;
  entry->level = level;

  // TODO: Figure out how to copy data from va_list
  // for future use (not pointers)
  char formattedMessage[ARCH_MAX_MESSAGE_LENGTH];

  va_list vaList;
  va_start(vaList, message);
  vsnprintf(formattedMessage, ARCH_MAX_MESSAGE_LENGTH, 
            message, vaList);
  va_end(vaList);

  memoryCopy(entry->message, formattedMessage, ARCH_MAX_MESSAGE_LENGTH);

  ++logger->entryCount;

  mutexUnlock(logger->mutex);

  if (logger->entryCount == 1) { condSignal(logger->cond); }
}
