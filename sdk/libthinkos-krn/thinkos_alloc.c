/* 
 * thinkos_alloc.c
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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif
#include <thinkos.h>
#include <sys/delay.h>
#include <sys/dcclog.h>

#if THINKOS_ENABLE_THREAD_ALLOC | THINKOS_ENABLE_MUTEX_ALLOC | \
	THINKOS_ENABLE_COND_ALLOC | THINKOS_ENABLE_SEM_ALLOC | \
	THINKOS_ENABLE_EVENT_ALLOC | THINKOS_ENABLE_FLAG_ALLOC | \
	THINKOS_ENABLE_GATE_ALLOC
static int __thinkos_bmp_alloc(uint32_t bmp[], unsigned int nbits) 
{
	unsigned int i;
	unsigned int j;

	for (i = 0; i < ((nbits + 31) / 32); ++i) {
		/* Look for an empty bit MSB first */
		if ((j = __thinkos_ffs(~(bmp[i]))) < 32) {
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
static void __thinkos_bmp_init(uint32_t bmp[], unsigned int nbits) 
{
	__thinkos_memset32(bmp, 0, nbits / 8);

	if (nbits % 32)
		bmp[nbits / 32] = 0xffffffff << (nbits % 32);
}
#endif

static int __thinkos_alloc_lo(uint32_t * ptr, int start) 
{
	uint32_t bmp = *ptr;
	int idx;

	if (start < 0)
		start = 0;

	/* Look for an empty bit MSB first */
	idx = __thinkos_ffs(~(bmp >> start)) + start;

	if (idx >= 32)
		return THINKOS_ENOMEM;
	/* Mark as used */
	__bit_mem_wr(ptr, idx, 1);  
	return idx;
}

static int __thinkos_alloc_hi(uint32_t * ptr, int start) 
{
	uint32_t bmp = *ptr;
	int idx;

	if (start > 31)
		start = 31;

	/* Look for an empty bit LSB first */
	idx = start - __clz(~(bmp << (31 - start)));
	if (idx < 0)
		return THINKOS_ENOMEM;
	/* Mark as used */
	__bit_mem_wr(ptr, idx, 1);  
	return idx;
}

static int __thinkos_hilo_alloc(uint32_t bmp[], int max, int tgt_idx)
{
	int idx;

	if (tgt_idx >= max) {
		idx = __thinkos_alloc_hi(bmp, max);
	} else {
		/* Look for the next available slot */
		idx = __thinkos_alloc_lo(bmp, tgt_idx);
		if (idx < 0)
			idx = __thinkos_alloc_hi(bmp, tgt_idx);
	}

	return idx;
}

#if (THINKOS_ENABLE_THREAD_ALLOC)
int __thinkos_thread_alloc(int tgt_idx)
{
	return __thinkos_hilo_alloc(thinkos_rt.th_alloc, 
								THINKOS_THREADS_MAX, 
								tgt_idx);
}
#endif

static uint32_t * const thinkos_obj_alloc_lut[] = {
#if (THINKOS_ENABLE_THREAD_ALLOC)
	[THINKOS_OBJ_READY]     = thinkos_rt.th_alloc,
#else
	[THINKOS_OBJ_READY]     = NULL,
#endif
	[THINKOS_OBJ_TMSHARE]   = NULL,
	[THINKOS_OBJ_CLOCK]     = NULL,
#if (THINKOS_ENABLE_MUTEX_ALLOC)
	[THINKOS_OBJ_MUTEX]      = thinkos_rt.mutex_alloc,
#else
	[THINKOS_OBJ_MUTEX]      = NULL,
#endif
#if (THINKOS_ENABLE_COND_ALLOC)
	[THINKOS_OBJ_COND]      = thinkos_rt.cond_alloc,
#else
	[THINKOS_OBJ_COND]      = NULL,
#endif
#if (THINKOS_ENABLE_SEM_ALLOC)
	[THINKOS_OBJ_SEMAPHORE] = thinkos_rt.sem_alloc,
#else
	[THINKOS_OBJ_SEMAPHORE] = NULL,
#endif
#if (THINKOS_ENABLE_EVENT_ALLOC)
	[THINKOS_OBJ_EVENT]     = thinkos_rt.ev_alloc,
#else
	[THINKOS_OBJ_EVENT]     = NULL,
#endif
#if (THINKOS_ENABLE_FLAG_ALLOC)
	[THINKOS_OBJ_FLAG]      = thinkos_rt.flag_alloc,
#else
	[THINKOS_OBJ_FLAG]      = NULL,
#endif
#if (THINKOS_ENABLE_GATE_ALLOC)
	[THINKOS_OBJ_GATE]      = thinkos_rt.gate_alloc,
#else
	[THINKOS_OBJ_GATE]      = NULL,
#endif
};

static const uint16_t thinkos_obj_base_lut[] = {
	[THINKOS_OBJ_READY]     = 1,
	[THINKOS_OBJ_TMSHARE]   = 0,
	[THINKOS_OBJ_CLOCK]     = 0,
#if (THINKOS_MUTEX_MAX) > 0
	[THINKOS_OBJ_MUTEX]     = THINKOS_MUTEX_BASE,
#endif
#if (THINKOS_COND_MAX) > 0
	[THINKOS_OBJ_COND]      = THINKOS_COND_BASE,
#endif
#if (THINKOS_SEMAPHORE_MAX) > 0
	[THINKOS_OBJ_SEMAPHORE] = THINKOS_SEM_BASE,
#endif
#if (THINKOS_EVENT_MAX) > 0
	[THINKOS_OBJ_EVENT]     = THINKOS_EVENT_BASE,
#endif
#if (THINKOS_FLAG_MAX) > 0
	[THINKOS_OBJ_FLAG]      = THINKOS_FLAG_BASE,
#endif
#if (THINKOS_GATE_MAX) > 0
	[THINKOS_OBJ_GATE]      = THINKOS_GATE_BASE
#endif
};

static const uint8_t thinkos_obj_cnt_lut[] = {
	[THINKOS_OBJ_READY]     = THINKOS_THREADS_MAX,
	[THINKOS_OBJ_TMSHARE]   = THINKOS_WQ_TMSHARE_CNT,
	[THINKOS_OBJ_CLOCK]     = THINKOS_WQ_CLOCK_CNT,
	[THINKOS_OBJ_MUTEX]     = THINKOS_WQ_MUTEX_CNT,
	[THINKOS_OBJ_COND]      = THINKOS_WQ_COND_CNT,
	[THINKOS_OBJ_SEMAPHORE] = THINKOS_WQ_SEMAPHORE_CNT,
	[THINKOS_OBJ_EVENT]     = THINKOS_WQ_EVENT_CNT,
	[THINKOS_OBJ_FLAG]      = THINKOS_WQ_FLAG_CNT,
	[THINKOS_OBJ_GATE]      = THINKOS_WQ_GATE_CNT,
};

bool __thinkos_obj_alloc_check(unsigned int oid)
{
	unsigned int kind;
	unsigned int idx;
	uint32_t * bmp;
	unsigned int max;
	unsigned int base;

	if (oid >= THINKOS_WQ_CNT) {
		return false;
	}

	kind = __thinkos_obj_kind(oid);

	if (kind > THINKOS_OBJ_GATE) {
		return true;
	}

	bmp = thinkos_obj_alloc_lut[kind];
	base = thinkos_obj_base_lut[kind];
	idx = oid - base;
	max = thinkos_obj_cnt_lut[kind];
	if (idx >= max) {
		return false;
	}

	return __bit_mem_rd(bmp, idx) ? true : false;
}


#if (THINKOS_ENABLE_OBJ_ALLOC)
void thinkos_obj_alloc_svc(int32_t * arg, int32_t self)
{
	unsigned int kind = arg[0];
	unsigned int base;
	unsigned int max;
	uint32_t * bmp;
	int idx;

#if 0
	switch (kind) {

#if (THINKOS_MUTEX_MAX > 0)
	case THINKOS_OBJ_MUTEX:
		bmp = thinkos_rt.mutex_alloc;
		base = THINKOS_MUTEX_BASE;
		max = THINKOS_MUTEX_MAX;
		break;
#endif 

#if (THINKOS_SEMAPHORE_MAX > 0)
	case THINKOS_OBJ_SEMAPHORE:
		bmp = thinkos_rt.sem_alloc;
		base = THINKOS_SEM_BASE;
		max = THINKOS_SEMAPHORE_MAX;
		break;
#endif 

#if (THINKOS_COND_MAX > 0)
	case THINKOS_OBJ_COND:
		bmp = thinkos_rt.cond_alloc;
		base = THINKOS_COND_BASE;
		max = THINKOS_COND_MAX;
		break;
#endif 

#if (THINKOS_FLAG_MAX > 0)
	case THINKOS_OBJ_FLAG:
		bmp = thinkos_rt.flag_alloc;
		base = THINKOS_FLAG_BASE;
		max = THINKOS_FLAG_MAX;
		break;
#endif 

#if (THINKOS_EVENT_MAX > 0)
	case THINKOS_OBJ_EVENT:
		bmp = thinkos_rt.ev_alloc;
		base = THINKOS_EVENT_BASE;
		max = THINKOS_EVENT_MAX;
		break;
#endif 

#if (THINKOS_GATE_MAX > 0)
	case THINKOS_OBJ_GATE:
		bmp = thinkos_rt.gate_alloc;
		base = THINKOS_GATE_BASE;
		max = THINKOS_GATE_MAX;
		break;
#endif 
	default:
		arg[0] = THINKOS_EINVAL;
		return;
	}
#else

#if (THINKOS_ENABLE_ARG_CHECK)
	if (kind > THINKOS_OBJ_GATE) {
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	bmp = thinkos_obj_alloc_lut[kind];
	max = thinkos_obj_cnt_lut[kind];
	base = thinkos_obj_base_lut[kind];

#endif

	DCC_LOG3(LOG_MSG, "kind=%d base=%d max=%d", kind, base, max);

	if (kind == THINKOS_OBJ_READY) {
		int32_t tgt_idx = arg[1];

		tgt_idx -= 1;  
		DCC_LOG1(LOG_MSG, "tgt_idx = %d", tgt_idx);

		if ((idx = __thinkos_hilo_alloc(bmp, max, tgt_idx)) >= 0) {
			idx += base;
		}
	} else {
		if ((idx = __thinkos_bmp_alloc(bmp, max)) >= 0) {
			idx += base;
		} 
	}
	DCC_LOG1(LOG_MSG, "idx = %d", idx);
	arg[0] = idx;
}

void __krn_alloc_init(struct thinkos_rt * krn)
{
#if THINKOS_ENABLE_THREAD_ALLOC
	/* initialize the thread allocation bitmap */ 
	__thinkos_bmp_init(krn->th_alloc, THINKOS_THREADS_MAX); 
#endif

#if THINKOS_ENABLE_MUTEX_ALLOC
	/* initialize the mutex allocation bitmap */ 
	__thinkos_bmp_init(krn->mutex_alloc, THINKOS_MUTEX_MAX); 
#endif

#if (THINKOS_ENABLE_SEM_ALLOC)
	/* initialize the semaphore allocation bitmap */ 
	__thinkos_bmp_init(krn->sem_alloc, THINKOS_SEMAPHORE_MAX); 
#endif

#if (THINKOS_ENABLE_COND_ALLOC)
	/* initialize the conditional variable allocation bitmap */ 
	__thinkos_bmp_init(krn->cond_alloc, THINKOS_COND_MAX); 
#endif

#if THINKOS_ENABLE_FLAG_ALLOC
	/* initialize the flag allocation bitmap */ 
	__thinkos_bmp_init(krn->flag_alloc, THINKOS_FLAG_MAX); 
#endif

#if (THINKOS_ENABLE_EVENT_ALLOC)
	/* initialize the event set allocation bitmap */ 
	__thinkos_bmp_init(krn->ev_alloc, THINKOS_EVENT_MAX); 
#endif

#if (THINKOS_ENABLE_GATE_ALLOC)
	/* initialize the gate allocation bitmap */ 
	__thinkos_bmp_init(krn->gate_alloc, THINKOS_GATE_MAX); 
#endif
}
#endif

#if (THINKOS_ENABLE_OBJ_FREE)

void thinkos_obj_free_svc(int32_t * arg, int32_t self)
{
	unsigned int oid = arg[0];
	unsigned int kind;
	unsigned int idx;
	uint32_t * bmp;
#if (THINKOS_ENABLE_ARG_CHECK)
	unsigned int max;
#endif

#if (THINKOS_ENABLE_ARG_CHECK)
	if (oid >= THINKOS_WQ_CNT) {
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	kind = __thinkos_obj_kind(oid);

#if 0

	switch (kind) {

#if (THINKOS_MUTEX_MAX) > 0
	case THINKOS_OBJ_MUTEX:
		bmp = thinkos_rt.mutex_alloc;
		idx = oid - THINKOS_MUTEX_BASE;
		break;
#endif 

#if (THINKOS_SEMAPHORE_MAX) > 0
	case THINKOS_OBJ_SEMAPHORE:
		bmp = thinkos_rt.sem_alloc;
		idx = oid - THINKOS_SEM_BASE;
		break;
#endif 

#if (THINKOS_COND_MAX) > 0
	case THINKOS_OBJ_COND:
		bmp = thinkos_rt.cond_alloc;
		idx = oid - THINKOS_COND_BASE;
		break;
#endif 

#if (THINKOS_FLAG_MAX) > 0
	case THINKOS_OBJ_FLAG:
		bmp = thinkos_rt.flag_alloc;
		idx = oid - THINKOS_FLAG_BASE;
		break;
#endif 

#if (THINKOS_EVENT_MAX) > 0
	case THINKOS_OBJ_EVENT:
		bmp = thinkos_rt.ev_alloc;
		idx = oid - THINKOS_EVENT_BASE;
		break;
#endif 

#if (THINKOS_GATE_MAX) > 0
	case THINKOS_OBJ_GATE:
		bmp = thinkos_rt.gate_alloc;
		idx = oid - THINKOS_GATE_BASE;
		break;
#endif 
	default:
		arg[0] = THINKOS_EINVAL;
		return;
	}

#else

#if (THINKOS_ENABLE_ARG_CHECK)
	if (kind > THINKOS_OBJ_GATE) {
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
	{
		unsigned int base;
		bmp = thinkos_obj_alloc_lut[kind];
		base = thinkos_obj_base_lut[kind];
		idx = oid - base;
#if (THINKOS_ENABLE_ARG_CHECK)
		max = thinkos_obj_cnt_lut[kind];
		if (idx > max) {
			arg[0] = THINKOS_ENOMEM;
			return;
		}
#endif
	}

#endif
		

	__bit_mem_wr(bmp, idx, 0);
	arg[0] = THINKOS_OK;
}

#endif

