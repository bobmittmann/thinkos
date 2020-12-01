/* 
 * suspended.c
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

bool __thinkos_suspended(void)
{
	struct thinkos_rt * krn = &thinkos_rt;
	unsigned int th;

	for (th = 1; th <= THINKOS_THREADS_MAX; ++th) {
		if (__thread_ctx_is_valid(krn, th)) {
			bool suspended = false;
			suspended |= __thread_enable_get(krn, th);
			suspended |= __thread_cancel_get(krn, th);
			suspended |= __thread_fault_get(krn, th);
			suspended |= __thread_pause_get(krn, th);
			if (!suspended)
				return false;
		}
	}

	return true;
}

