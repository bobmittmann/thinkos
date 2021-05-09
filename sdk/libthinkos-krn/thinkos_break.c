/* 
 * thinkos_break.c
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

#if (THINKOS_ENABLE_BREAK)

void thinkos_break_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	unsigned int wq = arg[0];
	int th;

#if (THINKOS_ENABLE_ARG_CHECK)
	if (wq >= THINKOS_WQ_CNT) {
		DCC_LOG1(LOG_ERROR, "invalid object %d!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_OBJECT_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}

	if (!__thinkos_obj_alloc_check(wq)) {
		DCC_LOG1(LOG_ERROR, "invalid object %d!", wq);
		__THINKOS_ERROR(self, THINKOS_ERR_OBJECT_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif 

	cm3_cpsid_i();

	/* remove all threads from the wait queue */
	if ((th = __thinkos_wq_head(wq)) != THINKOS_THREAD_NULL) {
		do {
			/* break (wakeup) from the wait queue and set
			 the return value to EINTR */
			__thinkos_wakeup_return(wq, th, THINKOS_EINTR);
			DCC_LOG2(LOG_INFO, "<%d> wakeup from %d.", th, wq);
		} while  ((th = __thinkos_wq_head(wq)) != THINKOS_THREAD_NULL);
		/* signal the scheduler ... */
		__thinkos_defer_sched();
	}

	cm3_cpsie_i();

	arg[0] = 0;
}

#endif /* THINKOS_ENABLE_BREAK */

