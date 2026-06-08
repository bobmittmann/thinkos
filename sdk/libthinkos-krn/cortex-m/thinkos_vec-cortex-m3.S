/* 
 * thinkos_core.c
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
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>

/* Table B1-8 EXC_RETURN definition of exception return behavior, no FP 
   extension
   EXC_RETURN | Return to    | Return stack   |
   0xFFFFFFF1 | Handler mode | Main           | 0001
   0xFFFFFFF9 | Thread mode  | Main           | 1001
   0xFFFFFFFD | Thread mode  | Process        | 1101
 */

/* Table B1-9 EXC_RETURN definition of exception return behavior, with FP 
   extension 
   EXC_RETURN | Return to    | Return stack | Frame type
   0xFFFFFFE1 | Handler mode | Main         | Extended
   0xFFFFFFE9 | Thread mode  | Main         | Extended
   0xFFFFFFED | Thread mode  | Process      | Extended
   0xFFFFFFF1 | Handler mode | Main         | Basic
   0xFFFFFFF9 | Thread mode  | Main         | Basic
   0xFFFFFFFD | Thread mode  | Process      | Basic

#define EXEC_RET_SPSEL (1 << 2)
#define EXEC_RET_MODE  (1 << 3)
#define EXEC_RET_nFPCA (1 << 4)

#define CONTROL_nPRIV       (1 << 0)
#define CONTROL_SPSEL       (1 << 1)
#define CONTROL_FPCA        (1 << 2)
*/


#define SCB_ICSR_OFFS  0x04

#define SCB_SHCSR_OFFS 0x24 /* System Handler Control and State */
#define SCB_CFSR_OFFS  0x28 /* Configurable Fault Status */
#define SCB_HFSR_OFF   0x2c /* Hard Fault Status */

#define SCB_DFSR_OFFS  0x30 /* Debug Fault Status */
#define SCB_MMFAR_OFFS 0x34 /* Mem Manage Address */
#define SCB_BFAR_OFFS  0x38 /* Bus Fault Address */

#define SCB_AFSR_OFFS  0x3c /* Auxiliary Fault Status */

#define DCB_DEMCR_OFFS 12 /* Debug Exception and Monitor Control Register */

#define SIZEOF_ARMV7M_BASIC_FRAME (8 *4)

	.extern _stack

	.syntax unified
	.cpu cortex-m3

	.section .core_vectors, "ax", %progbits
	.align	2
__core_vectors:
	.global __core_vectors
	.type __core_vectors, %object

	.word	_stack
	.word   _reset
	.word   cm3_nmi_isr
	.word   cm3_hard_fault_isr
	.word   cm3_mem_manage_isr
	.word   cm3_bus_fault_isr
	.word   cm3_usage_fault_isr
	.word   cm3_except7_isr
	.word   ice_comm_blk
	.word   cm3_except9_isr
	.word   cm3_except10_isr
	.word   cm3_svc_isr
	.word   cm3_debug_mon_isr
	.word   cm3_except13_isr
	.word   cm3_pendsv_isr
	.word   cm3_systick_isr

	.size   __core_vectors, . - __core_vectors

#define CFSR_UNDEFINSTR (UFSR_UNDEFINSTR << 16)
#define CFSR_MMRVALIED  (MMFSR_MMARVALID << 0)
/* A MemManage fault occurred during FP lazy state preservation */
#define CFSR_MLSPERR    (MMFSR_MLSPERR << 0)
/* A derived MemManage fault occurred on exception entry. */
#define CFSR_MSTKERR    (MMFSR_MSTKERR << 0) 
/*	A derived MemManage fault occurred on exception return. */
#define CFSR_MUNSTKERR  (MMFSR_MUNSTKERR << 0) 
/* Data access violation. The MMAR shows the data address that the 
	load or store tried to access. */
#define CFSR_DACCVIOL   (MMFSR_DACCVIOL << 0)
/*	MPU or Execute Never (XN) default memory map access violation on 
	an instruction fetch has occurred. The fault is signalled only if 
	the instruction is issued. */
#define CFSR_IACCVIOL  (MMFSR_IACCVIOL << 0)


	.text
	.align  2

/* --------------------------------------------------------------------------
 * ThinkOS - excption handler stubs
 * --------------------------------------------------------------------------*/

#if (THINKOS_ENABLE_HARDFAULT)
	.global	cm3_hard_fault_isr
	.thumb_func
	.type	cm3_hard_fault_isr, %function
cm3_hard_fault_isr:
	/* uses LR as a temporary variable */
	ldr     sp, .L_xcpt_stack
	stmia   sp!, {r4-r11}      /* save faulty registers */
	stmia   sp!, {r0-r3, r12}  /* save faulty stack frame but LR, PC and XPSR */

	tst     lr, #EXC_RET_SPSEL
	beq     .L_bypass
	mrs     r7, PSP 
	adds.n  r7, #(5*4)
	ldmia   r7!, {r0-r2}
.L_bypass:
	stmia   sp!, {r0-r2}        /* LR, PC and XPSR */
	movw    r0, #:lower16:thinkos_rt
	movt    r0, #:upper16:thinkos_rt
	mov     r1, sp
	subs    r1, #((16) * 4)
	mrs     r2, PSP
	mrs     r3, XPSR
	/* restore SP to the top of the exception stack */
	add     r4, r1, #THINKOS_EXCEPT_STACK_SIZE 
	mov     sp, r4
	b       thinkos_krn_hard_fault_handler

	.align  2
.L_xcpt_stack: 
	.word   thinkos_except_stack 

	.size   cm3_hard_fault_isr, . - cm3_hard_fault_isr
#endif /*( (THINKOS_ENABLE_HARDFAULT) */


#if (THINKOS_ENABLE_EXCEPTIONS)
	.global	__thinkos_krn_fault
	.thumb
	.thumb_func
	.type	__thinkos_krn_fault, %function
__thinkos_krn_fault:
	/* ---------------------------------------------------------------
     * This routine saves the basic context of the exception:
	 * R4..R11, R0-R3, R12, LR, PC, XPSR
	 * ---------------------------------------------------------------
	 */
	movw    r1, #:lower16:thinkos_fault_rt
	movt    r1, #:upper16:thinkos_fault_rt

	stmia   r1!, {r4-r11} /* save R4-R11 */
	pop     {r4-r11} /* read back R0-R3, R12,  LR, PC, XPSR */
	stmia   r1!, {r4-r11}  /* save faulty stack frame LR, PC and XPSR */
	subs.n  r1, #(16 * 4)

	/* store the return code lower byte */
	strb    lr, [r1, #OFFSETOF_FAULT_RET] /* fault.ret */
	mrs     r4, CONTROL
	strb    r4, [r1, #OFFSETOF_FAULT_CONTROL]

	/* Store the exception number */
	movw     r0, #:lower16:thinkos_rt
	movt     r0, #:upper16:thinkos_rt
	mrs     r2, IPSR
	strb    r2, [r0, #OFFSETOF_KRN_SCHED_XCP]

	/* store the sched.ctrl */
	ldr     r4, [r0, #OFFSETOF_KRN_SCHED_CTRL]
	str     r4, [r1, #OFFSETOF_FAULT_SCHED_CTRL]

	/* increment the sequence number */
	ldrb    r4, [r1, #OFFSETOF_FAULT_SEQ]
	add     r4, #1
	strb    r4, [r1, #OFFSETOF_FAULT_SEQ]

	ldr     r3, .L_cm3_scb
	/* grab and store fault information and system status */
	ldr.n   r4, [r3, #SCB_ICSR_OFFS]
	str     r4, [r1, #OFFSETOF_FAULT_ICSR]
	/* jump to the SHCSR */
	adds.n  r3, #SCB_SHCSR_OFFS
	ldmia   r3!, {r4-r9} /* load SHCSR, CFSR, HFSR, DFSR, MMFAR, BFAR */
	add     r7, r1, #OFFSETOF_FAULT_SHCSR
	stmia   r7, {r4,r5,r8,r9} /* save SHCSR, CFSR, MMFAR, BFAR */

	/* restore SP to the top of the exception stack */
	ldr     sp, .L_xcpt_stack_top
	/* make room for IDLE thread context */
	sub     sp, #(16*4)

	/* At this point the registers are:
	   R0 -> pointer to thinkos_rt
	   R1 -> pointer to the fault buffer 
	   R2 -> exception number
	   SP -> exception stack top
	 */
	
	mov     r4, r0
	mov     r5, r1
	ldr     r6, =__thinkos_krn_xcpt_unroll_again

	.align  2
.L_xcpt_stack_top: 
	.word   thinkos_except_stack + THINKOS_EXCEPT_STACK_SIZE

	.size   __thinkos_krn_fault, . - __thinkos_krn_fault

	.global	__thinkos_krn_xcpt_unroll_again
	.thumb
	.thumb_func
	.type	__thinkos_krn_xcpt_unroll_again, %function

__thinkos_krn_xcpt_unroll_again:
	push    {r0-r7} /* stack frame R0-R3, R12, LR, PC, XPSR */

	bl      krn_xcpt_unroll_ipsr_get
	cmp     r0, #0
	beq     __thinkos_krn_xcpt_done

	orr     r7, r0, #(1 << 24) /* set Thumb bit on XPSR */
	str     r7, [sp, #(7 * 4)] /* push XPSR onto stack frame */
	mov     lr, #CM3_EXC_RET_HANDLER
	bx      lr

	.size   __thinkos_krn_xcpt_unroll_again, . - __thinkos_krn_xcpt_unroll_again


	.global	__thinkos_krn_xcpt_done
	.thumb
	.thumb_func
	.type	__thinkos_krn_xcpt_done, %function

__thinkos_krn_xcpt_done:
	/* ensure privileged access in thread mode */
	msr     CONTROL, r0

	mov.n   r0, r4
	mov.n   r1, r5
	bl      thinkos_krn_fault_handler

	mov.n   r0, r4
	bl      thinkos_krn_idle_reset
	msr     MSP, r0

	sub     r4, r0, #(16 * 4)  /* make room for context dump R4-R11 */
	msr     PSP, r4

	ldr     r6, .L_krn_halt /* PC */
	mov     r7, #(1 << 24) /* XPSR set THUMB bit */
	stmia   r4, {r0-r7} /* push stack frame R0-R3, R12, LR, PC, XPSR */

	mov     lr, #CM3_EXC_RET_THREAD_PSP
	bx      lr

	.align  2
.L_xcpt_stack_bottom: 
	.word   thinkos_except_stack 

	.size   __thinkos_krn_xcpt_done, . - __thinkos_krn_xcpt_done

#endif /*( (THINKOS_ENABLE_EXCEPTIONS) */

#if (THINKOS_ENABLE_ERROR_TRAP)
	.global	__thinkos_krn_thread_err
	.thumb
	.thumb_func
	.type	__thinkos_krn_thread_err, %function

__thinkos_krn_thread_err:
	/* disable interrupts */
	cpsid   i
	/* check the stack of the exception (PSP or MSP ?)*/
	tst     lr, #EXC_RET_SPSEL
	beq     __thinkos_krn_fault
	push    {lr}
	ldr     lr, .L_fault_rt 
	stmia   lr!, {r4-r11} /* save R4-R11 */
	stmia   lr!, {r0-r3, r12} /* save R0-r3, R12 */
	mrs     r7, PSP 
	/* If the origin of error is the PSP being invalid then the 
	   next instruction will cause a hard fault. Possible solutions,
	   ignore the registers lr, pc and xpsr */
	ldr     r3, .L_cm3_scb
	ldr     r0, [r3, #SCB_CFSR_OFFS] /* Configurable Fault Status */
	tst     r0, #CFSR_MSTKERR  /* Stack error SP is invalid, reading from it
								  will cause a forced hard fault */
	bne     .L_stack_err
	/* SP is probably OK */ 
	adds.n  r7, #(5 * 4) /* top of faulty stack frame */
	ldmia   r7!, {r4,r5,r6}/* load from stack frame LR, PC, XPSR */
	b       2f
.L_stack_err:
	/* Stack error detected */
	/* load dummy values to LR, PC and XPSR */
	movs.n  r4, #0 /* lr */
	movs.n  r5, #0 /* pc */
	movs.n  r6, #0 /* xpsr */
2:
	stmia   lr!, {r4-r7} /* save LR, PC, XPSR, SP */

	/* get a pointer to the fault buffer */
	subs    r1, lr, #(17 * 4)

	pop     {lr}
	/* store the return code lower byte */
	strb    lr, [r1, #OFFSETOF_FAULT_RET] /* fault.ret */
	mrs     r4, CONTROL
	strb    r4, [r1, #OFFSETOF_FAULT_CONTROL]
	
	/* Store the exception number */
	ldr     r0, .L_thinkos_rt
	mrs     r2, IPSR
	strb    r2, [r0, #OFFSETOF_KRN_SCHED_XCP]

	/* store the sched.ctrl */
	ldr     r4, [r0, #OFFSETOF_KRN_SCHED_CTRL]
	str     r4, [r1, #OFFSETOF_FAULT_SCHED_CTRL]

	/* increment the sequence number */
	ldrb    r4, [r1, #OFFSETOF_FAULT_SEQ]
	add     r4, #1
	strb    r4, [r1, #OFFSETOF_FAULT_SEQ]

	/* grab and store fault information and system status */
	ldr.n   r4, [r3, #SCB_ICSR_OFFS]
	str     r4, [r1, #OFFSETOF_FAULT_ICSR]
	/* jump to the SHCSR */
	adds.n  r3, #SCB_SHCSR_OFFS
	ldmia   r3!, {r4-r9} /* loadb SHCSR, CFSR, HFSR, DFSR, MMFAR, BFAR */
	add     r7, r1, #OFFSETOF_FAULT_SHCSR
	stmia   r7, {r4,r5,r8,r9} /* save SHCSR, CFSR, MMFAR, BFAR */

	/* Adjust PSP to point to the fault buffer */
	add     r6, r1, #(8 * 4)
	msr     PSP, r6

	/* Call the thread exception handler */
	/* r0: krn 
	   r1: fault_rt
	   r2: errno/exception
	   r3: active thread */
	b       thinkos_krn_except_err_handler
	/* reenable interrupts */

	.align  2

.L_fault_rt: /* fault buffer */
	.word   thinkos_fault_rt
.L_cm3_scb:
	.word	CM3_SCB_BASE 
.L_krn_halt:
	.word   thinkos_krn_halt
.L_thinkos_rt:
	.word	thinkos_rt

	.size   __thinkos_krn_thread_err, . - __thinkos_krn_thread_err

#endif /* THINKOS_ENABLE_ERROR_TRAP */

#if (THINKOS_ENABLE_DBGMON)
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
	.size   cm3_debug_mon_isr, . - cm3_debug_mon_isr
#endif /* THINKOS_ENABLE_DBGMON */


	.text
	.align	2

	.thumb
	.global thinkos_krn_halt
	.thumb_func
	.type   thinkos_krn_halt, %function
thinkos_krn_halt:
	/* disable interrupts */
	cpsid   i
	b		.

	.align  2

	.size   thinkos_krn_halt, . - thinkos_krn_halt



/* --------------------------------------------------------------------------
 * ThinkOS - exception handler aliases
 * --------------------------------------------------------------------------*/

	.weak       cm3_nmi_isr
	.thumb_set  cm3_nmi_isr,thinkos_krn_halt

#if	!(THINKOS_ENABLE_HARDFAULT)
	.weak       cm3_hard_fault_isr
	.thumb_set  cm3_hard_fault_isr,thinkos_krn_halt
#endif

	.weak       cm3_mem_manage_isr
#if (THINKOS_ENABLE_MEMFAULT)
	.thumb_set  cm3_mem_manage_isr,__thinkos_krn_thread_err
#else
	.thumb_set  cm3_mem_manage_isr,thinkos_krn_halt
#endif

#if	(THINKOS_ENABLE_BUSFAULT)
	.weak       cm3_bus_fault_isr
	.thumb_set  cm3_bus_fault_isr,__thinkos_krn_thread_err
#else
	.thumb_set  cm3_bus_fault_isr,thinkos_krn_halt
#endif

	.weak       cm3_usage_fault_isr
#if (THINKOS_ENABLE_USAGEFAULT)
	.thumb_set  cm3_usage_fault_isr,__thinkos_krn_thread_err
#else
	.thumb_set  cm3_usage_fault_isr,thinkos_krn_halt
#endif

#if	!(THINKOS_ENABLE_I_CALLS) 
	.weak       cm3_except7_isr
	.thumb_set  cm3_except7_isr,thinkos_krn_halt
#else
	.extern     cm3_except7_isr
#endif

	.weak       ice_comm_blk
	.set		ice_comm_blk, __core_vectors

	.weak       cm3_except9_isr
	.thumb_set  cm3_except9_isr,thinkos_krn_halt

	.weak       cm3_except10_isr
	.thumb_set  cm3_except10_isr,thinkos_krn_halt

	.extern     cm3_svc_isr

#if !(THINKOS_ENABLE_DBGMON)
	.weak       cm3_debug_mon_isr
	.thumb_set  cm3_debug_mon_isr,thinkos_krn_halt
#endif

	.weak       cm3_except13_isr
	.thumb_set  cm3_except13_isr,thinkos_krn_halt

	.extern     cm3_pendsv_isr

	.extern     cm3_systick_isr


