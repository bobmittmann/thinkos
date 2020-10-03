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

#if (THINKOS_ENABLE_OBJ_ALLOC)
void thinkos_obj_alloc_svc(int32_t * arg, int32_t self)
{
	enum thinkos_obj_kind kind = arg[0];
	uint32_t * bmp;
	int base;
	int max;
	int idx;

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
		
	if ((idx = __thinkos_bmp_alloc(bmp, max)) >= 0) {
		unsigned int wq;

		wq = idx + base;
		arg[0] = wq;
	} else {
		arg[0] = idx;
	}
}
#endif

#if (THINKOS_ENABLE_OBJ_FREE)
extern const uint8_t thinkos_obj_type_lut[];

static inline int __thinkos_obj_type_get(unsigned int oid) {
	return (oid >= THINKOS_WQ_CNT) ? -1 : thinkos_obj_type_lut[oid];
}

void thinkos_obj_free_svc(int32_t * arg, int32_t self)
{
	unsigned int oid = arg[0];
	uint32_t * bmp;
	int type;
	int idx;

	if (oid >= THINKOS_WQ_CNT) {
		arg[0] = THINKOS_EINVAL;
		return;
	}

	type = thinkos_obj_type_lut[oid];

	switch (type) {

#if (THINKOS_MUTEX_MAX > 0)
	case THINKOS_OBJ_MUTEX:
		bmp = thinkos_rt.mutex_alloc;
		idx = oid - THINKOS_MUTEX_BASE;
		break;
#endif 

#if (THINKOS_SEMAPHORE_MAX > 0)
	case THINKOS_OBJ_SEMAPHORE:
		bmp = thinkos_rt.sem_alloc;
		idx = oid - THINKOS_SEM_BASE;
		break;
#endif 

#if (THINKOS_COND_MAX > 0)
	case THINKOS_OBJ_COND:
		bmp = thinkos_rt.cond_alloc;
		idx = oid - THINKOS_COND_BASE;
		break;
#endif 

#if (THINKOS_FLAG_MAX > 0)
	case THINKOS_OBJ_FLAG:
		bmp = thinkos_rt.flag_alloc;
		idx = oid - THINKOS_FLAG_BASE;
		break;
#endif 

#if (THINKOS_EVENT_MAX > 0)
	case THINKOS_OBJ_EVENT:
		bmp = thinkos_rt.ev_alloc;
		idx = oid - THINKOS_EVENT_BASE;
		break;
#endif 

#if (THINKOS_GATE_MAX > 0)
	case THINKOS_OBJ_GATE:
		bmp = thinkos_rt.gate_alloc;
		idx = oid - THINKOS_GATE_BASE;
		break;
#endif 
	default:
		arg[0] = THINKOS_EINVAL;
		return;
	}
		
	__bit_mem_wr(bmp, idx, 0);
	arg[0] = THINKOS_OK;
}

#endif

/* FIXME: this is a hack to force linking this file. 
 The linker then will override the weak alias for the cm3_svc_isr() */
const char thinkos_alloc_svc_nm[] = "ALC";

