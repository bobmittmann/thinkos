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
extern int32_t udelay_factor;

static void thinkos_krn_abort(struct thinkos_rt * krn)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_FGR " /!\\ Kernel Abort /!\\ " VT_POP);

//	__thinkos_krn_core_reset(krn);
	/* request scheduler to stop everything */
	__krn_sched_svc_set(krn, 1);
	/* Make sure to run the scheduler */
	__krn_defer_sched(krn);
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
		thinkos_krn_abort(krn);
		break;

/* XXX: Deprecated
	case THINKOS_CTL_TRACE:
		DCC_LOGSTR(LOG_MSG, "%s", (char *)arg[1]);
		break;
*/

	case THINKOS_CTL_REBOOT:
		DCC_LOG(LOG_WARNING, "Reboot!");

		if (arg[1] == THINKOS_CTL_REBOOT_KEY) {
			thinkos_krn_sysrst();
		}
		break;

#if (THINKOS_ENABLE_CTL_KRN_INFO)
#if (THINKOS_ENABLE_THREAD_INFO)
	case THINKOS_CTL_THREAD_INF: {
		arg[0] = __krn_threads_inf_get(krn, 
				(const struct thinkos_thread_inf **)arg[1], 
				(unsigned int)arg[2] >> 16,
				(unsigned int)arg[2] & 0xffff);
		}
		break;
#endif

#if (THINKOS_ENABLE_PROFILING)
	case THINKOS_CTL_THREAD_CYCCNT:
		arg[0] = __krn_threads_cyc_get(krn, (uint32_t *)arg[1], 
									   (unsigned int)arg[2] >> 16,
									   (unsigned int)arg[2] & 0xffff);
		break;
#endif

	case THINKOS_CTL_CYCCNT:
		/* Return the current value of the CPU cycle counter */
		arg[0] = CM3_DWT->cyccnt;
		break;
#endif

	default:
		DCC_LOG1(LOG_ERROR, "invalid CTL request %d!", req);
		__THINKOS_ERROR(self, THINKOS_ERR_CTL_REQINV);
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

#endif /* THINKOS_ENABLE_CTL */

