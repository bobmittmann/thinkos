/* 
 * thinkos_semaphore.c
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

#if (THINKOS_SEMAPHORE_MAX) > 0

static inline bool __attribute__((always_inline)) 
__krn_obj_is_sem(struct thinkos_rt * krn, unsigned int sem) {
	return __obj_is_valid(sem, THINKOS_SEM_BASE, THINKOS_SEMAPHORE_MAX);
}

#if (THINKOS_ENABLE_SEM_ALLOC)
static inline bool __attribute__((always_inline)) 
__krn_sem_is_alloc(struct thinkos_rt * krn, unsigned int sem) {
	return __bit_mem_rd(krn->sem_alloc, sem - THINKOS_SEM_BASE) ? 
		true : false;
}
#endif

#if THINKOS_ENABLE_ARG_CHECK
int krn_sem_check(struct thinkos_rt * krn, int sem)
{
	if (!__krn_obj_is_sem(krn, sem)) {
		return THINKOS_ERR_SEM_INVALID;
	}
#if (THINKOS_ENABLE_SEM_ALLOC)
	if (__krn_sem_is_alloc(krn, sem) == 0) {
		return THINKOS_ERR_SEM_ALLOC;
	}
#endif
	return THINKOS_OK;
}
#endif

void thinkos_sem_init_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	unsigned int sem = arg[0];
	uint32_t value = (uint32_t)arg[1];
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_sem_check(krn, sem)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid semaphore %d!", self, sem);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	DCC_LOG2(LOG_INFO, "sem[%d] <= %d", sem, value);
	krn->sem_val[sem - THINKOS_SEM_BASE] = value;
	arg[0] = THINKOS_OK;
}

void thinkos_sem_trywait_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	unsigned int sem = arg[0];
	uint32_t sem_val;
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = krn_sem_check(krn, sem)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid semaphore %d!", self, sem);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	/* avoid possible race condition on sem_val */
	/* this is only necessary in case we use the __uthread_sem_post() call
	   inside interrupt handlers */
	do {
		sem_val = __ldrex(&krn->sem_val[sem - THINKOS_SEM_BASE]);
		if (sem_val > 0) {
			sem_val--;
			arg[0] = THINKOS_OK;
		} else {
			arg[0] = THINKOS_EAGAIN;
		}
	} while (__strex(&krn->sem_val[sem - THINKOS_SEM_BASE], sem_val));
}

void thinkos_sem_wait_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	unsigned int sem = arg[0];
	uint32_t sem_val;
	uint32_t queue;
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = krn_sem_check(krn, sem)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid semaphore %d!", self, sem);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	/* avoid possible race condition on sem_val */
	/* this is only necessary in case we use the __uthread_sem_post() call
	   inside interrupt handlers */
again:
	sem_val = __ldrex(&krn->sem_val[sem - THINKOS_SEM_BASE]);
	if (sem_val > 0) {
		DCC_LOG2(LOG_INFO, "<%d> signaled %d...", self, sem);
		sem_val--;
		if (__strex(&krn->sem_val[sem - THINKOS_SEM_BASE], sem_val))
			goto again;
		arg[0] = THINKOS_OK;
		return;
	}

	/* (1) suspend the thread by removing it from the
	   ready wait queue. The __thinkos_suspend() call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access itself,
	   in case we have enabled the time sharing option.
	   It is not a problem having a thread not contained in any waiting
	   queue inside a system call. 
	 */
	__krn_thread_suspend(krn, self);
	/* update the thread status in preparation for event wait */
#if THINKOS_ENABLE_THREAD_STAT
	krn->th_stat[self] = sem << 1;
#endif
	/* (2) Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
/* NEW: 2020-12-02 performed by the svc call entry stub 
	__thread_ctx_flush(krn, arg, self);
 */

	/* insert into the event wait queue */
	queue = __ldrex(&krn->wq_lst[sem]);

	/* The semaphore may have been signaled while suspending (1).
	 If this is the case roll back and restart. */
	if ((volatile uint32_t)krn->sem_val[sem - THINKOS_SEM_BASE] > 0) {
		/* roll back */
		__thread_stat_clr(krn, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);  
		DCC_LOG2(LOG_WARNING, "<%d> rollback 1 %d...", self, sem);
		goto again;
	}

	queue |= (1 << (self - 1));
	if (__strex(&krn->wq_lst[sem], queue)) {
		/* roll back */
		__thread_stat_clr(krn, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);  
		DCC_LOG2(LOG_WARNING, "<%d> rollback 2 %d...", self, sem);
		goto again;
	}

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_INFO, "<%d> waiting on semaphore %d...", self, sem);
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}

#if THINKOS_ENABLE_TIMED_CALLS
void thinkos_sem_timedwait_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	unsigned int sem = arg[0];
	uint32_t ms = (uint32_t)arg[1];
	uint32_t sem_val;
	uint32_t queue;
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = krn_sem_check(krn, sem)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid semaphore %d!", self, sem);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

again:
	sem_val = __ldrex(&krn->sem_val[sem - THINKOS_SEM_BASE]);
	if (sem_val > 0) {
		DCC_LOG2(LOG_INFO, "<%d> signaled %d...", self, sem);
		sem_val--;
		if (__strex(&krn->sem_val[sem - THINKOS_SEM_BASE], sem_val))
			goto again;
		arg[0] = THINKOS_OK;
		return;
	}

	__krn_thread_suspend(krn, self);
#if THINKOS_ENABLE_THREAD_STAT
	/* update status, mark the thread clock enable bit */
	krn->th_stat[self] = (sem << 1) + 1;
#endif
	__thread_ctx_set(krn, self, (struct thinkos_context *)&arg[-CTX_R0],
							 CONTROL_SPSEL | CONTROL_nPRIV);
	queue = __ldrex(&krn->wq_lst[sem]);
	queue |= (1 << (self - 1));
	if (((volatile uint32_t)krn->sem_val[sem - THINKOS_SEM_BASE] > 0) ||
		__strex(&krn->wq_lst[sem], queue)) {
		/* roll back */
		__thread_stat_clr(krn, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);  
		goto again;
	}

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_INFO, "<%d> waiting on semaphore %d...", self, sem);
	/* set the clock */
	__thread_clk_itv_set(krn, self, ms);
	/* insert into the clock wait queue */
	__thread_clk_enable(krn, self);
	/* Set the default return value to timeout. The
	   sem_post call will change this to 0 */
	arg[0] = THINKOS_ETIMEDOUT;
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}
#endif

static void __krn_sem_post(struct thinkos_rt * krn, uint32_t sem) 
{
	uint32_t queue;
	int idx;
	int th;

	do {
		/* insert into the event wait queue */
		queue = __ldrex(&krn->wq_lst[sem]);
		/* get a thread from the queue bitmap */
		if ((idx = __thinkos_ffs(queue)) == 32) {
			uint32_t sem_val;
			/* no threads waiting on the semaphore, increment. */ 
			do {
				sem_val = __ldrex(&krn->sem_val[sem - THINKOS_SEM_BASE]);
				sem_val++;
			} while (__strex(&krn->sem_val[sem - THINKOS_SEM_BASE], sem_val));

			return;
		} 
	
		/* remove from the wait queue */
		queue &= ~(1 << idx);
	} while (__strex(&krn->wq_lst[sem], queue));
	th = idx + 1; 

	/* insert the thread into ready queue */
	__thread_ready_set(krn, th);  
#if THINKOS_ENABLE_TIMED_CALLS
	/* possibly remove from the time wait queue */
	__thread_clk_disable(krn, th);
	/* set the thread's return value */
	__thread_r0_set(krn, th, 0);
#endif
	/* update status */
	__thread_stat_clr(krn, th);
}

#if (THINKOS_ENABLE_I_CALLS)
void cm3_except7_isr(uint32_t sem)
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_sem_post(krn, sem); 
	/* signal the scheduler ... */
	__krn_preempt(krn);
}


void __thinkos_sem_post_i(uint32_t sem) 
{
	struct thinkos_rt * krn = &thinkos_rt;

	__krn_sem_post(krn, sem); 
}
#endif /* THINKOS_ENABLE_I_CALLS */

void thinkos_sem_post_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	unsigned int sem = arg[0];
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = krn_sem_check(krn, sem)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid semaphore %d!", self, sem);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	DCC_LOG1(LOG_INFO, "sem %d +++++++++++++ ", sem);

	arg[0] = THINKOS_OK;
	__krn_sem_post(krn, sem);
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}

#if (THINKOS_ENABLE_PAUSE)
bool semaphore_resume(struct thinkos_rt * krn, unsigned int th, 
							 unsigned int sem, bool tmw) 
{
	unsigned int idx = sem - THINKOS_SEM_BASE;

	DCC_LOG1(LOG_INFO, "PC=%08x ...........", __thread_pc_get(krn, th)); 

	if (krn->sem_val[idx] > 0) {
		krn->sem_val[idx]--;
		/* insert the thread into ready queue */
		__thread_ready_set(krn, th);
#if THINKOS_ENABLE_TIMED_CALLS
		/* set the thread's return value */
		__thread_r0_set(krn, th, 0);
#endif
		/* update status */
		__thread_stat_clr(krn, th);
	} else {
		__thread_wq_set(krn, th, sem);
#if THINKOS_ENABLE_TIMED_CALLS
		if (tmw)
			__thread_clk_enable(krn, th);
#endif
	}
	return true;
}
#endif

#endif /* THINKOS_SEM_MAX > 0 */

