/* 
 * thinkos_cond.c
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

/* -------------------------------------------------------------------------- 
 * conditional variables 
 * --------------------------------------------------------------------------*/

#if ((THINKOS_COND_MAX) > 0)

static inline bool __attribute__((always_inline)) 
__krn_obj_is_cond(struct thinkos_rt * krn, unsigned int cond) {
	return __obj_is_valid(cond, THINKOS_COND_BASE, THINKOS_COND_MAX);
}

static inline bool __attribute__((always_inline)) 
__krn_cond_is_alloc(struct thinkos_rt * krn, unsigned int cond) {
	return __bit_mem_rd(krn->cond_alloc, cond - THINKOS_COND_BASE) ? 
		true : false;
}

int krn_cond_check(struct thinkos_rt * krn, int cond)
{
#if THINKOS_ENABLE_ARG_CHECK
	if (!__krn_obj_is_mutex(krn, cond)) {
		return THINKOS_ERR_COND_INVALID;
	}
#if THINKOS_ENABLE_MUTEX_ALLOC
	if (__krn_mutex_is_alloc(krn, cond) == 0) {
		return THINKOS_ERR_COND_ALLOC;
	}
#endif
#endif
	return THINKOS_OK;
}

void thinkos_cond_wait_svc(int32_t * arg, int self)
{
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int cond = arg[0];
	unsigned int mutex = arg[1];
#if THINKOS_ENABLE_ARG_CHECK
	int ret;
#endif
	int th;

#if THINKOS_ENABLE_ARG_CHECK
	if ((ret = krn_cond_check(krn, cond)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%d> invalid conditional variable %d!", 
				 self, cond);
		__THINKOS_ERROR(self, THINKOS_ERR_COND_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}

	if ((ret = krn_mutex_check(krn, mutex)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%d> invalid mutex %d!", self, mutex);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if THINKOS_ENABLE_SANITY_CHECK
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

	/* insert into the cond wait queue */
	__krn_thread_wait(krn, self, cond);

	DCC_LOG3(LOG_INFO, "<%2d> mutex %d unlocked, waiting on cond %d...", 
			 self, mutex, cond);
    if ((th = krn_mutex_unlock_wakeup(krn, mutex)) != THINKOS_THREAD_NULL) {
		DCC_LOG2(LOG_MSG, "<%2d> mutex %d locked.", th, mutex);
	}

	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}


#if THINKOS_ENABLE_TIMED_CALLS
void thinkos_cond_timedwait_svc(int32_t * arg, int self)
{
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int cond = arg[0];
	unsigned int mutex = arg[1];
	uint32_t ms = (uint32_t)arg[2];
#if THINKOS_ENABLE_ARG_CHECK
	int ret;
#endif
	int th;

#if THINKOS_ENABLE_ARG_CHECK
	if ((ret = krn_cond_check(krn, cond)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%d> invalid conditional variable %d!", 
				 self, cond);
		__THINKOS_ERROR(self, THINKOS_ERR_COND_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}

	if ((ret = krn_mutex_check(krn, mutex)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%d> invalid mutex %d!", self, mutex);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if THINKOS_ENABLE_SANITY_CHECK
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

	/* Set the default return value to timeout. The
	   cond_signal call will change this to 0 */
	arg[0] = THINKOS_ETIMEDOUT;

	/* insert into the cond wait queue */
	__krn_thread_timedwait(krn, self, cond, ms);

	DCC_LOG3(LOG_INFO, "<%2d> mutex %d unlocked, waiting on cond %d...", 
			 self, mutex, cond);
    if ((th = krn_mutex_unlock_wakeup(krn, mutex)) != THINKOS_THREAD_NULL) {
		DCC_LOG2(LOG_MSG, "<%2d> mutex %d locked.", th, mutex);
	}

	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}
#endif

void thinkos_cond_signal_svc(int32_t * arg, unsigned int self)
{	
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int cond = arg[0];
#if THINKOS_ENABLE_ARG_CHECK
	int ret;
#endif
	int th;

#if THINKOS_ENABLE_ARG_CHECK
	if ((ret = krn_cond_check(krn, cond)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid condition %d!", self, cond);
		__THINKOS_ERROR(self, THINKOS_ERR_COND_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	if ((th = __krn_wq_head(krn, cond)) != THINKOS_THREAD_NULL) {
		DCC_LOG2(LOG_INFO, "<%2d> wakeup from condition %d.", th, cond);
		/* wakeup from the condition wait queue */
		__krn_wq_wakeup(krn, cond, th);
		/* signal the scheduler ... */
		__krn_defer_sched(krn);
	}

	/* Set the return value */
	arg[0] = THINKOS_OK;
}

void thinkos_cond_broadcast_svc(int32_t * arg, unsigned int self)
{	
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int cond = arg[0];
#if THINKOS_ENABLE_ARG_CHECK
	int ret;
#endif
	int th;

#if THINKOS_ENABLE_ARG_CHECK
	if ((ret = krn_cond_check(krn, cond)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid condition %d!", self, cond);
		__THINKOS_ERROR(self, THINKOS_ERR_COND_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	if ((th = __krn_wq_head(krn, cond)) != THINKOS_THREAD_NULL) {
		do {
			DCC_LOG2(LOG_INFO, "<%2d> wakeup from cond %d.", th, cond);
			/* wakeup from the cond wait queue */
			__krn_wq_wakeup(krn, cond, th);
			/* get the next thread */
		} while ((th = __krn_wq_head(krn, cond)) != THINKOS_THREAD_NULL);

		/* signal the scheduler ... */
		__krn_defer_sched(krn);
	}

	/* Set the return value */
	arg[0] = THINKOS_OK;
}

#endif /* THINKOS_COND_MAX > 0 */

