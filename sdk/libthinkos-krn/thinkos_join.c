/* 
 * thinkos_join.c
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

#if THINKOS_ENABLE_JOIN
void thinkos_join_svc(int32_t * arg, unsigned int self)
{
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int thread_no = (unsigned int)arg[0];
	unsigned int wq;

#if THINKOS_ENABLE_ARG_CHECK
	if (!__thread_is_valid(krn, thread_no)) {
		DCC_LOG2(LOG_ERROR, "<%2d> object %d is not a thread!", 
				 self, thread_no);
		__THINKOS_ERROR(self, THINKOS_ERR_THREAD_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if THINKOS_ENABLE_THREAD_ALLOC
	if (__thread_is_alloc(krn, thread_no) == 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid thread %d!", 
				 self, thread_no);
		__THINKOS_ERROR(self, THINKOS_ERR_THREAD_ALLOC);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	/* remove thread from the canceled wait queue */
	if (__thread_cancel_get(krn, thread_no)) {
		__thread_cancel_clr(krn, thread_no);
		__thread_ready_set(krn, thread_no);
	}

	/* insert the current thread (self) into the joining thread wait queue */
	wq = THINKOS_THREAD_BASE + thread_no;
	__thinkos_wq_insert(wq, self);

	DCC_LOG2(LOG_TRACE, "<%d> waiting to join at %d.", self, wq);

	/* wait for event */
	__thinkos_suspend(self);
	/* signal the scheduler ... */
	__thinkos_defer_sched();

	/* set the return to ERROR as a default value. The
	   exit function of the joining thread will set this to the 
	   appropriate return code */
	arg[0] = -1;
}
#endif

