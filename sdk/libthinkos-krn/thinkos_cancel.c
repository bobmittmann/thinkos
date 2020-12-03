/* 
 * thinkos_cancel.c
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

#if (THINKOS_ENABLE_CANCEL)
void thinkos_cancel_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int th = (unsigned int)((arg[0] == 0) ? self : arg[0]);
	int code = arg[1];
	unsigned int wq;
	uint32_t pc;
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = __krn_thread_check(krn, th)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%d> invalid thread %d!", self, th);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	wq = __thread_wq_get(krn, th);
	/* remove from other wait queue including wq_ready */
	/* possibly remove from the time wait queue */
	__wq_remove(krn, wq, th);

#if THINKOS_ENABLE_JOIN
	/* insert into the canceled wait queue and wait for a join call */ 
	wq = __wq_idx(krn, &krn->wq_canceled);
#else /* THINKOS_ENABLE_JOIN */
	/* if join is not enabled insert into the ready queue */
	wq = __wq_idx(krn, &krn->wq_ready);
#endif /* THINKOS_ENABLE_JOIN */

	__krn_wq_insert(krn, wq, th);

	/* possibly remove from the time share wait queue */
	__thread_tmshare_clr(krn, th);

	DCC_LOG3(LOG_TRACE, "<%d> cancel %d, with code %d!", 
			 self, th, code); 

	pc = (uint32_t)__thinkos_thread_terminate_stub;
	__thread_pc_set(krn, th, pc);
	__thread_r0_set(krn, th, code);
	arg[0] = THINKOS_OK;
}
#endif


