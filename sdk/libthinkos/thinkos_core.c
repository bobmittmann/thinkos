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
#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>
#include <thinkos.h>

#if (THINKOS_ASM_SCHEDULER)
#endif

void thinkos_idle_task(void);

/* -------------------------------------------------------------------------- 
 * Run Time ThinkOS block
 * --------------------------------------------------------------------------*/

struct thinkos_rt thinkos_rt;

uint32_t __attribute__((aligned(8))) thinkos_except_stack[THINKOS_EXCEPT_STACK_SIZE / 4];

const uint16_t thinkos_except_stack_size = sizeof(thinkos_except_stack);

#if THINKOS_ENABLE_SCHED_DEBUG
static inline void __attribute__((always_inline)) 
__dump_context(struct thinkos_context * __ctx) {
	DCC_LOG4(LOG_TRACE, "  r0=%08x  r1=%08x  r2=%08x  r3=%08x", 
			__ctx->r0, __ctx->r1, __ctx->r2, __ctx->r3);
	DCC_LOG4(LOG_TRACE, "  r4=%08x  r5=%08x  r6=%08x  r7=%08x", 
			__ctx->r4, __ctx->r7, __ctx->r6, __ctx->r7);
	DCC_LOG4(LOG_TRACE, "  r8=%08x  r9=%08x r10=%08x r11=%08x", 
			__ctx->r8, __ctx->r9, __ctx->r10, __ctx->r11);
	DCC_LOG4(LOG_TRACE, " r12=%08x  sp=%08x  lr=%08x  pc=%08x", 
			__ctx->r12, __ctx, __ctx->lr, __ctx->pc);
	DCC_LOG1(LOG_TRACE, "xpsr=%08x", __ctx->xpsr);
}
#endif
