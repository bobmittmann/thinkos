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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_IDLE__
#include <thinkos/idle.h>
#define __THINKOS_FLASH__
#include <thinkos/flash.h>
#include <thinkos.h>
#include <vt100.h>
#include <sys/dcclog.h>
#include <sys/delay.h>
/* FIXME: platform memory map should move from MONITOR and bootloader 
   to kernel... */
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>

/* -------------------------------------------------------------------------- 
 * Idle task
 * --------------------------------------------------------------------------*/

#if ((THINKOS_IDLE_STACK_ALLOC) || (THINKOS_IDLE_STACK_BSS) || \
	 (THINKOS_IDLE_STACK_CONST))
#error "Deprecated IDLE stack options!"
#endif

#if (THINKOS_ENABLE_IDLE_HOOKS)
void __attribute__((noreturn)) thinkos_idle_task(struct thinkos_idle_rt * idle)
#else
void __attribute__((noreturn, naked)) thinkos_idle_task(void)
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
			case IDLE_HOOK_NOTIFY_MONITOR:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_RED_
						"IDLE_HOOK_NOTIFY_MONITOR" _ATTR_POP_ );
				/* Notify the debug/monitor */
				monitor_signal(MONITOR_IDLE); 
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

#if (THINKOS_FLASH_MEM_MAX > 0)
			case IDLE_HOOK_FLASH_MEM0:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
						"IDLE_HOOK_FLASH_MEM" _ATTR_POP_ );
				thinkos_flash_drv_tasklet(0, &thinkos_rt.flash_drv[0]);
				break;

#if (THINKOS_FLASH_MEM_MAX > 1)
			case IDLE_HOOK_FLASH_MEM1:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
						"IDLE_HOOK_FLASH_MEM1" _ATTR_POP_ );
				thinkos_flash_drv_tasklet(1, &thinkos_rt.flash_drv[1]);
				break;
#if (THINKOS_FLASH_MEM_MAX > 2)
			case IDLE_HOOK_FLASH_MEM2:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
						"IDLE_HOOK_FLASH_MEM2" _ATTR_POP_ );
				thinkos_flash_drv_tasklet(2, &thinkos_rt.flash_drv[2]);
				break;
#endif
#if (THINKOS_FLASH_MEM_MAX > 3)
			case IDLE_HOOK_FLASH_MEM3:
				DCC_LOG(LOG_TRACE, _ATTR_PUSH_ _FG_GREEN_ 
						"IDLE_HOOK_FLASH_MEM2" _ATTR_POP_ );
				thinkos_flash_drv_tasklet(3, &thinkos_rt.flash_drv[3]);
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
	struct thinkos_context * ctx;
	uintptr_t arg;
	uintptr_t sp;
	uintptr_t sl;

	sl = (uintptr_t)THINKOS_IDLE_STACK_BASE;
	sp = sl + THINKOS_IDLE_STACK_SIZE;

#if (THINKOS_ENABLE_IDLE_HOOKS)
	/* clear all hook requests */
	thinkos_rt.idle_hooks.req_map = 0;
	arg = (uintptr_t)&thinkos_rt.idle_hooks;
#else
	arg = 0;
#endif

	ctx = __thinkos_thread_ctx_init(THINKOS_THREAD_IDLE, sp, 
									(uintptr_t)thinkos_idle_task,
									arg);
#if (THINKOS_ENABLE_THREAD_INFO)
	/* set the IDLE thread info */
	thinkos_rt.th_inf[THINKOS_THREAD_IDLE] = &thinkos_idle_inf;
#endif

#if (THINKOS_ENABLE_STACK_LIMIT)
	__thinkos_thread_sl_set(THINKOS_THREAD_IDLE, sl);
	DCC_LOG1(LOG_TRACE, " sl=%08x", thinkos_rt.th_sl[THINKOS_THREAD_IDLE]);
#endif

#if (THINKOS_ENABLE_THREAD_INFO)
	__thinkos_thread_inf_set(THINKOS_THREAD_IDLE, &thinkos_idle_inf);
#endif

#if (THINKOS_ENABLE_SCHED_DEBUG)
	/* commit the context to the kernel */ 
	__thinkos_thread_ctx_set(THINKOS_THREAD_IDLE, ctx, 0);
#endif

#if DEBUG
	ctx->r1 = (uint32_t)0x11111111;
	ctx->r2 = (uint32_t)0x22222222;
	ctx->r3 = (uint32_t)0x33333333;

	udelay(0x8000);
	DCC_LOG2(LOG_TRACE, _ATTR_PUSH_ _FG_CYAN_
			 "IDLE ctx=%08x except=%08x" _ATTR_POP_, ctx,
			 __thinkos_xcpt_stack_top());
#endif

	return ctx;
}

/* initialize the idle thread */
void thinkos_krn_idle_init(void)
{
	DCC_LOG1(LOG_TRACE, _ATTR_PUSH_ _FG_CYAN_
			"IDLE stack=%08x" _ATTR_POP_, THINKOS_IDLE_STACK_BASE);

#if (THINKOS_ENABLE_STACK_INIT)
	/* initialize idle stack */
	__thinkos_memset32(THINKOS_IDLE_STACK_BASE, 0xdeadbeef, 
					   THINKOS_IDLE_STACK_SIZE);
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

