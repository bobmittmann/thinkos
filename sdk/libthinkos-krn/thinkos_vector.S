/* 
 * thikos_core.c
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

#define SCB_ICSR_OFFS  0x04
#define SCB_SHCSR_OFFS 0x24 /* System Handler Control and State */
#define SCB_CFSR_OFFS  0x28 /* Configurable Fault Status */
#define SCB_HFSR_OFF   0x2c /* Hard Fault Status */
#define SCB_DFSR_OFFS  0x30 /* Debug Fault Status */
#define SCB_MMFAR_OFFS 0x34 /* Mem Manage Address */
#define SCB_BFAR_OFFS  0x38 /* Bus Fault Address */
#define SCB_AFSR_OFFS  0x3c /* Auxiliary Fault Status */

#define DCB_DEMCR_OFFS 12 /* Debug Exception and Monitor Control Register */

	.syntax unified
	.cpu cortex-m3

/* --------------------------------------------------------------------------
 * ThinkOS - excption handler stubs
 * --------------------------------------------------------------------------*/
#if THINKOS_ENABLE_EXCEPTIONS
	.thumb
	.text
	.align	2
	.global	xcpt_collect
	.thumb
	.thumb_func
	.type	xcpt_collect, %function
	/* Arguments:
	     r1: except number
	     r2: return (lr) 
	   Return:
	     r0: exception buffer */

xcpt_collect:
	mrs    r3, PRIMASK
	/* disable interrupts */
	cpsid   i
	ldr    r0, .L_xcpt_buf
	strb   r1, [r0, #0] /* errno */
	strb   r3, [r0, #3] /* primask */
	mrs    r3, FAULTMASK
	strb   r3, [r0, #4] /* faultmask */
	mrs    r3, BASEPRI 
	strb   r3, [r0, #5] /* basepri */
	mrs    r3, CONTROL
	strb   r3, [r0, #6] /* control */
	mrs    r3, IPSR
	strb   r3, [r0, #7] /* ipsr */

	adds   r3, r0, #8
#if (THINKOS_ENABLE_FPU)
	/* collect FP data */ 
	vstmia r3!, {s16-s31}
#endif
	stmia  r3!, {r4-r11} /* r4..r11 */
	tst    r2, #CM3_EXC_RET_SPSEL
	ite    eq 
	mrseq  r4, MSP 
	mrsne  r4, PSP 
#if (THINKOS_ENABLE_IDLE_MSP) || (THINKOS_ENABLE_FPU)
	str    r4, [r3, #0] /* sp */
	str    r2, [r3, #4] /* ret */
	adds   r3, r3, #8
#endif
	ldmia  r4, {r5-r12} /* r0..r3, r12, lr, pc, xpsr */
	stmia  r3!, {r5-r12} /* r0..r3, r12, lr, pc, xpsr */
#if (THINKOS_ENABLE_FPU)
	vstmia r3!, {s0-s15}
	vmrs   r4, FPSCR
	stmia  r3!, {r4-r5}
#endif
	mrs    r5, MSP 
	mrs    r6, PSP 
	stmia  r3!, {R5, R6} /* msp, psp */

	ldr    r7, .L_cm3_scb

	ldr    r5, [r7, #SCB_ICSR_OFFS] /* Interrupt Control State */
	ldr    r6, [r7, #SCB_SHCSR_OFFS] /* System Handler Control and State */
	stmia  r3!, {R5, R6} /* icsr, shcsr */

	ldr    r5, [r7, #SCB_CFSR_OFFS] /* Configurable Fault Status */
	ldr    r6, [r7, #SCB_HFSR_OFF] /* Hard Fault Status */
	stmia  r3!, {R5, R6} /* cfsr, hfsr*/

	ldr    r5, [r7, #SCB_MMFAR_OFFS] /* Mem Manage Address */
	ldr    r6, [r7, #SCB_BFAR_OFFS] /* Bus Fault Address */
	stmia  r3!, {R5, R6} /* mmfar, bfar */

	ldr    r4, .L_thinkos_rt
#if (THINKOS_ENABLE_PROFILING)
	/* DWT.SYCCNT */
	ldr.n   r5, .L_cm3_dwt
	/* r6: cyccnt = CM3_DWT->cyccnt */
	ldr.n   r5, [r5, #4]
	ldr    r6, [r4, #THINKOS_RT_CYCREF_OFFS]
	stmia  r3!, {R5, R6} /* cyccnt, cycref */
#endif

	ldr    r5, [r4, #THINKOS_RT_ACTIVE_OFFS]
	ldr    r6, [r4, #THINKOS_RT_READY_OFFS]
	stmia  r3!, {R5, R6}

	/* restore our context */
	adds   r3, r0, #8
#if (THINKOS_ENABLE_FPU)
	/* skip FP data */ 
	adds   r3, r3, #(16 * 4)
#endif

	/* restore lr */
	mov    r7, lr
	mov    lr, r2
	mov    r2, r7
	ldmia  r3, {r4-r11} /* r4..r11 */
	/* XXX: enable interrupts */
	cpsie   i
	/* return to the original handler */
	bx     r2

	.align  2
.L_xcpt_buf:
	.word	thinkos_except_buf
.L_thinkos_rt:
	.word	thinkos_rt
.L_cm3_scb:
	.word	CM3_SCB_BASE 
.L_cm3_dwt:
	.word   CM3_DWT_BASE    /* DWT Base Address */

	.size   xcpt_collect, . - xcpt_collect


	.thumb
	.text
	.align	2
	.global	xcpt_collect
	.thumb
	.thumb_func
	.type	xcpt_collect, %function
	/* Arguments:
	     r1: except number
	     r2: return (lr) 
	   Return:
	     r0: exception buffer */

#if	THINKOS_ENABLE_BUSFAULT 
	.thumb
	.text
	.align	2
	.global	cm3_bus_fault_isr
	.thumb
	.thumb_func
	.type	cm3_bus_fault_isr, %function
cm3_bus_fault_isr:
	mov    r2, lr
	movs   r1, #THINKOS_ERR_BUS_FAULT
	bl     xcpt_collect
	b      thinkos_bus_fault
	.size  cm3_bus_fault_isr, . - cm3_bus_fault_isr
#endif

#if	THINKOS_ENABLE_USAGEFAULT 
	.align	2
	.global	cm3_usage_fault_isr
	.thumb
	.thumb_func
	.type	cm3_usage_fault_isr, %function
cm3_usage_fault_isr:
	mov    r2, lr
	movs   r1, #THINKOS_ERR_USAGE_FAULT
	bl     xcpt_collect
	b      thinkos_usage_fault
	.size  cm3_usage_fault_isr, . - cm3_usage_fault_isr
#endif

#if	THINKOS_ENABLE_MEMFAULT
	.align	2
	.global	cm3_mem_manage_isr
	.thumb
	.thumb_func
	.type	cm3_mem_manage_isr, %function
cm3_mem_manage_isr:
	mov    r2, lr
	movs   r1, #THINKOS_ERR_MEM_MANAGE
	bl     xcpt_collect
	b      thinkos_mem_manage
	.size  cm3_mem_manage_isr, . - cm3_mem_manage_isr
#endif

	.align	2
	.global	cm3_hard_fault_isr
	.thumb
	.thumb_func
	.type	cm3_hard_fault_isr, %function
cm3_hard_fault_isr:
	mov    r2, lr
	movs   r1, #THINKOS_ERR_HARD_FAULT
	bl     xcpt_collect
	b      thinkos_hard_fault
	.size  cm3_hard_fault_isr, . - cm3_hard_fault_isr
#endif /* THINKOS_ENABLE_EXCEPTIONS */

#if THINKOS_ENABLE_MONITOR
	.align  2
	.global cm3_debug_mon_isr
	.thumb
	.thumb_func
	.type   cm3_debug_mon_isr, %function
cm3_debug_mon_isr:
	/* select the context stack according to the content of LR */
	mov     r1, lr
	tst     r1, #CM3_EXC_RET_SPSEL
	ite     eq
	mrseq   r0, MSP
	mrsne   r0, PSP
	b       thinkos_dbgmon_isr
#if 0
	/* FIXME: add suport for step on interrupt */
	push    {lr}
	bl      thinkos_dbgmon_isr
	pop     {lr}
	cbnz    r0, 1f
	bx      lr
1f:
	/* Disable low priority interrupts */
	mov     r2, #(1 << 5)
	msr	    BASEPRI, r2
	/* Step request */
	ldr.n   r3, .L_cm_dcb
	ldr     r1, [r3, #DCB_DEMCR_OFFS]
	orr     r1, r1, #DCB_DEMCR_MON_STEP
	str     r1, [r3, #DCB_DEMCR_OFFS]
	bx      lr
.L_cm_dcb:
	.word   CM3_DCB_BASE /* Core Debug Base Address */
#endif
	.size   cm3_debug_mon_isr, . - cm3_debug_mon_isr
#endif /* THINKOS_ENABLE_MONITOR */

	.align  2
	.global cm3_svc_isr
	.thumb
	.thumb_func
	.type   cm3_svc_isr, %function
cm3_svc_isr:
	ldr     r3, .L_thinkos_rt_1
	ldr     r1, [r3, #THINKOS_RT_ACTIVE_OFFS]
#if THINKOS_ENABLE_DEBUG_STEP
	ldr     r2, [r3, #THINKOS_RT_STEP_SVC_OFFS]
	adds    r0, r1, #1
	lsrs    r2, r0
	bcc     1f
	ldr     r3, .L_cm3_scb_1
	mov     r0, #SCB_ICSR_PENDSVSET
	str     r0, [r3, #SCB_ICSR_OFFS]
1:
#endif

#if (THINKOS_ENABLE_IDLE_MSP)
	/* select the context stack according to the content of LR */
	tst     lr, #4
	ite     eq
	mrseq   r0, MSP
	mrsne   r0, PSP
#else
	mrs     r0, PSP
#endif

	/* Get PC from the stack */
	ldr     r2, [r0, #(6 * 4)] 
	/* Read the SVC number from the instruction */
	ldrb    r2, [r2, #-2]
	b       thinkos_svc_isr
	.align  2
.L_thinkos_rt_1:
	.word   thinkos_rt
.L_cm3_scb_1:
	.word   CM3_SCB_BASE
	.size   cm3_svc_isr, . - cm3_svc_isr


	.align  2
	.global thinkos_vec_nm
	.section .rodata,"a",%progbits
	.type   thinkos_vec_nm, %object
	.size   thinkos_vec_nm, 4
thinkos_vec_nm:
	.ascii  "VEC\000"

