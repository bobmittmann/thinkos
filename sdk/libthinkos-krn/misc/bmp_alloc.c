/* 
 * thinkos_util.c
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
#include <thinkos.h>
#include <sys/dcclog.h>

#if THINKOS_ENABLE_THREAD_ALLOC | THINKOS_ENABLE_MUTEX_ALLOC | \
	THINKOS_ENABLE_COND_ALLOC | THINKOS_ENABLE_SEM_ALLOC | \
	THINKOS_ENABLE_EVENT_ALLOC | THINKOS_ENABLE_FLAG_ALLOC | \
	THINKOS_ENABLE_GATE_ALLOC
int __thinkos_bmp_alloc(uint32_t bmp[], int bits) 
{
	int i;
	int j;

	for (i = 0; i < ((bits + 31) / 32); ++i) {
		/* Look for an empty bit MSB first */
		if ((j = __clz(__rbit(~(bmp[i])))) < 32) {
			/* Mark as used */
			__bit_mem_wr(&bmp[i], j, 1);  
			return 32 * i + j;;
		}
	}
	return THINKOS_ENOMEM;
}
#endif

#if THINKOS_ENABLE_THREAD_ALLOC | THINKOS_ENABLE_MUTEX_ALLOC | \
	THINKOS_ENABLE_COND_ALLOC | THINKOS_ENABLE_SEM_ALLOC | \
	THINKOS_ENABLE_EVENT_ALLOC | THINKOS_ENABLE_FLAG_ALLOC | \
	THINKOS_ENABLE_GATE_ALLOC
void __thinkos_bmp_init(uint32_t bmp[], int bits) 
{
	int i;

	for (i = 0; i < bits / 32; ++i)
		bmp[i] = 0;
	if (bits % 32)
		bmp[i] = 0xffffffff << (bits % 32);
}
#endif


#if THINKOS_ENABLE_THREAD_ALLOC 
static int __thinkos_alloc_lo(uint32_t * ptr, int start) 
{
	int idx;

	if (start < 0)
		start = 0;

	/* Look for an empty bit MSB first */
	idx = __clz(__rbit(~(*ptr >> start))) + start;
	if (idx >= 32)
		return -1;
	/* Mark as used */
	__bit_mem_wr(ptr, idx, 1);  
	return idx;
}

static int __thinkos_alloc_hi(uint32_t * ptr, int start) 
{
	int idx;

	if (start > 31)
		start = 31;

	/* Look for an empty bit LSB first */
	idx = start - __clz(~(*ptr << (31 - start)));
	if (idx < 0)
		return -1;
	/* Mark as used */
	__bit_mem_wr(ptr, idx, 1);  
	return idx;
}

int __thinkos_thread_alloc(int target_id)
{
	int thread_id;

	DCC_LOG1(LOG_INFO, "thinkos_rt.th_alloc=0x%08x", thinkos_rt.th_alloc[0]);

	if (target_id >= THINKOS_THREADS_MAX) {
		thread_id = __thinkos_alloc_hi(thinkos_rt.th_alloc, 
				THINKOS_THREADS_MAX);
		DCC_LOG2(LOG_INFO, "thinkos_alloc_hi() %d -> %d.", target_id, 
				thread_id);
	} else {
		/* Look for the next available slot */
		if (target_id < 0)
			target_id = 0;
		thread_id = __thinkos_alloc_lo(thinkos_rt.th_alloc, target_id);
		DCC_LOG2(LOG_INFO, "thinkos_alloc_lo() %d -> %d.", 
				target_id, thread_id);
		if (thread_id < 0) {
			thread_id = __thinkos_alloc_hi(thinkos_rt.th_alloc, target_id);
			DCC_LOG2(LOG_INFO, "thinkos_alloc_hi() %d -> %d.", 
					target_id, thread_id);
		}
	}

	return thread_id;
}

#endif
