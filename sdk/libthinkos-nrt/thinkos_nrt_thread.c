/* 
 * thinkos_nonrt_thread.c
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

#define __THINKOS_NRT__
#include <thinkos/nrt.h>
#include <thinkos.h>
#include <stdbool.h>
#include <sys/dcclog.h>

bool __thinkos_nrt_thread_resume(unsigned int thread_id)
{
	return true;
}

int __thinkos_nrt_thread_init(unsigned int thread_id)
{
	int idx = thread_id - THINKOS_NRT_THREAD0; 

	DCC_LOG2(LOG_TRACE, "thread=%d idx=%d", thread_id + 1, idx);
	(void)idx;

	/* set the thread's return value */
	thinkos_rt.ctx[THINKOS_THREAD_NRT_SCHED]->r0 = NRT_THREAD_CREATE;
	__bit_mem_wr(&thinkos_rt.wq_ready, THINKOS_THREAD_NRT_SCHED, 1);

	return true;
}

