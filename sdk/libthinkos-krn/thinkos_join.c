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
void thinkos_join_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int thread = (unsigned int)arg[0];
#if THINKOS_ENABLE_ARG_CHECK
	int ret;

	if ((ret = __krn_thread_check(krn, thread)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid thread %d!", self, thread);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	/* remove thread from the canceled wait queue */
	if (__thread_cancel_get(krn, thread)) {
		__thread_cancel_clr(krn, thread);
		__thread_ready_set(krn, thread);
	}

	/* set the return to ERROR as a default value. The
	   exit function of the joining thread will set this to the 
	   appropriate return code */
	arg[0] = -1;

	/* insert the current thread (self) into the joining thread wait queue */
	DCC_LOG2(LOG_TRACE, "<%2d> waiting to join with <%2d>.", self, thread);
	__krn_thread_wait(krn, self, thread);
}
#endif

