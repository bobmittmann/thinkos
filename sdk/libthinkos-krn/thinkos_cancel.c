/* 
 * thinkos_cancel.c
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

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#if (THINKOS_ENABLE_CANCEL)
void thinkos_cancel_svc(int32_t * arg, unsigned int self)
{
	struct thinkos_rt * krn = &thinkos_rt;

	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	unsigned int thread = (unsigned int)arg[0];
	unsigned int thread_idx;
	int code = arg[1];
	unsigned int wq;
	uint32_t pc;

	if (thread == 0)
		thread_idx = self;
	else
		thread_idx = thread - 1;

#if THINKOS_ENABLE_ARG_CHECK
	if (thread_idx >= THINKOS_THREADS_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid thread %d!", thread_idx);
		__THINKOS_ERROR(self, THINKOS_ERR_THREAD_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if THINKOS_ENABLE_THREAD_ALLOC
	if (__bit_mem_rd(krn->th_alloc, thread_idx) == 0) {
		__THINKOS_ERROR(self, THINKOS_ERR_THREAD_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	wq = __thread_wq_get(krn, thread_idx);
	/* remove from other wait queue including wq_ready */
	__bit_mem_wr(&krn->wq_lst[wq], thread_idx, 0);

#if THINKOS_ENABLE_JOIN
	/* insert into the canceled wait queue and wait for a join call */ 
	wq = __wq_idx(krn, &krn->wq_canceled);
#else /* THINKOS_ENABLE_JOIN */
	/* if join is not enabled insert into the ready queue */
	wq = __wq_idx(krn, &krn->wq_ready);
#endif /* THINKOS_ENABLE_JOIN */

	__wq_insert(krn, wq, thread_idx);

#if THINKOS_ENABLE_TIMESHARE
	/* possibly remove from the time share wait queue */
	__bit_mem_wr(&krn->wq_tmshare, thread_idx, 0); 
#endif

#if THINKOS_ENABLE_CLOCK
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, thread_idx, 0);  
#endif

	DCC_LOG3(LOG_TRACE, "<%d> cancel %d, with code %d!", 
			 self + 1, thread_idx + 1, code); 

	pc = (uint32_t)__thinkos_thread_terminate_stub;
	__thread_pc_set(krn, thread_idx, pc);
	__thread_r0_set(krn, thread_idx, code);
	arg[0] = 0;
}
#endif


