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

#define CONTROL_nPRIV      (1 << 0)
#define CONTROL_SPSEL      (1 << 1)
#define CONTROL_FPCA       (1 << 2)

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
	ldr      r2, .L_thinkos_rt
	ldrb     r1, [r2, #OFFSETOF_KRN_SCHED_ACTIVE]
	mrs      r0, PSP

	/* NEW: 2020-12-02*/
	/* Flush the context pointer. This will allow interrupt handlers
	 and the monitor to correctly locate the thread's context stack. */
#if (THINKOS_ENABLE_PRIVILEGED_THREAD) || (THINKOS_ENABLE_FPU)
	mrs      r3, CONTROL
	adds     r3, r0
	adds     r3, #(CONTROL_SPSEL) - (8 * 4)
#else
	adds     r3, r0, #(CONTROL_SPSEL | CONTROL_nPRIV) - (8 * 4)
#endif
	str      r3, [r2, r1, lsl #2]

#if (THINKOS_ENABLE_DEBUG_STEP)
	ldr      r3, [r2, #OFFSETOF_KRN_STEP_SVC]
	lsrs     r3, r1
	bcc      1f
	ldr      r12, .L_cm3_scb
	movs     r3, #SCB_ICSR_PENDSVSET
	str      r3, [r12, #SCB_ICSR_OFFS]
1:
#endif
	
	/* Get PC from the stack */
	ldr      r3, [r0, #(6 * 4)] 
	/* Read the SVC number from the instruction */
	ldrb     r3, [r3, #-2]
    cmp      r3, #THINKOS_SYSCALL_CNT
    bhi      thinkos_nosys_svc
	ldr      r12, .L_call_tab
	ldr      pc, [r12, r3, lsl #2]
	.align   2
.L_call_tab:
	.word    thinkos_svc_call_tab

	.align   2
.L_thinkos_rt:
	.word    thinkos_rt
#if (THINKOS_ENABLE_DEBUG_STEP)
.L_cm3_scb:
	.word    CM3_SCB_BASE
#endif
	.size    cm3_svc_isr, . - cm3_svc_isr


