/* 
 * thinkos_crt0_cortex-m3.S
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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

	.syntax unified
	.cpu cortex-m3

	.extern  _init
	.extern main

	.thumb
	.section .init

	.align  4

	.global __os_tag
__os_tag:
	.type   __os_tag, %object
#ifdef THINKOS_CUSTOM_SYS_TAG
	.string THINKOS_CUSTOM_SYS_TAG
#else
	.string "ThinkOS"
#endif
	.p2align  3, 0x00
	.size   __os_tag, . - __os_tag

	.global __init_tab
__init_tab:
	.type   __init_tab, %object

	.word	__text_start
	.word	__text_end
	.word	__data_start
	.word	__data_end	
	.word	__bss_start
	.word	__bss_end
	.word   __ctors_start
	.word   __ctors_end

	.size   __init_tab, . - __init_tab

	.thumb_func
	.align  2
_reset:
	.global _reset
	.type   _reset, %function

	bl _init

	/* load header table */
    adr     r0, __init_tab
	ldmia   r0, {r0, r1, r2, r3, r4, r5, r6, r7}

	/* copy initialized variables .data section */
1:  cmp     r2, r3
	ittt	lo
	ldrlo   r0, [r1], #4
	strlo   r0, [r2], #4
	blo     1b

	/* clear .bss  */
	mov     r0, #0
2:  cmp     r4, r5
	itt		lo
	strlo   r0, [r4], #4
	blo     2b

	/* call constructors */
3:  cmp     r6, r7
	beq     4f
	ldr     r0, [r6], #4
	blx     r0
	b       3b
4:

	b       main

	.size   _reset, . - _reset

