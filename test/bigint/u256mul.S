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

	.include "bigmul.inc"

	.syntax unified
	.cpu cortex-m3

	.text
	.align	2

	.global	u256mul
	.thumb
	.thumb_func
u256mul:
	BIGMUL 8
#if 0
	/* r0: z[]
     * r1: x[]
     * r2: y[]
	 * r3, r4: acc
	 * r5: carry 
	 * r6, r7: mul */

	push   {r4, r5, r6, r7, r8, r9, lr}

	add    r2, #(4 * 8)
    ldmdb  r2!, { r3, r4, r5, r6 }   /* y[0] .. y[2] */
	push   { r6 }
	push   { r5 }
	push   { r4 }
	push   { r3 }
    ldmdb  r2!, { r3, r4, r5, r6 }   /* y[0] .. y[2] */
	push   { r6 }
	push   { r5 }
	push   { r4 }
	push   { r3 }

	movs   r3, #0
	movs   r4, #0
	movs   r8, #0
	movs   r9, r1

	INNER 8, 1
	INNER 8, 2
	INNER 8, 3
	INNER 8, 4
	INNER 8, 6
	INNER 8, 7
	INNER 8, 8

 	pop    {r4, r5, r6, r7, r8, r9, pc}
#endif
	.size   u256mul, . - u256mul

