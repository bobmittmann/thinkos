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

	.macro INNER digits count
	movs   r1, r9
	adds   r2, sp, #(\digits - \count) * 4
	movs   r3, r4
	movs   r4, r8
	movs   r5, #\count
1:
    ldmia  r1!, {r6}   /* x[4] */
    ldmia  r2!, {r7}    /* y[4] */
	umull  r6, r7, r6, r7   /* x[0] * y[4] */
	adds   r3, r6
    adcs   r4, r7
	adcs   r8, #0
	subs   r5, #1
	bne    1b
    stmia  r0!, {r3} /* z[4] */
	.endm

	.macro BIGMUL digits
	/* r0: z[]
     * r1: x[]
     * r2: y[]
	 * r3, r4: acc
	 * r5: carry 
	 * r6, r7: mul */

	push   {r4, r5, r6, r7, r8, r9, r10, lr}

	movs   r3, #(\digits / 4)
0:
    ldmia  r1!, { r4, r5, r6, r7 }   /* y[i] .. y[i + 3] */
	push   { r4 }
	push   { r5 }
	push   { r6 }
	push   { r7 }
	subs   r3, #1
	bne    0b

	movs   r4, r3
	movs   r8, r3
	movs   r9, r3
	movs   r10, r2
1:
	adds   r5, sp, #((\digits - 1) * 4)
	subs   r5, r5, r9, lsl #2
	movs   r6, r10
	movs   r3, r4
	movs   r4, r8
	movs   r7, r9
2:
    ldmia  r5!, {r1}    /* x[i] */
    ldmia  r6!, {r2}    /* y[j] */
	umull  r1, r2, r1, r2
	adds   r3, r1
    adcs   r4, r2
	adcs   r8, #0
	subs   r7, #1
	bne    2b
    stmia  r0!, {r3}   /* z[k] */
	adds   r9, #1
	cmp    r9, #\digits
	bne    1b

	add    sp, (\digits * 4)
 	pop    {r4, r5, r6, r7, r8, r9, r10, pc}

	.endm

