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
#if (THINKOS_ENABLE_IDLE_HOOKS)
	uint32_t map;
	int req;
#endif
	uint32_t clk = 0;

	DCC_LOG(LOG_TRACE, "ThinkOS Idle reset."); 

	for (;;) {

		clk++;

#if (THINKOS_ENABLE_IDLE_HOOKS)
		do {
			map = __ldrex((uint32_t *)&thinkos_rt.idle_hooks.req_map);
			req = __clz(__rbit(map));
			map &= ~(1 << req);
		} while (__strex((uint32_t *)&thinkos_rt.idle_hooks.req_map, map));

		if ((clk & 0x00ffffff) == 0) {
			DCC_LOG3(LOG_TRACE, "clk=%8d ctx=%08x sp=%08x", clk, 
					 thinkos_rt.ctx[THINKOS_THREAD_IDLE], cm3_sp_get()); 
			__thinkos_defer_sched();
		}
		if ((clk & 0x01ffffff) == 0x01000000) {
//			asm volatile ("wfi\n"); /* wait for interrupt */
		}

		switch (req) {
			case IDLE_HOOK_NOTIFY_DBGMON:
				/* Notify the debug/monitor */
				dbgmon_signal(DBGMON_IDLE); 
				break;
			case IDLE_HOOK_SYSRST:
				__thinkos_system_reset();
				break;

			case 32:
#endif

#if (THINKOS_ENABLE_CRITICAL)
				/* Force the scheduler to run if there are 
				   threads in the ready queue. */
				if (thinkos_rt.wq_ready != 0)
					__thinkos_defer_sched();
#endif

#if (THINKOS_ENABLE_IDLE_WFI)
//				asm volatile ("wfi\n"); /* wait for interrupt */
#endif

#if (THINKOS_ENABLE_IDLE_HOOKS)
				break;
			default:
				DCC_LOG1(LOG_TRACE, "%d", req); 
		}
#endif

	}
}

void __thinkos_idle_bootstrap(void * arg)
{
	DCC_LOG(LOG_TRACE, "ThinkOS Idle bootstrap.... main()"); 
}

#if (THIKNOS_ENABLE_IDLE_MSP) 

#define THINKOS_IDLE_STACK_SIZE (sizeof(thinkos_except_stack))
#define THINKOS_IDLE_STACK_BASE (uint32_t *)thinkos_except_stack

#else

#ifndef THINKOS_IDLE_STACK_SIZE 
#define THINKOS_IDLE_STACK_SIZE (sizeof(struct thinkos_context) + 128)
#endif

uint32_t thinkos_idle_stack[THINKOS_IDLE_STACK_SIZE / 4]  
	__attribute__((aligned(8)));
#define THINKOS_IDLE_STACK_BASE (uint32_t *)thinkos_idle_stack

#endif


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

uint32_t *  __thinkos_xcpt_stack_top(void)
{
	uintptr_t sp;

	sp = (uintptr_t)THINKOS_IDLE_STACK_BASE;
	sp += THINKOS_IDLE_STACK_SIZE - sizeof(struct thinkos_context);

	return (uint32_t *)sp;
}

/* resets the idle thread and context */
uint32_t __thinkos_idle_reset(void (* task_ptr)(void *), void * arg)
{
	struct thinkos_context * ctx;
	uintptr_t sp;

	sp = (uintptr_t)THINKOS_IDLE_STACK_BASE;
	sp += THINKOS_IDLE_STACK_SIZE - sizeof(struct thinkos_context);

	DCC_LOG1(LOG_TRACE, "Idle context=%08x.", sp); 

	ctx = (struct thinkos_context *)sp;
	__thinkos_memset32(ctx, 0, sizeof(struct thinkos_context));

	ctx->r0 = (uint32_t)arg;
	ctx->r1 = (uint32_t)0x11111111;
	ctx->r2 = (uint32_t)0x22222222;
	ctx->r3 = (uint32_t)0x33333333;
	ctx->r4 = (uint32_t)0x44444444;
	ctx->r5 = (uint32_t)0x55555555;
	ctx->r6 = (uint32_t)0x66666666;
	ctx->r7 = (uint32_t)0x77777777;
	ctx->r8 = (uint32_t)0x88888888;
	ctx->pc = (uint32_t)task_ptr & ~1;
	ctx->lr = (uint32_t)thinkos_idle_task; 
	ctx->xpsr = CM_EPSR_T; /* set the thumb bit */

#if (THINKOS_ENABLE_FPU) || (THIKNOS_ENABLE_IDLE_MSP) 
	ctx->sp = (uintptr_t)&ctx->r0;
  #if (THIKNOS_ENABLE_IDLE_MSP) 
	ctx->ret = CM3_EXC_RET_THREAD_MSP;
  #else
	ctx->ret = CM3_EXC_RET_THREAD_PSP;
  #endif
#endif

#if THINKOS_ENABLE_THREAD_INFO
	/* set the IDLE thread info */
	thinkos_rt.th_inf[THINKOS_THREAD_IDLE] = &thinkos_idle_inf; 
#endif

	thinkos_rt.ctx[THINKOS_THREAD_IDLE] = ctx;

	return sp;
}

/* initialize the idle thread */
void __thinkos_idle_init(void)
{

#if THINKOS_ENABLE_STACK_INIT
	/* initialize idle stack */
	__thinkos_memset32(THINKOS_IDLE_STACK_BASE, 0xdeadbeef, 
					   THINKOS_IDLE_STACK_SIZE);
#endif

 	__thinkos_idle_reset(__thinkos_idle_bootstrap, NULL);
}

