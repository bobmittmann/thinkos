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
#include <sys/dcclog.h>

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#if (THINKOS_ENABLE_SLEEP)
void thinkos_sleep_svc(int32_t * arg, int self, struct thinkos_rt * krn)
{
	uint32_t ms = (uint32_t)arg[0];

	DCC_LOG3(LOG_MSG, "self=%d krn=%08x clk=%08x", self, krn, krn->wq_clock);

	/* wait for event */
	__krn_thread_suspend(krn, self);
	/* mark the thread clock enable bit */
	__thread_stat_set(krn, self, THINKOS_WQ_CLOCK, true);
	/* set the clock */
	__thread_clk_itv_set(krn, self, ms);
	/* insert into the clock wait queue */
	__thread_clk_enable(krn, self) ;
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}
#endif

#if (THINKOS_ENABLE_ALARM)
void thinkos_alarm_svc(int32_t * arg, int self, struct thinkos_rt * krn)
{
	uint32_t clk = (uint32_t)arg[0];

	DCC_LOG2(LOG_TRACE, "<%2d> clk=%d", self, clk);

	/* set the clock */
	__thread_clk_set(krn, self, clk);
	/* insert into the clock wait queue */
	__thread_clk_enable(krn, self);
	/* mark the thread clock enable bit */
	__thread_stat_set(krn, self, THINKOS_WQ_CLOCK, true);
	/* wait for event */
	__krn_thread_suspend(krn, self);
	/* signal the scheduler ... */
	__krn_defer_sched(krn);
}
#endif

