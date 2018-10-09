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

	push   {r4-r11,lr}
	mrs    r3, APSR
	push   {r3}

/*	Cortex M4, 
	ARM ID number 752419 as Cat 2
	Description
	An interrupt occurring during the data-phase of a single word load to 
	the stack pointer (SP/R13) can cause an erroneous behavior of the device. 
	In addition, returning from the interrupt results in the load instruction 
	being executed with an additional time.
	For all the instructions performing an update of the base register, 
	the base register is erroneously updated on each execution, resulting 
	in the stack pointer being loaded from an incorrect memory location.
		The instructions affected by this limitation are the following:
		• LDR SP, [Rn],#imm
		• LDR SP, [Rn,#imm]!
		• LDR SP, [Rn,#imm]
		• LDR SP, [Rn]
		• LDR SP, [Rn,Rm] */
	mov    r3, sp
	ldr    r2, [r0] /* restore context */
	str    r3, [r0] /* save context */
	mov    sp, r2
#if 0
	mov    r0, r1 /* optional return value */
#endif
	pop    {r3}
	msr    APSR_nzcvq, r3
	pop    {r4-r11,lr}
	bx     lr

	.size   dbgmon_context_swap, . - dbgmon_context_swap

