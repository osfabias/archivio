#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>

#include "arch/arch.h"
#include "internal.h"

struct File   { FILE *file; };
struct Thread { pthread_t thread; };
struct Mutex  { pthread_mutex_t mutex; };
struct Cond   { pthread_cond_t cond; };

static const char* s_levelNames[ARCH_LOG_LEVEL_MAX_ENUM] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL",
};

static const char* s_defaultStyles[ARCH_LOG_LEVEL_MAX_ENUM] = {
  "\e[37m",
  "\e[34m",
  "\e[32m",
  "\e[1;30;43m",
  "\e[1;97;41m",
  "\e[1;97;41m",
};

void* memoryAlloc(uint64_t size) {
  return malloc(size);
}

void memoryFree(void *block) {
  free(block);
}

void memoryCopy(void *dst, const void *src, uint64_t size) {
  memcpy(dst, src, size);
}

void memoryMove(void *dst, const void *src, uint64_t size) {
  memmove(dst, src, size);
}

void formatString(const char *format, ArchLogLevel level,
                  char *out, uint64_t size) {
  memcpy(out, format, size);

  char tmpStr[size];

  // Get time for future use
  time_t currentTime;
  currentTime = time(0);
  struct tm *timeInfo;
  timeInfo = localtime(&currentTime);

  uint64_t messageTextInd;

  for (uint_fast64_t i = 0; i < (size - 1) || out[i] != '\0'; ++i) {
    if (out[i] != '#') { continue; }

    char specifier = out[i + 1];

    out[i]     = '%';
    out[i + 1] = 's';
    switch (specifier) {
      case 's': // seconds
        snprintf(tmpStr, size, out, "%02d"); 
        snprintf(out, size, tmpStr, timeInfo->tm_sec);
        break;
      case 'm': // minutes
        snprintf(tmpStr, size, out, "%02d"); 
        snprintf(out, size, tmpStr, timeInfo->tm_min);
        break;
      case 'h': // hours
        snprintf(tmpStr, size, out, "%02d"); 
        snprintf(out, size, tmpStr, timeInfo->tm_hour);
        break;
      case 'd': // days
        snprintf(tmpStr, size, out, "%02d"); 
        snprintf(out, size, tmpStr, timeInfo->tm_mday);
        break;
      case 'M': // month
        snprintf(tmpStr, size, out, "%02d"); 
        snprintf(out, size, tmpStr, timeInfo->tm_mon); 
        break;
      case 'y': // year
        snprintf(tmpStr, size, out, "%d"); 
        // adding 1900 because tm_year represent a number
        // of years passed since 1900
        snprintf(out, size, tmpStr, timeInfo->tm_year + 1900);
        break;
      case 'l': // level name
        memcpy(tmpStr, out, size);
        snprintf(out, size, tmpStr, s_levelNames[level]);
        break;
      case '{': // apply default style
        memcpy(tmpStr, out, size);
        snprintf(out, size, tmpStr, s_defaultStyles[level]);
        break;
      case '}': // reset style
        memcpy(tmpStr, out, size);
        snprintf(out, size, tmpStr, "\033[0m");
        break;
      case 't':
        out[i] = '#';
        messageTextInd = i;
        break;
      default: {
        out[i + 1] = 'n';
      }
    }

    ++i;
  }

  out[messageTextInd] = '%';
}

void print(const char *message) {
  fputs(message, stdout);
}

File* fileCreate(const char *fileNameFormat, const char *pathFormat) {
  File *file = malloc(sizeof(File));

  char fileName[256];
  formatString(fileNameFormat, 0, fileName, 256);
  
  char path[256 + 512];
  formatString(pathFormat, 0, path, 512);

  /* create dirs */
  uint16_t charInd = -1;
  const uint16_t pathLen = strlen(path);
  while(++charInd < pathLen) {
    if (path[charInd] != '/') { continue; }

    path[charInd] = '\0';

    DIR *dir = opendir(path);
    if (!dir) {
      mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    else { closedir(dir); }
    path[charInd] = '/';
  }

  strcat(path, fileName);

  file->file = fopen(path, "w");
  if (!file->file) { free(file); return 0; }
  return file;
}

void fileClose(File *file) {
  fclose(file->file);
  free(file);
}

void fileWrite(File *file, const char *message) {
  fputs(message, file->file);
}

Thread* threadCreate(void*(*func)(void *arg), void *arg) {
  Thread *thread = malloc(sizeof(Thread));
  if (pthread_create(&thread->thread, 0, func, arg) != 0) {
    free(thread);
    return 0;
  }
  return thread;
}

void threadDestroy(Thread *thread) {
  pthread_kill(thread->thread, 0);
  free(thread);
}

void threadWait(Thread *thread) {
  pthread_join(thread->thread, 0);
}

Cond* condCreate() {
  Cond *cond = malloc(sizeof(Cond));
  if (pthread_cond_init(&cond->cond, 0) != 0) {
    free(cond);
    return 0;
  }
  return cond;
}

Mutex* mutexCreate() {
  Mutex *mutex = malloc(sizeof(Cond));
  if (pthread_mutex_init(&mutex->mutex, 0) != 0) {
    free(mutex);
    return 0;
  }
  return mutex;
}

void mutexDestroy(Mutex *mutex) {
  pthread_mutex_destroy(&mutex->mutex);
  free(mutex);
}

void mutexLock(Mutex *mutex) {
  pthread_mutex_lock(&mutex->mutex);
}

void mutexUnlock(Mutex *mutex) {
  pthread_mutex_unlock(&mutex->mutex);
}

void condWait(Cond *cond, Mutex *mutex) {
  pthread_cond_wait(&cond->cond, &mutex->mutex);
}

void condSignal(Cond *cond) {
  pthread_cond_signal(&cond->cond);
}

void condDestroy(Cond *cond) {
  pthread_cond_destroy(&cond->cond);
  free(cond);
}
