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

#define __THINKOS_SYS__
#include <thinkos_sys.h>
#include <arch/cortex-m3.h>

#define SCB_ICSR_OFFS 4
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
	.global	collect_xcpt_inf
	.thumb
	.thumb_func
	.type	collect_xcpt_inf, %function
	/* r0: return (lr) */
	/* r1: except number */
collect_xcpt_inf:
	ldr    r3, .L_xcpt_buf
	stmia  r3, {r4-r11}
	add    r2, r3, #32
	tst    r0, #4 
	ite    eq 
	mrseq  r4, MSP 
	mrsne  r4, PSP 
	ldmia  r4, {r5-r12}
	stmia  r2, {r5-r12}

	mrs    r5, MSP 
	sub    r5, r5, #32
	mrs    r6, PSP 
	tst    r0, #4 
	it     ne 
	subne  r6, r6, #32
	ldr    r7, .L_cm3_scb
	ldr    r7, [r7, #SCB_ICSR_OFFS]
	str    r0, [r3, #64]
	str    r5, [r3, #68]
	str    r6, [r3, #72]
	str    r7, [r3, #76]
	mrs    r5, IPSR
	ldr    r2, .L_thinkos_rt
	ldr    r2, [r2, #THINKOS_RT_ACTIVE_OFFS]
	strb   r5, [r3, #80]
	strb   r2, [r3, #81]
	strb   r1, [r3, #82]

	mov    r7, r0
	mov    r6, r3
	bx     lr
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
	mov    r0, lr
	movs   r1, #CM3_EXCEPT_BUS_FAULT
	bl     collect_xcpt_inf
	mov    r0, r6
	bl     thinkos_bus_fault
	mov    r0, r6
	bl     thinkos_xcpt_process
	bx     r7
	.size  cm3_bus_fault_isr, . - cm3_bus_fault_isr
#endif

#if	THINKOS_ENABLE_USAGEFAULT 
	.align	2
	.global	cm3_usage_fault_isr
	.thumb
	.thumb_func
	.type	cm3_usage_fault_isr, %function
cm3_usage_fault_isr:
	mov    r0, lr
	movs   r1, #CM3_EXCEPT_BUS_FAULT
	bl     collect_xcpt_inf
	mov    r0, r6
	bl     thinkos_usage_fault
	mov    r0, r6
	bl     thinkos_xcpt_process
	bx     r7
	.size  cm3_usage_fault_isr, . - cm3_usage_fault_isr
#endif

#if	THINKOS_ENABLE_MEMFAULT
	.align	2
	.global	cm3_mem_manage_isr
	.thumb
	.thumb_func
	.type	cm3_mem_manage_isr, %function
cm3_mem_manage_isr:
	mov    r0, lr
	movs   r1, #CM3_EXCEPT_BUS_FAULT
	bl     collect_xcpt_inf
	mov    r0, r6
	bl     thinkos_mem_manage
	mov    r0, r6
	bl     thinkos_xcpt_process
	bx     r7
	.size  cm3_mem_manage_isr, . - cm3_mem_manage_isr
#endif

	.align	2
	.global	cm3_hard_fault_isr
	.thumb
	.thumb_func
	.type	cm3_hard_fault_isr, %function
cm3_hard_fault_isr:
	mov    r0, lr
	movs   r1, #CM3_EXCEPT_BUS_FAULT
	bl     collect_xcpt_inf
	mov    r0, r6
	bl     thinkos_hard_fault
	mov    r0, r6
	bl     thinkos_xcpt_process
	bx     r7
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
#if 0
	movs    r0, #1
	lsls    r0, r1
	tst     r2, r0
	beq     1f
#endif
	adds    r0, r1, #1
	lsrs    r2, r0
	bcc     1f
	ldr     r3, .L_cm3_scb_1
	mov     r0, #SCB_ICSR_PENDSVSET
	str     r0, [r3, #SCB_ICSR_OFFS]
1:
#endif
	mrs     r0, PSP
	/* Get PC from the stack */
	ldr     r2, [r0, #(6 *4)] 
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

