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
#include <thinkos.h>

/* -------------------------------------------------------------------------- 
 * Idle task
 * --------------------------------------------------------------------------*/

#if (THINKOS_IDLE_STACK_ALLOC + THINKOS_IDLE_STACK_BSS) > 1
#error "Invalid multiple IDLE stack options!"
#endif

#if (THINKOS_IDLE_STACK_ALLOC + THINKOS_IDLE_STACK_BSS) == 0
#error "Invalid IDLE stack option!"
#endif

void thinkos_idle_svc(int32_t * arg)
{
#if THINKOS_ENABLE_MONITOR
	dbgmon_signal_idle();
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

	DCC_LOG(LOG_TRACE, "ThinkOS Idle reset."); 

	for (;;) {
#if THINKOS_ENABLE_IDLE_WFI
		asm volatile ("wfi\n"); /* wait for interrupt */
#endif
#if THINKOS_ENABLE_MONITOR || THINKOS_ENABLE_CRITICAL
		thinkos_on_idle();
#endif
	}
}

#if THINKOS_IDLE_STACK_BSS
/* IDLE stack on .bss section */
struct thinkos_context __attribute__((aligned(8))) thinkos_idle_ctx;
#define THINKOS_IDLE_STACK_BASE (uint32_t *)&thinkos_idle_ctx
#endif

extern int _stack;

#if THINKOS_IDLE_STACK_ALLOC
  /* IDLE stack pre allocated on main stack */
  #define THINKOS_IDLE_STACK_BASE ((uint32_t *)&_stack - \
                                   sizeof(struct thinkos_context))
  #define THINKOS_MAIN_STACK_TOP THINKOS_IDLE_STACK_BASE
#else
  #define THINKOS_MAIN_STACK_TOP ((uint32_t *)&_stack)
#endif

#define THINKOS_IDLE_STACK_SIZE sizeof(struct thinkos_context)

#ifndef THINKOS_MAIN_STACK_SIZE
  #define THINKOS_MAIN_STACK_SIZE 4096
#endif

uint32_t * const thinkos_main_stack = THINKOS_MAIN_STACK_TOP;

#if THINKOS_ENABLE_THREAD_INFO
const struct thinkos_thread_inf thinkos_idle_inf = {
	.tag = "IDLE",
	.stack_ptr = THINKOS_IDLE_STACK_BASE,
	.stack_size = THINKOS_IDLE_STACK_SIZE,
	.priority = 255,
	.thread_id = THINKOS_THREAD_IDLE,
	.paused = 0
};

/* FIXME: move this definition elsewere, or allow it 
   to be configured by the user ... */
const struct thinkos_thread_inf thinkos_main_inf = {
	.tag = "MAIN",
	.stack_ptr = THINKOS_MAIN_STACK_TOP - THINKOS_MAIN_STACK_SIZE / 4,
	.stack_size = THINKOS_MAIN_STACK_SIZE,
	.priority = 0,
	.thread_id = 0,
	.paused = 0
};
#endif

/* initialize the idle thread */
struct thinkos_context * __thinkos_idle_init(void)
{
	struct thinkos_context * idle_ctx;

	idle_ctx = (struct thinkos_context *)THINKOS_IDLE_STACK_BASE;

#if THINKOS_IDLE_STACK_BSS
	DCC_LOG1(LOG_MSG, "BSS idle stack @ 0x%08x", THINKOS_IDLE_STACK_BASE);
#endif

#if THINKOS_IDLE_STACK_ALLOC
	DCC_LOG1(LOG_TRACE, "alloc idle stack @ 0x%08x", THINKOS_IDLE_STACK_BASE);
#endif

	idle_ctx->pc = (uint32_t)thinkos_idle_task & ~1;
	idle_ctx->lr = (uint32_t)__thinkos_thread_exit;
	idle_ctx->xpsr = CM_EPSR_T; /* set the thumb bit */

	thinkos_rt.ctx[THINKOS_THREAD_IDLE] = idle_ctx;

#if (THINKOS_THREADS_MAX < 32) 
	/* put the IDLE thread in the ready queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, THINKOS_THREADS_MAX, 1);
#endif

#if THINKOS_ENABLE_THREAD_INFO
	/* set the IDLE thread info */
	thinkos_rt.th_inf[THINKOS_THREAD_IDLE] = &thinkos_idle_inf; 
#endif

	return idle_ctx;
}

