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

#include "thinkos_krn-i.h"
#include <sys/dcclog.h>

#if (THINKOS_ENABLE_CTL)

#ifndef THINKOS_ENABLE_CTL_KRN_INFO
#define THINKOS_ENABLE_CTL_KRN_INFO 0
#endif

#if (THINKOS_ENABLE_PROFILING) && (THINKOS_ENABLE_CTL_KRN_INFO)
static int thinkos_cycnt_get(uint32_t cycnt[], unsigned int max)
{
	uint32_t cyccnt;
	int32_t delta;
	int self;
	int cnt = MIN((int)max, THINKOS_THREADS_MAX + 1);

	self = __thinkos_active_get();
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

void thinkos_krn_abort(struct thinkos_rt * krn)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_FGR " /!\\ Kernel Abort /!\\ " VT_POP);

	__thinkos_krn_core_reset(krn);

#if (THINKOS_ENABLE_MONITOR)
	monitor_signal_break(MONITOR_KRN_ABORT);
#endif
}


void thinkos_ctl_svc(int32_t * arg, unsigned int self)
{
	struct thinkos_rt * krn = &thinkos_rt;
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
		DCC_LOG(LOG_WARNING, "System control: !!Abort!!");
		thinkos_krn_abort(krn);
		break;

/* XXX: Deprecated
	case THINKOS_CTL_TRACE:
		DCC_LOGSTR(LOG_MSG, "%s", (char *)arg[1]);
		break;
*/

	case THINKOS_CTL_REBOOT:
		DCC_LOG(LOG_WARNING, "Reboot!");
		thinkos_krn_sysrst();
		break;

#if (THINKOS_ENABLE_CTL_KRN_INFO)
#if (THINKOS_ENABLE_THREAD_INFO)
	case THINKOS_CTL_THREAD_INF: {
		unsigned int cnt;

		cnt = MIN(THINKOS_THREADS_MAX + 1, arg[2]);
		__thinkos_memcpy32((void *)arg[1], thinkos_rt.th_inf,
						   sizeof(void *) * cnt); 
		arg[0] = cnt;
		}
		break;
#endif

#if (THINKOS_ENABLE_PROFILING)
	case THINKOS_CTL_THREAD_CYCCNT:
		arg[0] = thinkos_cycnt_get((uint32_t *)arg[1], (unsigned int)arg[2]);
		break;
#endif

	case THINKOS_CTL_CYCCNT:
		/* Return the current value of the CPU cycle counter */
		arg[0] = CM3_DWT->cyccnt;
		break;
#endif

	default:
		DCC_LOG1(LOG_ERROR, "invalid sysinfo request %d!", req);
		__THINKOS_ERROR(self, THINKOS_ERR_CTL_REQINV);
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

#endif /* THINKOS_ENABLE_CTL */

