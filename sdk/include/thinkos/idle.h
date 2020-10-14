/* 
 * thinkos/idle.h
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

#ifndef __THINKOS_IDLE_H__
#define __THINKOS_IDLE_H__

#ifndef __THINKOS_IDLE__
#error "Never use <thinkos/idle.h> directly; include <thinkos.h> instead."
#endif 

#ifndef __THINKOS_KERNEL_H__
#error "Need <thinkos/kernel.h>"
#endif 

/* ----------------------------------------------------------------------------
 *  Idle hooks signals 
 * ----------------------------------------------------------------------------
 */

enum idle_hook_signal {
	/* The higest priority goes to system reset request */
	IDLE_HOOK_SYSRST = 0,
	/* Debug/Monitor IDLE signal is next in the list */
	IDLE_HOOK_NOTIFY_MONITOR = 1,
	/* */
	IDLE_HOOK_SOFTRST = 2,
	IDLE_HOOK_EXCEPT_DONE = 3,
	/* The higest priority goes to the Debug/Monitor */
	IDLE_HOOK_FLASH_MEM = 30,
	IDLE_HOOK_STACK_SCAN = 31
};

#if (THINKOS_ENABLE_IDLE_HOOKS)
struct thinkos_idle_rt {
	volatile uint32_t req_map;
	volatile uint32_t ack_map;
};

extern struct thinkos_idle_rt thinkos_idle_rt;
#endif

#if THINKOS_ENABLE_THREAD_INFO
extern const struct thinkos_thread_inf thinkos_idle_inf;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if (THINKOS_ENABLE_IDLE_HOOKS)
static inline void __idle_hook_req(unsigned int req) {
		uint32_t map;
		do {
			map = __ldrex((uint32_t *)&thinkos_idle_rt.req_map);
			map |= (1 << req);
		} while (__strex((uint32_t *)&thinkos_idle_rt.req_map, map));
	}

static inline void __idle_hook_clr(unsigned int req) {
		uint32_t map;
		do {
			map = __ldrex((uint32_t *)&thinkos_idle_rt.req_map);
			map &= ~(1 << req);
		} while (__strex((uint32_t *)&thinkos_idle_rt.req_map, map));
	}
#endif

void __thinkos_idle_init(void);
void __thinkos_idle_reset(void);

struct thinkos_context * __thinkos_idle_ctx(void);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_IDLE_H__ */

