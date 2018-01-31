/* 
 * thinkos_ctl.c
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
 * but WITHOUT ANY WARRANTY; without flagen the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#include <thinkos.h>
#include <sys/param.h>
#include <sys/sysclk.h>
#include <sys/delay.h>

#if THINKOS_ENABLE_CTL

#if THINKOS_ENABLE_RT_DEBUG
static void rt_snapshot(uint32_t * dst)
{
	uint32_t pri = cm3_primask_get();
	uint32_t * src;
	int i;

	cm3_primask_set(1);
#if THINKOS_ENABLE_PROFILING
	{
		int self = thinkos_rt.active;
		uint32_t cyccnt = CM3_DWT->cyccnt;
		int32_t delta = cyccnt - thinkos_rt.cycref;
		/* update the reference */
		thinkos_rt.cycref = cyccnt;
		/* update thread's cycle counter */
		thinkos_rt.cyccnt[self] += delta; 
	}
#endif

	src = (uint32_t *)&thinkos_rt;

	for (i = 0; i < (sizeof(struct thinkos_rt) / 4); ++i)
		dst[i] = src[i];

#if THINKOS_ENABLE_PROFILING
	/* Reset cycle counters */
	for (i = 0; i < THINKOS_THREADS_MAX + 1; i++)
		thinkos_rt.cyccnt[i] = 0; 
#endif

	cm3_primask_set(pri);
}
#endif


#if THINKOS_ENABLE_PROFILING
static int thinkos_cycnt_get(uint32_t cycnt[], unsigned int max)
{
	uint32_t cyccnt;
	int32_t delta;
	int self;
	int cnt = MIN(max, THINKOS_THREADS_MAX + 1);

	self = thinkos_rt.active;
	cyccnt = CM3_DWT->cyccnt;
	delta = cyccnt - thinkos_rt.cycref;
	/* update the reference */
	thinkos_rt.cycref = cyccnt;
	/* update thread's cycle counter */
	thinkos_rt.cyccnt[self] += delta; 
	/* copy cycle counters */
	__thinkos_memcpy32(cycnt, thinkos_rt.cyccnt, cnt * sizeof(uint32_t));
	/* reset cycle counters */
	__thinkos_memset32(thinkos_rt.cyccnt, 0, cnt * sizeof(uint32_t));

	return cnt; 
}
#endif


extern int32_t udelay_factor;

void thinkos_ctl_svc(int32_t * arg)
{
	unsigned int req = arg[0];
	int32_t * pval;
	const uint32_t ** ptr;

	arg[0] = 0;
	
	DCC_LOG(LOG_MSG, ".........................");

	switch (req) {
	case THINKOS_CTL_CLOCKS:
		ptr = (const uint32_t **)arg[1];
		*ptr = sysclk_hz;
		break;

	case THINKOS_CTL_UDELAY_FACTOR:
		pval = (int32_t *)arg[1];
		*pval = udelay_factor;
		break;

	case THINKOS_CTL_ABORT:
		DCC_LOG(LOG_WARNING, "Abort!");
		__thinkos_pause_all();
		__thinkos_defer_sched();
#if THINKOS_ENABLE_MONITOR
		__bkpt(3);
#endif
		break;

/* XXX: Deprecated
	case THINKOS_CTL_TRACE:
		DCC_LOGSTR(LOG_MSG, "%s", (char *)arg[1]);
		break;
*/

	case THINKOS_CTL_REBOOT:
		DCC_LOG(LOG_WARNING, "Reboot!");
		udelay(32768);
		cm3_sysrst();
		break;

#if THINKOS_ENABLE_RT_DEBUG
	case THINKOS_CTL_SNAPSHOT:
		rt_snapshot((uint32_t *)arg[1]);
		break;
#endif

#if THINKOS_ENABLE_THREAD_INFO
	case THINKOS_CTL_THREAD_INF: {
		unsigned int cnt;

		cnt = MIN(THINKOS_THREADS_MAX + 1, arg[2]);
		__thinkos_memcpy32((void *)arg[1], thinkos_rt.th_inf,
						   sizeof(void *) * cnt); 
		arg[0] = cnt;
		}
		break;
#endif

#if THINKOS_ENABLE_PROFILING
	case THINKOS_CTL_THREAD_CYCCNT:
		arg[0] = thinkos_cycnt_get((uint32_t *)arg[1], (unsigned int)arg[2]);
		break;
#endif

	case THINKOS_CTL_CYCCNT:
		/* Return the current value of the CPU cycle counter */
		arg[0] = CM3_DWT->cyccnt;
		break;

	default:
		DCC_LOG1(LOG_ERROR, "invalid sysinfo request %d!", req);
		__thinkos_error(THINKOS_ERR_CTL_REQINV);
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

#endif /* THINKOS_ENABLE_CTL */

