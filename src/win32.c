/**
 * @file win32.c
 * @brief The implementation file of the OS-specific internal Archivio functions for Windows OS.
 *
 * This file contains struct definitions and function implementations
 * of OS-specific internal Archivio functions for Windows OS.
 *
 * @copyright Copyright (c) 2023-2024 Osfabias
 * @license Licensed under the Apache License, Version 2.0.
 */
#include <stdlib.h>
#include <sys/stat.h>
#include <windows.h>

#include "internal.h"

struct _thread { HANDLE hthread; };
struct _mutex  { CRITICAL_SECTION critical_section; };
struct _cond   { CONDITION_VARIABLE cond; };

bool _dir_is_exists(const char *path) {
    const DWORD dwAttrib = GetFileAttributes(path);
    return dwAttrib != INVALID_FILE_ATTRIBUTES &&
           (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

void _mkdir(const char *path) {
  CreateDirectory(path, 0);
}

_thread_t* _thread_create(LPTHREAD_START_ROUTINE func) {
  _thread_t *thread;
  if (!(thread = malloc(sizeof(_thread_t)))) { return 0; };

  DWORD thread_id;
  thread->hthread = CreateThread(
    0,
    0,
    func,
    0,
    0,
    &thread_id
  );
  
  if (!thread->hthread) {
    free(thread);
    return 0;
  }

  return thread;
}

bool _thread_destroy(_thread_t *thread) {
  int res = TerminateThread(thread->hthread, 0); // TODO: use return code
  free(thread);
  return 1;
}

bool _thread_wait(_thread_t *thread) {
  return WaitForSingleObject(thread->hthread, INFINITE);
}

bool _thread_kill(_thread_t *thread) {
  return TerminateThread(thread->hthread, 0);
}

_mutex_t* _mutex_create(void) {
  _mutex_t *mutex;
  if (!(mutex = malloc(sizeof(_cond_t)))) { return 0; }

  InitializeCriticalSection(&mutex->critical_section);
  return mutex;
}

bool _mutex_destroy(_mutex_t *mutex) {
  DeleteCriticalSection(&mutex->critical_section);
  return 1;
}

bool _mutex_lock(_mutex_t *mutex) {
  EnterCriticalSection(&mutex->critical_section);
  return 1;
}

bool _mutex_unlock(_mutex_t *mutex) {
  LeaveCriticalSection(&mutex->critical_section);
  return 1;
}

_cond_t* _cond_create(void) {
  _cond_t *cond;
  if (!(cond = malloc(sizeof(_cond_t)))) { return 0; }
  InitializeConditionVariable(&cond->cond);
  return cond;
}

bool _cond_wait(_cond_t *cond, _mutex_t *mutex) {
  SleepConditionVariableCS(&cond->cond, &mutex->critical_section,
                           INFINITE);
  return 1;
}

bool _cond_signal(_cond_t *cond) {
  WakeConditionVariable(&cond->cond);
  return 1;
}

bool _cond_destroy(_cond_t *cond) {
  free(cond);
  return 1;
}

