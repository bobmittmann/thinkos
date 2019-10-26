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
#include <vt100.h>
#include <sys/dcclog.h>

/* -------------------------------------------------------------------------- 
 * Idle task
 * --------------------------------------------------------------------------*/

#if ((THINKOS_IDLE_STACK_ALLOC) || (THINKOS_IDLE_STACK_BSS) || \
	 (THINKOS_IDLE_STACK_CONST))
#error "Deprecated IDLE stack options!"
#endif

#if (THINKOS_ENABLE_IDLE_HOOKS)
struct thinkos_idle_rt thinkos_idle_rt;
#endif

void __attribute__((noreturn, naked)) thinkos_idle_task(void * arg)
{
#if (THINKOS_ENABLE_IDLE_HOOKS)
	uint32_t map;
	int req;
#endif

	for (;;) {
		/* Chek for threads in the ready queue. */
#if (THINKOS_ENABLE_IDLE_WFI)
		asm volatile ("wfi\n"); /* wait for interrupt */
#endif

#if (THINKOS_ENABLE_IDLE_HOOKS)
		do {
			map = __ldrex((uint32_t *)&thinkos_idle_rt.req_map);
			req = __clz(__rbit(map));
			if (map != 0) {
				uint32_t y;

				y = map & ~(1 << req);

				DCC_LOG3(LOG_TRACE, _ATTR_PUSH_ _FG_CYAN_ 
						 "map=%08x y=%08x req=%d" _ATTR_POP_ , 
						 map, y, req);
				map = y;
			}
		} while (__strex((uint32_t *)&thinkos_idle_rt.req_map, map));

		switch (req) {
			case IDLE_HOOK_NOTIFY_DBGMON:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_RED_
						"IDLE_HOOK_NOTIFY_DBGMON" _ATTR_POP_ );
				/* Notify the debug/monitor */
				dbgmon_signal(DBGMON_IDLE); 
				break;

#if 0 
			/* FIXME: IDLE hooks or not, see KERNEL_ERROR */
			case IDLE_HOOK_SYSRST:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_YELLOW_ 
						"IDLE_HOOK_SYSRST" _ATTR_POP_ );
				__thinkos_system_reset();
				break;
			case IDLE_HOOK_SOFTRST:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
						"IDLE_HOOK_SOFTRST" _ATTR_POP_ );
				dbgmon_signal(DBGMON_SOFTRST); 
				break;

#if THINKOS_ENABLE_EXCEPTIONS
			case IDLE_HOOK_EXCEPT_DONE: {
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
						"IDLE_HOOK_EXCEPT_DONE" _ATTR_POP_ );
				struct thinkos_except * xcpt = __thinkos_except_buf();
				thinkos_exception_dsr(xcpt);
				}
				break;
#endif
#endif

			case 32:
#endif /* THINKOS_ENABLE_IDLE_HOOKS */

#if (THINKOS_ENABLE_CRITICAL)
				/* Force the scheduler to run if there are 
				   threads in the ready queue. */
				if (thinkos_rt.wq_ready != 0) {
					__thinkos_defer_sched();
				}
#endif

#if (THINKOS_ENABLE_TRACE)
				/* Try to send one trace entry to a remote host... */
				__thinkos_trace_try_send();
#endif

#if (THINKOS_ENABLE_IDLE_HOOKS)
				break;
			default:
				DCC_LOG1(LOG_TRACE, "%d", req); 
		}
#endif
	}
}

#if (THINKOS_ENABLE_IDLE_MSP) 

void __thinkos_idle_bootstrap(void * arg)
{
	DCC_LOG(LOG_TRACE, "ThinkOS Idle bootstrap.... main()"); 
}

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

struct thinkos_context * __thinkos_idle_ctx(void)
{
	struct thinkos_context * ctx;
	uintptr_t sp;

	sp = (uintptr_t)THINKOS_IDLE_STACK_BASE;
	sp += THINKOS_IDLE_STACK_SIZE - sizeof(struct thinkos_context);
	ctx = (struct thinkos_context *)sp;

	return ctx;
}

/* resets the idle thread and context */
uint32_t __thinkos_idle_reset(void (* task_ptr)(void *), void * arg)
{
	struct thinkos_context * ctx;

	ctx  = __thinkos_idle_ctx();

	ctx->r0 = (uint32_t)arg;
#if DEBUG
	ctx->r1 = (uint32_t)0x11111111;
	ctx->r2 = (uint32_t)0x22222222;
	ctx->r3 = (uint32_t)0x33333333;
	ctx->r4 = (uint32_t)0x44444444;
	ctx->r5 = (uint32_t)0x55555555;
	ctx->r6 = (uint32_t)0x66666666;
	ctx->r7 = (uint32_t)0x77777777;
	ctx->r8 = (uint32_t)0x88888888;
	ctx->r9 = (uint32_t)0x99999999;
	ctx->r10 = (uint32_t)0xaaaaaaaa;
	ctx->r11 = (uint32_t)0xbbbbbbbb;
	ctx->r12 = (uint32_t)0xcccccccc;
#endif
	ctx->pc = (uint32_t)task_ptr & ~1;
	ctx->lr = (uint32_t)thinkos_idle_task; 
	ctx->xpsr = CM_EPSR_T; /* set the thumb bit */

#if (THINKOS_ENABLE_FPU) || (THINKOS_ENABLE_IDLE_MSP) 
	ctx->sp = (uintptr_t)&ctx->r0;
  #if (THINKOS_ENABLE_IDLE_MSP) 
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

	return (uintptr_t)ctx;
}

/* initialize the idle thread */
void __thinkos_idle_init(void)
{
#if THINKOS_ENABLE_STACK_INIT
	/* initialize idle stack */
	__thinkos_memset32(THINKOS_IDLE_STACK_BASE, 0xdeadbeef, 
					   THINKOS_IDLE_STACK_SIZE);
#endif

 	__thinkos_idle_reset(thinkos_idle_task, NULL);
}

