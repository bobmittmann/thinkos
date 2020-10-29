/* 
 * monitor_swap.S
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

/* --------------------------------------------------------------------------
 * ThinkOS - debug monitor context swap
 * --------------------------------------------------------------------------*/

/**
 * Monitor Scheduler
 *
 *
 * The monitor runs as special thread in the system. With a priority higher 
 * than all the low level interrupts.
 *
 *
 * The context/semaphore variable indicates the state of this thread:
 *
 * - 0 BUSY The thread is either active or was not initialized
 * - 1 WAITING For a signal or sleep
 * - x SLEEPING Any other value other than 0 or 1. The variable is a 
 *     pointer to the monitor context.
 *
 * The Systick IRQ updates the clock if needed then execute the Monitor
 * scheduler. Depending on the monitor state:  
 *
 * - BUSY Just return does nothing.
 *
 * - WAITING - if there are no pending events, saves the ``Monitor thread``
 *   context (PSP), set the ``semaphore`` to PSP, restores the 
 * ``Handler mode`` context (MSP) switch to ``Handler mode``.
 *   When peding events do exist set the monitor to BUSY (0) and returns.
 *
 * - SLEEPING - if no pending events just return, otherwise saves the 
 *   ``Handler mode`` context (MSP), restore the ``Monitor thread`` context 
 *    switch to ```privileged Thread mode``.
 *
 *
 * Note: This mechanism depends on the NONBASETHRDENA bit to be enabled on the 
 * System Control Register (SCR).
 *
 * NONBASETHRDENA, bit[0] - Controls whether the processor can enter Thread 
 * mode with exceptions active:
 *
 * - 0 Any attempt to enter Thread mode with exceptions active faults.
 * - 1 The processor can enter Thread mode with exceptions active because 
 *   of a controlled return value. 
 *
 *
 *  Handler mode context
 *  --------------------
 *
 *    CTRL
 *      R4
 *      R5
 *      R6
 *      R7
 *      R8
 *      R9
 *     R10
 *     R11
 *     PSP
 *     RET
 *      R0
 *      R1
 *      R2
 *      R3
 *     R12
 *      LR
 *      PC
 *    XPSR
 *
 *
 *
 * 
 *  Monitor thread context
 *  ----------------------
 *
 *      R4
 *      R5
 *      R6
 *      R7
 *      R8
 *      R9
 *     R10
 *     R11
 *      R0
 *      R1
 *      R2
 *      R3
 *     R12
 *      LR
 *      PC
 *    XPSR
 *
 *
 */

#define CONTROL_nPRIV      (1 << 0)
#define CONTROL_SPSEL      (1 << 1)
#define CONTROL_FPCA       (1 << 2)

#define EXC_RETURN_THREAD  (1 << 1)
#define EXC_RETURN_SPSEL   (1 << 2)
#define EXC_RETURN_nFPCA   (1 << 4)

	.syntax unified
	.cpu cortex-m3

	.text
	.align	2
	.global	__thinkos_monitor_sched
	.global	__thinkos_monitor_sched
	.thumb
	.thumb_func
__thinkos_monitor_sched:

	/* r0: thinkos_rt.monitor */
	ldr      r1, [r0]
	/* r1: monitor context pointer / semaphore */
	/* BUSY ? */
	cbz      r1, .L_return /* BUSY */

	/* get the events and mask bitmap */
	ldrd     r2, r3, [r0, #MONITOR_EVS_OFFS]
	/* r2: monitor events bitmap */
	/* r3: monitor mask */
	/* Apply the event mask */
	ands     r2, r3

	/* WAITING ? */
	cmp      r1, #1
	beq      __monitor_waiting
	/* SLEEPING */
__monitor_is_sleeping:
	/* Check for any pending unmasked event */
	cbz      r2, .L_return
__monitor_wakeup:

#if 0
    mov      r3, #1
1:	cmp      r3, #0
	bne      1b
#endif


    /* The processor at this point is executing an interrupt handler. 
	   Save at MSP the current execution environment. */
	mrs      r12, PSP
	mrs      r3, CONTROL
	push     {r3, r4-r12, lr}
	/* restore Monitor context */
	ldmia    r1!, {r4-r11}
	/* restore PSP stack pointer */
	msr      PSP, r1
	/* set the control register */
	mov      r1, #0
	msr      CONTROL, r1
	isb
	/* synthesizes exception return */
	mov      lr, #CM3_EXC_RET_THREAD_PSP
	/* set to BUSY */
	str      r1, [r0]
.L_return:
	bx       lr

__monitor_waiting:
	/* Check for any pending unmasked event */
	cbnz     r2, .L_set_busy_and_return

.L_go_to_sleep:
#if 0
    mov      r2, #1
1:	cmp      r2, #0
	bne      1b
#endif

	mrs      r1, PSP
	/* save Monitor context */
	stmdb    r1!, {r4-r11}
	/* set to SLEEPING */
	str      r1, [r0]
	/* restore Handler context */
	pop      {r3, r4-r12, lr}
	msr      CONTROL, r3
	isb
	msr      PSP, r12
	bx       lr

.L_set_busy_and_return:
	mov      r1, #0
	str      r1, [r0]
	bx       lr

	.size   __thinkos_monitor_sched , . - __thinkos_monitor_sched 

