#pragma once

#include <stdarg.h>
#include <stdint.h>

#include "arch/arch.h"

typedef struct File File;
typedef struct Thread Thread;
typedef struct Mutex Mutex;
typedef struct Cond Cond;

void* memoryAlloc(uint64_t size);
void  memoryFree(void *block);
void  memoryCopy(void *dst, const void *src, uint64_t size);
void  memoryMove(void *dst, const void *src, uint64_t size);

void formatString(const char *format, ArchLogLevel level,
		  char *out, uint64_t size);

void print(const char *message);

File* fileCreate(const char *fileNameFormat, const char *pathFormat);
void  fileClose(File *file);
void  fileWrite(File *file, const char *message);

Thread* threadCreate(void*(*func)(void *arg), void *arg);
void    threadDestroy(Thread *thread);
void    threadWait(Thread *thread);

Mutex* mutexCreate();
void   mutexDestroy(Mutex *mutex);
void   mutexLock(Mutex *mutex);
void   mutexUnlock(Mutex *mutex);

Cond* condCreate();
void  condWait(Cond *cond, Mutex *mutex);
void  condSignal(Cond *cond);
void  condDestroy(Cond *cond);
