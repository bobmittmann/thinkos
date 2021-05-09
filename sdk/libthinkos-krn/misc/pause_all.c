/* 
 * pause_all.c
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

void __krn_pause_all(struct thinkos_rt * krn)
{
	int32_t th;

	for (th = THINKOS_THREAD_FIRST; th <= THINKOS_THREAD_LAST; ++th) {
		if (__thread_ctx_is_valid(krn, th)) {
			__krn_thread_pause(krn, th);
		}
	}

	__krn_defer_sched(krn);
}

