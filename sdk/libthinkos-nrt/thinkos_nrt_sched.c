/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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

/** 
 * @file nrt_test.c
 * @brief application test
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#define __THINKOS_NRT__
#include <thinkos/nrt.h>
#include <arch/cortex-m3.h>
#include <sys/dcclog.h>
#include <stdlib.h>
#include <thinkos.h>

/* wait */
void thinkos_wait_svc(int32_t * arg, int self)
{
	__thinkos_suspend(self);
	/* signal the scheduler ... */
	__thinkos_defer_sched(); 
}

int thinkos_nrt_sched_task(void * arg)
{
	int event;
	DCC_LOG(LOG_TRACE, "...");

	for (;;) {
		event = thinkos_sleep(1000);
		switch (event) {
		case NRT_THREAD_CREATE:
			DCC_LOG(LOG_TRACE, "NRT_THREAD_CREATE...");
			break;
		default:
			DCC_LOG(LOG_TRACE, "...");
			break;
		};
	}

	return 0;
}

uint32_t thinkos_nrt_sched_stack[256];

#if THINKOS_ENABLE_THREAD_INFO
const struct thinkos_thread_inf thinkos_nrt_sched_inf = {
	.tag = "NRT-SCH",
	.stack_ptr = thinkos_nrt_sched_stack,
	.stack_size = sizeof(thinkos_nrt_sched_stack),
	.priority = 254,
	.thread_id = THINKOS_THREAD_NRT_SCHED + 1,
	.paused = 0
};
#endif

int thinkos_nrt_init(void)
{
	int ret;

#if THINKOS_ENABLE_THREAD_INFO
	ret = thinkos_thread_create_inf(thinkos_nrt_sched_task, 
									thinkos_nrt_sched_stack, 
									&thinkos_nrt_sched_inf);
#else
	ret = thinkos_thread_create(thinkos_nrt_sched_task, NULL, 
								thinkos_nrt_sched_stack,
								sizeof(thinkos_nrt_sched_stack) | 
								THINKOS_OPT_ID(THINKOS_THREAD_NRT_SCHED + 1));
#endif
	thinkos_sleep(1);

	DCC_LOG(LOG_TRACE, "...");

	return ret;
}

