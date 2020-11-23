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

#if ((THINKOS_ENABLE_THREAD_FAULT) || (THINKOS_ENABLE_PAUSE)) && \
		(THINKOS_ENABLE_THREAD_STAT)
void __krn_pause_all(struct thinkos_rt * krn)
{
	unsigned int idx;

	for (idx = 0; idx < THINKOS_THREADS_MAX; ++idx) {
		if (__thread_ctx_is_valid(krn, idx)) {
			DCC_LOG1(LOG_JABBER, "th=%d", idx + 1);
			__krn_thread_pause(krn, idx);
		}
	}

	DCC_LOG1(LOG_TRACE, "active=%d", __thread_active_get(krn) + 1);
}
#else

#endif

