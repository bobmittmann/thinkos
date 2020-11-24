/* 
 * thinkos_sleep.c
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

#if (THINKOS_ENABLE_SLEEP)
void thinkos_sleep_svc(int32_t * arg, int self)
{
	struct thinkos_rt * krn = &thinkos_rt;
	uint32_t ms = (uint32_t)arg[0];

#if (THINKOS_ENABLE_CLOCK)
	/* set the clock */
	__thread_clk_itv_set(krn, self, ms);
	/* insert into the clock wait queue */
	__bit_mem_wr(&krn->wq_clock, self, 1);
	/* mark the thread clock enable bit */
	__thread_stat_set(krn, self, THINKOS_WQ_CLOCK, true);
	/* wait for event */
	__thread_suspend(krn, self);
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
#else
	udelay(1000 * ms);
#endif
}
#endif

#if (THINKOS_ENABLE_ALARM)
void thinkos_alarm_svc(int32_t * arg, int self)
{
	struct thinkos_rt * krn = &thinkos_rt;
	uint32_t ms = (uint32_t)arg[0];

	/* set the clock */
	__thread_clk_set(krn, self, ms);
	/* insert into the clock wait queue */
	__bit_mem_wr(&krn->wq_clock, self, 1);
	/* mark the thread clock enable bit */
	__thread_stat_set(krn, self, THINKOS_WQ_CLOCK, true);
	/* wait for event */
	__thread_suspend(krn, self);
	/* signal the scheduler ... */
	
}
#endif

