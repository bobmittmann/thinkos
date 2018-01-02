/* 
 * thikos_debug.c
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
#include <stdio.h>

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>

#include <thinkos.h>

void thinkos_trace_rt(struct thinkos_rt * rt)
{
	uint32_t * wq;
	int i;

	for (wq = rt->wq_lst; wq != rt->wq_end; ++wq) {
		int oid;
		int type;
		if (*wq) { 
			oid = wq - rt->wq_lst;
			type = thinkos_obj_type_get(oid);
			switch (type) {
			case THINKOS_OBJ_READY:
				DCC_LOG2(LOG_TRACE, "READY %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_TMSHARE:
				DCC_LOG2(LOG_TRACE, "TMSHARE %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_CANCELED:
				DCC_LOG2(LOG_TRACE, "CANCELED %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_PAUSED:
				DCC_LOG2(LOG_TRACE, "PAUSED %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_CLOCK:
				DCC_LOG2(LOG_TRACE, "CLOCK %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_MUTEX:
				DCC_LOG2(LOG_TRACE, "MUTEX %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_COND:
				DCC_LOG2(LOG_TRACE, "COND %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_SEMAPHORE:
				DCC_LOG2(LOG_TRACE, "SEMAPHORE %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_EVENT:
				DCC_LOG2(LOG_TRACE, "EVENT %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_FLAG:
				DCC_LOG2(LOG_TRACE, "FLAG %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_JOIN:
				DCC_LOG2(LOG_TRACE, "JOIN %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_FAULT:
				DCC_LOG2(LOG_TRACE, "FAULT %d: 0x%08x", oid, *wq);
				break;
			case THINKOS_OBJ_INVALID:
				DCC_LOG2(LOG_TRACE, "INVALID %d: 0x%08x", oid, *wq);
				break;
			}
		}
	}

#if THINKOS_ENABLE_CLOCK
	DCC_LOG1(LOG_TRACE, "Ticks = %d", rt->ticks);
#endif

	for (i = 0; i < THINKOS_THREADS_MAX; ++i) {
		struct thinkos_context * ctx;

		if ((ctx = rt->ctx[i]) != NULL) {
#if THINKOS_ENABLE_THREAD_STAT
			DCC_LOG5(LOG_TRACE, "%2d %3d sp=%08x lr=%08x pc=%08x", 
					 i, rt->th_stat[i] >> 1, (uint32_t)ctx, ctx->lr, ctx->pc);
#else
			DCC_LOG4(LOG_TRACE, "%2d sp=%08x lr=%08x pc=%08x", 
					 i, (uint32_t)ctx, ctx->lr, ctx->pc);
#endif
		}
	}


}

void __sched_trace(struct thinkos_context * __ctx, 
				   unsigned int __thread_id)
{
	DCC_LOG4(LOG_TRACE, "th=%d sp=%08x pc=%08x rdy=%08x", 
			 __thread_id + 1, __ctx, __ctx->pc, thinkos_rt.wq_ready);
	DCC_LOG4(LOG_INFO, "  r0=%08x  r1=%08x  r2=%08x  r3=%08x", 
			 __ctx->r0, __ctx->r1, __ctx->r2, __ctx->r3);
	DCC_LOG4(LOG_INFO, "  r4=%08x  r5=%08x  r6=%08x  r7=%08x", 
			 __ctx->r4, __ctx->r7, __ctx->r6, __ctx->r7);
	DCC_LOG4(LOG_INFO, "  r8=%08x  r9=%08x r10=%08x r11=%08x", 
			 __ctx->r8, __ctx->r9, __ctx->r10, __ctx->r11);
	DCC_LOG4(LOG_INFO, " r12=%08x  sp=%08x  lr=%08x  pc=%08x", 
			 __ctx->r12, __ctx, __ctx->lr, __ctx->pc);
	DCC_LOG1(LOG_INFO, "xpsr=%08x", __ctx->xpsr);
}

void thinkos_sched_dbg(struct thinkos_context * __ctx, 
						 unsigned int __new_thread_id,
						 unsigned int __prev_thread_id) 
{
	__sched_trace(__ctx, __new_thread_id);
}

void thinkos_sched_step_dbg(struct thinkos_context * __ctx, 
							 unsigned int __new_thread_id,
							 unsigned int __prev_thread_id) 
{
	DCC_LOG(LOG_INFO, "single step ... ");
	__sched_trace(__ctx, __new_thread_id);
}

