/* 
 * thinkos_systmr.c
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
#include <sys/delay.h>
#include <sys/dcclog.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

static void __thinkos_time_wakeup(struct thinkos_rt * krn, int th) 
{
#if (THINKOS_ENABLE_THREAD_STAT)
	int wq;
	/* update the thread status */
	wq = __thread_stat_wq_get(krn, th);
	__thread_stat_clr(krn, th);
	/* remove from other wait queue, if any */
	__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 0);  
#endif
	DCC_LOG1(LOG_MSG, "<%2d> wakeup!", th);
	/* remove from the time wait queue */
	__bit_mem_wr(&krn->wq_clock, (th - 1), 0);  
	/* insert into the ready wait queue */
	__bit_mem_wr(&krn->wq_ready, (th - 1), 1);

	__krn_preempt(krn);
}


#if (THINKOS_ENABLE_TIMESHARE)
static void __thinkos_timeshare(void) 
{
	int32_t idx;

	idx = __thinkos_active_get();

	/*  */
	thinkos_rt.sched_val[idx] -= thinkos_rt.sched_pri[idx];
	if (thinkos_rt.sched_val[idx] < 0) {
		thinkos_rt.sched_val[idx] += thinkos_rt.sched_limit;
		if (__bit_mem_rd(&thinkos_rt.wq_ready, idx) == 0) {
			DCC_LOG1(LOG_TRACE, "thread %d is active but not ready!!!", 
					 idx);
		} else {
			/* insert into the CPU wait queue */
			__bit_mem_wr(&thinkos_rt.wq_tmshare, idx, 1);  
			__thinkos_suspend(idx);
			__thinkos_preempt();
		}
	}
}
#endif /* THINKOS_ENABLE_TIMESHARE */


/* --------------------------------------------------------------------------
 * ThinkOS - system timer
 * --------------------------------------------------------------------------*/

#if (THINKOS_ENABLE_MONITOR_SCHED)
void __attribute__((aligned(16))) cm3_systick_isr(void)
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct cm3_systick * systick = CM3_SYSTICK;

	DCC_LOG(LOG_MSG, "tick...");

	if (systick->csr & SYSTICK_CSR_COUNTFLAG)
	{
		uint32_t ticks;
		uint32_t wq;
		int j;

		ticks = krn->ticks; 
		ticks++;
		krn->ticks = ticks; 

		wq = __rbit(krn->wq_clock);
		while ((j = __clz(wq)) < 32) {
			int32_t th = j + 1;
			wq &= ~(0x80000000 >> j);  
			if ((int32_t)(krn->clock[th] - ticks) <= 0) {
				__thinkos_time_wakeup(krn, th); 
			}
		}

    #if (THINKOS_ENABLE_MONITOR_CLOCK)
		if ((int32_t)(krn->monitor_clock - ticks) == 0) {
			monitor_signal(MONITOR_ALARM);
		}
    #endif

    #if (THINKOS_ENABLE_TIMESHARE)
		__thinkos_timeshare(); 
    #endif /* THINKOS_ENABLE_TIMESHARE */
	}

	__thinkos_monitor_sched(&krn->krn->monitor);
}

#else /* THINKOS_ENABLE_MONITOR_SCHED */

void __thinkos_monitor_on_reset(void);
void __monitor_context_swap(uint32_t ** pctx); 

void __attribute__((aligned(16))) cm3_systick_isr(void)
{
	struct thinkos_rt * krn = &thinkos_rt;
#if (THINKOS_ENABLE_DATE_AND_TIME)
    struct krn_clock * clk = &krn->time_clk;
#endif
  #if (THINKOS_ENABLE_MONITOR)
	struct cm3_systick * systick = CM3_SYSTICK;
	do {
		uint32_t sigset;
		uint32_t sigmsk;
		uint32_t sigact;

		if (systick->csr & SYSTICK_CSR_COUNTFLAG)
  #endif
		{
			uint32_t ticks;
			uint32_t wq;
			int j;

#if (THINKOS_ENABLE_DATE_AND_TIME)
			uint64_t ts;

			ts = clk->timestamp;
			ts += clk->increment;
			clk->timestamp = ts;
#endif

			ticks = krn->ticks; 
			ticks++;
			krn->ticks = ticks; 

			wq = __rbit(krn->wq_clock);

			while ((j = __clz(wq)) < 32) {
				int32_t th = j + 1;
				wq &= ~(0x80000000 >> j);  
				if ((int32_t)(krn->th_clk[th] - ticks) <= 0) {
					__thinkos_time_wakeup(krn, th); 
				}
			}

    #if (THINKOS_ENABLE_MONITOR_CLOCK)
			if ((int32_t)(krn->monitor_clock - ticks) == 0) {
				monitor_signal(MONITOR_ALARM);
			}
    #endif

    #if (THINKOS_ENABLE_TIMESHARE)
			__thinkos_timeshare(); 
    #endif /* THINKOS_ENABLE_TIMESHARE */
		}

  #if (THINKOS_ENABLE_MONITOR)
		sigset = krn->monitor.events;
		sigmsk = krn->monitor.mask;

		sigact = sigset & sigmsk;

		/* Process monitor events */
		if (sigact == 0)
			break;

		if (sigact & (1 << MONITOR_TASK_INIT)) {
			/* clear the TASK_INIT event */
			krn->monitor.events = sigset & ~(1 << MONITOR_TASK_INIT);
			__thinkos_monitor_on_reset();
		}

		__monitor_context_swap(&krn->monitor.ctx); 

	} while (1);

  #endif /* THINKOS_ENABLE_MONITOR */
}
#endif /* !THINKOS_ENABLE_MONITOR_SCHED */

void thinkos_krn_systick_init(void)
{
	struct cm3_systick * systick = CM3_SYSTICK;

	DCC_LOG(LOG_TRACE, "Initializing SysTick..."); 
	/* Initialize the SysTick module */
	systick->rvr = cm3_systick_load_1ms; /* 1ms tick period */
	systick->cvr = 0;

	systick->csr = SYSTICK_CSR_ENABLE | SYSTICK_CSR_TICKINT;
}

#if (THINKOS_ENABLE_UDELAY_CALIBRATE)
static unsigned int __get_ticks(void)
{
	return 0xffffffff - (CM3_SYSTICK->cvr << 8);
}

void thinkos_krn_udelay_calibrate(void)
{
	struct cm3_systick * systick = CM3_SYSTICK;
	uint32_t ticks1ms;
	uint32_t rvr;
	uint32_t csr;

	rvr = systick->rvr;
	csr = systick->csr;

	systick->rvr = 0x00ffffff;
	systick->csr = SYSTICK_CSR_ENABLE;
	ticks1ms = cm3_systick_load_1ms << 8;

	udelay_calibrate(ticks1ms, __get_ticks);

	systick->rvr = rvr;
	systick->csr = csr;
}
#endif

#if (THINKOS_ENABLE_PAUSE)
bool clock_resume(struct thinkos_rt * krn, unsigned int th, 
				  unsigned int wq, bool tmw) 
{
//	if ((int32_t)(krn->clock[th] - krn->ticks) <= 0) {
		/* thread's clock is in the past, wakeup now. */
//		DCC_LOG1(LOG_INFO, "timeout PC=%08x .......", __thinkos_thread_pc_get(th)); 
		/* update the thread status */
//		__thinkos_thread_stat_clr(th);
		/* insert into the ready wait queue */
//		__bit_mem_wr(&krn->wq_ready, th, 1);  
//	} else {
	DCC_LOG2(LOG_INFO, "th=%d PC=%08x +++++", th, __thread_pc_get(krn, th)); 
	if (tmw)
		__thread_clk_enable(krn, th);
//	}
	return true;
}
#endif


