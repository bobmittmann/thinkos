/* 
 * dbgmon_swap.S
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
#include <arch/cortex-m3.h>

/* --------------------------------------------------------------------------
 * ThinkOS - debug monitor context swap
 * --------------------------------------------------------------------------*/

	.syntax unified
	.cpu cortex-m3

	.text
	.align	2
	.global	dbgmon_context_swap
	.global	dbgmon_context_swap_ext
	.thumb
	.thumb_func
dbgmon_context_swap:

#if 0
	mov    r1, #0
	.type	dbgmon_context_swap, %function
	.type	dbgmon_context_swap_ext, %function
dbgmon_context_swap_ext:
#endif
	push   {r4-r11,lr}
	mrs    r3, APSR
	push   {r3}
	mov    r3, sp
	ldr    sp, [r0] /* restore context */
	str    r3, [r0] /* save context */
#if 0
	mov    r0, r1 /* optional return value */
#endif
	pop    {r3}
	msr    APSR_nzcvq, r3
	pop    {r4-r11,lr}
	bx     lr

	.size   dbgmon_context_swap, . - dbgmon_context_swap

