/* 
 * thread_get.c
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
 * but WITHOUT ANY WARRANTY; without flagen the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#include "thinkos_krn-i.h"

int __thinkos_thread_get(unsigned int thread_id, 
						 struct thinkos_thread * st, 
						 struct cortex_m_context * ctx)
{
	struct thinkos_rt * rt = &thinkos_rt;

	if ((thread_id >= THINKOS_THREADS_MAX) || 
		!__thinkos_thread_ctx_is_valid(thread_id)) {
		return -1;
	}

	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	st->no = thread_id + 1;

#if THINKOS_ENABLE_THREAD_STAT
	st->wq = rt->th_stat[thread_id] >> 1;
	st->tmw = rt->th_stat[thread_id] & 1;
#else
	{
		int i;
		for (i = 0; i < THINKOS_WQ_CNT; ++i) {
			if (rt->wq_lst[i] & (1 << thread_id))
				break;
		}
		if (i == THINKOS_WQ_CNT)
			return -1; /* not found */
		st->wq = i;
	}
 #if THINKOS_ENABLE_CLOCK
	st->tmw = rt->wq_clock & (1 << thread_id) ? 1 : 0;
 #else
	st->tmw = 0;
 #endif
#endif /* THINKOS_ENABLE_THREAD_STAT */

#if THINKOS_ENABLE_THREAD_ALLOC
	st->alloc = rt->th_alloc[0] & (1 << thread_id) ? 1 : 0;
#else
	st->alloc = 0;
#endif

#if THINKOS_ENABLE_TIMESHARE
	st->sched_val = rt->sched_val[thread_id];
	st->sched_pri = rt->sched_pri[thread_id]; 
#else
	st->sched_val = 0;
	st->sched_pri = 0;
#endif

#if THINKOS_ENABLE_CLOCK
	st->clock = (int32_t)(rt->clock[thread_id] - rt->ticks); 
#else
	st->clock = -1;
#endif

#if THINKOS_ENABLE_PROFILING
	st->cyccnt = rt->cyccnt[thread_id];
#else
	st->cyccnt = 0;
#endif

#if THINKOS_ENABLE_THREAD_INFO
	st->inf = rt->th_inf[thread_id];
#else
	st->inf = NULL;
#endif

	if (ctx != NULL) {
		struct thinkos_context * src;
		src = __thinkos_thread_ctx_get(thread_id);

		ctx->r0 = src->r0;
		ctx->r1 = src->r1;
		ctx->r2 = src->r2;
		ctx->r3 = src->r3;
		ctx->r4 = src->r4;
		ctx->r5 = src->r5;
		ctx->r6 = src->r6;
		ctx->r7 = src->r7;
		ctx->r8 = src->r8;
		ctx->r9 = src->r9;
		ctx->r10 = src->r10;
		ctx->r11 = src->r11;
		ctx->r12 = src->r12;
		ctx->sp = (uint32_t)src;
		ctx->lr = src->lr;
		ctx->pc = src->pc;
		ctx->xpsr = src->xpsr;
	}

	return 0;
}

