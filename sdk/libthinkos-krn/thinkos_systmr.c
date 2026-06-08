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

void __attribute__((noinline)) __thinkos_krn_clk_wakeup(struct thinkos_rt * krn, 
														 unsigned int th) 
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

//	__krn_preempt(krn);
	__krn_sched_defer(krn);
}

#if (THINKOS_ENABLE_TIMESHARE)
static void __thinkos_krn_timeshare(struct thinkos_rt * krn) 
{
	int32_t idx;

	idx = __krn_sched_active_get(krn);

	/*  */
	thinkos_rt.sched_val[idx] -= thinkos_rt.sched_pri[idx];
	if (thinkos_rt.sched_val[idx] < 0) {
		thinkos_rt.sched_val[idx] += thinkos_rt.sched_limit;
		if (__bit_mem_rd(&thinkos_rt.wq_ready, idx) == 0) {
			DCC_LOG1(LOG_TRACE, "thread %d is active but not ready!!!", idx);
		} else {
			/* insert into the CPU wait queue */
			__bit_mem_wr(&thinkos_rt.wq_tmshare, idx, 1);  
			__krn_thread_suspend(krn, idx + 1);
			__krn_preempt(krn);
		}
	}
}
#endif /* THINKOS_ENABLE_TIMESHARE */


/* --------------------------------------------------------------------------
 * ThinkOS - system timer
 * --------------------------------------------------------------------------*/

void __monitor_context_swap(uint32_t ** pctx); 

void __krn_clk_set_wakeup(struct thinkos_rt * krn, uint32_t bmp)
{
	uint32_t msk;

	msk = krn->wq_clock;
	bmp &= msk;

	if (bmp != 0) {
#if (THINKOS_ENABLE_THREAD_STAT)
		int j;

		bmp = __rbit(bmp);
		while ((j = __clz(bmp)) < 32) {
			int32_t th = j + 1;
			bmp &= ~(0x80000000 >> j);  
			int wq;
			/* update the thread status */
			wq = __thread_stat_wq_get(krn, th);
			__thread_stat_clr(krn, th);
			/* remove from other wait queue, if any */
			__bit_mem_wr(&krn->wq_lst[wq], (th - 1), 0);  
			DCC_LOG1(LOG_TRACE, "<%2d> wakeup!", th);
			/* remove from the time wait queue */
			__bit_mem_wr(&krn->wq_clock, (th - 1), 0);  
			/* insert into the ready wait queue */
			__bit_mem_wr(&krn->wq_ready, (th - 1), 1);
		}
#else
		uint32_t ready;

		do {
			ready = __ldrex((uint32_t *)&krn->wq_ready);
			ready |= bmp;
		} while (__strex((uint32_t *)&krn->wq_ready, ready));

		krn->wq_clock = msk & ~bmp;
#endif
/*		__krn_preempt(krn);
	FIXME: not sure if preempt is the way to go about... 
 */
		__krn_sched_defer(krn);
	}
}

void __attribute__((aligned(16))) cm3_systick_isr(void)
{
	struct thinkos_rt * krn = &thinkos_rt;
  #if (THINKOS_ENABLE_MONITOR)
	struct cm3_systick * systick = CM3_SYSTICK;
	do {
		uint32_t sigset;
		uint32_t sigmsk;
		uint32_t sigact;

		if (systick->csr & SYSTICK_CSR_COUNTFLAG)
  #endif
		{
			uint32_t clk;
#if (THINKOS_ENABLE_DATE_AND_TIME)
			union krn_time ts;

			ts.sec = krn->clk.timestamp.sec;
			ts.frac = krn->clk.timestamp.frac;

			ts.u64 += krn->clk.increment;
			krn->clk.timestamp.sec = ts.sec;
			krn->clk.timestamp.frac = ts.frac;
#endif
			clk = krn->clk.time; 
			/* clock count milliseconds */
			clk += 1;
			krn->clk.time = clk; 

			uint32_t wq = __rbit(krn->wq_clock);
			int j;

			while ((j = __clz(wq)) < 32) {
				int32_t th = j + 1;
				wq &= ~(0x80000000 >> j);  
				if ((int32_t)(krn->clk.th_tmr[th] - clk) <= 0) {
					__thinkos_krn_clk_wakeup(krn, th); 
				}
			}

  #if (THINKOS_ENABLE_MONITOR)
    #if (THINKOS_ENABLE_MONITOR_CLOCK)
			if (((int32_t)(krn->clk.th_tmr[0] - clk)) <= 0) {
				__monitor_event_set(krn, 1 << MONITOR_ALARM); 
			}
    #endif
  #endif
		

    #if (THINKOS_ENABLE_TIMESHARE)
			__thinkos_krn_timeshare(krn); 
    #endif /* THINKOS_ENABLE_TIMESHARE */
		}

  #if (THINKOS_ENABLE_MONITOR)
		sigset = krn->monitor.events;
		sigmsk = krn->monitor.mask;
		sigact = sigset & sigmsk;

		/* Process monitor events */
		if (sigact == 0)
			break;

#if (THINKOS_ENABLE_DEFERRED_ISR)
		{
			int ev;

			if ((ev = __clz(sigact)) < 8) {
				/* clear the TASK_INIT event */
				sigset &= ~(1 << (31 - ev));
				DCC_LOG2(LOG_MSG, "DSR sigset=%08x, ev=%0d", sigset, ev); 
				krn->monitor.events = sigset;
				krn->monitor.svc->on_event[ev](krn, krn->monitor.env);
				continue;
			} 
		}
#endif
		DCC_LOG2(LOG_MSG, "swap sigact=%08x sched=%08x.", sigact,
				 krn->sched.ctrl); 
		__monitor_context_swap(&krn->monitor.ctx); 

	} while (1);

  #endif /* THINKOS_ENABLE_MONITOR */
}

#define THINKOS_SYSTICK_FREQ (1000) /* T = 1ms */

#define THINKOS_CLK_INCREMENT (((uint64_t)(1LL << 32) / \
								(THINKOS_SYSTICK_FREQ)) >> 12)

#define THINKOS_CLK_RESOLUTION (((uint64_t)(1LL << 32) / \
								(THINKOS_SYSTICK_FREQ)))

void thinkos_krn_systick_init(struct thinkos_rt * krn)
{
	struct cm3_systick * systick = CM3_SYSTICK;

	DCC_LOG(LOG_INFO, "Initializing SysTick..."); 
	/* Initialize the SysTick module */
	systick->rvr = cm3_systick_load_1ms; /* 1ms tick period */
	systick->cvr = 0;

	systick->csr = SYSTICK_CSR_ENABLE | SYSTICK_CSR_TICKINT;

#if (THINKOS_ENABLE_DATE_AND_TIME)
	krn->clk.resolution = THINKOS_CLK_RESOLUTION;
	krn->clk.increment = krn->clk.resolution;
	DCC_LOG1(LOG_INFO, "clk.increment=%u", krn->clk.increment); 
#elif (THINKOS_ENABLE_FRACTIONAL_CLOCK)
	krn->clk.increment = THINKOS_CLK_INCREMENT;
	DCC_LOG1(LOG_INFO, "clk.increment=%u", krn->clk.increment); 
#endif
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
	DCC_LOG2(LOG_MSG, "th=%d PC=%08x +++++", th, __thread_pc_get(krn, th)); 
	if (tmw)
		__thread_clk_enable(krn, th);
//	}
	return true;
}
#endif

