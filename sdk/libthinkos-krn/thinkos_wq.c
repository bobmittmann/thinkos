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

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

struct thread_waitqueue * thinkos_krn_wq_from_oid(unsigned int oid)
{
	struct thinkos_rt * krn = &thinkos_rt;
	struct thread_waitqueue * wq;
	unsigned int idx = oid - THINKOS_OBJECT_FIRST;

	if (idx > THINKOS_OBJECT_LAST) {
		return NULL;
	}

	wq = (struct thread_waitqueue *)&krn->wq_lst[idx];

	return wq;
}

bool thinkos_krn_wq_contains(struct thread_waitqueue * wq, unsigned int thread)
{
	struct thinkos_rt * krn = &thinkos_rt;

	if ((thread < THINKOS_THREAD_FIRST) || (thread > THINKOS_THREAD_LAST)) {
		return false;
	}

	return __krn_wq_contain(krn, wq, thread);
}

bool thinkos_krn_wq_is_empty(struct thread_waitqueue * wq)
{
	struct thinkos_rt * krn = &thinkos_rt;

	return __krn_wq_is_empty(krn, wq);
}

