/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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

/** 
 * @file nrt_test.c
 * @brief application test
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#define __THINKOS_NRT__
#include <thinkos/nrt.h>
#include <arch/cortex-m3.h>
#include <sys/dcclog.h>
#include <stdlib.h>
#include <thinkos.h>

struct {
	/* time wait bitmap */
	uint64_t tmw_bmp;
	uint16_t wq[THINKOS_NRT_THREADS_MAX];
	uint32_t clock[THINKOS_NRT_THREADS_MAX];
} nrt;

#if THINKOS_ENABLE_SLEEP
void thinkos_nrt_sleep_svc(int32_t * arg, int self)
{
	uint32_t ms = (uint32_t)arg[0];
	uint32_t ticks = thinkos_rt.ticks;
	int idx = self - THINKOS_NRT_THREAD0; 
	int i;

	DCC_LOG2(LOG_TRACE, "thread=%d time=%dms..", self + 1, arg[0]);

	/* set the clock */
	nrt.clock[idx] = ticks + ms;
	nrt.wq[idx] = THINKOS_WQ_CLOCK; 

	for (i = 0; i < THINKOS_NRT_THREADS_MAX; ++i) {
		if (nrt.wq[idx] == THINKOS_WQ_CLOCK) {
			uint32_t dt = (nrt.clock[i] - ticks);
			if (dt < ms)
				ms = dt;
		}
	}

	arg[0] = ms;
}
#endif

void thinkos_svc_isr(int32_t * arg, int32_t self, uint32_t svc);

void thinkos_nrt_svc_isr(int32_t * arg, int32_t self, uint32_t svc)
{
	switch (svc) {
	case THINKOS_SLEEP:
#if THINKOS_ENABLE_SLEEP
		thinkos_nrt_sleep_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;
	}

	/* Call the RT service for the non realtime scheduler thread instead */
	thinkos_svc_isr(arg, THINKOS_THREAD_NRT_SCHED, svc);
}
