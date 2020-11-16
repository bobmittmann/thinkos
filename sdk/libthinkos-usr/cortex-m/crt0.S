/* 
 * crt0.S
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

#define __THINKOS_PROFILE__
#include <thinkos/profile.h>

	.syntax unified
	.cpu cortex-m3
	.thumb

	.section .init
__text_start:

	.align  2
_init_hdr:
	.global _init_hdr
	.type   _init_hdr, %function

	.word   _reset
_hdr_stack:
	.word  __stack_top
_hdr_opt:
	.word  __stack_size
_hdr_size:
	.word  __image_size 

_hdr_os_tag:
	.string "ThinkOS"
	.size   _init_hdr, . - _init_hdr

_hdr_sys_tag:
#ifdef THINKOS_APP_TAG
	.string THINKOS_APP_TAG
	.p2align  2, 0x00
#else
	.word 0 
	.word 0
#endif

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

	.section .init
	.thumb_func
	.align  2
_reset:
	.global _reset
	.type   _reset, %function

	/* save call args */
    mov     r8, r0
    mov     r9, r1
    mov     r10, r2
    mov     r11, r3

	/* load header table */
    adr     r12, __init_tab
	ldmia   r12, {r0, r1, r2, r3, r4, r5, r6, r7}

	/* copy initialized variables .data section */
1:  cmp     r2, r3
	ittt	lo
	ldrlo   r0, [r2], #4
	strlo   r0, [r1], #4
	blo     1b

	/* clear .bss  */
	mov     r0, #0
2:  cmp     r4, r5
	itt		lo
	strlo   r0, [r4], #4
	blo     2b

	/* save lr */
    mov     r5, lr
	/* call constructors */
3:  cmp     r6, r7
	beq     4f
	ldr     r0, [r6], #4
	blx     r0
	b       3b

4:  /* restore lr */
    mov     lr, r5

	/* clear frame pointers */
    mov     fp, #0
	mov     r7, #0

	/* restore call arguments */
	mov     r0, r8
	mov     r1, r9
	mov     r2, r10
	mov     r3, r11
	/* go to C main function */
	b       main

	.size   _reset, . - _reset

	.text
	.align  2
	.global _halt
	.thumb
	.thumb_func
	.type   _halt, %function
_halt:
    mov     r0, #1
2:
	cmp     r0, #0
	bne     2b
	.size   _halt, . - _halt

	.weak       halt
	.thumb_set  halt,_halt

	.weak       abort
	.thumb_set  abort,_halt

	.weak       apptag0
	.set 		apptag0, 0x00000000
	.weak       apptag1
	.set 		apptag1, 0x00000000
	.weak       apptag2
	.set 		apptag2, 0x00000000

	.end

