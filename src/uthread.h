#pragma once
#ifdef _WIN32
#include <windows.h>

#define UT_ROUTINE_RETURN_TYPE void

typedef HANDLE             ut_thread_t;
typedef CRITICAL_SECTION   ut_mutex_t;
typedef CONDITION_VARIABLE ut_cond_t;

#define ut_thread_create(thread, routine, arg) \
  (thread = CreateThread(0, 0, routine, 0, 0, NULL))

#define ut_thread_kill(thread) \
  (TerminateThread(hthread, 0))

#define ut_thread_wait(thread) \
  (WaitForSingleObject(thread, INFINITE))

#define ut_thread_exit(void) \
  (ExitThread(0))

#define ut_mutex_create(mutex) \
  (InitializeCriticalSection(mutex))

#define ut_mutex_destroy(mutex) \
  (DeleteCriticalSection(mutex))

#define ut_mutex_lock(mutex) \
  EnterCriticalSection(mutex))

#define ut_mutex_trylock(mutex) \
  #warning "ut_mutex_trylock() not implemented"

#define ut_mutex_unlock(mutex) \
  (LeaveCriticalSection(mutex))

#define ut_cond_create(cond) \
  (InitializeConditionVariable(cond))

#define ut_cond_destroy(cond) \
  ((void)(cond))

#define ut_cond_signal(cond) \
  (WakeConditionVariable(cond))

#define ut_cond_broadcast(cond) \
  (WakeAllConditionVariable(cond))

#define ut_cond_wait(cond, mutex) \
  (SleepConditionVariableCS(cond, mutex, INFINITE))

#define ut_sleep(int ms)

#else
#include <unistd.h>
#include <pthread.h>

#define UT_ROUTINE_RETURN_TYPE void*

typedef pthread_t       ut_thread_t;
typedef pthread_mutex_t ut_mutex_t;
typedef pthread_cond_t  ut_cond_t;

#define ut_thread_create(thread, routine, arg) \
  (pthread_create(thread, 0, routine, arg) == 0)

#define ut_thread_kill(thread) \
  (pthread_kill(thread, 0) == 0)

#define ut_thread_wait(thread) \
  (pthread_join(thread, NULL) == 0)

#define ut_thread_exit(void) \
  (pthread_exit(NULL))

#define ut_mutex_create(mutex) \
  (pthread_mutex_init(mutex, 0) == 0)

#define ut_mutex_destroy(mutex) \
  (pthread_mutex_destroy(mutex) == 0)

#define ut_mutex_lock(mutex) \
  (pthread_mutex_lock(mutex) == 0)

#define ut_mutex_trylock(mutex) \
  (pthread_mutex_trylock(mutex) == 0)

#define ut_mutex_unlock(mutex) \
  (pthread_mutex_unlock(mutex) == 0)

#define ut_cond_create(cond) \
  (pthread_cond_init(cond, 0) == 0)

#define ut_cond_destroy(cond) \
  (pthread_cond_destroy(cond) == 0)

#define ut_cond_signal(cond) \
  (pthread_cond_signal(cond) == 0)

#define ut_cond_broadcast(cond) \
  (pthread_cond_broadcast(cond) == 0)

#define ut_cond_wait(cond, mutex) \
  (pthread_cond_wait(cond, mutex) == 0)

#define ut_sleep(ms) \
  (usleep(ms * 1000) == 0)
#endif

