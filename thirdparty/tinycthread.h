/* -*- mode: c; tab-width: 2; indent-tabs-mode: nil; -*-
Copyright (c) 2012 Marcus Geelnard
Copyright (c) 2013-2016 Evan Nemerson

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#ifndef _TINYCTHREAD_H_
#define _TINYCTHREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
* @file
* @mainpage TinyCThread API Reference
*
* @section intro_sec Introduction
* TinyCThread is a minimal, portable implementation of basic threading
* classes for C.
*
* They closely mimic the functionality and naming of the C11 standard, and
* should be easily replaceable with the corresponding standard variants.
*
* @section port_sec Portability
* The Win32 variant uses the native Win32 API for implementing the thread
* classes, while for other systems, the POSIX threads API (pthread) is used.
*
* @section misc_sec Miscellaneous
* The following special keywords are available: #_Thread_local.
*
* For more detailed information, browse the different sections of this
* documentation. A good place to start is:
* tinycthread.h.
*/

/* Which platform are we on? */
#if !defined(_TTHREAD_PLATFORM_DEFINED_)
  #if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
    #define _TTHREAD_WIN32_
  #else
    #define _TTHREAD_POSIX_
  #endif
  #define _TTHREAD_PLATFORM_DEFINED_
#endif

/* Activate some POSIX functionality (e.g. clock_gettime and recursive mutexes) */
#if defined(_TTHREAD_POSIX_)
  #undef _FEATURES_H
  #if !defined(_GNU_SOURCE)
    #define _GNU_SOURCE
  #endif
  #if !defined(_POSIX_C_SOURCE) || ((_POSIX_C_SOURCE - 0) < 199309L)
    #undef _POSIX_C_SOURCE
    #define _POSIX_C_SOURCE 199309L
  #endif
  #if !defined(_XOPEN_SOURCE) || ((_XOPEN_SOURCE - 0) < 500)
    #undef _XOPEN_SOURCE
    #define _XOPEN_SOURCE 500
  #endif
  #define _XPG6
#endif

/* Generic includes */
#include <time.h>

/* Platform specific includes */
#if defined(_TTHREAD_POSIX_)
  #include <pthread.h>
#elif defined(_TTHREAD_WIN32_)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #define __UNDEF_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #ifdef __UNDEF_LEAN_AND_MEAN
    #undef WIN32_LEAN_AND_MEAN
    #undef __UNDEF_LEAN_AND_MEAN
  #endif
#endif

/* Compiler-specific information */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  #define TTHREAD_NORETURN _Noreturn
#elif defined(__GNUC__)
  #define TTHREAD_NORETURN __attribute__((__noreturn__))
#else
  #define TTHREAD_NORETURN
#endif

/* If TIME_UTC is missing, provide it and provide a wrapper for
   timespec_get. */
#ifndef TIME_UTC
#define TIME_UTC 1
#define _TTHREAD_EMULATE_TIMESPEC_GET_

#if defined(_TTHREAD_WIN32_)
struct _tthread_timespec {
  time_t tv_sec;
  long   tv_nsec;
};
#define timespec _tthread_timespec
#endif

int _tthread_timespec_get(struct timespec *ts, int base);
#define timespec_get _tthread_timespec_get
#endif

/** TinyCThread version (major number). */
#define TINYCTHREAD_VERSION_MAJOR 1
/** TinyCThread version (minor number). */
#define TINYCTHREAD_VERSION_MINOR 2
/** TinyCThread version (full version). */
#define TINYCTHREAD_VERSION (TINYCTHREAD_VERSION_MAJOR * 100 + TINYCTHREAD_VERSION_MINOR)

/**
* @def _Thread_local
* Thread local storage keyword.
* A variable that is declared with the @c _Thread_local keyword makes the
* value of the variable local to each thread (known as thread-local storage,
* or TLS). Example usage:
* @code
* // This variable is local to each thread.
* _Thread_local int variable;
* @endcode
* @note The @c _Thread_local keyword is a macro that maps to the corresponding
* compiler directive (e.g. @c __declspec(thread)).
* @note This directive is currently not supported on Mac OS X (it will give
* a compiler error), since compile-time TLS is not supported in the Mac OS X
* executable format. Also, some older versions of MinGW (before GCC 4.x) do
* not support this directive, nor does the Tiny C Compiler.
* @hideinitializer
*/

#if !(defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201102L)) && !defined(_Thread_local)
 #if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__SUNPRO_CC) || defined(__IBMCPP__)
  #define _Thread_local __thread
 #else
  #define _Thread_local __declspec(thread)
 #endif
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) && (((__GNUC__ << 8) | __GNUC_MINOR__) < ((4 << 8) | 9))
 #define _Thread_local __thread
#endif

/* Macros */
#if defined(_TTHREAD_WIN32_)
#define TSS_DTOR_ITERATIONS (4)
#else
#define TSS_DTOR_ITERATIONS PTHREAD_DESTRUCTOR_ITERATIONS
#endif

/* Function return values */
#define thrd_error    0 /**< The requested operation failed */
#define thrd_success  1 /**< The requested operation succeeded */
#define thrd_timedout 2 /**< The time specified in the call was reached without acquiring the requested resource */
#define thrd_busy     3 /**< The requested operation failed because a tesource requested by a test and return function is already in use */
#define thrd_nomem    4 /**< The requested operation failed because it was unable to allocate memory */

/* Mutex types */
#define mtx_plain     0
#define mtx_timed     1
#define mtx_recursive 2

/* Mutex */
#if defined(_TTHREAD_WIN32_)
typedef struct {
  union {
    CRITICAL_SECTION cs;      /* Critical section handle (used for non-timed mutexes) */
    HANDLE mut;               /* Mutex handle (used for timed mutex) */
  } mHandle;                  /* Mutex handle */
  int mAlreadyLocked;         /* TRUE if the mutex is already locked */
  int mRecursive;             /* TRUE if the mutex is recursive */
  int mTimed;                 /* TRUE if the mutex is timed */
} mtx_t;
#else
typedef pthread_mutex_t mtx_t;
#endif

/** Create a mutex object.
* @param mtx A mutex object.
* @param type Bit-mask that must have one of the following six values:
*   @li @c mtx_plain for a simple non-recursive mutex
*   @li @c mtx_timed for a non-recursive mutex that supports timeout
*   @li @c mtx_plain | @c mtx_recursive (same as @c mtx_plain, but recursive)
*   @li @c mtx_timed | @c mtx_recursive (same as @c mtx_timed, but recursive)
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int mtx_init(mtx_t *mtx, int type);

/** Release any resources used by the given mutex.
* @param mtx A mutex object.
*/
void mtx_destroy(mtx_t *mtx);

/** Lock the given mutex.
* Blocks until the given mutex can be locked. If the mutex is non-recursive, and
* the calling thread already has a lock on the mutex, this call will block
* forever.
* @param mtx A mutex object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int mtx_lock(mtx_t *mtx);

/** Lock the given mutex, or block until a specific point in time.
* Blocks until either the given mutex can be locked, or the specified TIME_UTC
* based time.
* @param mtx A mutex object.
* @param ts A UTC based calendar time
* @return @ref The mtx_timedlock function returns thrd_success on success, or
* thrd_timedout if the time specified was reached without acquiring the
* requested resource, or thrd_error if the request could not be honored.
*/
int mtx_timedlock(mtx_t *mtx, const struct timespec *ts);

/** Try to lock the given mutex.
* The specified mutex shall support either test and return or timeout. If the
* mutex is already locked, the function returns without blocking.
* @param mtx A mutex object.
* @return @ref thrd_success on success, or @ref thrd_busy if the resource
* requested is already in use, or @ref thrd_error if the request could not be
* honored.
*/
int mtx_trylock(mtx_t *mtx);

/** Unlock the given mutex.
* @param mtx A mutex object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int mtx_unlock(mtx_t *mtx);

/* Condition variable */
#if defined(_TTHREAD_WIN32_)
typedef struct {
  HANDLE mEvents[2];                  /* Signal and broadcast event HANDLEs. */
  unsigned int mWaitersCount;         /* Count of the number of waiters. */
  CRITICAL_SECTION mWaitersCountLock; /* Serialize access to mWaitersCount. */
} cnd_t;
#else
typedef pthread_cond_t cnd_t;
#endif

/** Create a condition variable object.
* @param cond A condition variable object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int cnd_init(cnd_t *cond);

/** Release any resources used by the given condition variable.
* @param cond A condition variable object.
*/
void cnd_destroy(cnd_t *cond);

/** Signal a condition variable.
* Unblocks one of the threads that are blocked on the given condition variable
* at the time of the call. If no threads are blocked on the condition variable
* at the time of the call, the function does nothing and return success.
* @param cond A condition variable object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int cnd_signal(cnd_t *cond);

/** Broadcast a condition variable.
* Unblocks all of the threads that are blocked on the given condition variable
* at the time of the call. If no threads are blocked on the condition variable
* at the time of the call, the function does nothing and return success.
* @param cond A condition variable object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int cnd_broadcast(cnd_t *cond);

/** Wait for a condition variable to become signaled.
* The function atomically unlocks the given mutex and endeavors to block until
* the given condition variable is signaled by a call to cnd_signal or to
* cnd_broadcast. When the calling thread becomes unblocked it locks the mutex
* before it returns.
* @param cond A condition variable object.
* @param mtx A mutex object.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int cnd_wait(cnd_t *cond, mtx_t *mtx);

/** Wait for a condition variable to become signaled.
* The function atomically unlocks the given mutex and endeavors to block until
* the given condition variable is signaled by a call to cnd_signal or to
* cnd_broadcast, or until after the specified time. When the calling thread
* becomes unblocked it locks the mutex before it returns.
* @param cond A condition variable object.
* @param mtx A mutex object.
* @param xt A point in time at which the request will time out (absolute time).
* @return @ref thrd_success upon success, or @ref thrd_timeout if the time
* specified in the call was reached without acquiring the requested resource, or
* @ref thrd_error if the request could not be honored.
*/
int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts);

/* Thread */
#if defined(_TTHREAD_WIN32_)
typedef HANDLE thrd_t;
#else
typedef pthread_t thrd_t;
#endif

/** Thread start function.
* Any thread that is started with the @ref thrd_create() function must be
* started through a function of this type.
* @param arg The thread argument (the @c arg argument of the corresponding
*        @ref thrd_create() call).
* @return The thread return value, which can be obtained by another thread
* by using the @ref thrd_join() function.
*/
typedef int (*thrd_start_t)(void *arg);

/** Create a new thread.
* @param thr Identifier of the newly created thread.
* @param func A function pointer to the function that will be executed in
*        the new thread.
* @param arg An argument to the thread function.
* @return @ref thrd_success on success, or @ref thrd_nomem if no memory could
* be allocated for the thread requested, or @ref thrd_error if the request
* could not be honored.
* @note A thread’s identifier may be reused for a different thread once the
* original thread has exited and either been detached or joined to another
* thread.
*/
int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);

/** Identify the calling thread.
* @return The identifier of the calling thread.
*/
thrd_t thrd_current(void);

/** Dispose of any resources allocated to the thread when that thread exits.
 * @return thrd_success, or thrd_error on error
*/
int thrd_detach(thrd_t thr);

/** Compare two thread identifiers.
* The function determines if two thread identifiers refer to the same thread.
* @return Zero if the two thread identifiers refer to different threads.
* Otherwise a nonzero value is returned.
*/
int thrd_equal(thrd_t thr0, thrd_t thr1);

/** Terminate execution of the calling thread.
* @param res Result code of the calling thread.
*/
TTHREAD_NORETURN void thrd_exit(int res);

/** Wait for a thread to terminate.
* The function joins the given thread with the current thread by blocking
* until the other thread has terminated.
* @param thr The thread to join with.
* @param res If this pointer is not NULL, the function will store the result
*        code of the given thread in the integer pointed to by @c res.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int thrd_join(thrd_t thr, int *res);

/** Put the calling thread to sleep.
* Suspend execution of the calling thread.
* @param duration  Interval to sleep for
* @param remaining If non-NULL, this parameter will hold the remaining
*                  time until time_point upon return. This will
*                  typically be zero, but if the thread was woken up
*                  by a signal that is not ignored before duration was
*                  reached @c remaining will hold a positive time.
* @return 0 (zero) on successful sleep, -1 if an interrupt occurred,
*         or a negative value if the operation fails.
*/
int thrd_sleep(const struct timespec *duration, struct timespec *remaining);

/** Yield execution to another thread.
* Permit other threads to run, even if the current thread would ordinarily
* continue to run.
*/
void thrd_yield(void);

/* Thread local storage */
#if defined(_TTHREAD_WIN32_)
typedef DWORD tss_t;
#else
typedef pthread_key_t tss_t;
#endif

/** Destructor function for a thread-specific storage.
* @param val The value of the destructed thread-specific storage.
*/
typedef void (*tss_dtor_t)(void *val);

/** Create a thread-specific storage.
* @param key The unique key identifier that will be set if the function is
*        successful.
* @param dtor Destructor function. This can be NULL.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
* @note On Windows, the @c dtor will definitely be called when
* appropriate for threads created with @ref thrd_create.  It will be
* called for other threads in most cases, the possible exception being
* for DLLs loaded with LoadLibraryEx.  In order to be certain, you
* should use @ref thrd_create whenever possible.
*/
int tss_create(tss_t *key, tss_dtor_t dtor);

/** Delete a thread-specific storage.
* The function releases any resources used by the given thread-specific
* storage.
* @param key The key that shall be deleted.
*/
void tss_delete(tss_t key);

/** Get the value for a thread-specific storage.
* @param key The thread-specific storage identifier.
* @return The value for the current thread held in the given thread-specific
* storage.
*/
void *tss_get(tss_t key);

/** Set the value for a thread-specific storage.
* @param key The thread-specific storage identifier.
* @param val The value of the thread-specific storage to set for the current
*        thread.
* @return @ref thrd_success on success, or @ref thrd_error if the request could
* not be honored.
*/
int tss_set(tss_t key, void *val);

#if defined(_TTHREAD_WIN32_)
  typedef struct {
    LONG volatile status;
    CRITICAL_SECTION lock;
  } once_flag;
  #define ONCE_FLAG_INIT {0,}
#else
  #define once_flag pthread_once_t
  #define ONCE_FLAG_INIT PTHREAD_ONCE_INIT
#endif

/** Invoke a callback exactly once
 * @param flag Flag used to ensure the callback is invoked exactly
 *        once.
 * @param func Callback to invoke.
 */
#if defined(_TTHREAD_WIN32_)
  void call_once(once_flag *flag, void (*func)(void));
#else
  #define call_once(flag,func) pthread_once(flag,func)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _TINYTHREAD_H_ */

#ifdef TINYCTHREAD_IMPL

/* -*- mode: c; tab-width: 2; indent-tabs-mode: nil; -*-
Copyright (c) 2012 Marcus Geelnard
Copyright (c) 2013-2016 Evan Nemerson

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#include <stdlib.h>

/* Platform specific includes */
#if defined(_TTHREAD_POSIX_)
  #include <signal.h>
  #include <sched.h>
  #include <unistd.h>
  #include <sys/time.h>
  #include <errno.h>
#elif defined(_TTHREAD_WIN32_)
  #include <process.h>
  #include <sys/timeb.h>
#endif

/* Standard, good-to-have defines */
#ifndef NULL
  #define NULL (void*)0
#endif
#ifndef TRUE
  #define TRUE 1
#endif
#ifndef FALSE
  #define FALSE 0
#endif

#ifdef __cplusplus
extern "C" {
#endif


int mtx_init(mtx_t *mtx, int type)
{
#if defined(_TTHREAD_WIN32_)
  mtx->mAlreadyLocked = FALSE;
  mtx->mRecursive = type & mtx_recursive;
  mtx->mTimed = type & mtx_timed;
  if (!mtx->mTimed)
  {
    InitializeCriticalSection(&(mtx->mHandle.cs));
  }
  else
  {
    mtx->mHandle.mut = CreateMutex(NULL, FALSE, NULL);
    if (mtx->mHandle.mut == NULL)
    {
      return thrd_error;
    }
  }
  return thrd_success;
#else
  int ret;
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  if (type & mtx_recursive)
  {
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  }
  ret = pthread_mutex_init(mtx, &attr);
  pthread_mutexattr_destroy(&attr);
  return ret == 0 ? thrd_success : thrd_error;
#endif
}

void mtx_destroy(mtx_t *mtx)
{
#if defined(_TTHREAD_WIN32_)
  if (!mtx->mTimed)
  {
    DeleteCriticalSection(&(mtx->mHandle.cs));
  }
  else
  {
    CloseHandle(mtx->mHandle.mut);
  }
#else
  pthread_mutex_destroy(mtx);
#endif
}

int mtx_lock(mtx_t *mtx)
{
#if defined(_TTHREAD_WIN32_)
  if (!mtx->mTimed)
  {
    EnterCriticalSection(&(mtx->mHandle.cs));
  }
  else
  {
    switch (WaitForSingleObject(mtx->mHandle.mut, INFINITE))
    {
      case WAIT_OBJECT_0:
        break;
      case WAIT_ABANDONED:
      default:
        return thrd_error;
    }
  }

  if (!mtx->mRecursive)
  {
    while(mtx->mAlreadyLocked) Sleep(1); /* Simulate deadlock... */
    mtx->mAlreadyLocked = TRUE;
  }
  return thrd_success;
#else
  return pthread_mutex_lock(mtx) == 0 ? thrd_success : thrd_error;
#endif
}

int mtx_timedlock(mtx_t *mtx, const struct timespec *ts)
{
#if defined(_TTHREAD_WIN32_)
  struct timespec current_ts;
  DWORD timeoutMs;

  if (!mtx->mTimed)
  {
    return thrd_error;
  }

  timespec_get(&current_ts, TIME_UTC);

  if ((current_ts.tv_sec > ts->tv_sec) || ((current_ts.tv_sec == ts->tv_sec) && (current_ts.tv_nsec >= ts->tv_nsec)))
  {
    timeoutMs = 0;
  }
  else
  {
    timeoutMs  = (DWORD)(ts->tv_sec  - current_ts.tv_sec)  * 1000;
    timeoutMs += (ts->tv_nsec - current_ts.tv_nsec) / 1000000;
    timeoutMs += 1;
  }

  /* TODO: the timeout for WaitForSingleObject doesn't include time
     while the computer is asleep. */
  switch (WaitForSingleObject(mtx->mHandle.mut, timeoutMs))
  {
    case WAIT_OBJECT_0:
      break;
    case WAIT_TIMEOUT:
      return thrd_timedout;
    case WAIT_ABANDONED:
    default:
      return thrd_error;
  }

  if (!mtx->mRecursive)
  {
    while(mtx->mAlreadyLocked) Sleep(1); /* Simulate deadlock... */
    mtx->mAlreadyLocked = TRUE;
  }

  return thrd_success;
#elif defined(_POSIX_TIMEOUTS) && (_POSIX_TIMEOUTS >= 200112L) && defined(_POSIX_THREADS) && (_POSIX_THREADS >= 200112L)
  switch (pthread_mutex_timedlock(mtx, ts)) {
    case 0:
      return thrd_success;
    case ETIMEDOUT:
      return thrd_timedout;
    default:
      return thrd_error;
  }
#else
  int rc;
  struct timespec cur, dur;

  /* Try to acquire the lock and, if we fail, sleep for 5ms. */
  while ((rc = pthread_mutex_trylock (mtx)) == EBUSY) {
    timespec_get(&cur, TIME_UTC);

    if ((cur.tv_sec > ts->tv_sec) || ((cur.tv_sec == ts->tv_sec) && (cur.tv_nsec >= ts->tv_nsec)))
    {
      break;
    }

    dur.tv_sec = ts->tv_sec - cur.tv_sec;
    dur.tv_nsec = ts->tv_nsec - cur.tv_nsec;
    if (dur.tv_nsec < 0)
    {
      dur.tv_sec--;
      dur.tv_nsec += 1000000000;
    }

    if ((dur.tv_sec != 0) || (dur.tv_nsec > 5000000))
    {
      dur.tv_sec = 0;
      dur.tv_nsec = 5000000;
    }

    nanosleep(&dur, NULL);
  }

  switch (rc) {
    case 0:
      return thrd_success;
    case ETIMEDOUT:
    case EBUSY:
      return thrd_timedout;
    default:
      return thrd_error;
  }
#endif
}

int mtx_trylock(mtx_t *mtx)
{
#if defined(_TTHREAD_WIN32_)
  int ret;

  if (!mtx->mTimed)
  {
    ret = TryEnterCriticalSection(&(mtx->mHandle.cs)) ? thrd_success : thrd_busy;
  }
  else
  {
    ret = (WaitForSingleObject(mtx->mHandle.mut, 0) == WAIT_OBJECT_0) ? thrd_success : thrd_busy;
  }

  if ((!mtx->mRecursive) && (ret == thrd_success))
  {
    if (mtx->mAlreadyLocked)
    {
      LeaveCriticalSection(&(mtx->mHandle.cs));
      ret = thrd_busy;
    }
    else
    {
      mtx->mAlreadyLocked = TRUE;
    }
  }
  return ret;
#else
  return (pthread_mutex_trylock(mtx) == 0) ? thrd_success : thrd_busy;
#endif
}

int mtx_unlock(mtx_t *mtx)
{
#if defined(_TTHREAD_WIN32_)
  mtx->mAlreadyLocked = FALSE;
  if (!mtx->mTimed)
  {
    LeaveCriticalSection(&(mtx->mHandle.cs));
  }
  else
  {
    if (!ReleaseMutex(mtx->mHandle.mut))
    {
      return thrd_error;
    }
  }
  return thrd_success;
#else
  return pthread_mutex_unlock(mtx) == 0 ? thrd_success : thrd_error;;
#endif
}

#if defined(_TTHREAD_WIN32_)
#define _CONDITION_EVENT_ONE 0
#define _CONDITION_EVENT_ALL 1
#endif

int cnd_init(cnd_t *cond)
{
#if defined(_TTHREAD_WIN32_)
  cond->mWaitersCount = 0;

  /* Init critical section */
  InitializeCriticalSection(&cond->mWaitersCountLock);

  /* Init events */
  cond->mEvents[_CONDITION_EVENT_ONE] = CreateEvent(NULL, FALSE, FALSE, NULL);
  if (cond->mEvents[_CONDITION_EVENT_ONE] == NULL)
  {
    cond->mEvents[_CONDITION_EVENT_ALL] = NULL;
    return thrd_error;
  }
  cond->mEvents[_CONDITION_EVENT_ALL] = CreateEvent(NULL, TRUE, FALSE, NULL);
  if (cond->mEvents[_CONDITION_EVENT_ALL] == NULL)
  {
    CloseHandle(cond->mEvents[_CONDITION_EVENT_ONE]);
    cond->mEvents[_CONDITION_EVENT_ONE] = NULL;
    return thrd_error;
  }

  return thrd_success;
#else
  return pthread_cond_init(cond, NULL) == 0 ? thrd_success : thrd_error;
#endif
}

void cnd_destroy(cnd_t *cond)
{
#if defined(_TTHREAD_WIN32_)
  if (cond->mEvents[_CONDITION_EVENT_ONE] != NULL)
  {
    CloseHandle(cond->mEvents[_CONDITION_EVENT_ONE]);
  }
  if (cond->mEvents[_CONDITION_EVENT_ALL] != NULL)
  {
    CloseHandle(cond->mEvents[_CONDITION_EVENT_ALL]);
  }
  DeleteCriticalSection(&cond->mWaitersCountLock);
#else
  pthread_cond_destroy(cond);
#endif
}

int cnd_signal(cnd_t *cond)
{
#if defined(_TTHREAD_WIN32_)
  int haveWaiters;

  /* Are there any waiters? */
  EnterCriticalSection(&cond->mWaitersCountLock);
  haveWaiters = (cond->mWaitersCount > 0);
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* If we have any waiting threads, send them a signal */
  if(haveWaiters)
  {
    if (SetEvent(cond->mEvents[_CONDITION_EVENT_ONE]) == 0)
    {
      return thrd_error;
    }
  }

  return thrd_success;
#else
  return pthread_cond_signal(cond) == 0 ? thrd_success : thrd_error;
#endif
}

int cnd_broadcast(cnd_t *cond)
{
#if defined(_TTHREAD_WIN32_)
  int haveWaiters;

  /* Are there any waiters? */
  EnterCriticalSection(&cond->mWaitersCountLock);
  haveWaiters = (cond->mWaitersCount > 0);
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* If we have any waiting threads, send them a signal */
  if(haveWaiters)
  {
    if (SetEvent(cond->mEvents[_CONDITION_EVENT_ALL]) == 0)
    {
      return thrd_error;
    }
  }

  return thrd_success;
#else
  return pthread_cond_broadcast(cond) == 0 ? thrd_success : thrd_error;
#endif
}

#if defined(_TTHREAD_WIN32_)
static int _cnd_timedwait_win32(cnd_t *cond, mtx_t *mtx, DWORD timeout)
{
  DWORD result;
  int lastWaiter;

  /* Increment number of waiters */
  EnterCriticalSection(&cond->mWaitersCountLock);
  ++ cond->mWaitersCount;
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* Release the mutex while waiting for the condition (will decrease
     the number of waiters when done)... */
  mtx_unlock(mtx);

  /* Wait for either event to become signaled due to cnd_signal() or
     cnd_broadcast() being called */
  result = WaitForMultipleObjects(2, cond->mEvents, FALSE, timeout);
  if (result == WAIT_TIMEOUT)
  {
    /* The mutex is locked again before the function returns, even if an error occurred */
    mtx_lock(mtx);
    return thrd_timedout;
  }
  else if (result == WAIT_FAILED)
  {
    /* The mutex is locked again before the function returns, even if an error occurred */
    mtx_lock(mtx);
    return thrd_error;
  }

  /* Check if we are the last waiter */
  EnterCriticalSection(&cond->mWaitersCountLock);
  -- cond->mWaitersCount;
  lastWaiter = (result == (WAIT_OBJECT_0 + _CONDITION_EVENT_ALL)) &&
               (cond->mWaitersCount == 0);
  LeaveCriticalSection(&cond->mWaitersCountLock);

  /* If we are the last waiter to be notified to stop waiting, reset the event */
  if (lastWaiter)
  {
    if (ResetEvent(cond->mEvents[_CONDITION_EVENT_ALL]) == 0)
    {
      /* The mutex is locked again before the function returns, even if an error occurred */
      mtx_lock(mtx);
      return thrd_error;
    }
  }

  /* Re-acquire the mutex */
  mtx_lock(mtx);

  return thrd_success;
}
#endif

int cnd_wait(cnd_t *cond, mtx_t *mtx)
{
#if defined(_TTHREAD_WIN32_)
  return _cnd_timedwait_win32(cond, mtx, INFINITE);
#else
  return pthread_cond_wait(cond, mtx) == 0 ? thrd_success : thrd_error;
#endif
}

int cnd_timedwait(cnd_t *cond, mtx_t *mtx, const struct timespec *ts)
{
#if defined(_TTHREAD_WIN32_)
  struct timespec now;
  if (timespec_get(&now, TIME_UTC) == TIME_UTC)
  {
    unsigned long long nowInMilliseconds = now.tv_sec * 1000 + now.tv_nsec / 1000000;
    unsigned long long tsInMilliseconds  = ts->tv_sec * 1000 + ts->tv_nsec / 1000000;
    DWORD delta = (tsInMilliseconds > nowInMilliseconds) ?
      (DWORD)(tsInMilliseconds - nowInMilliseconds) : 0;
    return _cnd_timedwait_win32(cond, mtx, delta);
  }
  else
    return thrd_error;
#else
  int ret;
  ret = pthread_cond_timedwait(cond, mtx, ts);
  if (ret == ETIMEDOUT)
  {
    return thrd_timedout;
  }
  return ret == 0 ? thrd_success : thrd_error;
#endif
}

#if defined(_TTHREAD_WIN32_)
struct TinyCThreadTSSData {
  void* value;
  tss_t key;
  struct TinyCThreadTSSData* next;
};

static tss_dtor_t _tinycthread_tss_dtors[1088] = { NULL, };

static _Thread_local struct TinyCThreadTSSData* _tinycthread_tss_head = NULL;
static _Thread_local struct TinyCThreadTSSData* _tinycthread_tss_tail = NULL;

static void _tinycthread_tss_cleanup (void);

static void _tinycthread_tss_cleanup (void) {
  struct TinyCThreadTSSData* data;
  int iteration;
  unsigned int again = 1;
  void* value;

  for (iteration = 0 ; iteration < TSS_DTOR_ITERATIONS && again > 0 ; iteration++)
  {
    again = 0;
    for (data = _tinycthread_tss_head ; data != NULL ; data = data->next)
    {
      if (data->value != NULL)
      {
        value = data->value;
        data->value = NULL;

        if (_tinycthread_tss_dtors[data->key] != NULL)
        {
          again = 1;
          _tinycthread_tss_dtors[data->key](value);
        }
      }
    }
  }

  while (_tinycthread_tss_head != NULL) {
    data = _tinycthread_tss_head->next;
    free (_tinycthread_tss_head);
    _tinycthread_tss_head = data;
  }
  _tinycthread_tss_head = NULL;
  _tinycthread_tss_tail = NULL;
}

static void NTAPI _tinycthread_tss_callback(PVOID h, DWORD dwReason, PVOID pv)
{
  (void)h;
  (void)pv;

  if (_tinycthread_tss_head != NULL && (dwReason == DLL_THREAD_DETACH || dwReason == DLL_PROCESS_DETACH))
  {
    _tinycthread_tss_cleanup();
  }
}

#if defined(_MSC_VER)
  #ifdef _M_X64
    #pragma const_seg(".CRT$XLB")
  #else
    #pragma data_seg(".CRT$XLB")
  #endif
  PIMAGE_TLS_CALLBACK p_thread_callback = _tinycthread_tss_callback;
  #ifdef _M_X64
    #pragma data_seg()
  #else
    #pragma const_seg()
  #endif
#else
  PIMAGE_TLS_CALLBACK p_thread_callback __attribute__((section(".CRT$XLB"))) = _tinycthread_tss_callback;
#endif

#endif /* defined(_TTHREAD_WIN32_) */

/** Information to pass to the new thread (what to run). */
typedef struct {
  thrd_start_t mFunction; /**< Pointer to the function to be executed. */
  void * mArg;            /**< Function argument for the thread function. */
} _thread_start_info;

/* Thread wrapper function. */
#if defined(_TTHREAD_WIN32_)
static DWORD WINAPI _thrd_wrapper_function(LPVOID aArg)
#elif defined(_TTHREAD_POSIX_)
static void * _thrd_wrapper_function(void * aArg)
#endif
{
  thrd_start_t fun;
  void *arg;
  int  res;

  /* Get thread startup information */
  _thread_start_info *ti = (_thread_start_info *) aArg;
  fun = ti->mFunction;
  arg = ti->mArg;

  /* The thread is responsible for freeing the startup information */
  free((void *)ti);

  /* Call the actual client thread function */
  res = fun(arg);

#if defined(_TTHREAD_WIN32_)
  if (_tinycthread_tss_head != NULL)
  {
    _tinycthread_tss_cleanup();
  }

  return (DWORD)res;
#else
  return (void*)(intptr_t)res;
#endif
}

int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
  /* Fill out the thread startup information (passed to the thread wrapper,
     which will eventually free it) */
  _thread_start_info* ti = (_thread_start_info*)malloc(sizeof(_thread_start_info));
  if (ti == NULL)
  {
    return thrd_nomem;
  }
  ti->mFunction = func;
  ti->mArg = arg;

  /* Create the thread */
#if defined(_TTHREAD_WIN32_)
  *thr = CreateThread(NULL, 0, _thrd_wrapper_function, (LPVOID) ti, 0, NULL);
#elif defined(_TTHREAD_POSIX_)
  if(pthread_create(thr, NULL, _thrd_wrapper_function, (void *)ti) != 0)
  {
    *thr = 0;
  }
#endif

  /* Did we fail to create the thread? */
  if(!*thr)
  {
    free(ti);
    return thrd_error;
  }

  return thrd_success;
}

thrd_t thrd_current(void)
{
#if defined(_TTHREAD_WIN32_)
  return GetCurrentThread();
#else
  return pthread_self();
#endif
}

int thrd_detach(thrd_t thr)
{
#if defined(_TTHREAD_WIN32_)
  /* https://stackoverflow.com/questions/12744324/how-to-detach-a-thread-on-windows-c#answer-12746081 */
  return CloseHandle(thr) != 0 ? thrd_success : thrd_error;
#else
  return pthread_detach(thr) == 0 ? thrd_success : thrd_error;
#endif
}

int thrd_equal(thrd_t thr0, thrd_t thr1)
{
#if defined(_TTHREAD_WIN32_)
  return GetThreadId(thr0) == GetThreadId(thr1);
#else
  return pthread_equal(thr0, thr1);
#endif
}

void thrd_exit(int res)
{
#if defined(_TTHREAD_WIN32_)
  if (_tinycthread_tss_head != NULL)
  {
    _tinycthread_tss_cleanup();
  }

  ExitThread((DWORD)res);
#else
  pthread_exit((void*)(intptr_t)res);
#endif
}

int thrd_join(thrd_t thr, int *res)
{
#if defined(_TTHREAD_WIN32_)
  DWORD dwRes;

  if (WaitForSingleObject(thr, INFINITE) == WAIT_FAILED)
  {
    return thrd_error;
  }
  if (res != NULL)
  {
    if (GetExitCodeThread(thr, &dwRes) != 0)
    {
      *res = (int) dwRes;
    }
    else
    {
      return thrd_error;
    }
  }
  CloseHandle(thr);
#elif defined(_TTHREAD_POSIX_)
  void *pres;
  if (pthread_join(thr, &pres) != 0)
  {
    return thrd_error;
  }
  if (res != NULL)
  {
    *res = (int)(intptr_t)pres;
  }
#endif
  return thrd_success;
}

int thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{
#if !defined(_TTHREAD_WIN32_)
  int res = nanosleep(duration, remaining);
  if (res == 0) {
    return 0;
  } else if (errno == EINTR) {
    return -1;
  } else {
    return -2;
  }
#else
  struct timespec start;
  DWORD t;

  timespec_get(&start, TIME_UTC);

  t = SleepEx((DWORD)(duration->tv_sec * 1000 +
              duration->tv_nsec / 1000000 +
              (((duration->tv_nsec % 1000000) == 0) ? 0 : 1)),
              TRUE);

  if (t == 0) {
    return 0;
  } else {
    if (remaining != NULL) {
      timespec_get(remaining, TIME_UTC);
      remaining->tv_sec -= start.tv_sec;
      remaining->tv_nsec -= start.tv_nsec;
      if (remaining->tv_nsec < 0)
      {
        remaining->tv_nsec += 1000000000;
        remaining->tv_sec -= 1;
      }
    }

    return (t == WAIT_IO_COMPLETION) ? -1 : -2;
  }
#endif
}

void thrd_yield(void)
{
#if defined(_TTHREAD_WIN32_)
  Sleep(0);
#else
  sched_yield();
#endif
}

int tss_create(tss_t *key, tss_dtor_t dtor)
{
#if defined(_TTHREAD_WIN32_)
  *key = TlsAlloc();
  if (*key == TLS_OUT_OF_INDEXES)
  {
    return thrd_error;
  }
  _tinycthread_tss_dtors[*key] = dtor;
#else
  if (pthread_key_create(key, dtor) != 0)
  {
    return thrd_error;
  }
#endif
  return thrd_success;
}

void tss_delete(tss_t key)
{
#if defined(_TTHREAD_WIN32_)
  struct TinyCThreadTSSData* data = (struct TinyCThreadTSSData*) TlsGetValue (key);
  struct TinyCThreadTSSData* prev = NULL;
  if (data != NULL)
  {
    if (data == _tinycthread_tss_head)
    {
      _tinycthread_tss_head = data->next;
    }
    else
    {
      prev = _tinycthread_tss_head;
      if (prev != NULL)
      {
        while (prev->next != data)
        {
          prev = prev->next;
        }
      }
    }

    if (data == _tinycthread_tss_tail)
    {
      _tinycthread_tss_tail = prev;
    }

    free (data);
  }
  _tinycthread_tss_dtors[key] = NULL;
  TlsFree(key);
#else
  pthread_key_delete(key);
#endif
}

void *tss_get(tss_t key)
{
#if defined(_TTHREAD_WIN32_)
  struct TinyCThreadTSSData* data = (struct TinyCThreadTSSData*)TlsGetValue(key);
  if (data == NULL)
  {
    return NULL;
  }
  return data->value;
#else
  return pthread_getspecific(key);
#endif
}

int tss_set(tss_t key, void *val)
{
#if defined(_TTHREAD_WIN32_)
  struct TinyCThreadTSSData* data = (struct TinyCThreadTSSData*)TlsGetValue(key);
  if (data == NULL)
  {
    data = (struct TinyCThreadTSSData*)malloc(sizeof(struct TinyCThreadTSSData));
    if (data == NULL)
    {
      return thrd_error;
    }

    data->value = NULL;
    data->key = key;
    data->next = NULL;

    if (_tinycthread_tss_tail != NULL)
    {
      _tinycthread_tss_tail->next = data;
    }
    else
    {
      _tinycthread_tss_tail = data;
    }

    if (_tinycthread_tss_head == NULL)
    {
      _tinycthread_tss_head = data;
    }

    if (!TlsSetValue(key, data))
    {
      free (data);
      return thrd_error;
    }
  }
  data->value = val;
#else
  if (pthread_setspecific(key, val) != 0)
  {
    return thrd_error;
  }
#endif
  return thrd_success;
}

#if defined(_TTHREAD_EMULATE_TIMESPEC_GET_)
int _tthread_timespec_get(struct timespec *ts, int base)
{
#if defined(_TTHREAD_WIN32_)
  struct _timeb tb;
#elif !defined(CLOCK_REALTIME)
  struct timeval tv;
#endif

  if (base != TIME_UTC)
  {
    return 0;
  }

#if defined(_TTHREAD_WIN32_)
  _ftime_s(&tb);
  ts->tv_sec = (time_t)tb.time;
  ts->tv_nsec = 1000000L * (long)tb.millitm;
#elif defined(CLOCK_REALTIME)
  base = (clock_gettime(CLOCK_REALTIME, ts) == 0) ? base : 0;
#else
  gettimeofday(&tv, NULL);
  ts->tv_sec = (time_t)tv.tv_sec;
  ts->tv_nsec = 1000L * (long)tv.tv_usec;
#endif

  return base;
}
#endif /* _TTHREAD_EMULATE_TIMESPEC_GET_ */

#if defined(_TTHREAD_WIN32_)
void call_once(once_flag *flag, void (*func)(void))
{
  /* The idea here is that we use a spin lock (via the
     InterlockedCompareExchange function) to restrict access to the
     critical section until we have initialized it, then we use the
     critical section to block until the callback has completed
     execution. */
  while (flag->status < 3)
  {
    switch (flag->status)
    {
      case 0:
        if (InterlockedCompareExchange (&(flag->status), 1, 0) == 0) {
          InitializeCriticalSection(&(flag->lock));
          EnterCriticalSection(&(flag->lock));
          flag->status = 2;
          func();
          flag->status = 3;
          LeaveCriticalSection(&(flag->lock));
          return;
        }
        break;
      case 1:
        break;
      case 2:
        EnterCriticalSection(&(flag->lock));
        LeaveCriticalSection(&(flag->lock));
        break;
    }
  }
}
#endif /* defined(_TTHREAD_WIN32_) */

#ifdef __cplusplus
}

#endif
#endif /* TINYCTHREAD_IMPL */
