/* 
 * thinkos_init.c
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

#include "thinkos_krn-i.h"

#if (THINKOS_ENABLE_THREAD_INFO)
const struct thinkos_thread_inf thinkos_main_inf = {
	.tag = "MAIN",
	.stack_ptr = &__krn_stack_start,
	.stack_size = (uintptr_t)&__krn_stack_size,
	.priority = 0,
	.thread_id = 1,
	.privileged = 0,
	.paused = 0
};
#endif

static int __thinkos_init_main(struct thinkos_rt * krn, uintptr_t sp, 
							   uint32_t opt)
{
#if THINKOS_ENABLE_TIMESHARE
	int priority = __PRIORITY(opt);
#endif
	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	int idx = __ID(opt) - 1;

#if (THINKOS_ENABLE_STACK_LIMIT)
	uintptr_t sl = (uintptr_t)&__krn_stack_start;
#endif

	if (idx < 0)
		idx = 0;

#if THINKOS_ENABLE_THREAD_ALLOC
	/* alloc main thread */
	idx = __thinkos_thread_alloc(idx);
#else
	if (idx >= THINKOS_THREADS_MAX)
		idx = THINKOS_THREADS_MAX - 1;
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

	krn->sched_pri[idx] = priority;
	krn->sched_val[idx] = priority / 2;

	/* set the initial schedule limit */
	krn->sched_limit = priority;
	if (krn->sched_limit < THINKOS_SCHED_LIMIT_MIN)
		krn->sched_limit = THINKOS_SCHED_LIMIT_MIN;
#endif /* THINKOS_ENABLE_TIMESHARE */

	DCC_LOG3(LOG_TRACE, "<%2d> threads_max=%d ready=%08x", 
			 idx + 1, THINKOS_THREADS_MAX, krn->wq_ready);

#if (THINKOS_ENABLE_STACK_LIMIT)
	__thread_sl_set(krn, idx, (uintptr_t)sl);
	DCC_LOG1(LOG_TRACE, " sl=%08x", __thread_sl_get(krn, idx));
#endif


#if (THINKOS_ENABLE_THREAD_INFO)
	__thread_inf_set(krn, idx, (struct thinkos_thread_inf *)
					 &thinkos_main_inf);
#endif

	return idx;
}

int thinkos_krn_init(unsigned int opt, const struct thinkos_mem_map * map,
					 const struct thinkos_thread_initializer * lst[])
{
	struct thinkos_rt * krn = &thinkos_rt;
	int thread_idx;
	uint32_t ctrl;
	uint32_t ccr;
	uintptr_t sp;

	/* Static sanity check: */

	_Static_assert (offsetof(struct thinkos_rt, idle_ctx) == 
					THINKOS_RT_IDLE_CTX_OFFS, "THINKOS_RT_IDLE_CTX_OFFS");

#if (THINKOS_ENABLE_PROFILING)
	_Static_assert (offsetof(struct thinkos_rt, cycref) == 
					THINKOS_RT_CYCREF_OFFS, "THINKOS_RT_CYCREF_OFFS");

	_Static_assert (offsetof(struct thinkos_rt, th_cyc) == 
					THINKOS_RT_CYCCNT_OFFS, "THINKOS_RT_CYCCNT_OFFS");
#endif

#if (THINKOS_ENABLE_DEBUG_BKPT)
#if (THINKOS_ENABLE_DEBUG_STEP)
	_Static_assert (offsetof(struct thinkos_rt, step_req) == 
					THINKOS_RT_STEP_REQ_OFFS, "THINKOS_RT_STEP_REQ_OFFS");
	_Static_assert (offsetof(struct thinkos_rt, step_svc) == 
					THINKOS_RT_STEP_SVC_OFFS, "THINKOS_RT_STEP_SVC_OFFS");
#endif
	_Static_assert (offsetof(struct thinkos_rt, xcpt_ipsr) == 
					THINKOS_RT_XCPT_IPSR_OFFS, "THINKOS_RT_XCPT_IPSR_OFFS");
	_Static_assert (offsetof(struct thinkos_rt, step_id) == 
					THINKOS_RT_STEP_ID_OFFS, "THINKOS_RT_STEP_ID_OFFS");
	_Static_assert (offsetof(struct thinkos_rt, brk_idx) == 
					THINKOS_RT_BREAK_ID_OFFS, "THINKOS_RT_BREAK_ID_OFFS");
#endif

#if (THINKOS_ENABLE_CRITICAL)
	_Static_assert (offsetof(struct thinkos_rt, critical_cnt) == 
					THINKOS_RT_CRITCNT_OFFS, "THINKOS_RT_CRITCNT_OFFS");
#endif


	_Static_assert (offsetof(struct thinkos_rt, active) == 
					THINKOS_RT_ACTIVE_OFFS, "THINKOS_RT_ACTIVE_OFFS");

#if (THINKOS_ENABLE_RUNMASK)
	_Static_assert (offsetof(struct thinkos_rt, runmask) == 
					THINKOS_RT_RUNMASK_OFFS, "THINKOS_RT_RUNMASK_OFFS");
#endif

	_Static_assert (offsetof(struct thinkos_rt, wq_ready) == 
					THINKOS_RT_READY_OFFS, "THINKOS_RT_READY_OFFS");

#if (THINKOS_ENABLE_STACK_LIMIT)
	_Static_assert (offsetof(struct thinkos_rt, th_sl) == 
					THINKOS_RT_TH_SL_OFFS, "THINKOS_RT_TH_SL_OFFS");
#endif

	/* disable interrupts */
	thinkos_krn_irq_off();

	/* sanity check */
#if (THINKOS_ENABLE_SANITY_CHECK)
	ctrl = cm3_control_get();
	if (ctrl & CONTROL_nPRIV) {
		DCC_LOG(LOG_PANIC, "0. Non privileged ...");
		return THINKOS_EFAULT; 
	}

	if (ctrl & CONTROL_SPSEL) {
		DCC_LOG(LOG_PANIC, "0. Using PSP ...");
		return THINKOS_EFAULT; 
	}
#endif

#if (THINKOS_ENABLE_MEMORY_CLEAR)
	DCC_LOG(LOG_TRACE, "1. cleanup memory().");
	/* clear the ThinkOS runtime structure */
	__thinkos_memset32(krn, 0, sizeof(struct thinkos_rt));  
#endif

#if (THINKOS_ENABLE_UDELAY_CALIBRATE)
	DCC_LOG(LOG_TRACE, "1. thinkos_krn_udelay_calibrate().");
	thinkos_krn_udelay_calibrate();
#endif
//	DCC_LOG1(LOG_TRACE, "udelay_factor=%d.", udelay_factor);

	__thinkos_krn_core_init(krn);

	DCC_LOG1(LOG_MSG, "thinkos_rt=@%08x", krn);

	/* adjust exception priorities */
	/*
	 *  0x00 - low latency interrupts
     *
	 *  0x20 - high priority interrupts
	 *  0x40   .
	 *
	 *  0x80 - regular priority interrupts
	 *  0xe0 - very low priority interrupts

	 *  0xa0 - SysTick
	 *  0x60 - SVC
	 *  0xc0 - PendSV (scheduler)
	 */

	/* SysTick interrupt has to have a lower priority */ 
	cm3_except_pri_set(CM3_EXCEPT_SYSTICK, CLOCK_PRIORITY);

	/* SVC should not be preempted by the scheduler, thus it runs 
	   at same priority. */
	cm3_except_pri_set(CM3_EXCEPT_SVC, SYSCALL_PRIORITY);

	/* PendSV interrupt has to have the lowest priority among
	   regular interrupts (higher number) */
	cm3_except_pri_set(CM3_EXCEPT_PENDSV, SCHED_PRIORITY);

#if (THINKOS_ENABLE_DEBUG)
	cm3_except_pri_set(CM3_EXCEPT_DEBUG_MONITOR, DEBUGGER_PRIORITY);
#endif

	/* Cortex-M configuration */
	DCC_LOG(LOG_INFO, "Cortex-M configuration:"); 

	/* System Control Register
	   The SCR controls features of entry to and exit from low power state. */
	CM3_SCB->scr = 0; 
	DCC_LOG1(LOG_INFO, "SCB->SCR=0x%08x", CM3_SCB->scr); 

	/* Configuration and Control Register
		The CCR controls entry to Thread mode and enables:
		- the handlers for NMI, hard fault and faults escalated by FAULTMASK 
		  to ignore BusFaults
		- trapping of divide by zero and unaligned accesses
		- access to the STIR by unprivileged software.
		*/
	ccr = 0;

/*	NONBASETHRDENA, bit[0]
    Controls whether the processor can enter Thread mode with 
	exceptions active:
    - 0 Any attempt to enter Thread mode with exceptions active faults.
    - 1 The processor can enter Thread mode with exceptions active because 
	of a controlled return value. */
#if (THINKOS_ENABLE_MONITOR_SCHED)
	ccr |= SCB_CCR_NONBASETHRDENA;
#endif

/*	USERSETMPEND, bit[1]
    Controls whether unprivileged software can access the Software 
	Triggered Interrupt Register (STIR):
    - 0 Unprivileged software cannot access the STIR.
    - 1 Unprivileged software can access the STIR. */
#if 0
	/* FIXME: disable this by default... */
	ccr |= SCB_CCR_USERSETMPEND;
#endif

#if (THINKOS_ENABLE_STACK_ALIGN)
	ccr |= SCB_CCR_STKALIGN;
/*	Determines whether the exception entry sequence guarantees 8-byte 
	stack frame alignment, adjusting the SP if necessary before saving state:
    - 0 Guaranteed SP alignment is 4-byte, no SP adjustment is performed.
    - 1 8-byte alignment guaranteed, SP adjusted if necessary. */

#endif
#if (THINKOS_ENABLE_UNALIGN_TRAP)
	ccr |= SCB_CCR_UNALIGN_TRP;
/* Controls the trapping of unaligned word or halfword accesses:
   - 0 Trapping disabled.
   - 1 Trapping enabled.
   Unaligned load-store multiples and word or halfword exclusive accesses 
   always fault. */
#endif
#if (THINKOS_ENABLE_DIV0_TRAP)
	ccr |= SCB_CCR_DIV_0_TRP;
#endif

	DCC_LOG1(LOG_INFO, "SCB->CCR=0x%08x", ccr); 

	CM3_SCB->ccr = ccr;

	/* Configure FPU */
#if (THINKOS_ENABLE_FPU) 
	DCC_LOG(LOG_INFO, ".. FPU"); 
#if (THINKOS_ENABLE_FPU_LS)
#error "FP lazy context save unsupported!"
	/* FIXME: Enable FP lazy context save */
	CM3_SCB->fpccr = SCB_FPCCR_ASPEN | SCB_FPCCR_LSPEN;
#else
	DCC_LOG(LOG_INFO, "Enabling FPU with auto context save."); 
	/* Enable automatic FP context save */
	CM3_SCB->fpccr = SCB_FPCCR_ASPEN;
#endif
	/* Enable FPU access */
	CM3_SCB->cpacr |= CP11_SET(3) | CP10_SET(3);
#endif

#if (THINKOS_ENABLE_PROFILING)
	DCC_LOG(LOG_INFO, "Enable trace"); 
	/* Enable trace */
	CM3_DCB->demcr |= DCB_DEMCR_TRCENA;
	DCC_LOG(LOG_INFO, "Enable cycle counter"); 
	/* Enable cycle counter */
	CM3_DWT->ctrl |= DWT_CTRL_CYCCNTENA;
	/* set the reference to now */
	thinkos_rt.cycref = CM3_DWT->cyccnt;
#endif

	/* Configure the thread stack ?? 
	   If we already using the PSP nothing changes, if on the other 
	   hand MSP is our stack we will move it to PSP 
	   and adjust the CONTROL register accordingly. */
	sp = cm3_sp_get();
	cm3_psp_set(sp);
	/* Configure the use of PSP in thread mode */
	ctrl = CONTROL_SPSEL;
	cm3_control_set(ctrl);
	/* configure the main stack */
	cm3_msp_set((uintptr_t)__thinkos_xcpt_stack_top());

	DCC_LOG(LOG_TRACE, "Memory management init...");
	__thinkos_krn_mem_init(krn, map);

#if (THINKOS_ENABLE_EXCEPTIONS)
	DCC_LOG(LOG_TRACE, "Exceptions init....");
	thinkos_krn_exception_init();
#endif

	DCC_LOG(LOG_TRACE, "IDLE init...");
	thinkos_krn_idle_init();

#if (THINKOS_ENABLE_DATE_AND_TIME)
	__thinkos_krn_time_init(krn);
#endif

#if (THINKOS_ENABLE_CLOCK) || (THINKOS_ENABLE_MONITOR)
	DCC_LOG(LOG_TRACE, "SYysTick init...");
	thinkos_krn_systick_init();
#endif

#if (THINKOS_ENABLE_CONSOLE)
	DCC_LOG(LOG_TRACE, "Console init...");
	thinkos_krn_console_init();
#endif

	DCC_LOG(LOG_TRACE, "Memory management init...");
	__thinkos_krn_mem_init(krn, map);

	DCC_LOG(LOG_TRACE, "Interrupts init init...");
	__thinkos_krn_irq_init(krn);

	if (lst == NULL) {
		bool privileged;

#if (THINKOS_ENABLE_PRIVILEGED_THREAD)
		privileged  = __PRIVILEGED(opt);
#else
		ctrl = CONTROL_SPSEL | CONTROL_nPRIV;
		privileged = false;
#endif

		DCC_LOG(LOG_TRACE, "Main thread init...");
		thread_idx = __thinkos_init_main(krn, sp, opt);

		/* Set the initial thread */
		__thread_active_set(krn, thread_idx);
		/* add to the ready queue */
		krn->wq_ready = 1 << thread_idx;

		if (privileged) {
			DCC_LOG(LOG_WARNING , "!! Main thread is Privileged !!");
		 } else {
			DCC_LOG(LOG_TRACE, "Main thread is unprivileged");
			DCC_LOG(LOG_TRACE, "Enabling interrupts");
			/* enable interrupts */
			thinkos_krn_irq_on();
		}

		/* Adjust privilege */
		ctrl = privileged ? CONTROL_SPSEL : CONTROL_SPSEL | CONTROL_nPRIV;
		cm3_control_set(ctrl);

		DCC_LOG4(LOG_TRACE, "<%d> MSP=%08x PSP=%08x CTRL=%02x", 
				 thread_idx + 1, cm3_msp_get(), cm3_psp_get(), 
				 cm3_control_get());
	} else {
		/* FIXME: not implemented... */
		return THINKOS_ENOSYS;
	}

	__thread_enable_all(krn);

	return thread_idx + 1;
}

