/* 
 * resume_all.c
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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

void __thinkos_resume_all(void)
{
	int32_t th;

	for (th = 0; th < THINKOS_THREADS_MAX; ++th) {
		if (thinkos_rt.ctx[th] != NULL) {
			__thinkos_thread_resume(th);
		}
	}

	DCC_LOG(LOG_TRACE, "....");
	
	__thinkos_defer_sched();
}

