/* 
 * thikos.c
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
#include <thinkos.h>
#include <sys/delay.h>

#if THINKOS_ENABLE_EXIT
static void __exit_stub(int code)
{
	thinkos_exit(code);
}
#endif

void __thinkos_thread_init(unsigned int thread_id, uint32_t sp, 
						   void * task, void * arg)
{
	struct thinkos_context * ctx;
	uint32_t pc;

	pc = (uint32_t)task & 0xfffffffe;
	sp &= 0xfffffff8; /* 64bits alignemnt */


	sp -= sizeof(struct thinkos_context);
	ctx = (struct thinkos_context *)sp;

	__thinkos_memset32(ctx, 0, sizeof(struct thinkos_context));

	ctx->r0 = (uint32_t)arg;
#if THINKOS_ENABLE_EXIT
	ctx->lr = (uint32_t)__exit_stub;
#else
	ctx->lr = (uint32_t)__thinkos_thread_exit;
#endif
	ctx->pc = pc;
	ctx->xpsr = CM_EPSR_T; /* set the thumb bit */
	thinkos_rt.ctx[thread_id] = ctx;

#if THINKOS_ENABLE_PAUSE
	/* insert into the paused list */
	__bit_mem_wr(&thinkos_rt.wq_paused, thread_id, 1);  
#endif

	DCC_LOG4(LOG_TRACE, "thread_id=%d sp=%08x lr=%08x pc=%08x", 
			 thread_id, sp, ctx->lr, ctx->pc);
	DCC_LOG4(LOG_MSG, "r0=%08x r1=%08x r2=%08x r3=%08x", 
			 ctx->r0, ctx->r1, ctx->r2, ctx->r3);
	DCC_LOG3(LOG_MSG, "msp=%08x psp=%08x ctrl=%02x", 
			 cm3_msp_get(), cm3_psp_get(), cm3_control_get());
}

/* initialize a thread context */
void thinkos_thread_create_svc(int32_t * arg)
{
	struct thinkos_thread_init * init = (struct thinkos_thread_init *)arg;
	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	int target_id = init->opt.id - 1;
	int thread_id;
	uint32_t sp;

#if THINKOS_ENABLE_THREAD_ALLOC
	DCC_LOG1(LOG_INFO, "thinkos_rt.th_alloc=0x%08x", thinkos_rt.th_alloc[0]);

	if (target_id >= THINKOS_THREADS_MAX) {
		thread_id = thinkos_alloc_hi(thinkos_rt.th_alloc, THINKOS_THREADS_MAX);
		DCC_LOG2(LOG_INFO, "thinkos_alloc_hi() %d -> %d.", target_id, 
				 thread_id);
	} else {
		/* Look for the next available slot */
		if (target_id < 0)
			target_id = 0;
		thread_id = thinkos_alloc_lo(thinkos_rt.th_alloc, target_id);
		DCC_LOG2(LOG_INFO, "thinkos_alloc_lo() %d -> %d.", 
				 target_id, thread_id);
		if (thread_id < 0) {
			thread_id = thinkos_alloc_hi(thinkos_rt.th_alloc, target_id);
			DCC_LOG2(LOG_INFO, "thinkos_alloc_hi() %d -> %d.", 
					target_id, thread_id);
		}
	}

	if (thread_id < 0) {
		__thinkos_error(THINKOS_ERR_THREAD_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#else
	thread_id = target_id;
	if (thread_id >= THINKOS_THREADS_MAX) {
		__thinkos_error(THINKOS_ERR_THREAD_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	sp = (uint32_t)init->stack_ptr + init->opt.stack_size;

#if THINKOS_ENABLE_SANITY_CHECK
	if (init->opt.stack_size < sizeof(struct thinkos_context)) {
		DCC_LOG1(LOG_INFO, "stack too small. size=%d", init->opt.stack_size);
		__thinkos_error(THINKOS_ERR_THREAD_SMALLSTACK);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if THINKOS_ENABLE_STACK_INIT
	/* initialize stack */
	__thinkos_memset32(init->stack_ptr, 0xdeadbeef, init->opt.stack_size);
#endif

#if THINKOS_ENABLE_THREAD_INFO
	thinkos_rt.th_inf[thread_id] = init->inf;
#endif

#if THINKOS_ENABLE_TIMESHARE
	thinkos_rt.sched_pri[thread_id] = init->opt.priority;
	if (thinkos_rt.sched_pri[thread_id] > THINKOS_SCHED_LIMIT_MAX)
		thinkos_rt.sched_pri[thread_id] = THINKOS_SCHED_LIMIT_MAX;

	/* update schedule limit */
	if (thinkos_rt.sched_limit < thinkos_rt.sched_pri[thread_id]) {
		thinkos_rt.sched_limit = thinkos_rt.sched_pri[thread_id];
	}
	thinkos_rt.sched_val[thread_id] = thinkos_rt.sched_limit / 2;
#endif

	__thinkos_thread_init(thread_id, sp, init->task, init->arg);

#if THINKOS_ENABLE_PAUSE
	if (!init->opt.paused)
#endif
	{
		DCC_LOG(LOG_JABBER, "__thinkos_thread_resume()");
		__thinkos_thread_resume(thread_id);
		DCC_LOG(LOG_JABBER, "__thinkos_defer_sched()");
		__thinkos_defer_sched();
	}

	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	arg[0] = thread_id + 1;
}


