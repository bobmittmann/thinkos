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

#define __THINKOS_SYS__
#include <thinkos_sys.h>
#include <arch/cortex-m3.h>

/* --------------------------------------------------------------------------
 * ThinkOS - debug monitor context swap
 * --------------------------------------------------------------------------*/
	.syntax unified
	.cpu cortex-m3

	.text
	.align	2
	.global	dmon_context_swap
	.thumb
	.thumb_func
	.type	dmon_context_swap, %function

dmon_context_swap:

	push   {r4-r11,lr}
	mrs    r1, APSR
	push   {r1}
	mov    r1, sp
	ldr    sp, [r0] /* restore context */
	str    r1, [r0] /* save context */
	pop    {r1}
	msr    APSR_nzcvq, r1
	pop    {r4-r11,lr}

	.size   dmon_context_swap, . - dmon_context_swap

