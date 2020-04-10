/* 
 * stm32-crt0.S
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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#include <arch/cortex-m3.h>

	.syntax unified
	.cpu cortex-m3
	.altmacro

	/* ---------------------------------------------------------------------
	 * Generate the isr stubs 
	 * --------------------------------------------------------------------- */
#define SCS_NVIC 0x100

	.macro  __mklabel, name, number
\name\number\():
	.endm

	.macro  mklabel name, number
	__mklabel \name, %number
	.endm

	.macro  __mkglobal, name, number
	.global \name\number\()
	.endm

	.macro  mkglobal name, number
	__mkglobal \name, %number
	.endm

	.macro  __mkalias, name, number, alias
	.weak       \name\number\()
	.thumb_set  \name\number\(), \alias
	.endm

	.macro  mkalias name, number, alias
	__mkalias \name, %number, alias
	.endm


	.macro  __mkentry, name, number
	.section .text.\name\number\()
	.align  2
	.global \name\number\()
	.thumb
	.thumb_func
	.type \name\number\(), %function
\name\number\():
	.endm

	.macro  mkentry name, number
	__mkentry \name, %number
	.endm

	.macro  __mkexit, name, number
	.size   \name\number\(), . - \name\number\()
	.endm

	.macro  mkexit name, number
	__mkexit \name, %number
	.endm

	.macro thinkos_isr irq:req
mkentry thinkos_isr,\irq
#if (THINKOS_ENABLE_IRQ_CYCCNT)
	/* DWT.SYCCNT */
	ldr     r1, =CM3_DWT_BASE
	/* r1: cyccnt = CM3_DWT->cyccnt */
	ldr     r1, [r1, #DWT_CYCCNT]
#endif
	/* disable this interrupt source */
	movs    r0, #(\irq)
	movs    r2, #1
	lsls    r2, #(\irq % 32)
	movs    r3, #(CM3_SCS_BASE)
	str     r2, [r3, #(SCS_NVIC + NVIC_ICER0 + (\irq / 32))]
	b       thinkos_default_isr
mkexit thinkos_isr,\irq
	.endm

	.macro thinkos_dma_isr irq:req 
mkentry thinkos_dma_isr,\irq
#if (THINKOS_ENABLE_IRQ_CYCCNT)
	/* DWT.SYCCNT */
	ldr     r1, =CM3_DWT_BASE
	/* r1: cyccnt = CM3_DWT->cyccnt */
	ldr     r1, [r1, #DWT_CYCCNT]
#endif
	/* disable this interrupt source */
	movs    r0, #(\irq)
	movs    r2, #1
	lsls    r2, #(\irq % 32)
	movs    r3, #(CM3_SCS_BASE)
	str     r2, [r3, #(SCS_NVIC + NVIC_ICER0 + (\irq / 32))]
	b       thinkos_default_isr
mkexit thinkos_dma_isr,\irq
	.endm

#if (THINKOS_ENABLE_IRQ_ASM_FAST)
	.set irq, 0
	.rept (THINKOS_IRQ_MAX + 1)
	thinkos_isr irq
	.set irq, irq + 1
	.endr

	.set irq, 0
	.rept (THINKOS_IRQ_MAX)
	thinkos_dma_isr irq
	.set irq, irq + 1
	.endr

#else
	.text
	.align  2
	.global thinkos_isr

	.set irq, 0
	.rept (THINKOS_IRQ_MAX + 1)
	mkglobal thinkos_isr irq
	mkglobal thinkos_dma_isr irq
	.set irq, irq + 1
	.endr

	.thumb
	.thumb_func
	.type   thinkos_isr, %function
thinkos_isr:

#if (THINKOS_ENABLE_IRQ_CYCCNT)
	/* DWT.SYCCNT */
	ldr     r1, =CM3_DWT_BASE
	/* r1: cyccnt = CM3_DWT->cyccnt */
	ldr     r1, [r1, #DWT_CYCCNT]
#endif
	mrs     r0, IPSR
	subs    r0, #16
	/* disable this interrupt source */
	movs    r3, #1
	ands    r2, r0, #31
	lsls    r2, r3, r2
	lsrs    r3, r0, #5
	lsls    r3, r3, #2
	adds    r3, r3, #(CM3_SCS_BASE)
	str     r2, [r3, #(SCS_NVIC + NVIC_ICER0)]

	b       thinkos_default_isr
	.size   thinkos_isr, . - thinkos_isr

	.set irq, 0
	.rept (THINKOS_IRQ_MAX + 1)
	mkalias thinkos_isr irq thinkos_isr 
	mkalias thinkos_dma_isr irq thinkos_isr 
	.set irq, irq + 1
	.endr

#endif /* THINKOS_ENABLE_IRQ_ASM_FAST */

	.end
