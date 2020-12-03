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

#include "thinkos_krn-i.h"

#include <sys/dcclog.h>

int __krn_thread_check(struct thinkos_rt * krn, unsigned int th)
{
#if THINKOS_ENABLE_ARG_CHECK
	if (!__krn_obj_is_thread(krn, th)) {
		return THINKOS_ERR_THREAD_INVALID;
	}
#if THINKOS_ENABLE_THREAD_ALLOC
	if (__krn_thread_is_alloc(krn, th) == 0) {
		return THINKOS_ERR_THREAD_ALLOC;
	}
#endif
#endif
	return THINKOS_OK;
}


int thinkos_krn_thread_init(struct thinkos_rt * krn,
							unsigned int thread_no,
							struct thinkos_thread_initializer * init)
{
	const struct thinkos_thread_inf * inf = init->inf;
	uintptr_t stack_base = init->stack_base;
	uintptr_t stack_size = init->stack_size;
	uintptr_t task_entry = init->task_entry;
	uintptr_t task_exit = init->task_exit;
	uint32_t * task_arg = init->task_arg;
	int priority = init->priority;
	bool paused = init->paused;
	bool privileged = init->privileged;
	struct thinkos_context * ctx;
	uint32_t ctrl;
	uint32_t free;
	uint32_t stack_top;

	stack_top = stack_base + stack_size;

	if (inf != NULL) {
		DCC_LOG1(LOG_TRACE, "  tag: \"%s\"", inf->tag);
	}
	DCC_LOG3(LOG_TRACE, "stack: top=%08x base=%08x size=%d", 
			 stack_top, stack_base, stack_size);
	DCC_LOG2(LOG_TRACE, " task: entry=%08x exit=%08x", task_entry, task_exit);
	DCC_LOG4(LOG_TRACE, "  arg: %08x %08x %08x %08x", task_arg[0], 
			 task_arg[1], task_arg[2], task_arg[3]);

#if (THINKOS_ENABLE_SANITY_CHECK)
	if (!__thinkos_mem_usr_rw_chk(stack_base, stack_size)) {
		DCC_LOG2(LOG_WARNING, "stack address invalid! base=%08x size=%d", 
				 stack_base, stack_size);
		return THINKOS_ERR_THREAD_STACKADDR;
	}

	if (stack_size < sizeof(struct thinkos_context)) {
		DCC_LOG1(LOG_WARNING, "stack too small. size=%d", stack_size);
		return THINKOS_ERR_THREAD_SMALLSTACK;
	}

#if (THINKOS_ENABLE_PRIVILEGED_THREAD)
	/* Set the thread privilege */
	if (!privileged) {
		if (!__thinkos_mem_usr_rx_chk(task_entry, 8)) {
			DCC_LOG1(LOG_WARNING, "entry point invalid! pc=%08x", task_entry);
			return THINKOS_ERR_THREAD_ENTRYADDR;
		}

		if (!__thinkos_mem_usr_rx_chk(task_exit, 8)) {
			DCC_LOG1(LOG_WARNING, "exit point invalid! lr=%08x", task_exit);
			return THINKOS_ERR_THREAD_EXITADDR;
		}
	}
#endif
#endif
	if (stack_top & (STACK_ALIGN_MSK)) {
		DCC_LOG1(LOG_PANIC, "stack_top=%08x unaligned", stack_top); 
		return THINKOS_ERR_THREAD_STACKALIGN;
	}

	if (stack_size & (STACK_ALIGN_MSK)) {
		DCC_LOG1(LOG_PANIC, "stack_Size=%08x unaligned", stack_size); 
		return THINKOS_ERR_THREAD_STACKALIGN;
	}

	free = stack_size - sizeof(struct thinkos_context);

	(void)free;
	(void)ctrl;
	(void)paused;
	(void)privileged;
	(void)priority;
#if (THINKOS_ENABLE_STACK_INIT)
	/* initialize stack */
	__thinkos_memset32((void *)stack_base, 0xdeadbeef, free);
#elif (THINKOS_ENABLE_MEMORY_CLEAR)
	__thinkos_memset32(stack_base, 0, free);
#endif

#if (THINKOS_NRT_THREADS_MAX > 0)
	if (thread_no >= (THINKOS_THREADS_MAX)) {
		/* TODO: implement NRT */
		return THINKOS_ERR_NOT_IMPLEMENTED;
	}
#endif

	ctx = __thinkos_thread_ctx_init(stack_top, stack_size,
									task_entry, task_exit, task_arg);

	__thread_fault_clr(krn, thread_no);

	__thread_sl_set(krn, thread_no, stack_base);

	__thread_priority_set(krn, thread_no, priority);

	__thread_inf_set(krn, thread_no, inf);

#if (THINKOS_ENABLE_PRIVILEGED_THREAD)
	/* Set the thread privilege */
	ctrl = privileged ? CONTROL_SPSEL : CONTROL_SPSEL | CONTROL_nPRIV;
#else
	ctrl = CONTROL_SPSEL | CONTROL_nPRIV;
#endif
	/* commit the context to the kernel */ 
	DCC_LOG4(LOG_TRACE, "<%d> ctx=%08x ctrl=%d pc=%08x", 
			 thread_no, ctx, ctrl, ctx->pc);
	__thread_ctx_set(krn, thread_no, ctx, ctrl);

	__thread_enable(krn, thread_no);

	if (paused) {
		DCC_LOG1(LOG_TRACE, "<%d> thread paused...", thread_no);
		__thread_pause_set(krn, thread_no);
	} else {
		DCC_LOG1(LOG_TRACE, "<%d> thread ready...", thread_no);
		__thread_ready_set(krn, thread_no);
		__krn_defer_sched(krn);
	}

	return 0;
}

/* initialize a thread */
void thinkos_thread_init_svc(int32_t * arg, unsigned int self)
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct thinkos_thread_initializer * init;
	unsigned int thread_no;
	int ret;

	/* collect call arguments */
	thread_no = arg[0];
	init = (struct thinkos_thread_initializer *)arg[1];
	
#if THINKOS_ENABLE_ARG_CHECK
	if ((ret = __krn_thread_check(krn, thread_no)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%d> invalid thread %d!", 
				 self, thread_no);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

#if (THINKOS_ENABLE_SANITY_CHECK)
	if (__thread_ctx_is_valid(krn, thread_no)) {
		DCC_LOG2(LOG_ERROR, "thread %d already exists, ctx=%08x", 
				 thread_no, __thread_ctx_get(krn, thread_no));
		__THINKOS_ERROR(self, THINKOS_ERR_THREAD_EXIST);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	DCC_LOG3(LOG_TRACE, "<%2d> thread=%d init=0x%08x",
			 self, thread_no, init);

	if ((ret = thinkos_krn_thread_init(krn, thread_no, init))) {
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	};

	arg[0] = thread_no;

	return;
}

#if (THINKOS_ENABLE_THREAD_FAULT)
int __thinkos_thread_fault_code(unsigned int thread_no)
{
	struct thinkos_except * xcpt = __thinkos_except_buf();
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int insn;
	uint16_t * pc;
	int code;

#if 0
FIXME:
	if (!__thread_isfaulty(krn, thread_no))
		return 0;
#endif

	if (__xcpt_active_get(xcpt) == (int)thread_no)
		return xcpt->errno;

	pc = (uint16_t *)__thread_pc_get(krn, thread_no);
	insn = pc[0];
	code = insn & 0x00ff;

	return code - THINKOS_BKPT_EXCEPT_OFF;
}

struct thinkos_context * __thinkos_thread_ctx(unsigned int thread_no)
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct thinkos_except * xcpt = __thinkos_except_buf();

	if (__xcpt_active_get(xcpt) == (int)thread_no)
		return &xcpt->ctx.core;

	return __thread_ctx_get(krn, thread_no);
}
#endif

void __krn_thread_wait(struct thinkos_rt * krn, unsigned int th, 
					   unsigned int wq) 
{
	/* (1) suspend the thread by removing it from the
	   ready wait queue. */
	__krn_thread_suspend(krn, th);
	__krn_wq_insert(krn, wq, th);
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}

void __krn_thread_timedwait(struct thinkos_rt * krn, unsigned int th, 
							unsigned int wq, unsigned int ms) {
	__krn_tmdwq_insert(krn, wq, th, ms);
	__krn_thread_suspend(krn, th);
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}

void __krn_thread_clk_itv_wait(struct thinkos_rt * krn, unsigned int th, 
							  unsigned int ms) 
{
	/* Set the default return value to timeout. 
	   The wake up call will change this to 0 */
	__thread_r0_set(krn, th, THINKOS_ETIMEDOUT);
	/* set the clock */
	__thread_clk_itv_set(krn, th, ms);
	/* insert into the clock wait queue */
	__thread_clk_enable(krn, th);
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}

void __krn_wq_wakeup_all(struct thinkos_rt * krn, unsigned int wq)
{
	unsigned int th;

	if ((th = __krn_wq_head(krn, wq)) != THINKOS_THREAD_NULL) {
		do {
			DCC_LOG2(LOG_INFO, "<%2d> wakeup from %d.", th, wq);
			/* wakeup from the cond wait queue */
			__krn_wq_wakeup(krn, wq, th);
			/* get the next thread */
		} while ((th = __krn_wq_head(krn, wq)) != THINKOS_THREAD_NULL);

		/* signal the scheduler ... */
		__krn_defer_sched(krn);
	}
}
