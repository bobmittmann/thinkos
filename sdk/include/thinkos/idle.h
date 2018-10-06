/* 
 * thikos/idle.h
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

/* ----------------------------------------------------------------------------
 *  Idle hooks signals 
 * ----------------------------------------------------------------------------
 */

enum idle_hook_signal {
	/* The higest priority gos to the Debug/Monitor */
	IDLE_HOOK_NOTIFY_DBGMON = 0,
	IDLE_HOOK_STACK_SCAN = 31
};

#if THINKOS_ENABLE_THREAD_INFO
extern const struct thinkos_thread_inf thinkos_idle_inf;
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline void __idle_hook_req(unsigned int req) {
		uint32_t map;
		do {
			map = __ldrex((uint32_t *)&thinkos_rt.idle_hooks.req_map);
			map |= (1 << req);
		} while (__strex((uint32_t *)&thinkos_rt.idle_hooks.req_map, map));
	}

static inline void __idle_hook_clr(unsigned int req) {
		uint32_t map;
		do {
			map = __ldrex((uint32_t *)&thinkos_rt.idle_hooks.req_map);
			map &= ~(1 << req);
		} while (__strex((uint32_t *)&thinkos_rt.idle_hooks.req_map, map));
	}

struct thinkos_context * __thinkos_idle_init(void);
struct thinkos_context * __thinkos_idle_reset(void);

void __attribute__((noreturn)) thinkos_idle_task(void);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_IDLE_H__ */

