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

	/* select the context stack according to the content of LR */
	tst     lr, #EXC_RET_SPSEL
	ite     eq
	mrseq   r12, MSP
	mrsne   r12, PSP

__thinkos_krn_fault:
	/* ---------------------------------------------------------------
     *
	 * ---------------------------------------------------------------
	 */
	ldr     sp, .L_xcpt_stack_top
	sub     sp, #(4 * 4) /* make room for r12, lr, pc, zpsr */
	push    {r0-r3}
	adds    r12, #(8 * 4) /* top of faulty stack frame */
	ldmdb   r12!, {r0-r3}  /* get r12, lr, pc, xpsr */
	push    {r4-r11} /* save faulty registers */
	adds    r4, sp, #(12 * 4)
	stmia   r4!, {r0-r3} /* save r12, lr, pc, xpsr */

	/* Adjust the control register */
	movs    r0, #0
	msr     CONTROL, r0
	/* pointer to the kernel core block */
	ldr     r0, .L_thinkos_rt
	/* pointer to the exception stack frame */
	mov     r1, sp
	/* get the faulty sp */
	subs    r2, r12, #(4 * 4)
	/* new UNROLL stack frame */
	ldr     r5, =__thinkos_krn_xcpt_done
	ldr     r6, =__thinkos_krn_xcpt_unroll_again
	push    {r0-r7} /* r0-r3, r12, lr, pc, xpsr */
	b       __thinkos_krn_xcpt_unroll

	.size   cm3_hard_fault_isr, . - cm3_hard_fault_isr

#endif /*( (THINKOS_ENABLE_HARDFAULT) */

	.global	__thinkos_krn_xcpt_unroll_again
	.thumb
	.thumb_func
	.type	__thinkos_krn_xcpt_unroll_again, %function

__thinkos_krn_xcpt_unroll_again:
	/* reuse same stack frame */
	sub     sp, #(8 * 4)
__thinkos_krn_xcpt_unroll:
	bl      krn_xcpt_unroll_ipsr_get
	cbz.n   r0, .L_return_to_base
	/* set Thumb bit */
	adds    r0, #(1 << 24)
	/* push XPSR onto stack frame */
	str     r0, [sp, #(7 * 4)]
	mov     lr, #CM3_EXC_RET_HANDLER
	bx      lr
.L_return_to_base:
	ldr     r6, =thinkos_krn_fatal_except
	mov     r7, #(1 << 24) /* Thumb bit */
	str     r6, [sp, #(6 * 4)]
	str     r7, [sp, #(7 * 4)]
	/* Synthesizes exception return */
	mov     lr, #CM3_EXC_RET_THREAD_MSP
	bx      lr
	.size   __thinkos_krn_xcpt_unroll_again, . - __thinkos_krn_xcpt_unroll_again

	.global	__thinkos_krn_xcpt_done
	.thumb
	.thumb_func
	.type	__thinkos_krn_xcpt_done, %function

__thinkos_krn_xcpt_done:
	ldr     sp, .L_xcpt_stack_top
	b       __thinkos_krn_xcpt_done

	.align  2
	/* exception frame */
	/*            reserved space for r12, lr, pc, xpsr */
.L_xcpt_stack_top: 
	.word   thinkos_except_stack + THINKOS_EXCEPT_STACK_SIZE

	.size   __thinkos_krn_xcpt_done, . - __thinkos_krn_xcpt_done


	.global	__thinkos_krn_thread_err
	.thumb
	.thumb_func
	.type	__thinkos_krn_thread_err, %function

__thinkos_krn_thread_err:

	ldr     r1, .L_xcpt_buf
	stmia   r1!, {r4-r11} /* save */
	mrs     r12, PSP 
	ldmia   r12!, {r4-r11}/* read r0-r4, r12, lr, pc, xpsr */
	stmia   r1!, {r4-r12} /* save r0-r4, r12, lr, pc, xpsr, sp */

	/* roll back to the exception buffer base */
	subs    r1, #(17 * 4)

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
	/* Call the thread exception handler */
	b       thinkos_krn_thread_except


	.align  2
.L_xcpt_buf: /* exception buffer is at the bottom of the exception stack */
	.word   thinkos_except_stack
.L_thinkos_rt:
	.word	thinkos_rt
.L_cm3_scb:
	.word	CM3_SCB_BASE 

	.size   __thinkos_krn_thread_err, . - __thinkos_krn_thread_err

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
	/* check the stack of the exception (PSP or MSP ?)*/
	tst     lr, #EXC_RET_SPSEL
	beq     __thinkos_krn_fault
	movs   r2, #THINKOS_ERR_BUS_FAULT
	b       __thinkos_krn_thread_err
	.size  cm3_bus_fault_isr, . - cm3_bus_fault_isr
#endif

#define CFSR_UNDEFINSTR (UFSR_UNDEFINSTR << 16)

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
	ldr     r3, .L_cm3_scb
	ldr     r3, [r3, #SCB_CFSR_OFFS] /* Configurable Fault Status */
	tst     r3, #CFSR_UNDEFINSTR /* Undefined instruction ? */
	beq     __thinkos_krn_xcpt
	/* Code running from the process stack (non-privileged) causes a fault.
	   (Select the context stack according to the content of LR */
	tst     lr, #4
	bne     __thinkos_krn_xcpt
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
	cpsid   i
	/* check the stack of the exception (PSP or MSP ?)*/
	tst     lr, #EXC_RET_SPSEL
	beq     __thinkos_krn_fault
	movs    r2, #THINKOS_ERR_USAGE_FAULT
	b       __thinkos_krn_thread_err
#endif

	.size  cm3_usage_fault_isr, . - cm3_usage_fault_isr
#endif

#if	(THINKOS_ENABLE_MEMFAULT)
	.align	2
	.global	cm3_mem_manage_isr
	.thumb
	.thumb_func
	.type	cm3_mem_manage_isr, %function
cm3_mem_manage_isr:
	/* disable interrupts */
	cpsid   i
	/* check the stack of the exception (PSP or MSP ?)*/
	tst     lr, #EXC_RET_SPSEL
	beq     __thinkos_krn_fault
	movs    r2, #THINKOS_ERR_MEM_MANAGE
	b       __thinkos_krn_thread_err

	.size  cm3_mem_manage_isr, . - cm3_mem_manage_isr
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

#if	!(THINKOS_ENABLE_USAGEFAULT) 
	.weak       cm3_bus_fault_isr
	.thumb_set  cm3_bus_fault_isr,_fault
#endif

#if	!(THINKOS_ENABLE_BUSFAULT)
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

#if (THINKOS_ENABLE_CLOCK) || (THINKOS_ENABLE_MONITOR)
	.extern     cm3_systick_isr
#else
	.weak       cm3_systick_isr
	.thumb_set  cm3_systick_isr,_fault
#endif


