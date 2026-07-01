/* 
 * thinkos_init.c
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

#if (THINKOS_ENABLE_THREAD_INFO)
const struct thinkos_thread_inf thinkos_main_inf = {
	.tag = "MAIN",
	.stack_ptr = &__krn_stack_start,
	.stack_size = (uintptr_t)&__krn_stack_size,
	.priority = 0,
	.thread_id = 1,
	.privileged = 0,
	.paused = 0
};
#endif

int __thinkos_init_main(struct thinkos_krn * krn, uintptr_t sp, uint32_t opt)
{
#if (THINKOS_ENABLE_TIMESHARE)
	int priority = __PRIORITY(opt);
#endif
	int th = __ID(opt);

#if (THINKOS_ENABLE_STACK_LIMIT)
	uintptr_t sl = (uintptr_t)&__krn_stack_start;
#endif

	if (th < 1)
		th = 1;

#if (THINKOS_ENABLE_THREAD_ALLOC)
	/* alloc main thread */
	th = __thinkos_thread_alloc(th);
#else
	if (th > THINKOS_THREADS_MAX)
		th = THINKOS_THREADS_MAX;
#endif

#if (THINKOS_ENABLE_TIMESHARE)

#if (THINKOS_SCHED_LIMIT_MIN) < 1
#error "THINKOS_SCHED_LIMIT_MIN must be at least 1"
#endif

#if (THINKOS_SCHED_LIMIT_MAX) < (THINKOS_SCHED_LIMIT_MIN)
#error "THINKOS_SCHED_LIMIT_MAX < THINKOS_SCHED_LIMIT_MIN !!!"
#endif
	if (priority > THINKOS_SCHED_LIMIT_MAX)
		priority = THINKOS_SCHED_LIMIT_MAX;

	krn->sched_pri[th] = priority;
	krn->sched_val[th] = priority / 2;

	/* set the initial schedule limit */
	krn->sched_limit = priority;
	if (krn->sched_limit < (THINKOS_SCHED_LIMIT_MIN))
		krn->sched_limit = (THINKOS_SCHED_LIMIT_MIN);
#endif /* THINKOS_ENABLE_TIMESHARE */

	DCC_LOG3(LOG_TRACE, "<%2d> threads_max=%d ready=%08x", 
			 th, THINKOS_THREADS_MAX, krn->wq_ready);

#if (THINKOS_ENABLE_STACK_LIMIT)
	__thread_sl_set(krn, th, (uintptr_t)sl);
	DCC_LOG1(LOG_TRACE, " sl=%08x", __thread_sl_get(krn, th));
#endif

#if (THINKOS_ENABLE_THREAD_INFO)
	__thread_inf_set(krn, th, (struct thinkos_thread_inf *)
					 &thinkos_main_inf);
#endif

	return th;
}

extern void * __krn_stack_start;
extern void * __krn_stack_end;
extern int __krn_stack_size;

int thinkos_main_thread_create(int (* entry)(void *, unsigned int), void * arg,
						  void (* on_exit)(unsigned int), bool privileged)
{
#if (THINKOS_ENABLE_THREAD_INFO)
	const struct thinkos_thread_inf * inf = &thinkos_main_inf;
#endif
	struct thinkos_krn * krn = &thinkos_krn;
	struct thinkos_thread_initializer init;
	unsigned int thread;
	uintptr_t stack_base;
	uint32_t stack_size;
	int ret;

#if (THINKOS_ENABLE_THREAD_INFO)
	thread = (inf->thread_id > 0) ? inf->thread_id : 1;
	stack_base = (uintptr_t)inf->stack_ptr;
	stack_size = inf->stack_size;
#else
	thread = 1;
	stack_base = (uintptr_t)&__krn_stack_start;
	stack_size = (uint32_t)&__krn_stack_size;
#endif

#if (THINKOS_THREAD_STACK_MAX)
	if (stack_size > (THINKOS_THREAD_STACK_MAX))
		stack_size = THINKOS_THREAD_STACK_MAX;
#endif

	/* force allocate the thread block */
	__thread_alloc_set(krn, thread);

	if (stack_base & (STACK_ALIGN_MSK)) {
		DCC_LOG1(LOG_PANIC, "stack_top=%08x unaligned", stack_base); 
		return -1;
	}

	if (stack_size & (STACK_ALIGN_MSK)) {
		DCC_LOG1(LOG_TRACE, "stack_Size=%08x unaligned", stack_size); 
		stack_size &= ~(STACK_ALIGN_MSK);
	}

	init.stack_base = stack_base;
	init.stack_size = stack_size;
	init.task_entry = (uintptr_t)entry;
	init.task_exit = (uintptr_t)on_exit;
	init.task_arg[0] = (uintptr_t)arg;
	init.task_arg[1] = thread;
	init.task_arg[2] = 0;
	init.task_arg[3] = 0;
	init.priority = 0;
	init.paused = false;
	init.privileged = privileged;
#if (THINKOS_ENABLE_THREAD_INFO)
	init.inf = inf;
#endif

	/* Make sure there are no pending errors or exceptions */
	__krn_sched_err_clr(krn);
	__krn_sched_xcp_clr(krn);

	if ((ret = thinkos_krn_thread_init(krn, thread, &init))) {
		return -ret;
	};

	DCC_LOG1(LOG_TRACE, "thread=%d", thread);

	return thread;
}

