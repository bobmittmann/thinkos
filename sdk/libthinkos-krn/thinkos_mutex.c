/* 
 * thinkos_mutex.c
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#include "thinkos_krn-i.h"
#include <sys/dcclog.h>

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#if (THINKOS_MUTEX_MAX) > 0

int krn_mutex_check(struct thinkos_rt * krn, int mtx)
{
#if (THINKOS_ENABLE_ARG_CHECK)
	if (!__krn_obj_is_mutex(krn, mtx)) {
		return THINKOS_ERR_MUTEX_INVALID;
	}
#if (THINKOS_ENABLE_MUTEX_ALLOC)
	if (__krn_mutex_is_alloc(krn, mtx) == 0) {
		return THINKOS_ERR_MUTEX_ALLOC;
	}
#endif
#endif
	return THINKOS_OK;
}

int krn_mutex_unlock_wakeup(struct thinkos_rt * krn, int mtx) 
{
	int th;

	if ((th = __krn_wq_head(krn, mtx)) == THINKOS_THREAD_NULL) {
		/* no threads waiting on the lock, just release
		   the lock */
		__krn_mutex_unlock(krn, mtx);
	} else {
		/* set the mutex ownership to the new thread */
		__krn_mutex_lock(krn, mtx, th);
		/* wakeup from the mutex wait queue */
		__krn_wq_wakeup(krn, mtx, th);
	}

	return th;
}

void thinkos_mutex_trylock_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int mutex = arg[0];
	int ret;

	if ((ret = krn_mutex_check(krn, mutex)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid mutex %d!", self, mutex);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}

	if (!__krn_mutex_is_locked(krn, mutex)) {
		/* Lock the mutex */
		__krn_mutex_lock(krn, mutex, self);
		DCC_LOG2(LOG_MSG, "<%2d> mutex %d locked", self, mutex);
		arg[0] = THINKOS_OK;
		return;
	} 

#if (THINKOS_ENABLE_DEADLOCK_CHECK)
	if (__krn_mutex_lock_get(krn, mutex) == self) {
		DCC_LOG2(LOG_WARNING, "<%2d> mutex %d, deadlock!", self, mutex);
		__THINKOS_ERROR(self, THINKOS_ERR_MUTEX_LOCKED);
		arg[0] = THINKOS_EDEADLK;
		return;
	} 
#endif

	DCC_LOG2(LOG_MSG, "<%2d> mutex %d busy...", self, mutex);
	arg[0] = THINKOS_EAGAIN;
}


void thinkos_mutex_lock_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int mutex = arg[0];
	int ret;

	if ((ret = krn_mutex_check(krn, mutex)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid mutex %d!", self, mutex);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}

	if (!__krn_mutex_is_locked(krn, mutex)) {
		/* Lock the mutex */
		__krn_mutex_lock(krn, mutex, self);
		DCC_LOG2(LOG_MSG, "<%2d> mutex %d locked", self, mutex);
		arg[0] = THINKOS_OK;
		return;
	} 

#if (THINKOS_ENABLE_DEADLOCK_CHECK)
		/* Sanity check: the current thread already owns the lock */
		if (__krn_mutex_lock_get(krn, mutex) == self) {
			DCC_LOG2(LOG_WARNING, "<%2d> mutex %d, deadlock!", self, mutex);
			__THINKOS_ERROR(self, THINKOS_ERR_MUTEX_LOCKED);
			arg[0] = THINKOS_EDEADLK;
			return;
		}
#endif

	DCC_LOG2(LOG_MSG , "<%2d> waiting on mutex %d...", self, mutex);

	/* Set the return value */
	arg[0] = THINKOS_OK;
	__krn_thread_wait(krn, self, mutex);
}

#if (THINKOS_ENABLE_TIMED_CALLS)
void thinkos_mutex_timedlock_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int mutex = arg[0];
	int32_t ms = arg[1];
	int ret;

	if ((ret = krn_mutex_check(krn, mutex)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid mutex %d!", self, mutex);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}

	if (!__krn_mutex_is_locked(krn, mutex)) {
		/* Lock the mutex */
		__krn_mutex_lock(krn, mutex, self);
		DCC_LOG2(LOG_MSG, "<%2d> mutex %d locked", self, mutex);
		arg[0] = THINKOS_OK;
		return;
	}

#if (THINKOS_ENABLE_DEADLOCK_CHECK)
	if (__krn_mutex_lock_get(krn, mutex) == self) {
		DCC_LOG2(LOG_WARNING, "<%2d> mutex %d, deadlock!", self, mutex);
		__THINKOS_ERROR(self, THINKOS_ERR_MUTEX_LOCKED);
		arg[0] = THINKOS_EDEADLK;
		return;
	}
#endif
		
	DCC_LOG2(LOG_MSG, "<%2d> waiting on mutex %d...", self, mutex);
	/* Set the default return value to timeout. The
	   mutex_unlock() call will change this to 0 */
	arg[0] = THINKOS_ETIMEDOUT;
	__krn_thread_timedwait(krn, self, mutex, ms);
}
#endif

void thinkos_mutex_unlock_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int mutex = arg[0];
	unsigned int th;
	int ret;

	if ((ret = krn_mutex_check(krn, mutex)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid mutex %d!", self, mutex);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}

#if (THINKOS_ENABLE_SANITY_CHECK)
	/* sanity check: avoid unlock the mutex by a thread that 
	   does not own the lock */
	if (__krn_mutex_lock_get(krn, mutex) != self) {
		DCC_LOG3(LOG_ERROR, "<%2d> mutex %d is locked by <%2d>!", 
				 self, mutex, __krn_mutex_lock_get(krn, mutex));
		__THINKOS_ERROR(self, THINKOS_ERR_MUTEX_NOTMINE);
		arg[0] = THINKOS_EPERM;
		return;
	}
#endif

	/* Set the return value */
	arg[0] = THINKOS_OK;

	DCC_LOG2(LOG_MSG, "<%2d> mutex %d unlocked.", self, mutex);

	if ((th = krn_mutex_unlock_wakeup(krn, mutex)) != THINKOS_THREAD_NULL) {
		DCC_LOG2(LOG_MSG, "<%2d> mutex %d locked.", th, mutex);
		/* signal the scheduler ... */
		__krn_defer_sched(krn);
	}
}

bool krn_mutex_resume(unsigned int thread_id, unsigned int mutex, bool tmw) 
{
	struct thinkos_rt * krn = &thinkos_rt;

	if (!__krn_mutex_is_locked(krn, mutex)) {
		/* Lock the mutex */
		__krn_mutex_lock(krn, mutex, thread_id);
		__krn_wq_wakeup(krn, mutex, thread_id);
	}

	return true;
}

#endif /* THINKOS_MUTEX_MAX > 0 */

