/* 
 * thikos_core.c
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
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_IDLE__
#include <thinkos/idle.h>
#include <thinkos.h>

/* -------------------------------------------------------------------------- 
 * Idle task
 * --------------------------------------------------------------------------*/

#if ((THINKOS_IDLE_STACK_ALLOC) || (THINKOS_IDLE_STACK_BSS) || \
	 (THINKOS_IDLE_STACK_CONST))
#error "Deprecated IDLE stack options!"
#endif

void __attribute__((noreturn, naked)) thinkos_idle_task(void)
{
#if THINKOS_ENABLE_IDLE_HOOKS
	uint32_t map;
	int req;
#endif

	DCC_LOG(LOG_TRACE, "ThinkOS Idle reset."); 

	for (;;) {
#if THINKOS_ENABLE_IDLE_WFI
		asm volatile ("wfi\n"); /* wait for interrupt */
#endif

#if THINKOS_ENABLE_CRITICAL
		/* Force the scheduler to run if there are 
		   threads in the ready queue. */
		if (thinkos_rt.wq_ready != 0) {
			__thinkos_defer_sched();
			continue;
		}
#endif

#if THINKOS_ENABLE_IDLE_HOOKS
		do {
			map = __ldrex((uint32_t *)&thinkos_rt.idle_hooks.req_map);
			req = __clz(__rbit(map));
			map &= ~(1 << req);
		} while (__strex((uint32_t *)&thinkos_rt.idle_hooks.req_map, map));


		switch (req) {
			case IDLE_HOOK_NOTIFY_DBGMON:
				DCC_LOG1(LOG_TRACE, "req=%d", req); 
				dbgmon_signal(DBGMON_IDLE); 
				break;
		}
#endif
	}
}

/* IDLE stack on .bss section */
#define THINKOS_IDLE_STACK_SIZE (sizeof(struct thinkos_context) + 64)

uint32_t thinkos_idle_stack[THINKOS_IDLE_STACK_SIZE / 4]  
__attribute__((aligned(8)));

#define THINKOS_IDLE_STACK_BASE (uint32_t *)thinkos_idle_stack

#if THINKOS_ENABLE_THREAD_INFO
const struct thinkos_thread_inf thinkos_idle_inf = {
	.tag = "IDLE",
	.stack_ptr = THINKOS_IDLE_STACK_BASE,
	.stack_size = THINKOS_IDLE_STACK_SIZE,
	.priority = 255,
	.thread_id = (THINKOS_THREAD_IDLE + 1),
	.paused = 0
};
#endif

/* initialize the idle thread */
struct thinkos_context * __thinkos_idle_init(void)
{
	struct thinkos_context * idle_ctx;
	uint32_t sp;

	sp = (uint32_t)thinkos_idle_stack;
	sp += THINKOS_IDLE_STACK_SIZE - sizeof(struct thinkos_context);
	sp &= 0xfffffff8; /* 64bits alignemnt */

	DCC_LOG1(LOG_TRACE, "Idle context=%08x.", sp); 

	idle_ctx = (struct thinkos_context *)sp;
	idle_ctx->pc = (uint32_t)thinkos_idle_task & ~1;
	idle_ctx->lr = (uint32_t)thinkos_idle_task; 
	idle_ctx->xpsr = CM_EPSR_T; /* set the thumb bit */

	thinkos_rt.ctx[THINKOS_THREAD_IDLE] = idle_ctx;

#if THINKOS_ENABLE_THREAD_INFO
	/* set the IDLE thread info */
	thinkos_rt.th_inf[THINKOS_THREAD_IDLE] = &thinkos_idle_inf; 
#endif

	return idle_ctx;
}

