/**
 * @file unix.c
 * @brief The implementation file of the OS-specific internal Archivio functions for Unix systems.
 *
 * This file contains struct definitions and function implementations
 * of OS-specific internal Archivio functions for Unix systems.
 *
 * @copyright Copyright (c) 2023-2024 Osfabias
 * @license Licensed under the Apache License, Version 2.0.
 */
#include <stdlib.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/stat.h>

#include "internal.h"

struct _thread { pthread_t thread;      };
struct _mutex  { pthread_mutex_t mutex; };
struct _cond   { pthread_cond_t cond;   };

bool _dir_is_exists(const char *path) {
  DIR *dir = opendir(path);
  bool _dir_is_exists = !dir;
  if (dir) {
    closedir(dir); // ignore failure
  }
  return !_dir_is_exists;
}

bool _mkdir(const char *path) {
  return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}

_thread_t* _thread_create(void*(*func)(void *arg)) {
  _thread_t *thread;
  if (!(thread = malloc(sizeof(_thread_t)))) { return 0; };

  if (pthread_create(&thread->thread, 0, func, 0) != 0) {
    free(thread);
    return 0;
  }
  return thread;
}

bool _thread_destroy(_thread_t *thread) {
  bool res = pthread_kill(thread->thread, 0) == 0;
  free(thread);
  return res;
}

bool _thread_wait(_thread_t *thread) {
  return pthread_join(thread->thread, 0) == 0;
}

bool _thread_kill(_thread_t *thread) {
  return pthread_kill(thread->thread, 0);
}

_cond_t* _cond_create(void) {
  _cond_t *cond;
  if (!(cond = malloc(sizeof(_cond_t)))) { return 0; }

  if (pthread_cond_init(&cond->cond, 0) != 0) {
    free(cond);
    return 0;
  }
  return cond;
}

_mutex_t* _mutex_create(void) {
  _mutex_t *mutex;
  if (!(mutex = malloc(sizeof(_cond_t)))) { return 0; }

  if (pthread_mutex_init(&mutex->mutex, 0) != 0) {
    free(mutex);
    return NULL;
  }
  return mutex;
}

bool _mutex_destroy(_mutex_t *mutex) {
  bool res = pthread_mutex_destroy(&mutex->mutex) == 0;
  free(mutex);
  return res;
}

bool _mutex_lock(_mutex_t *mutex) {
  return pthread_mutex_lock(&mutex->mutex) == 0;
}

bool _mutex_unlock(_mutex_t *mutex) {
  return pthread_mutex_unlock(&mutex->mutex) == 0;
}

bool _cond_wait(_cond_t *cond, _mutex_t *mutex) {
  return pthread_cond_wait(&cond->cond, &mutex->mutex) == 0;
}

bool _cond_signal(_cond_t *cond) {
  return pthread_cond_signal(&cond->cond) == 0;
}

bool _cond_destroy(_cond_t *cond) {
  bool res = pthread_cond_destroy(&cond->cond) == 0;
  free(cond);
  return res;
}
