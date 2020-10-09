/* 
 * File:	 /thinkos/dbgmon.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __THINKOS_DBGMON_H__
#define __THINKOS_DBGMON_H__

#ifndef __THINKOS_DBGMON__
#error "Never use <thinkos/dbgmon.h> directly; include <thinkos.h> instead."
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#define __THINKOS_EXCEPT__
#include <thinkos/except.h>

#define __THINKOS_IRQ__
#include <thinkos/irq.h>

#include <thinkos.h>

#include <sys/memory.h>

/* Mark for debug/monitor breakpoint numbers. */
#define __BKPT(__NO) asm volatile ("bkpt %0\n" : : "I" __NO)

struct dbgmon_thread_inf {
	int8_t thread_id;
	uint8_t errno;
	uint32_t pc;
	uint32_t sp;
	struct thinkos_context * ctx;
};

#ifdef __cplusplus
extern "C" {
#endif


/* ----------------------------------------------------------------------------
 *  Debug/Monitor watchpoint/breakpoint API
 * ----------------------------------------------------------------------------
 */
bool dbgmon_breakpoint_set(uint32_t addr, uint32_t size);

bool dbgmon_breakpoint_clear(uint32_t addr, uint32_t size);

void dbgmon_breakpoint_clear_all(void);

bool dbgmon_watchpoint_set(uint32_t addr, uint32_t size, int access);

bool dbgmon_watchpoint_clear(uint32_t addr, uint32_t size);

void dbgmon_watchpoint_clear_all(void);

bool dbgmon_breakpoint_disable(uint32_t addr);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_DBGMON_H__ */

