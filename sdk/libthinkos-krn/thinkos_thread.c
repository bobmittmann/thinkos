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
#if (THINKOS_ENABLE_ARG_CHECK)
	if (!__krn_obj_is_thread(krn, th)) {
		return THINKOS_ERR_THREAD_INVALID;
	}
#if (THINKOS_ENABLE_THREAD_ALLOC)
	if (__krn_thread_is_alloc(krn, th) == 0) {
		return THINKOS_ERR_THREAD_ALLOC;
	}
#endif
#endif
	return THINKOS_OK;
}

int thinkos_krn_thread_init(
	struct thinkos_rt * krn, unsigned int thread_no,
	const struct thinkos_thread_initializer * init)
{
	const struct thinkos_thread_inf * inf = init->inf;
	uintptr_t stack_base = init->stack_base;
	uintptr_t stack_size = init->stack_size;
	uintptr_t task_entry = init->task_entry;
	uintptr_t task_exit = init->task_exit;
	uint32_t * task_arg = (uint32_t *)init->task_arg;
	int priority = init->priority;
	bool paused = init->paused;
	bool privileged = init->privileged;
	struct thinkos_context * ctx;
	uint32_t ctrl;
	uint32_t free;
	uint32_t stack_top;

	stack_top = stack_base + stack_size;

	if (inf != NULL) {
		DCC_LOG1(LOG_YAP, "  tag: \"%s\"", inf->tag);
	}
	DCC_LOG3(LOG_TRACE, "stack: top=%08x base=%08x size=%d", 
			 stack_top, stack_base, stack_size);
	DCC_LOG2(LOG_TRACE, " task: entry=%08x exit=%08x", task_entry, task_exit);
	DCC_LOG4(LOG_TRACE, " args: %08x %08x %08x %08x", task_arg[0], 
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

	DCC_LOG2(LOG_TRACE, "SP=%08x PSP=%08x", cm3_sp_get(), cm3_psp_get()); 

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

#if (THINKOS_NRT_THREADS_MAX) > 0
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
	ctrl = privileged ? CONTROL_SPSEL : (CONTROL_SPSEL | CONTROL_nPRIV);
#else
	ctrl = 0;
#endif
	/* commit the context to the kernel */ 
	__thread_ctx_set(krn, thread_no, ctx, ctrl);

#if (THINKOS_ENABLE_PAUSE)
	if (paused) {
		DCC_LOG4(LOG_TRACE, "<%d> ctx=%08x ctrl=%d pc=%08x paused...", 
				 thread_no, ctx, ctrl, ctx->pc);
		__thread_pause_set(krn, thread_no);
	} else 
#endif
	{
		DCC_LOG4(LOG_TRACE, "<%d> ctx=%08x ctrl=%d pc=%08x ready.", 
				 thread_no, ctx, ctrl, ctx->pc);
		__thread_ready_set(krn, thread_no);
#if (THINKOS_ENABLE_READY_MASK)
		/* enable the thread to be scheduled ... */
		__thread_enable(krn, thread_no);
#endif
		__krn_sched_defer(krn);
	}

	__kdump(krn);

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

#if (THINKOS_ENABLE_ARG_CHECK)
	if ((ret = __krn_thread_check(krn, thread_no)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%d> invalid thread %d!", self, thread_no);
		__THINKOS_ERROR(self, ret);
		arg[SVC_RETURN] = THINKOS_EINVAL;
		return;
	}
#endif

#if (THINKOS_ENABLE_SANITY_CHECK)
	if (__thread_ctx_is_valid(krn, thread_no)) {
		DCC_LOG2(LOG_ERROR, "thread %d already exists, ctx=%08x", 
				 thread_no, __thread_ctx_get(krn, thread_no));
		__THINKOS_ERROR(self, THINKOS_ERR_THREAD_EXIST);
		arg[SVC_RETURN] = THINKOS_EINVAL;
		return;
	}
#endif

	DCC_LOG3(LOG_TRACE, "<%2d> thread=%d init=0x%08x",
			 self, thread_no, init);

	if ((ret = thinkos_krn_thread_init(krn, thread_no, init))) {
		__THINKOS_ERROR(self, ret);
		arg[SVC_RETURN] = THINKOS_EINVAL;
		return;
	};

#if (DEBUG)
  #if (LOG_LEVEL) < (LOG_INFO)
	__kdump(krn);
  #endif		
#endif

	arg[SVC_RETURN] = thread_no;

	return;
}

void __krn_thread_wait(struct thinkos_rt * krn, unsigned int th, 
					   unsigned int wq) 
{
	/* (1) suspend the thread by removing it from the
	   ready wait queue. */
	__krn_thread_suspend(krn, th);
	__krn_wq_insert(krn, wq, th);
	/* signal the scheduler ... */
	__krn_sched_defer(krn);
}

#if (THINKOS_ENABLE_TIMED_CALLS)
void __krn_thread_timedwait(struct thinkos_rt * krn, unsigned int th, 
							unsigned int wq, unsigned int ms) {
	__krn_thread_suspend(krn, th);
	__krn_tmdwq_insert(krn, wq, th, ms);
	/* signal the scheduler ... */
	__krn_sched_defer(krn);
}
#endif

void __krn_thread_clk_itv_wait(struct thinkos_rt * krn, unsigned int th, 
							  unsigned int ms) 
{
	/* Set the default return value to timeout. 
	   The wake up call will change this to 0 */
	__thread_return_set(krn, th, THINKOS_ETIMEDOUT);
	/* set the clock */
	__thread_clk_itv_set(krn, th, ms);
	/* insert into the clock wait queue */
	__thread_clk_enable(krn, th);
	/* signal the scheduler ... */
	__krn_sched_defer(krn);
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
		__krn_sched_defer(krn);
	}
}

unsigned int __krn_wq_wakeup_head(struct thinkos_rt * krn, unsigned int wq)
{
	unsigned int th;
	uint32_t queue;
	int j;

	do {
		/* insert into the event wait queue */
		queue = __ldrex(&krn->wq_lst[wq]);
		/* get all thread from the queue bitmap */
		if ((j = __thinkos_ffs(queue)) == 32) {
			/* no threads waiting o the waitibg  queue. */ 
			return 0;
		} 
		/* remove from the wait queue */
		queue &= ~(1 << j);
	} while (__strex(&krn->wq_lst[wq], queue));
	th = j + 1;

	/* insert the thread into ready queue */
	__thread_ready_set(krn, th);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__thread_clk_disable(krn, th);
#endif
	/* update status */
	__thread_stat_clr(krn, th);
	/* signal the scheduler ... */
	__krn_sched_defer(krn);

	return th;
}

#if 0
void __krn_suspend_all(struct thinkos_rt * krn) 
{
	/* remove all threads from the ready wait queue */
	__wq_ready_clr(krn);
}
#endif

bool __krn_thread_ctx_is_valid(struct thinkos_rt * krn, unsigned int th) 
{
	return __krn_obj_is_thread(krn, th) && __krn_thread_is_alloc(krn, th) && 
		__thread_ctx_is_valid(krn, th);
}

static int __krn_thread_errno_get(struct thinkos_rt * krn, unsigned int th)
{
	if (__krn_sched_brk_get(krn) == th) {
		int error = __krn_sched_err_get(krn);

		return error; 
	}

	return 0;
}

/*
 */
int thinkos_krn_threads_cyc_get(uint32_t cyc[], unsigned int from, 
								unsigned int cnt)
{
	struct thinkos_rt * krn = &thinkos_rt;

	return __krn_threads_cyc_get(krn, cyc, from, cnt);
}

int thinkos_krn_active_get(void)
{
	struct thinkos_rt * krn = &thinkos_rt;

	return __krn_sched_act_get(krn);
}

bool thinkos_krn_thread_state_get(unsigned int thread_id, 
								  struct krn_thread_state * st)
{
	struct thinkos_rt * krn = &thinkos_rt;

	if (!__krn_thread_ctx_is_valid(krn, thread_id)) {
		return false;
	}

	if (st != NULL) {
		st->thread_id = thread_id;
#if (THINKOS_ENABLE_EXCEPTIONS)
		struct thinkos_fault * fault = __thinkos_fault_rt();

    	if (__fault_thread_get(fault) == thread_id) {
			st->ctx = &fault->ctx;
			st->sp = fault->sp;
			st->ctrl = fault->control;
			st->errno = __fault_errno_get(fault);
		} else 
#endif
		{
			st->ctx = __thread_ctx_get(krn, thread_id);
			st->sp = __thread_sp_get(krn, thread_id);
			st->ctrl = __thread_ctrl_get(krn, thread_id);
			st->errno = __krn_thread_errno_get(krn, thread_id);
		}
		st->sl = __thread_sl_get(krn, thread_id);
		st->tag = __thread_tag_get(krn, thread_id);
		st->wq = __thread_wq_get(krn, thread_id);
		st->tmw = __thread_tmw_get(krn, thread_id);
		st->clk = __thread_clk_get(krn, thread_id);
		st->irq = __krn_thread_irq_get(krn, thread_id);
		st->ready = __thread_ready_get(krn, thread_id);
		st->itv = __thread_clk_itv_get(krn, thread_id);
		st->cycnt = __thread_cyccnt_get(krn, thread_id);
		st->stack_base = __thread_stack_base_get(krn, thread_id);
		st->stack_size = __thread_stack_size_get(krn, thread_id);
	}

	return true;
}

