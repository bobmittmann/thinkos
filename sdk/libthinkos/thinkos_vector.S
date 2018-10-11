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

#define SCB_ICSR_OFFS 4
#define SCB_CFSR_OFFS 0x28
#define DCB_DEMCR_OFFS 12 /* Debug Exception and Monitor Control Register */
#define SCB_MMFAR_OFFS   0x34
#define SCB_BFAR_OFFS    0x38

#define CFSR_BFARVALID (1 << (7 + 8))
#define CFSR_MMARVALID (1 << 7)

	.syntax unified
	.cpu cortex-m3

/* --------------------------------------------------------------------------
 * ThinkOS - excption handler stubs
 * --------------------------------------------------------------------------*/
#if THINKOS_ENABLE_EXCEPTIONS
	.thumb
	.text
	.align	2
	.global	collect_xcpt_inf
	.thumb
	.thumb_func
	.type	collect_xcpt_inf, %function
	/* Arguments:
	     r1: except number
	     r3: return (lr) 
	   Return:
	     r0: exception buffer */

collect_xcpt_inf:
	ldr    r0, .L_xcpt_buf
	mov    r2, r0
#if (THINKOS_ENABLE_FPU)
	vstmia r2!, {s16-s31}
#endif
	tst    r3, #CM3_EXC_RET_SPSEL
	ite    eq 
	mrseq  r4, MSP 
	mrsne  r4, PSP 
	stmia  r2!, {r4-r11} /* r4..r11 */
	ldmia  r4, {r5-r12} /* r0..r3, r12, lr, pc, xpsr */
#if (THINKOS_ENABLE_IDLE_MSP)
	stmia  r2!, {r4} /* sp */
	stmia  r2!, {r3,r5-r12} /* return r0..r3, r12, lr, pc, xpsr */
#else
	stmia  r2!, {r5-r12} /* r0..r3, r12, lr, pc, xpsr */
#endif
#if (THINKOS_ENABLE_FPU)
	vstmia r2!, {s0-s15}
	vmrs   r4, FPSCR
	stmia  r2!, {r4-r5}
#endif
	mrs    r5, MSP 
	mrs    r6, PSP 
	str    r5, [r2, #0] /* msp */
	str    r6, [r2, #4] /* psp */
	ldr    r7, .L_cm3_scb
	ldr    r5, [r7, #SCB_ICSR_OFFS]
	ldr    r6, [r7, #SCB_CFSR_OFFS]
	str    r5, [r2, #8] /* icsr */
	str    r6, [r2, #12] /* cfsr */
	tst    r6, #CFSR_MMARVALID
	itt    ne 
	ldrne  r5, [r7, #SCB_MMFAR_OFFS]
	strne  r5, [r2, #16] /* mmfar */
	tst    r6, #CFSR_BFARVALID
	itt    ne 
	ldrne  r5, [r7, #SCB_BFAR_OFFS]
	strne  r5, [r2, #20] /* bfar */
	/* clear fault status */
	str    r6, [r7, #SCB_CFSR_OFFS]
	mrs    r5, IPSR
	strb   r5, [r2, #24] /* ipsr */
	mrs    r5, CONTROL
	strb   r5, [r2, #25] /* ctrl */
	ldr    r4, .L_thinkos_rt
	ldr    r4, [r4, #THINKOS_RT_ACTIVE_OFFS]
	strb   r4, [r2, #26] /* active */
	strb   r1, [r2, #27] /* type */

	mov    r7, lr
	mov    lr, r3
	bx     r7
	.align  2
.L_xcpt_buf:
	.word	thinkos_except_buf
.L_thinkos_rt:
	.word	thinkos_rt
.L_cm3_scb:
	.word	CM3_SCB_BASE 

	.size   collect_xcpt_inf, . - collect_xcpt_inf

#if	THINKOS_ENABLE_BUSFAULT 
	.thumb
	.text
	.align	2
	.global	cm3_bus_fault_isr
	.thumb
	.thumb_func
	.type	cm3_bus_fault_isr, %function
cm3_bus_fault_isr:
	mov    r3, lr
	movs   r1, #CM3_EXCEPT_BUS_FAULT
	bl     collect_xcpt_inf
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
	mov    r3, lr
	movs   r1, #CM3_EXCEPT_USAGE_FAULT
	bl     collect_xcpt_inf
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
	mov    r3, lr
	movs   r1, #CM3_EXCEPT_MEM_MANAGE
	bl     collect_xcpt_inf
	b      thinkos_mem_manage
	.size  cm3_mem_manage_isr, . - cm3_mem_manage_isr
#endif

	.align	2
	.global	cm3_hard_fault_isr
	.thumb
	.thumb_func
	.type	cm3_hard_fault_isr, %function
cm3_hard_fault_isr:
#if DEBUG
	nop
	b      cm3_hard_fault_isr
#else
	mov    r3, lr
	movs   r1, #CM3_EXCEPT_HARD_FAULT
	bl     collect_xcpt_inf
	b      thinkos_hard_fault
#endif
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
	tst     lr, #4
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

