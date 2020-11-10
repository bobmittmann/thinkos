/* 
 * thinkos_svc-cortex-m3.S
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
#define __THINKOS_SYSCALLS__
#include <thinkos/syscalls.h>
#include <arch/cortex-m3.h>

#define SCB_ICSR_OFFS  0x04
#define SCB_SHCSR_OFFS 0x24 /* System Handler Control and State */
#define SCB_CFSR_OFFS  0x28 /* Configurable Fault Status */
#define SCB_HFSR_OFF   0x2c /* Hard Fault Status */
#define SCB_DFSR_OFFS  0x30 /* Debug Fault Status */
#define SCB_MMFAR_OFFS 0x34 /* Mem Manage Address */
#define SCB_BFAR_OFFS  0x38 /* Bus Fault Address */
#define SCB_AFSR_OFFS  0x3c /* Auxiliary Fault Status */


/* --------------------------------------------------------------------------
 * ThinkOS - Sevice Multiplexer 
 * --------------------------------------------------------------------------*/

	.syntax unified
	.cpu cortex-m3

	.text
	.align	3
	.global cm3_svc_isr
	.thumb
	.thumb_func
	.type   cm3_svc_isr, %function
cm3_svc_isr:
	/* The IDLE thread is the only using the main stack MSP, all
	   other threads use the PSP. No exception is allowed to call
	   SVC */
	ldr      r3, .L_thinkos_rt
	ldr      r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
#if (THINKOS_ENABLE_STACK_LIMIT)
	/* get the effective thread id */
	ands     r1, #0x3f
	/* TODO: check the stack limit */
#endif

#if (THINKOS_ENABLE_DEBUG_STEP)
	ldr      r2, [r3, #THINKOS_RT_STEP_SVC_OFFS]
	adds     r3, r1, #1
	lsrs     r2, r3
	bcc      1f
	ldr      r3, .L_cm3_scb
	mov      r2, #SCB_ICSR_PENDSVSET
	str      r2, [r3, #SCB_ICSR_OFFS]
1:
#endif
	mrs      r0, PSP
	/* Get PC from the stack */
	ldr      r2, [r0, #(6 * 4)] 
	/* Read the SVC number from the instruction */
	ldrb     r2, [r2, #-2]
    cmp      r2, #THINKOS_SYSCALL_CNT
    bhi      2f
	ldr      r3, .L_call_tab
	ldr      pc, [r3, r2, lsl #2]
	.align   2
.L_call_tab:
	.word    thinkos_call_tab

2:
	movs     r0, THINKOS_ERR_SYSCALL_INVALID
	b        thinkos_krn_xcpt_raise

	.align   2
.L_thinkos_rt:
	.word    thinkos_rt
.L_cm3_scb:
	.word    CM3_SCB_BASE
	.size    cm3_svc_isr, . - cm3_svc_isr


