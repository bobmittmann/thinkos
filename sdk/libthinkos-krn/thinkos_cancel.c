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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif
#include <thinkos.h>
#include <sys/delay.h>
#include <sys/dcclog.h>

#if THINKOS_ENABLE_CANCEL
void thinkos_cancel_svc(int32_t * arg, int self)
{
	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	unsigned int thread = (unsigned int)arg[0];
	unsigned int thread_id;
	int code = arg[1];
	unsigned int wq;
	int stat;

	if (thread == 0)
		thread_id = self;
	else
		thread_id = thread - 1;

#if THINKOS_ENABLE_ARG_CHECK
	if (thread_id >= THINKOS_THREADS_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid thread %d!", thread_id);
		__THINKOS_ERROR(THINKOS_ERR_THREAD_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if THINKOS_ENABLE_THREAD_ALLOC
	if (__bit_mem_rd(thinkos_rt.th_alloc, thread_id) == 0) {
		__THINKOS_ERROR(THINKOS_ERR_THREAD_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

#if (THINKOS_ENABLE_THREAD_STAT == 0)
#error "thinkos_cancel() depends on THINKOS_ENABLE_THREAD_STAT"	
#endif
	stat = thinkos_rt.th_stat[thread_id];
	/* remove from other wait queue including wq_ready */
	__bit_mem_wr(&thinkos_rt.wq_lst[stat >> 1], thread_id, 0);

#if THINKOS_ENABLE_JOIN
	/* insert into the canceled wait queue and wait for a join call */ 
	wq = __wq_idx(&thinkos_rt.wq_canceled);
#else /* THINKOS_ENABLE_JOIN */
	/* if join is not enabled insert into the ready queue */
	wq = __wq_idx(&thinkos_rt.wq_ready);
#endif /* THINKOS_ENABLE_JOIN */

	__thinkos_wq_insert(wq, thread_id);

#if THINKOS_ENABLE_TIMESHARE
	/* possibly remove from the time share wait queue */
	__bit_mem_wr(&thinkos_rt.wq_tmshare, thread_id, 0); 
#endif

#if THINKOS_ENABLE_CLOCK
	/* possibly remove from the time wait queue */
	__bit_mem_wr(&thinkos_rt.wq_clock, thread_id, 0);  
#endif

	DCC_LOG3(LOG_TRACE, "<%d> cancel %d, with code %d!", 
			 thinkos_rt.active, thread_id, code); 

	thinkos_rt.ctx[thread_id]->pc = (uint32_t)__thinkos_thread_terminate_stub;
	thinkos_rt.ctx[thread_id]->r0 = code;
	arg[0] = 0;
}
#endif


