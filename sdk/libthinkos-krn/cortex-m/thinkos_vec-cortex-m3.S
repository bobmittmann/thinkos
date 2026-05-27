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
#if (THINKOS_ENABLE_EXCEPTIONS)

#if (THINKOS_ENABLE_HARDFAULT)
	.global	cm3_hard_fault_isr
	.thumb_func
	.type	cm3_hard_fault_isr, %function
cm3_hard_fault_isr:
	ldr     lr, .L_xcpt_buf /* uses LR as a temporary variable */
	add     lr, #((16-3) * 4) /* make room for LR, PC and XPSR */
	stmdb   lr!, {r0-r3, r12}  /* save faulty stack frame but LR, PC and XPSR */
	stmdb   lr!, {r4-r11}      /* save faulty registers */
	adds    r2, sp, #(8 * 4)   /* save the stack frame */
	/* If the origin of error is the MSP being invalid then the 
	   next instruction will cause a hard fault. Possible solutions,
	   ignore the registers LR, PC and XPSR */
	ldr     r0, .L_thinkos_rt
	movs    r1, lr
	mrs     r3, XPSR
	/* restore SP to the top of the exception stack */
	ldr     sp, .L_xcpt_stack_top
	bl      thinkos_krn_fatal_except
.L_halt:
	b       .L_halt

__thinkos_krn_fault:
	/* ---------------------------------------------------------------
     * This routine saves the basic context of the exception:
	 * R4..R11, R0-R3, R12, LR, PC, XPSR
	 * ---------------------------------------------------------------
	 */
	ldr     lr, .L_xcpt_buf /* uses LR as a temporary variable */
	add     lr, #((16-3) * 4) /* make room for LR, PC and XPSR */
	stmdb   lr!, {r0-r3, r12}  /* save faulty stack frame but LR, PC and XPSR */
	stmdb   lr!, {r4-r11}      /* save faulty registers */
	adds    r2, sp, #(8 * 4)   /* save the stack frame */
	/* If the origin of error is the MSP being invalid then the 
	   next instruction will cause a hard fault. Possible solutions,
	   ignore the registers LR, PC and XPSR */
	ldmdb   r2, {r5-r7}  /* read LR, PC, XPSR */
	adds    r1, lr, #(16 * 4) /* top of exception struct */
	stmdb   r1, {r5-r7} /* save lr, pc, xpsr */

	ldr     r0, .L_thinkos_rt
	movs    r1, lr
	movs    r3, r7  
	/* restore SP to the top of the exception stack */
	ldr     sp, .L_xcpt_stack_top

	/* At this point the registers are:
	   R0 -> pointer to thinkos_rt
	   R1 -> pointer to the exception buffer 
	   R2 -> SP at the time of the fault 
       R3 -> XPSR at the time of fault
	   SP -> exception stack top
	 */

	ldr     r6, =__thinkos_krn_xcpt_unroll_again
	.size   cm3_hard_fault_isr, . - cm3_hard_fault_isr

	.global	__thinkos_krn_xcpt_unroll_again
	.thumb
	.thumb_func
	.type	__thinkos_krn_xcpt_unroll_again, %function

__thinkos_krn_xcpt_unroll_again:
	push    {r0-r7} /* stack frame R0-R3, R12, LR, PC, XPSR */

	bl      krn_xcpt_unroll_ipsr_get
	cmp     r0, #0
	beq     __thinkos_krn_xcpt_done

	adds    r7, r0, #(1 << 24) /* set Thumb bit on XPSR */
	str     r7, [sp, #(7 * 4)] /* push XPSR onto stack frame */
	mov     lr, #CM3_EXC_RET_HANDLER
	bx      lr

	.size   __thinkos_krn_xcpt_unroll_again, . - __thinkos_krn_xcpt_unroll_again

	.align  2
	/* exception frame */
	/*            reserved space for r12, lr, pc, xpsr */
.L_xcpt_stack_top: 
	.word   thinkos_except_stack + THINKOS_EXCEPT_STACK_SIZE

	.global	__thinkos_krn_xcpt_done
	.thumb
	.thumb_func
	.type	__thinkos_krn_xcpt_done, %function

__thinkos_krn_xcpt_done:

	ldr     r0, .L_thinkos_rt
	bl      thinkos_krn_fault_handler

	ldr     r6, =thinkos_idle_task
	mov     r7, #(1 << 24) /* Thumb bit */
	str     r6, [sp, #(6 * 4)] /* push PC onto stack frame */
	str     r7, [sp, #(7 * 4)] /* push XPSR onto stack frame */

	mov     lr, #CM3_EXC_RET_THREAD_MSP
	bx      lr

	.size   __thinkos_krn_xcpt_done, . - __thinkos_krn_xcpt_done

#endif /*( (THINKOS_ENABLE_HARDFAULT) */

#if (THINKOS_ENABLE_ERROR_TRAP)
	.global	__thinkos_krn_thread_err
	.thumb
	.thumb_func
	.type	__thinkos_krn_thread_err, %function

__thinkos_krn_thread_err:
	ldr     r1, .L_xcpt_buf
	stmia   r1!, {r4-r11} /* save R4-R11 */
	pop     {r4, r5, r6, r7, r8} /* read back r0-r3, r12 */
	mrs     r12, PSP 
	/* If the origin of error is the PSP being invalid then the 
	   next instruction will cause a hard fault. Possible solutions,
	   ignore the registers lr, pc and xpsr */

	ldr     r3, .L_cm3_scb
	ldr     r0, [r3, #SCB_CFSR_OFFS] /* Configurable Fault Status */
	tst     r0, #CFSR_MSTKERR  /* Stack error SP is invalid, reading from it
								  will cause a forced hard fault */
	bne     .L_stack_err

	/* SP is probably OK */ 
	adds    r12, #(8 * 4) /* top of faulty stack frame */
	ldmdb   r12, {r9,r10,r11}/* load from stack frame LR, PC, XPSR */
	subs    r12, #(8 * 4) 
	b       2f

.L_stack_err:
	/* Stack error detected */
	/* load dummy values to LR, PC and XPSR */
	movs    r9, #0 /* lr */
	movs    r10, #0 /* pc */
	movs    r11, #0 /* xpsr */
	/* Clear the SVC call pending to avoid returning to a service routine
	 this will cause another stack fault */
//	ldr     r0, [r3, SCB_SHCSR_OFFS]
//	bic     r0, #SCB_SHCSR_SVCALLPENDED
//	str     r0, [r3, SCB_SHCSR_OFFS]
2:
	stmia   r1!, {r4-r12} /* save R0-R3, R12, LR, PC, XPSR, SP */
	add     r3, #SCB_SHCSR_OFFS
	ldmia   r3!, {r4-r9} /* loadb SHCSR, CFSR, HFSR, DFSR, MMFAR, BFAR */
	stmia   r1, {r4,r5,r8,r9} /* save SHCSR, CFSR, MMFAR, BFAR */

	/* roll back to the exception buffer base */
	subs    r1, #OFFSETOF_XCPT_SHCSR

	strb    lr, [r1, #OFFSETOF_XCPT_RET]
	mrs     r4, CONTROL
	strb    r4, [r1, #OFFSETOF_XCPT_CONTROL]
	strb    r2, [r1, #OFFSETOF_XCPT_ERRNO]
	
	ldr     r0, .L_thinkos_rt

	ldrb    r3, [r0, #OFFSETOF_KRN_SCHED_THREAD]
	strb    r3, [r1, #OFFSETOF_XCPT_THREAD]

	ldrb    r4, [r1, #OFFSETOF_XCPT_SEQ]
	adds    r4, #1
	strb    r4, [r1, #OFFSETOF_XCPT_SEQ]

	/* Adjust the control register */
	movs    R4, #0
	msr     CONTROL, r4
	/* Synthesizes exception return */
	mov     lr, #CM3_EXC_RET_THREAD_MSP
	/* restore Idle core context */
	pop      {r4-r11}

	/* Call the thread exception handler */
	/* r0: krn 
	   r1: xcept buf
	   r2: errno
	   r3: active thread */
	b       thinkos_krn_except_err_handler

	.align  2

.L_xcpt_buf: /* exception buffer is at the bottom of the exception stack */
	.word   thinkos_except_stack
#if 0

	ldr      r0, .L_thinkos_rt
	strb     r2,  [r0, #OFFSETOF_KRN_SCHED_XCPT]

	/* rise a pending service interrupt */
	ldr      r12, .L_cm3_scb
	movs     r3, #SCB_ICSR_PENDSVSET
	str      r3, [r12, #SCB_ICSR_OFFS]
	/* should reach the PendSV (scheduler) */
	bx       lr
#endif

.L_cm3_scb:
	.word	CM3_SCB_BASE 

	.size   __thinkos_krn_thread_err, . - __thinkos_krn_thread_err
#endif /* THINKOS_ENABLE_ERROR_TRAP */

#if	(THINKOS_ENABLE_MEMFAULT)
	.align	2
	.global	cm3_mem_manage_isr
	.thumb
	.thumb_func
	.type	cm3_mem_manage_isr, %function
cm3_mem_manage_isr:
	/* disable interrupts */
	cpsid   i
#if (THINKOS_ENABLE_ERROR_TRAP)
	/* check the stack of the exception (PSP or MSP ?)*/
	tst     lr, #EXC_RET_SPSEL
	beq     __thinkos_krn_fault
	push    {r0, r1, r2, r3, r12}
	movs    r2, #THINKOS_ERR_MEM_MANAGE
	b       __thinkos_krn_thread_err
#else
	b       .
#endif /* THINKOS_ENABLE_ERROR_TRAP */
	.size  cm3_mem_manage_isr, . - cm3_mem_manage_isr
#endif

#if	(THINKOS_ENABLE_BUSFAULT)
	.thumb
	.align	2
	.global	cm3_bus_fault_isr
	.thumb
	.thumb_func
	.type	cm3_bus_fault_isr, %function
cm3_bus_fault_isr:
	/* disable interrupts */
	cpsid   i
#if (THINKOS_ENABLE_ERROR_TRAP)
	/* check the stack of the exception (PSP or MSP ?)*/
	tst     lr, #EXC_RET_SPSEL
	beq     __thinkos_krn_fault
	push    {r0, r1, r2, r3, r12}
	movs    r2, #THINKOS_ERR_BUS_FAULT
	b       __thinkos_krn_thread_err
#else
	b       .
#endif /* THINKOS_ENABLE_ERROR_TRAP */
	.size  cm3_bus_fault_isr, . - cm3_bus_fault_isr
#endif

#if	(THINKOS_ENABLE_USAGEFAULT) 
	.align	2
	.global	cm3_usage_fault_isr
	.thumb
	.thumb_func
	.type	cm3_usage_fault_isr, %function
cm3_usage_fault_isr:
#if	(THINKOS_ENABLE_KRNSVC) 
	/* Kernel services -------------------------------------------- */
	/* Check for undefined instruction flag on UFSR */
	push    {r0, r1, r2, r3, r12}
	ldr     r3, .L_cm3_scb
	ldr     r3, [r3, #SCB_CFSR_OFFS] /* Configurable Fault Status */
	tst     r3, #CFSR_UNDEFINSTR /* Undefined instruction ? */
	beq     __thinkos_krn_thread_err
	/* Code running from the process stack (non-privileged) causes a fault.
	   (Select the context stack according to the content of LR */
	tst     lr, #4
	bne     __thinkos_krn_thread_err
	/* Get the call frame stack */
	mov     r0, sp
	/* Get PC from the stack */
	ldr     r3, [r0, #(6 * 4)] 
	/* Read the Krn SVC number from the instruction */
	ldrb    r1, [r3, #0]
	/* Skip undefined instruction, by adjusting the call stack (PC = PC + 2) */
	adds    r3, r3, #2
	str     r3, [r0, #(6 * 4)] 
	/* Call the kernel service demultiplexer */
	b       thinkos_krnsvc_isr
#else
	/* disable interrupts */
	cpsid   i
#if (THINKOS_ENABLE_ERROR_TRAP)
	/* check the stack of the exception (PSP or MSP ?)*/
	tst     lr, #EXC_RET_SPSEL
	beq     __thinkos_krn_fault
	push    {r0, r1, r2, r3, r12}
	movs    r2, #THINKOS_ERR_USAGE_FAULT
	b       __thinkos_krn_thread_err
#else
	b       .
#endif /* THINKOS_ENABLE_ERROR_TRAP */
#endif

	.size  cm3_usage_fault_isr, . - cm3_usage_fault_isr
#endif

#endif /* THINKOS_ENABLE_EXCEPTIONS */



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
	.global _fault
	.thumb_func
	.type   _fault, %function
_fault:
	b		.

	.align  2

.L_thinkos_rt:
	.word	thinkos_rt

	.size   _fault, . - _fault



/* --------------------------------------------------------------------------
 * ThinkOS - exception handler aliases
 * --------------------------------------------------------------------------*/

	.weak       cm3_nmi_isr
	.thumb_set  cm3_nmi_isr,_fault

#if	!(THINKOS_ENABLE_HARDFAULT)
	.weak       cm3_hard_fault_isr
	.thumb_set  cm3_hard_fault_isr,_fault
#endif

#if	!(THINKOS_ENABLE_MEMFAULT)
	.weak       cm3_mem_manage_isr
	.thumb_set  cm3_mem_manage_isr,_fault
#endif

#if	!(THINKOS_ENABLE_BUSFAULT)
	.weak       cm3_bus_fault_isr
	.thumb_set  cm3_bus_fault_isr,_fault
#endif

#if	!(THINKOS_ENABLE_USAGEFAULT) 
	.weak       cm3_usage_fault_isr
	.thumb_set  cm3_usage_fault_isr,_fault
#endif

	.weak       cm3_except7_isr
	.thumb_set  cm3_except7_isr,_fault

	.weak       ice_comm_blk
	.set		ice_comm_blk, __core_vectors

	.weak       cm3_except9_isr
	.thumb_set  cm3_except9_isr,_fault

	.weak       cm3_except10_isr
	.thumb_set  cm3_except10_isr,_fault

	.extern     cm3_svc_isr

#if !(THINKOS_ENABLE_DBGMON)
	.weak       cm3_debug_mon_isr
	.thumb_set  cm3_debug_mon_isr,_fault
#endif

	.weak       cm3_except13_isr
	.thumb_set  cm3_except13_isr,_fault

	.extern     cm3_pendsv_isr

	.extern     cm3_systick_isr


