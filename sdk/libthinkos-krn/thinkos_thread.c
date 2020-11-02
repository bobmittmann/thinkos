/* 
 * thinkos_thread.c
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
#define __THINKOS_NRT__
#include <thinkos/nrt.h>
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#include <thinkos.h>
#include <sys/delay.h>
#include <sys/dcclog.h>

#if (THINKOS_ENABLE_DEBUG_FAULT)
int __thinkos_thread_fault_code(unsigned int thread_id)
{
	struct thinkos_except * xcpt = __thinkos_except_buf();
	unsigned int insn;
	uint16_t * pc;
	int code;

	if (!__thinkos_thread_isfaulty(thread_id))
		return 0;

	if (__xcpt_active_get(xcpt) == thread_id)
		return xcpt->errno;

	pc = (uint16_t *)__thinkos_thread_pc_get(thread_id);
	insn = pc[0];
	code = insn & 0x00ff;

	return code - THINKOS_BKPT_EXCEPT_OFF;
}

struct thinkos_context * __thinkos_thread_ctx(unsigned int thread_id)
{
	struct thinkos_except * xcpt = __thinkos_except_buf();

	if (__xcpt_active_get(xcpt) == thread_id)
		return &xcpt->ctx.core;

	return __thinkos_thread_ctx_get(thread_id);
}
#endif

/* Initilize a context at SP 
 Return a context structure */
struct thinkos_context * __thinkos_thread_ctx_init(unsigned int thread_id, 
												   uintptr_t sp, 
												   uintptr_t task,
												   uintptr_t arg)
{
	struct thinkos_context * ctx;
	uint32_t pc;

	pc = (uint32_t)task & 0xfffffffe;
	sp &= 0xfffffff0; /* 64bits alignemnt */

	sp -= sizeof(struct thinkos_context);
	ctx = (struct thinkos_context *)sp;

#if (THINKOS_ENABLE_STACK_INIT) && (THINKOS_ENABLE_MEMORY_CLEAR)
	__thinkos_memset32(ctx, 0, sizeof(struct thinkos_context));
#endif

	ctx->r0 = (uint32_t)arg;
	ctx->pc = pc;
#if (THINKOS_ENABLE_EXIT)
	ctx->lr = (uint32_t)__thinkos_thread_exit_stub;
#else
	ctx->lr = (uint32_t)__thinkos_thread_terminate_stub;
#endif
	ctx->xpsr = CM_EPSR_T; /* set the thumb bit */

#if 1
	DCC_LOG4(LOG_TRACE, "thread=%d sp=%08x lr=%08x pc=%08x", 
			 thread_id + 1, sp, ctx->lr, ctx->pc);
	DCC_LOG4(LOG_TRACE, "r0=%08x r1=%08x r2=%08x r3=%08x", 
			 ctx->r0, ctx->r1, ctx->r2, ctx->r3);
	DCC_LOG3(LOG_TRACE, "msp=%08x psp=%08x ctrl=%02x", 
			 cm3_msp_get(), cm3_psp_get(), cm3_control_get());
#endif
	return ctx;
}

/* initialize a thread context */
void thinkos_thread_create_svc(int32_t * arg)
{
	struct thinkos_thread_create_args * init = 
		(struct thinkos_thread_create_args *)arg;
	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	int target_id = init->opt.id - 1;
#if (THINKOS_ENABLE_THREAD_INFO)
	struct thinkos_thread_inf * inf = init->inf;
#endif
	uint32_t stack_base = (uint32_t)init->stack_ptr;
	uint32_t stack_size = init->opt.stack_size;
	struct thinkos_context * ctx;
	int thread_id;
	uint32_t sp;

#if (THINKOS_ENABLE_THREAD_ALLOC)
	thread_id = __thinkos_thread_alloc(target_id);

	if (thread_id < 0) {
		__THINKOS_ERROR(THINKOS_ERR_THREAD_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#else
	thread_id = target_id;
	if (thread_id >= (THINKOS_THREADS_MAX) + (THINKOS_NRT_THREADS_MAX)) {
		__THINKOS_ERROR(THINKOS_ERR_THREAD_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if (THINKOS_ENABLE_SANITY_CHECK)
	if (__thinkos_thread_ctx_is_valid(thread_id)) {
		DCC_LOG3(LOG_ERROR, "thread %d already exists, ctx=%08x", 
				 thread_id + 1, __thinkos_thread_ctx_get(thread_id));
		return false;
	}
#endif 


#endif

	sp = stack_base + stack_size;

#if (THINKOS_ENABLE_SANITY_CHECK)
	if (!__thinkos_mem_usr_rw_chk(stack_base, stack_size)) {
		DCC_LOG2(LOG_WARNING, "stack address invalid! base=%08x size=%d", 
				 stack_base, stack_size);
		__THINKOS_ERROR(THINKOS_ERR_THREAD_STACKADDR);
		arg[0] = THINKOS_EINVAL;
		return;
	}

	if (stack_size < sizeof(struct thinkos_context)) {
		DCC_LOG1(LOG_WARNING, "stack too small. size=%d", stack_size);
		__THINKOS_ERROR(THINKOS_ERR_THREAD_SMALLSTACK);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	DCC_LOG2(LOG_INFO, "stack base=%08x size=%d", stack_base, stack_size);

#if (THINKOS_ENABLE_STACK_INIT)
	/* initialize stack */
	__thinkos_memset32((void *)stack_base, 0xdeadbeef, stack_size);
#elif (THINKOS_ENABLE_MEMORY_CLEAR)
	__thinkos_memset32(stack_base, 0, stack_size);
#endif

#if (THINKOS_NRT_THREADS_MAX > 0)
	if (thread_id >= (THINKOS_THREADS_MAX)){
		arg[0] = __thinkos_nrt_thread_init(thread_id);
		return;
	}
#endif

	ctx = __thinkos_thread_ctx_init(thread_id, sp, (uintptr_t)init->task, 
						  (uintptr_t)init->arg);

#if (THINKOS_ENABLE_PAUSE)
	__thinkos_thread_pause_set(thread_id);
#endif

#if (THINKOS_ENABLE_DEBUG_FAULT)
	__thinkos_thread_fault_clr(thread_id);
#endif

#if (THINKOS_ENABLE_STACK_LIMIT)
	__thinkos_thread_sl_set(thread_id, stack_base);
#endif

#if (THINKOS_ENABLE_TIMESHARE)
	thinkos_rt.sched_pri[thread_id] = init->opt.priority;
	if (thinkos_rt.sched_pri[thread_id] > THINKOS_SCHED_LIMIT_MAX)
		thinkos_rt.sched_pri[thread_id] = THINKOS_SCHED_LIMIT_MAX;

	/* update schedule limit */
	if (thinkos_rt.sched_limit < thinkos_rt.sched_pri[thread_id]) {
		thinkos_rt.sched_limit = thinkos_rt.sched_pri[thread_id];
	}
	thinkos_rt.sched_val[thread_id] = thinkos_rt.sched_limit / 2;
#endif

#if (THINKOS_ENABLE_THREAD_INFO)
	DCC_LOG(LOG_MSG, "__thinkos_thread_inf_set()");
	__thinkos_thread_inf_set(thread_id, inf);
#endif

	/* commit the context to the kernel */ 
	__thinkos_thread_ctx_set(thread_id, ctx, CONTROL_SPSEL | CONTROL_nPRIV);

#if (THINKOS_ENABLE_PAUSE)
	if (!init->opt.paused)
#endif
	{
		DCC_LOG(LOG_JABBER, "__thinkos_thread_resume()");
		if (__thinkos_thread_resume(thread_id))
			__thinkos_defer_sched();
	}



	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	arg[0] = thread_id + 1;

#if (THINKOS_ENABLE_MONITOR_THREADS)
	/* Notify the monitor */
	__monitor_signal_thread_create(thread_id);
#endif
}

