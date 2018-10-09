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
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_EXCEPT__
#include <thinkos/except.h>
#define __THINKOS_IDLE__
#include <thinkos/idle.h>
#include <thinkos.h>

#include <stdio.h>
#include <string.h>

extern const char thinkos_svc_nm[];
extern const char thinkos_sch_nm[];
extern const char thinkos_xcp_nm[];
extern const char thinkos_vec_nm[];
extern const char thinkos_irq_nm[];

void __thinkos_kill_all(void) 
{
	int wq;

	/* clear all wait queues */
	for (wq = 0; wq < THINKOS_WQ_LST_END; ++wq) 
		thinkos_rt.wq_lst[wq] = 0;

#if THINKOS_ENABLE_THREAD_VOID 
	/* discard current thread context */
	thinkos_rt.active = THINKOS_THREAD_VOID;
#else
	DCC_LOG(LOG_PANIC, "can't set current thread to void!"); 
#endif
	/* signal the scheduler ... */
	__thinkos_defer_sched();
}

#define __PRIORITY(OPT)   (((OPT) >> 16) & 0xff)
#define __ID(OPT)         (((OPT) >> 24) & 0x7f)
#define __PAUSED(OPT)     (((OPT) >> 31) & 0x01)
#define __STACK_SIZE(OPT) ((OPT) & 0xffff)


static int __thinkos_init_main(struct thinkos_context *ctx, uint32_t opt)
{
#if THINKOS_ENABLE_TIMESHARE
	int priority = __PRIORITY(opt);
#endif
	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	int id = __ID(opt) - 1;

	if (id < 0)
		id = 0;
#if THINKOS_ENABLE_THREAD_ALLOC
	/* alloc main thread */
	id = thinkos_alloc_lo(thinkos_rt.th_alloc, id);
#else
	if (id >= THINKOS_THREADS_MAX)
		id = THINKOS_THREADS_MAX - 1;
#endif

	/* initialize the main thread */ 

#if THINKOS_ENABLE_THREAD_INFO
	thinkos_rt.th_inf[id] = (struct thinkos_thread_inf *)&thinkos_main_inf;
#endif

#if THINKOS_ENABLE_TIMESHARE

#if THINKOS_SCHED_LIMIT_MIN < 1
#error "THINKOS_SCHED_LIMIT_MIN must be at least 1"
#endif

#if THINKOS_SCHED_LIMIT_MAX < THINKOS_SCHED_LIMIT_MIN
#error "THINKOS_SCHED_LIMIT_MAX < THINKOS_SCHED_LIMIT_MIN !!!"
#endif

	if (priority > THINKOS_SCHED_LIMIT_MAX)
		priority = THINKOS_SCHED_LIMIT_MAX;

	thinkos_rt.sched_pri[id] = priority;
	thinkos_rt.sched_val[id] = priority / 2;

	/* set the initial schedule limit */
	thinkos_rt.sched_limit = priority;
	if (thinkos_rt.sched_limit < THINKOS_SCHED_LIMIT_MIN)
		thinkos_rt.sched_limit = THINKOS_SCHED_LIMIT_MIN;
#endif /* THINKOS_ENABLE_TIMESHARE */

	/* set the active thread */
	thinkos_rt.active = id;
	thinkos_rt.wq_ready = 1 << id;

	DCC_LOG3(LOG_TRACE, "<%d> threads_max=%d ready=%08x", 
			 id, THINKOS_THREADS_MAX, thinkos_rt.wq_ready);

#if THINKOS_ENABLE_PAUSE
	if (__PAUSED(opt)) {
		/* insert into the paused list */
		thinkos_rt.wq_paused = 1 << id;
	} 
#endif

	thinkos_rt.ctx[id] = ctx;

#if 0
	/* Invoke the scheduler */
	__thinkos_defer_sched();
#endif

	return id;
}

int thinkos_init(uint32_t opt)
{
	struct thinkos_context * ctx;
	uint32_t sp;
	int thread_id;

#if (THINKOS_MUTEX_MAX > 0)
	DCC_LOG3(LOG_INFO, "    mutex: %2d (%2d .. %2d)", THINKOS_MUTEX_MAX,
			 THINKOS_MUTEX_BASE,
			 THINKOS_MUTEX_BASE + THINKOS_MUTEX_MAX - 1);
#endif
#if (THINKOS_COND_MAX > 0)
	DCC_LOG3(LOG_INFO, "  condvar: %2d (%2d .. %2d)", THINKOS_COND_MAX,
			 THINKOS_COND_BASE,
			 THINKOS_COND_BASE + THINKOS_COND_MAX - 1);
#endif
#if (THINKOS_SEMAPHORE_MAX > 0)
	DCC_LOG3(LOG_INFO, "semaphore: %2d (%2d .. %2d)", THINKOS_SEMAPHORE_MAX,
			 THINKOS_SEM_BASE,
			 THINKOS_SEM_BASE + THINKOS_SEMAPHORE_MAX - 1);
#endif
#if (THINKOS_EVENT_MAX > 0)
	DCC_LOG3(LOG_INFO, "    evset: %2d (%2d .. %2d)", THINKOS_EVENT_MAX,
			 THINKOS_EVENT_BASE,
			 THINKOS_EVENT_BASE + THINKOS_EVENT_MAX - 1);
#endif
#if (THINKOS_FLAG_MAX > 0)
	DCC_LOG3(LOG_INFO, "     flag: %2d (%2d .. %2d)", THINKOS_FLAG_MAX,
			 THINKOS_FLAG_BASE,
			 THINKOS_FLAG_BASE + THINKOS_FLAG_MAX - 1);
#endif
#if (THINKOS_GATE_MAX > 0)
	DCC_LOG3(LOG_INFO, "     gate: %2d (%2d .. %2d)", THINKOS_GATE_MAX,
			 THINKOS_GATE_BASE,
			 THINKOS_GATE_BASE + THINKOS_GATE_MAX - 1);
#endif
#if THINKOS_ENABLE_JOIN
	DCC_LOG3(LOG_INFO, "     join: %2d (%2d .. %2d)", THINKOS_THREADS_MAX,
			 THINKOS_JOIN_BASE,
			 THINKOS_JOIN_BASE + THINKOS_THREADS_MAX - 1);
#endif
#if THINKOS_ENABLE_CONSOLE
	DCC_LOG2(LOG_INFO, "  console: (wr:%2d rd:%2d)", 
			THINKOS_WQ_CONSOLE_WR,  THINKOS_WQ_CONSOLE_RD); 
#endif
#if THINKOS_ENABLE_PAUSE
	DCC_LOG1(LOG_INFO, "   paused: (%2d)", THINKOS_WQ_PAUSED); 
#endif
#if THINKOS_ENABLE_JOIN
	DCC_LOG1(LOG_INFO, " canceled: (%2d)", THINKOS_WQ_CANCELED); 
#endif
#if THINKOS_ENABLE_DEBUG_FAULT
	DCC_LOG1(LOG_INFO, "    fault: (%2d)", THINKOS_WQ_FAULT); 
#endif

	/* disable interrupts */
	cm3_cpsid_i();

#if THINKOS_ENABLE_STACK_INIT
	/* initialize exception stack */
	__thinkos_memset32(thinkos_except_stack, 0xdeadbeef, 
					   sizeof(thinkos_except_stack));
#endif

	__thinkos_core_reset();

	DCC_LOG1(LOG_MSG, "thinkos_rt=@%08x", &thinkos_rt);


	/* Cortex-M configuration */
	DCC_LOG(LOG_INFO, "Cortex-M configuration:"); 

#ifdef DEBUG
	{
		uint32_t ctrl = cm3_control_get();

		DCC_LOG4(LOG_INFO, "CTRL=%02x { nPRIV=%d SPSEL=%d FPCA=%d }",
				 ctrl, 
				 ctrl & CONTROL_nPRIV ? 1 : 0,
				 ctrl & CONTROL_SPSEL? 1 : 0,
				 ctrl & CONTROL_FPCA? 1 : 0);
	}
#endif

	DCC_LOG(LOG_TRACE, "1. SCB->SCR"); 
	/* System Control Register
	   The SCR controls features of entry to and exit from low power state. */
	CM3_SCB->scr = 0; 

	DCC_LOG(LOG_INFO, "2. SCB->CCR"); 
	/* Configuration and Control Register
		The CCR controls entry to Thread mode and enables:
		- the handlers for NMI, hard fault and faults escalated by FAULTMASK 
		  to ignore BusFaults
		- trapping of divide by zero and unaligned accesses
		- access to the STIR by unprivileged software.
		*/
#if THINKOS_ENABLE_ALIGN
	CM3_SCB->ccr = SCB_CCR_USERSETMPEND | SCB_CCR_STKALIGN | \
				   SCB_CCR_UNALIGN_TRP;
#else
	CM3_SCB->ccr = SCB_CCR_USERSETMPEND;
#endif
//		| SCB_CCR_NONBASETHRDENA;
	/*  NONBASETHRDENA
		Indicates how the processor enters Thread mode:
		0 = processor can enter Thread mode only when no exception is active.
		1 = processor can enter Thread mode from any level under the 
		control of an
		EXC_RETURN value, see Exception return on page 2-28. */

	/* Configure FPU */
#if (THINKOS_ENABLE_FPU) 
	DCC_LOG(LOG_INFO, ".. FPU"); 
#if (THINKOS_ENABLE_FPU_LS)
#error "FP lazy context save unsupported!"
	/* FIXME: Enable FP lazy context save */
	CM3_SCB->fpccr = SCB_FPCCR_ASPEN | SCB_FPCCR_LSPEN;
#else
	/* Enable automatic FP context save */
	CM3_SCB->fpccr = SCB_FPCCR_ASPEN;
#endif
	/* Enable FPU access */
	CM3_SCB->cpacr |= CP11_SET(3) | CP10_SET(3);
#endif

	DCC_LOG(LOG_INFO, "3. PSP"); 
	/* Configure the thread stack ?? 
	   If we already using the PSP nothing changes, if on the other 
	   hand MSP is our stack we will move it to PSP 
	   and adjust the CONTROL register accordingly. */
	sp = cm3_sp_get();
	cm3_psp_set(sp);

	DCC_LOG(LOG_INFO, "5. CONTROL"); 
	/* configure the use of PSP in thread mode */
	cm3_control_set(CONTROL_THREAD_PSP | CONTROL_THREAD_PRIV);

	ctx = (struct thinkos_context *)sp - 1;
	if ((thread_id = __thinkos_init_main(ctx, opt)) < 0)
		return thread_id;

	/* everything good with the main thread, we need to configure 
	   idle thread and exceptions. ... */

	DCC_LOG(LOG_INFO, "6. Idle thread"); 
	__thinkos_idle_init();

	/* configure the main stack */
	cm3_msp_set((uintptr_t)__thinkos_xcpt_stack_top());

#if THINKOS_ENABLE_EXCEPTIONS
	thinkos_exception_init();
#endif

	DCC_LOG4(LOG_TRACE, "<%d> MSP=%08x PSP=%08x CTRL=%02x", 
			 thread_id, cm3_msp_get(), cm3_psp_get(), cm3_control_get());


	DCC_LOG(LOG_INFO, "7. enabling interrupts!");
	cm3_cpsie_i();

	return thread_id + 1;
}

extern int _stack;
#define THINKOS_MAIN_STACK_TOP ((uint32_t *)&_stack)
#ifndef THINKOS_MAIN_STACK_SIZE
  #define THINKOS_MAIN_STACK_SIZE 4096
#endif

#if THINKOS_ENABLE_THREAD_INFO
uint32_t * const thinkos_main_stack = THINKOS_MAIN_STACK_TOP;

const struct thinkos_thread_inf thinkos_main_inf = {
	.tag = "MAIN",
	.stack_ptr = THINKOS_MAIN_STACK_TOP - THINKOS_MAIN_STACK_SIZE / 4,
	.stack_size = THINKOS_MAIN_STACK_SIZE,
	.priority = 0,
	.thread_id = 1,
	.paused = 0
};
#endif


const char * const thinkos_svc_link = thinkos_svc_nm;
const char * const thinkos_xcp_link = thinkos_xcp_nm;
const char * const thinkos_vec_link = thinkos_vec_nm;
const char * const thinkos_sch_link = thinkos_sch_nm;
const char * const thinkos_irq_link = thinkos_irq_nm;

