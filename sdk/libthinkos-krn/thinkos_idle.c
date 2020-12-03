/* 
 * thinkos_core.c
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

/* -------------------------------------------------------------------------- 
 * Idle task
 * --------------------------------------------------------------------------*/

#if ((THINKOS_IDLE_STACK_ALLOC) || (THINKOS_IDLE_STACK_BSS) || \
	 (THINKOS_IDLE_STACK_CONST))
#error "Deprecated IDLE stack options!"
#endif

#if (THINKOS_ENABLE_IDLE_HOOKS)
void __attribute__((noreturn)) thinkos_idle_task(struct thinkos_rt * krn,
												 struct thinkos_idle_rt * idle)
#else
void __attribute__((noreturn)) thinkos_idle_task(struct thinkos_rt * krn)
#endif
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
		map = __ldrex((uint32_t *)&idle->req_map);
		if (map == 0) 
			continue;

		req = __thinkos_ffs(map);
		map &= ~(1 << req);
		if (__strex((uint32_t *)&idle->req_map, map))
			continue;

		switch (req) {
			case IDLE_HOOK_MONITOR_WAKEUP:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_RED_
						"IDLE_HOOK_NOTIFY_MONITOR" _ATTR_POP_ );
				/* Notify the debug/monitor */
				thinkos_monitor_wakeup(); 
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
				monitor_signal(MONITOR_SOFTRST); 
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

#if ((THINKOS_FLASH_MEM_MAX) > 0)
			case IDLE_HOOK_FLASH_MEM0:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
						"IDLE_HOOK_FLASH_MEM" _ATTR_POP_ );
				thinkos_flash_drv_tasklet(krn, 0, &thinkos_rt.flash_drv[0]);
				break;

#if ((THINKOS_FLASH_MEM_MAX) > 1)
			case IDLE_HOOK_FLASH_MEM1:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
						"IDLE_HOOK_FLASH_MEM1" _ATTR_POP_ );
				thinkos_flash_drv_tasklet(krn, 1, &thinkos_rt.flash_drv[1]);
				break;
#if ((THINKOS_FLASH_MEM_MAX) > 2)
			case IDLE_HOOK_FLASH_MEM2:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
						"IDLE_HOOK_FLASH_MEM2" _ATTR_POP_ );
				thinkos_flash_drv_tasklet(krn, 2, &thinkos_rt.flash_drv[2]);
				break;
#endif
#if ((THINKOS_FLASH_MEM_MAX) > 3)
			case IDLE_HOOK_FLASH_MEM3:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
						"IDLE_HOOK_FLASH_MEM2" _ATTR_POP_ );
				thinkos_flash_drv_tasklet(krn, 3, &thinkos_rt.flash_drv[3]);
				break;
#endif
#endif
#endif

			case 32:
#endif /* THINKOS_ENABLE_IDLE_HOOKS */

#if (THINKOS_ENABLE_CRITICAL)
				/* Force the scheduler to run if there are 
				   threads in the ready queue. */
				if (thinkos_rt.wq_ready != 0) {
					__krn_defer_sched(krn);
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

#define THINKOS_IDLE_STACK_SIZE (sizeof(thinkos_except_stack))
#define THINKOS_IDLE_STACK_BASE (uint32_t *)thinkos_except_stack

#if (THINKOS_ENABLE_THREAD_INFO)
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
struct thinkos_context * thinkos_krn_idle_reset(void)
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct thinkos_context * ctx;
	uintptr_t stack_top;
	uintptr_t stack_size;
	uintptr_t stack_base;
	uintptr_t task_entry;
	uintptr_t task_exit;
	uintptr_t task_arg[4];

	stack_base = (uintptr_t)THINKOS_IDLE_STACK_BASE;
	stack_size = THINKOS_IDLE_STACK_SIZE;
	stack_top = stack_base + stack_size;
	task_entry = (uintptr_t)thinkos_idle_task;
	task_exit = (uintptr_t)thinkos_idle_task;

#if (THINKOS_ENABLE_IDLE_HOOKS)
	/* clear all hook requests */
	krn->idle_hooks.req_map = 0;
	task_arg[0] = (uintptr_t)krn;
	task_arg[1] = (uintptr_t)&krn->idle_hooks;
#else
	task_arg[0] = (uintptr_t)krn;
	task_arg[1] = 0;
#endif

#if DEBUG
	task_arg[2] = (uint32_t)0x22222222;
	task_arg[3] = (uint32_t)0x33333333;
#else
	task_arg[2] = 0;
	task_arg[3] = 0;
#endif

	ctx = __thinkos_thread_ctx_init(stack_top, stack_size,
									task_entry, task_exit, task_arg);

	__thread_sl_set(krn, THINKOS_THREAD_IDLE, stack_base);

#if (THINKOS_ENABLE_THREAD_INFO)
	__thread_inf_set(krn, THINKOS_THREAD_IDLE, &thinkos_idle_inf);
#endif

	/* commit the context to the kernel */ 
	__thread_ctx_set(krn, THINKOS_THREAD_IDLE, ctx, 0);

#if DEBUG
	udelay(0x8000);
	DCC_LOG2(LOG_TRACE, VT_PSH VT_BRI VT_FCY
			 "<IDLE> ctx=%08x top=%08x" VT_POP, 
			 ctx, stack_top);
	DCC_LOG2(LOG_TRACE, VT_PSH VT_BRI VT_FCY
			 "<IDLE> sl=%08x sp=%08x" VT_POP, 
			 __thread_sl_get(krn, THINKOS_THREAD_IDLE),
			 __thread_sp_get(krn, THINKOS_THREAD_IDLE));
#endif

	return ctx;
}

/* initialize the idle thread */
void thinkos_krn_idle_init(void)
{
	uintptr_t stack_base;
	uint32_t free;

	stack_base = (uintptr_t)THINKOS_IDLE_STACK_BASE;
	(void)stack_base;

	free = THINKOS_IDLE_STACK_SIZE - sizeof(struct thinkos_context);
	(void)free;

#if (THINKOS_ENABLE_STACK_INIT)
	/* initialize thread stack */
	__thinkos_memset32((void *)stack_base, 0xdeadbeef, free);
#elif (THINKOS_ENABLE_MEMORY_CLEAR)
	__thinkos_memset32((void *)stack_base, 0, free);
#endif
 	thinkos_krn_idle_reset();
}


#if (THINKOS_ENABLE_IDLE_HOOKS)
void __idle_hook_req(unsigned int req) 
{
	uint32_t map;
	do {
		map = __ldrex((uint32_t *)&thinkos_rt.idle_hooks.req_map);
		map |= (1 << req);
	} while (__strex((uint32_t *)&thinkos_rt.idle_hooks.req_map, map));
}

void __idle_hook_clr(unsigned int req) 
{
	uint32_t map;
	do {
		map = __ldrex((uint32_t *)&thinkos_rt.idle_hooks.req_map);
		map &= ~(1 << req);
	} while (__strex((uint32_t *)&thinkos_rt.idle_hooks.req_map, map));
}
#endif

