/* 
 * thinkos_exit.c
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

void __thinkos_krn_thread_abort(struct thinkos_rt * krn, unsigned int th)
{
	DCC_LOG1(LOG_TRACE, "(thread=%d)", th); 

#if (THINKOS_ENABLE_TIMESHARE)
	{
		unsigned int j;

		for (j = THINKOS_THREAD_FIRST; j <= THINKOS_THREAD_LAST; ++j) {
			if (j == th)
				continue;
			if (!__thread_ctx_is_valid(krn, j))
				continue;
			/* schedule limit reevaluation */
			if (krn->sched_limit < krn->sched_pri[j])
				krn->sched_limit = krn->sched_pri[j];
		}
	}
#endif

#if !(THINKOS_ENABLE_THREAD_ALLOC) && (THINKOS_ENABLE_TIMESHARE)
	/* clear the schedule priority. In case the thread allocation
	 is disabled, the schedule limit reevaluation may produce inconsistent
	 results ... */
	krn->sched_pri[th] = 0;
#endif

#if (THINKOS_ENABLE_THREAD_ALLOC)
	/* Releases the thread block */
	__thread_alloc_clr(krn, th);
#endif

	if (th == __krn_active_get(krn)) {
		/* discard current thread context */
		__krn_sched_discard_active(krn);
	}

	/* clear context. */
	__thread_ctx_clr(krn, th);

	__krn_thread_suspend(krn, th);

	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}

void __thinkos_krn_abort_all(struct thinkos_rt * krn)
{
	unsigned int j;

	for (j = THINKOS_THREAD_FIRST; j <= THINKOS_THREAD_LAST; ++j) {
		__thinkos_krn_thread_abort(krn, j);
	}
}

#if (THINKOS_ENABLE_TERMINATE)
/* Terminate the target thread */
void thinkos_terminate_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int thread_id;
	int code = arg[1];

	(void)code;

	if ((unsigned int)arg[0] == 0) {
		thread_id = self;
		DCC_LOG2(LOG_WARNING, "<%d> terminate(0, %d)", self , code); 
	} else {
		thread_id = arg[0];
		DCC_LOG3(LOG_WARNING, "<%d> terminate(%d, %d)", self, 
				 thread_id, code); 
	}
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = __krn_thread_check(krn, thread_id)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%d> invalid thread %d!", self, thread_id);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif


#if (THINKOS_ENABLE_TIMESHARE)
	/* possibly remove from the time share wait queue */
	__bit_mem_wr((uint32_t *)&krn->wq_tmshare, thread_id - 1, 0);  
#endif

#if (THINKOS_ENABLE_CANCEL)
#if (THINKOS_ENABLE_THREAD_STAT)
	{
		int stat;
		/* update the thread status */
		stat = krn->th_stat[thread_id];
		DCC_LOG1(LOG_INFO, "wq=%d", stat >> 1); 
		__thread_stat_clr(krn, thread_id);
		/* remove from other wait queue, if any */
		__bit_mem_wr(&krn->wq_lst[stat >> 1], thread_id  - 1, 0);  
	}
#else
	{
		int i;
		/* remove from other wait queues, if any */
		for (i = 0; i < __wq_idx(krn->wq_end); ++i)
			__bit_mem_wr(&krn->wq_lst[i], thread_id  - 1, 0);  
	}
#endif
#endif /* THINKOS_ENABLE_CANCEL */

#if (THINKOS_ENABLE_JOIN)
	{
		unsigned int wq = thread_id;
		int th;

		if ((th = __krn_wq_head(krn, wq)) != THINKOS_THREAD_VOID) {
			do {
				DCC_LOG2(LOG_INFO, "<%2d> wakeup from join %d.", th + 1, wq);
				/* wakeup all remaining threads */
				__krn_wq_wakeup(krn, wq, th);
				__thread_r0_set(krn, th, code);
			} while ((th = __krn_wq_head(krn, wq)) != THINKOS_THREAD_VOID);
			/* signal the scheduler ... */
			__krn_defer_sched(krn);
		}
	}
#endif

	__thinkos_krn_thread_abort(krn, thread_id);

#if (THINKOS_ENABLE_MONITOR_THREADS)
	DCC_LOG2(LOG_WARNING, "monitor_signal_terminate: thread=%d code=%d", 
			 thread_id, code);
	monitor_signal_thread_terminate(thread_id, code);
#endif
}
#endif /* THINKOS_ENABLE_TERMINATE */

void __attribute__((noreturn)) __thinkos_thread_terminate_stub(int code)
{
#if (THINKOS_ENABLE_TERMINATE)
	thinkos_terminate(0, code);
#endif
	for(;;);
}

#if (THINKOS_ENABLE_EXIT)
void thinkos_exit_svc(struct cm3_except_context * ctx, int self,
					  struct thinkos_rt * krn)
{
	DCC_LOG2(LOG_INFO, "<%2d> exit with code %d!", self, ctx->r0); 

#if (THINKOS_ENABLE_JOIN)
	if (krn->wq_lst[self] == 0) {
		DCC_LOG1(LOG_MSG, "<%2d> canceled...", self); 
		/* insert into the canceled wait queue and wait for a join call */ 
		__krn_wq_insert(krn, THINKOS_WQ_CANCELED, self);
		/* remove from the ready wait queue */
		__krn_thread_suspend(krn, self);
		/* signal the scheduler ... */
		__krn_defer_sched(krn);
	}
#endif /* THINKOS_ENABLE_JOIN */

	/* adjust PC to the exit continuation call */
	ctx->pc = (uint32_t)__thinkos_thread_terminate_stub;
}

#endif /* THINKOS_ENABLE_EXIT */

