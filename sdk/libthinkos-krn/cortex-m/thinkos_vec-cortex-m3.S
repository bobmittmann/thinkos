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
	movs   r0, #THINKOS_ERR_HARD_FAULT
	/* Fall-through */
	.size  cm3_hard_fault_isr, . - cm3_hard_fault_isr
#endif /*( (THINKOS_ENABLE_HARDFAULT) */


	.global	__thinkos_krn_xcpt
	.thumb
	.thumb_func
	.type	__thinkos_krn_xcpt, %function

__thinkos_krn_xcpt:
/* ThinkOS except processing:

	 1. Collect the special-purpose registers
	 	- Get PRIMASK special-purpose registers
		- Disable Interrupts
		- Get FAULTMASK, BASEPRI, CONTROL and IPSR

	 2. Collect the core registers (context)

	 3. Collect memory mapped system registers
	 	- ICSR, SHCSR, CFSR, HFSR, MMFAR, SCBFAR

	 4. Collect ThinkOS kernel info
	 	- CYCCNT, CYCREF
	 	- active, ready

	 5. Invalidate current thread context
	    - Set the active thread to IDLE
	    - Set the current thread pointer to NULL

	 6. Clear the ready queue 
		- optionally disable the scheduler ? TODO: check this possibility

     7. Does the exception occurred in a thread? (RETTOBASE set on ICSR)
	
	 8. <Y> Is IDLE the current thread ?

	 9. <Y><Y> Reset the IDLE thread. 
	 	- Adjust the IDLE stack
		- Reset the IDLE context pointer
	 	- Possibly adjust the main stack MSP

	10. <Y> TODO: it is safe to call the exception handler?
		 - Do we have enough space on the stack.

	10. <Y> Call the exception handler

	11. <Y> Get the IDLE context pointer
	

	12. Return 
*/

	/* Arguments:
	     r0: except number
	   Return:
	     r0: exception buffer */

	mrs     r2, PRIMASK

/*	mov    r1, #32
	0:
	cmp    r1, 0
	bne    0b */

	/* disable interrupts */
	cpsid   i

	ldr     r1, .L_xcpt_buf
	/* update sequence count */
	ldrb    r3, [r1, #OFFSETOF_XCPT_SEQ]  /* sequence */
	/* increment sequence */
	adds    r3, #1
	strb    r3, [r1, #OFFSETOF_XCPT_SEQ]  /* sequence */

	strb    r0, [r1, #OFFSETOF_XCPT_ERRNO] /* errno */

	mrs     r3, IPSR
	strb    r3, [r1, #OFFSETOF_XCPT_IPSR] /* IPSR */

	strb    lr, [r1, #OFFSETOF_XCPT_RET] /* return (lr) */

	/* BASEPRI */
	mrs     r3, BASEPRI 
	strb    r3, [r1, #OFFSETOF_XCPT_BASEPRI] 
	/* PRIMASK/BASEPRI/FAULTMASK/CONTROL */
	mrs     r3, CONTROL
	strb    r3, [r1, #OFFSETOF_XCPT_CONTROL] 
	/* PRIMASK/FAULTMASK */
	mrs     r3, FAULTMASK
	lsls    r3, #4 
	adds    r3, r2
	strb    r3, [r1, #OFFSETOF_XCPT_PRIMASK] 

	/* save basic frame */
	movs    r2, r1
	adds    r2, #OFFSETOF_XCPT_CONTEXT
	stmia   r2!, {r4-r11} /* r4..r11 */

	mrs     r4, PSP 
	tst     lr, #EXC_RET_SPSEL
	ite     eq 
	/* TODO: This call could cause an exception if PSP is invalid */
	ldmiaeq sp!, {r5-r12} /* r0..r1, r12, lr, pc, xpsr */
	ldmiane r4!, {r5-r12} /* r0..r1, r12, lr, pc, xpsr */
	stmia   r2!, {r5-r12} /* r0..r1, r12, lr, pc, xpsr */
	mov     r5, sp
	ldr     r3, .L_thinkos_rt
	ldr     r6, [r3, #OFFSETOF_KRN_SCHED]
	ldr     r7, [r3, #OFFSETOF_KRN_READY]
	/* PSP / MSP / SCHED / READY */
	stmia   r2!, {r4, r5, r6, r7} 

	ldr     r3, .L_cm3_scb
	ldr     r4, [r3, #SCB_CFSR_OFFS] /* Configurable Fault Status */
	ldr     r5, [r3, #SCB_HFSR_OFF] /* Hard Fault Status */
	ldr     r6, [r7, #SCB_MMFAR_OFFS] /* Mem Manage Address */
	ldr     r7, [r7, #SCB_BFAR_OFFS] /* Bus Fault Address */
	stmia   r2!, {r4, r5, r6, r7} 

	ldr     r4, [r3, #SCB_ICSR_OFFS] /* Interrupt Control State */
	ldr     r5, [r3, #SCB_SHCSR_OFFS] /* System Handler Control and State */
#if (THINKOS_ENABLE_PROFILING)
#endif
	stmia   r2!, {r4, r5} 

	/* check the mode of the exception (Thread or handler)*/
	tst     lr, #EXC_RET_MODE
	beq     .L_ret_handler

	/* check the stack of the exception (PSP or MSP ?)*/
	tst     lr, #EXC_RET_SPSEL
	beq     .L_ret_msp

#if CHECK_RETTOBASE
	/* Exception on interrupt handler using PSP ? */
	tst     r4, #SCB_ICSR_RETTOBASE 
	beq     .L_no_rettobase
#endif
	/* Check for stack availability */
	mov      r3, sp
	cmp      r2, r3
	bge     .L_no_stack

	/* Adjust the control register */
	movs    R3, #0
	msr     CONTROL, r3
	/* Synthesizes exception return */
	mov     lr, #CM3_EXC_RET_THREAD_MSP
	/* Call the thread exception handler */
	b       thinkos_krn_thread_except

.L_ret_msp:
	mov     r4, #THINKOS_ERR_KRN_RETMSP

.L_reset_idle:
	/* Reset the exception stack */
	bl      thinkos_krn_idle_reset
	mov     sp, r0
	ldr     r0, .L_xcpt_buf
	mov     r1, r4
	/* Restore idle core context */
	pop     {r4-r11}
	/* Synthesizes exception return */
	mov     lr, #CM3_EXC_RET_THREAD_MSP
	/* Call the fatal exception handler */
	b       thinkos_krn_fatal_except

.L_no_stack:
	mov     r4, #THINKOS_ERR_KRN_STACKOVF
	b       .L_reset_idle

.L_ret_handler:
	mov     r4, #THINKOS_ERR_KRN_RETHANDLER
	b       .L_reset_idle


#if CHECK_RETTOBASE
.L_no_rettobase:
	mov     r4, #THINKOS_ERR_KRN_NORETTOBASE    
	b       .L_reset_idle
#endif

#if 0
	/* Load the idle context pointer from the context vector */
	ldr     r6, .L_thinkos_rt
	ldr     r7, [r6, #THINKOS_THREAD_IDLE * 4]

	/* Restore idle core context */
	mov     r6, sp
	cmp     r7, r6
	bne     .L_idle_unstack

.L_idle_unstack:
	mov     r4, #THINKOS_ERR_KRN_UNSTACK
	b       .L_reset_idle
#endif


	.align  2
.L_xcpt_buf: /* exception buffer is at the bottom of the exception stack */
	.word   thinkos_except_stack
.L_thinkos_rt:
	.word	thinkos_rt
.L_cm3_scb:
	.word	CM3_SCB_BASE 
.L_cm3_dwt:
	.word   CM3_DWT_BASE    /* DWT Base Address */

	.size   __thinkos_krn_xcpt, . - __thinkos_krn_xcpt

#if	(THINKOS_ENABLE_BUSFAULT)
	.thumb
	.align	2
	.global	cm3_bus_fault_isr
	.thumb
	.thumb_func
	.type	cm3_bus_fault_isr, %function
cm3_bus_fault_isr:
	movs   r0, #THINKOS_ERR_BUS_FAULT
	b      __thinkos_krn_xcpt
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
	movs   r0, #THINKOS_ERR_USAGE_FAULT
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
	b       __thinkos_krn_xcpt
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
	movs   r0, #THINKOS_ERR_MEM_MANAGE
	b      __thinkos_krn_xcpt
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


