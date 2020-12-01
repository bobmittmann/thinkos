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

#if THINKOS_SEMAPHORE_MAX > 0

void thinkos_sem_init_svc(int32_t * arg, unsigned int self)
{	
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int wq = arg[0];
	unsigned int sem = wq - THINKOS_SEM_BASE;
	uint32_t value = (uint32_t)arg[1];

#if THINKOS_ENABLE_ARG_CHECK
	if (sem >= THINKOS_SEMAPHORE_MAX) {
		DCC_LOG1(LOG_ERROR, "object %d is not a semaphore!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_SEM_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if THINKOS_ENABLE_SEM_ALLOC
	if (__bit_mem_rd(krn->sem_alloc, sem) == 0) {
		DCC_LOG1(LOG_ERROR, "invalid semaphore %d!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_SEM_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	DCC_LOG2(LOG_INFO, "sem[%d] <= %d", sem, value);

	krn->sem_val[sem] = value;
	arg[0] = 0;
}

void thinkos_sem_trywait_svc(int32_t * arg, unsigned int self)
{	
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int wq = arg[0];
	unsigned int sem = wq - THINKOS_SEM_BASE;
	uint32_t sem_val;

#if THINKOS_ENABLE_ARG_CHECK
	if (sem >= THINKOS_SEMAPHORE_MAX) {
		DCC_LOG1(LOG_ERROR, "object %d is not a semaphore!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_SEM_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if THINKOS_ENABLE_SEM_ALLOC
	if (__bit_mem_rd(krn->sem_alloc, sem) == 0) {
		DCC_LOG1(LOG_ERROR, "invalid semaphore %d!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_SEM_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	/* avoid possible race condition on sem_val */
	/* this is only necessary in case we use the __uthread_sem_post() call
	   inside interrupt handlers */
	do {
		sem_val = __ldrex(&krn->sem_val[sem]);
		if (sem_val > 0) {
			sem_val--;
			arg[0] = 0;
		} else {
			arg[0] = THINKOS_EAGAIN;
		}
	} while (__strex(&krn->sem_val[sem], sem_val));
}

void thinkos_sem_wait_svc(int32_t * arg, unsigned int self)
{	
	struct thinkos_rt * krn = &thinkos_rt; 
	unsigned int wq = arg[0];
	unsigned int sem = wq - THINKOS_SEM_BASE;
	uint32_t sem_val;
	uint32_t queue;

#if THINKOS_ENABLE_ARG_CHECK
	if (sem >= THINKOS_SEMAPHORE_MAX) {
		DCC_LOG1(LOG_ERROR, "object %d is not a semaphore!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_SEM_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if THINKOS_ENABLE_SEM_ALLOC
	if (__bit_mem_rd(krn->sem_alloc, sem) == 0) {
		DCC_LOG1(LOG_ERROR, "invalid semaphore %d!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_SEM_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	/* avoid possible race condition on sem_val */
	/* this is only necessary in case we use the __uthread_sem_post() call
	   inside interrupt handlers */
again:
	sem_val = __ldrex(&krn->sem_val[sem]);
	if (sem_val > 0) {
		DCC_LOG2(LOG_INFO, "<%d> signaled %d...", self, wq);
		sem_val--;
		if (__strex(&krn->sem_val[sem], sem_val))
			goto again;
		arg[0] = 0;
		return;
	}

	/* (1) suspend the thread by removing it from the
	   ready wait queue. The __thinkos_suspend() call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access itself,
	   in case we have enabled the time sharing option.
	   It is not a problem having a thread not contained in any waiting
	   queue inside a system call. 
	 */
	__thinkos_suspend(self);
	/* update the thread status in preparation for event wait */
#if THINKOS_ENABLE_THREAD_STAT
	krn->th_stat[self] = wq << 1;
#endif
	/* (2) Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
	__thread_ctx_flush(krn, arg, self);

	/* insert into the event wait queue */
	queue = __ldrex(&krn->wq_lst[wq]);

	/* The semaphore may have been signaled while suspending (1).
	 If this is the case roll back and restart. */
	if ((volatile uint32_t)krn->sem_val[sem] > 0) {
		/* roll back */
		__thinkos_thread_stat_clr(self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);  
		DCC_LOG2(LOG_WARNING, "<%d> rollback 1 %d...", self, wq);
		goto again;
	}

	queue |= (1 << (self - 1));
	if (__strex(&krn->wq_lst[wq], queue)) {
		/* roll back */
		__thinkos_thread_stat_clr(self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);  
		DCC_LOG2(LOG_WARNING, "<%d> rollback 2 %d...", self, wq);
		goto again;
	}

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_INFO, "<%d> waiting on semaphore %d...", self, wq);
	/* signal the scheduler ... */
	__thinkos_defer_sched(); 
}

#if THINKOS_ENABLE_TIMED_CALLS
void thinkos_sem_timedwait_svc(int32_t * arg, unsigned int self)
{	
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int wq = arg[0];
	unsigned int sem = wq - THINKOS_SEM_BASE;
	uint32_t ms = (uint32_t)arg[1];
	uint32_t sem_val;
	uint32_t queue;

#if THINKOS_ENABLE_ARG_CHECK
	if (sem >= THINKOS_SEMAPHORE_MAX) {
		DCC_LOG1(LOG_ERROR, "object %d is not a semaphore!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_SEM_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if THINKOS_ENABLE_SEM_ALLOC
	if (__bit_mem_rd(krn->sem_alloc, sem) == 0) {
		DCC_LOG1(LOG_ERROR, "invalid semaphore %d!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_SEM_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

again:
	sem_val = __ldrex(&krn->sem_val[sem]);
	if (sem_val > 0) {
		DCC_LOG2(LOG_INFO, "<%d> signaled %d...", self, wq);
		sem_val--;
		if (__strex(&krn->sem_val[sem], sem_val))
			goto again;
		arg[0] = 0;
		return;
	}

	__thinkos_suspend(self);
#if THINKOS_ENABLE_THREAD_STAT
	/* update status, mark the thread clock enable bit */
	krn->th_stat[self] = (wq << 1) + 1;
#endif
	__thinkos_thread_ctx_set(self, (struct thinkos_context *)&arg[-CTX_R0],
							 CONTROL_SPSEL | CONTROL_nPRIV);
	queue = __ldrex(&krn->wq_lst[wq]);
	queue |= (1 << (self - 1));
	if (((volatile uint32_t)krn->sem_val[sem] > 0) ||
		__strex(&krn->wq_lst[wq], queue)) {
		/* roll back */
		__thinkos_thread_stat_clr(self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);  
		goto again;
	}

	/* -- wait for event ---------------------------------------- */
	DCC_LOG2(LOG_INFO, "<%d> waiting on semaphore %d...", self, wq);
	/* set the clock */
	krn->th_clk[self] = krn->ticks + ms;
	/* insert into the clock wait queue */
	__bit_mem_wr(&krn->wq_clock, self - 1, 1);  
	/* Set the default return value to timeout. The
	   sem_post call will change this to 0 */
	arg[0] = THINKOS_ETIMEDOUT;
	/* signal the scheduler ... */
	__thinkos_defer_sched(); 
}
#endif

void __thinkos_sem_post_i(uint32_t wq) 
{
	struct thinkos_rt * krn = &thinkos_rt;
	uint32_t queue;
	int idx;
	int th;

	do {
		/* insert into the event wait queue */
		queue = __ldrex(&krn->wq_lst[wq]);
		/* get a thread from the queue bitmap */
		if ((idx = __thinkos_ffs(queue)) == 32) {
			int sem = wq - THINKOS_SEM_BASE;
			uint32_t sem_val;

			/* no threads waiting on the semaphore, increment. */ 
			do {
				sem_val = __ldrex(&krn->sem_val[sem]);
				sem_val++;
			} while (__strex(&krn->sem_val[sem], sem_val));

			return;
		} 
	
		/* remove from the wait queue */
		queue &= ~(1 << idx);
	} while (__strex(&krn->wq_lst[wq], queue));
	th = idx + 1; 

	/* insert the thread into ready queue */
	__thread_ready_set(krn, th);  
#if THINKOS_ENABLE_TIMED_CALLS
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, th - 1, 0);  
	/* set the thread's return value */
	__thinkos_thread_r0_set(th, 0);
#endif
	/* update status */
	__thinkos_thread_stat_clr(th);
}

#if (THINKOS_ENABLE_I_CALLS)
void cm3_except7_isr(uint32_t wq)
{
	__thinkos_sem_post_i(wq); 
	/* signal the scheduler ... */
	__thinkos_preempt();
}
#endif /* THINKOS_ENABLE_I_CALLS */

void thinkos_sem_post_svc(int32_t * arg, unsigned int self)
{	
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int wq = arg[0];

#if THINKOS_ENABLE_ARG_CHECK
	unsigned int sem = wq - THINKOS_SEM_BASE;

	if (sem >= THINKOS_SEMAPHORE_MAX) {
		DCC_LOG1(LOG_ERROR, "object %d is not a semaphore!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_SEM_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if THINKOS_ENABLE_SEM_ALLOC
	if (__bit_mem_rd(krn->sem_alloc, sem) == 0) {
		DCC_LOG1(LOG_ERROR, "invalid semaphore %d!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_SEM_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	DCC_LOG1(LOG_INFO, "sem %d +++++++++++++ ", wq);

	arg[0] = 0;
	__thinkos_sem_post_i(wq);
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}

#endif /* THINKOS_SEM_MAX > 0 */

