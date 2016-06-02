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

	.global	u128mul
	.thumb
	.thumb_func
u128mul:
	BIGMUL 4
#if 0
	/* r0: z[]
     * r1: x[]
     * r2: y[]
	 * r3, r4: acc
	 * r5: carry 
	 * r6, r7: mul */

	push   {r4, r5, r6, r7, r8, r9, r10, r11, lr}
    ldmia  r1, { r8, r9, r10 }   /* x[0] .. x[2] */
    ldmia  r2, { r11, r12, lr }   /* y[0] .. y[2] */

	umull  r3, r4, r8, r11 /* acc = x[0] * y[0] */
    str    r3, [r0, #0]   /* z[0] */

	movs   r5, #0
	umull  r6, r7, r8, r12 /* acc += x[0] * y[1] */
	adds   r6, r4
    adcs   r7, r5
	umull  r3, r4, r9, r11 /* acc += x[1] * y[0] */
	adds   r3, r6
    adcs   r4, r7
	adcs   r5, r5
    str    r3, [r0, #4]   /* z[1] */

	umull  r6, r7, r8, lr /* acc += x[0] * y[2] */
	adds   r6, r4
    adcs   r7, r5
	umull  r3, r4, r9, r12 /* acc += x[1] * y[1] */
	adds   r3, r6
    adcs   r4, r7
	adcs   r5, #0 
	umull  r6, r7, r10, r11 /* acc += x[2] * y[0] */
	adds   r3, r6
    adcs   r4, r7
	adcs   r5, #0
    str    r3, [r0, #8]   /* z[2] */

	umull  r6, r7, r10, r12 /* acc += x[2] * y[1] */
	adds   r6, r4
    adcs   r7, r5
	umull  r3, r4, r9, lr /* acc += x[1] * y[2] */
	adds   r3, r6
    adcs   r4, r7
    ldr    r1, [r1, #12]   /* x[3] */
    ldr    r2, [r2, #12]   /* y[3] */
	umull  r6, r7, r8, r2 /* acc += x[0] * y[3] */
	adds   r3, r6
    adcs   r4, r7
	umull  r6, r7, r1, r11 /* acc += x[3] * y[0] */
	adds   r3, r6
    adcs   r4, r7
    str    r3, [r0, #12]   /* z[3] */

 	pop    {r4, r5, r6, r7, r8, r9, r10, r11, pc}
#endif
	.size   u128mul, . - u128mul

