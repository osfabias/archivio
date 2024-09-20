/**
 * @file internal.h
 * @brief The header of the boolernal Archivio functions.
 *
 * This file contains type and function definitions of the boolernal
 * OS-specific Archivio functions.
 *
 * @copyright Copyright (c) 2023-2024 Osfabias
 * @license Licensed under the Apache License, Version 2.0.
 */
#pragma once
#include <stdarg.h>  // variadic arguments
#include <stdbool.h> // bool

#ifdef _WIN32
#include <windows.h>
#define ROUTINE_PRFX WIN_API
#define ROUTINE_PARAMS LPVOID arg
#define ROUTINE_PARAMS_UNUSED (void)(arg);
typedef LPTHREAD_START_ROUTINE threadRoutine;
#else
#define ROUTINE_PRFX   void*
#define ROUTINE_PARAMS void *arg
#define ROUTINE_PARAMS_UNUSED (void)(arg);
typedef void*(*threadRoutine)(void*);
#endif

typedef struct _thread _thread_t;
typedef struct _mutex  _mutex_t;
typedef struct _cond   _cond_t;

bool _dir_is_exists(const char *path);
bool _mkdir(const char *path);

_thread_t* _thread_create(threadRoutine func);
bool    _thread_destroy(_thread_t *thread);
bool    _thread_wait(_thread_t *thread);
bool    _thread_kill(_thread_t *thread);

_mutex_t* _mutex_create(void);
bool   _mutex_destroy(_mutex_t *mutex);
bool   _mutex_lock(_mutex_t *mutex);
bool   _mutex_unlock(_mutex_t *mutex);

_cond_t* _cond_create(void);
bool  _cond_wait(_cond_t *cond, _mutex_t *mutex);
bool  _cond_signal(_cond_t *cond);
bool  _cond_destroy(_cond_t *cond);
