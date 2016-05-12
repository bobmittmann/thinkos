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

#define __THINKOS_SYS__
#include <thinkos_sys.h>
#define __THINKOS_DMON__
#include <thinkos_dmon.h>
#include <thinkos.h>

/* -------------------------------------------------------------------------- 
 * Idle task
 * --------------------------------------------------------------------------*/

#if (THINKOS_IDLE_STACK_ALLOC + THINKOS_IDLE_STACK_CONST + \
	 THINKOS_IDLE_STACK_BSS) > 1
#error "Invalid multiple IDLE stack options!"
#endif

#if (THINKOS_IDLE_STACK_ALLOC + THINKOS_IDLE_STACK_CONST + \
	 THINKOS_IDLE_STACK_BSS) == 0
#error "Invalid IDLE stack option!"
#endif

void thinkos_idle_svc(int32_t * arg)
{
#if (THINKOS_ENABLE_MONITOR)
	thinkos_dbgmon_idle_signal();
#endif

#if THINKOS_ENABLE_CRITICAL
	/* Force the scheduler to run if there are 
	   threads in the ready queue. */
 #if ((THINKOS_THREADS_MAX) < 32) 
	if (thinkos_rt.wq_ready != (1 << (THINKOS_THREADS_MAX)))
 #else
	if (thinkos_rt.wq_ready != 0) 
 #endif
	{
		__thinkos_defer_sched();
	}
#endif
}

void __attribute__((noreturn, naked)) thinkos_idle_task(void)
{
	asm volatile ("nop\n"); 

//	DCC_LOG(LOG_TRACE, "ThinkOS Idle started..."); 

	for (;;) {
#if THINKOS_ENABLE_IDLE_WFI
		asm volatile ("wfi\n"); /* wait for interrupt */
#endif
#if THINKOS_ENABLE_MONITOR || THINKOS_ENABLE_CRITICAL
		thinkos_on_idle();
#endif
	}
}

#if THINKOS_IDLE_STACK_CONST
/* Constant IDLE stack:

   Define the IDLE context (stack) in Flash or read only memory. This
   is helpful in two aspects:
   1 - it reduces the memory footprint.
   2 - it won't be unintetionally modified by a misbehaved application. */
const struct thinkos_context __attribute__((aligned(8))) thinkos_idle_ctx = {
	.pc = (uint32_t)thinkos_idle_task,
	.xpsr = CM_EPSR_T /* set the thumb bit */
};
#define THINKOS_IDLE_STACK_BASE (uint32_t *)&thinkos_idle_ctx
#endif

#if THINKOS_IDLE_STACK_BSS
/* IDLE stack on .bss section */
struct thinkos_context __attribute__((aligned(8))) thinkos_idle_ctx;
#define THINKOS_IDLE_STACK_BASE (uint32_t *)&thinkos_idle_ctx
#endif

#if THINKOS_IDLE_STACK_ALLOC
extern uint32_t _stack[];
/* IDLE stack pre allocated on main stack */
#define THINKOS_IDLE_STACK_BASE (uint32_t *)&_stack[0]
#endif

#define THINKOS_IDLE_STACK_SIZE sizeof(struct thinkos_context)

#if THINKOS_ENABLE_THREAD_INFO
const struct thinkos_thread_inf thinkos_idle_inf = {
	.tag = "IDLE",
	.stack_ptr = THINKOS_IDLE_STACK_BASE,
	.stack_size = THINKOS_IDLE_STACK_SIZE,
	.priority = 255,
	.thread_id = THINKOS_THREAD_IDLE,
	.paused = 0
};
#else
uint32_t * const thinkos_idle_stack_ptr = THINKOS_IDLE_STACK_BASE;
#endif

/* initialize the idle thread */
void __thinkos_idle_init(void)
{
	struct thinkos_context * idle_ctx;

	idle_ctx = (struct thinkos_context *)THINKOS_IDLE_STACK_BASE;

#if THINKOS_IDLE_STACK_BSS
	DCC_LOG1(LOG_TRACE, "BSS idle stack @ 0x%08x", THINKOS_IDLE_STACK_BASE);
#endif

#if THINKOS_IDLE_STACK_ALLOC
	DCC_LOG1(LOG_TRACE, "Alloc idle stack @ 0x%08x", THINKOS_IDLE_STACK_BASE);
#endif

#if THINKOS_IDLE_STACK_CONST
	DCC_LOG1(LOG_TRACE, "Const idle stack @ 0x%08x", THINKOS_IDLE_STACK_BASE);
#endif

#if !THINKOS_IDLE_STACK_CONST
	idle_ctx->pc = (uint32_t)thinkos_idle_task;
	idle_ctx->lr = (uint32_t)__thinkos_thread_exit;
	idle_ctx->xpsr = CM_EPSR_T; /* set the thumb bit */
#endif /* THINKOS_IDLE_STACK_CONST */

	thinkos_rt.ctx[THINKOS_THREAD_IDLE] = idle_ctx;

#if (THINKOS_THREADS_MAX < 32) 
	/* put the IDLE thread in the ready queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, THINKOS_THREADS_MAX, 1);
#endif

#if THINKOS_ENABLE_THREAD_INFO
	/* set the IDLE thread info */
	thinkos_rt.th_inf[THINKOS_THREAD_IDLE] = &thinkos_idle_inf; 
#endif

}

