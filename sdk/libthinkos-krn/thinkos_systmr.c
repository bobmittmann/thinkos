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

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#if (THINKOS_ENABLE_CLOCK)
static void __thinkos_time_wakeup(int thread_id) 
{
#if (THINKOS_ENABLE_THREAD_STAT)
	int wq;
	/* update the thread status */
	wq = __thinkos_thread_stat_wq_get(thread_id);
	__thinkos_thread_stat_clr(thread_id);
	/* remove from other wait queue, if any */
	__bit_mem_wr(&thinkos_rt.wq_lst[wq], thread_id, 0);  
#endif
	/* remove from the time wait queue */
	__bit_mem_wr(&thinkos_rt.wq_clock, thread_id, 0);  
	DCC_LOG1(LOG_MSG, "Wakeup %d...", thread_id + 1);
	/* insert into the ready wait queue */
	__bit_mem_wr(&thinkos_rt.wq_ready, thread_id, 1);  
	__thinkos_preempt();
}
#endif /* THINKOS_ENABLE_CLOCK */

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
					 idx + 1);
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
#if (THINKOS_ENABLE_CLOCK) || (THINKOS_ENABLE_MONITOR)

#if (THINKOS_ENABLE_MONITOR_SCHED)
void __attribute__((aligned(16))) cm3_systick_isr(void)
{
	struct cm3_systick * systick = CM3_SYSTICK;

  #if (THINKOS_ENABLE_CLOCK)
	if (systick->csr & SYSTICK_CSR_COUNTFLAG)
	{
		uint32_t ticks;
		uint32_t wq;
		int j;

		ticks = thinkos_rt.ticks; 
		ticks++;
		thinkos_rt.ticks = ticks; 

		wq = __rbit(thinkos_rt.wq_clock);
		while ((j = __clz(wq)) < 32) {
			wq &= ~(0x80000000 >> j);  
			if ((int32_t)(thinkos_rt.clock[j] - ticks) <= 0) {
				__thinkos_time_wakeup(j); 
			}
		}

    #if (THINKOS_ENABLE_MONITOR_CLOCK)
		if ((int32_t)(thinkos_rt.monitor_clock - ticks) == 0) {
			monitor_signal(MONITOR_ALARM);
		}
    #endif

    #if (THINKOS_ENABLE_TIMESHARE)
		__thinkos_timeshare(); 
    #endif /* THINKOS_ENABLE_TIMESHARE */
	}
  #endif /* THINKOS_ENABLE_CLOCK */

	__thinkos_monitor_sched(&thinkos_rt.monitor);
}

#else /* THINKOS_ENABLE_MONITOR_SCHED */

void __thinkos_monitor_on_reset(void);
void __monitor_context_swap(uint32_t ** pctx); 

void __attribute__((aligned(16))) cm3_systick_isr(void)
{
#if (THINKOS_ENABLE_DATE_AND_TIME)
    struct krn_clock * clk = &thinkos_rt.time_clk;
#endif
  #if (THINKOS_ENABLE_MONITOR)
	struct cm3_systick * systick = CM3_SYSTICK;
	do {
		uint32_t sigset;
		uint32_t sigmsk;
		uint32_t sigact;

		if (systick->csr & SYSTICK_CSR_COUNTFLAG)
  #endif
  #if (THINKOS_ENABLE_CLOCK)
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

			ticks = thinkos_rt.ticks; 
			ticks++;
			thinkos_rt.ticks = ticks; 

			wq = __rbit(thinkos_rt.wq_clock);
			while ((j = __clz(wq)) < 32) {
				wq &= ~(0x80000000 >> j);  
				if ((int32_t)(thinkos_rt.th_clk[j] - ticks) <= 0) {
					__thinkos_time_wakeup(j); 
				}
			}

    #if (THINKOS_ENABLE_MONITOR_CLOCK)
			if ((int32_t)(thinkos_rt.monitor_clock - ticks) == 0) {
				monitor_signal(MONITOR_ALARM);
			}
    #endif

    #if (THINKOS_ENABLE_TIMESHARE)
			__thinkos_timeshare(); 
    #endif /* THINKOS_ENABLE_TIMESHARE */
		}
  #endif /* THINKOS_ENABLE_CLOCK */

  #if (THINKOS_ENABLE_MONITOR)
		sigset = thinkos_rt.monitor.events;
		sigmsk = thinkos_rt.monitor.mask;

		sigact = sigset & sigmsk;

		/* Process monitor events */
		if (sigact == 0)
			break;

		if (sigact & (1 << MONITOR_RESET)) {
			/* clear the RESET event */
			thinkos_rt.monitor.events = sigset & ~(1 << MONITOR_RESET);
			__thinkos_monitor_on_reset();
		}

		__monitor_context_swap(&thinkos_rt.monitor.ctx); 

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

#if (THINKOS_ENABLE_CLOCK) 
	systick->csr = SYSTICK_CSR_ENABLE | SYSTICK_CSR_TICKINT;
#else
	systick->csr = SYSTICK_CSR_ENABLE;
#endif
}

const char thinkos_clk_nm[] = "CLK";

#endif /* THINKOS_ENABLE_CLOCK || THINKOS_ENABLE_MONITOR */

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

	cm3_cpsid_i();
	rvr = systick->rvr;
	csr = systick->csr;

	systick->rvr = 0x00ffffff;
	systick->csr = SYSTICK_CSR_ENABLE;
	ticks1ms = cm3_systick_load_1ms << 8;

	udelay_calibrate(ticks1ms, __get_ticks);

	systick->rvr = rvr;
	systick->csr = csr;

	cm3_cpsie_i();
}


