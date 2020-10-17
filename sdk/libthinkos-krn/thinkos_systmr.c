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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#include <thinkos.h>
#include <sys/dcclog.h>

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
	DCC_LOG1(LOG_TRACE, "Wakeup %d...", thread_id + 1);
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
void __attribute__((aligned(16))) cm3_systick_isr(void)
{
#if (THINKOS_ENABLE_MONITOR)
	struct cm3_systick * systick = CM3_SYSTICK;
	do {
		if (systick->csr & SYSTICK_CSR_COUNTFLAG)
#endif
#if (THINKOS_ENABLE_CLOCK)
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

#if (THINKOS_ENABLE_MONITOR)
	} while (__thinkos_monitor_isr());
#endif /* THINKOS_ENABLE_MONITOR */

}

void __krn_systick_init(void)
{
	struct cm3_systick * systick = CM3_SYSTICK;

	DCC_LOG(LOG_INFO, "Initialize the SysTick"); 
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

