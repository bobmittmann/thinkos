/* 
 * thinkos_kr-i.h
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

#ifndef __THINKOS_NRT_I_H__
#define __THINKOS_NRT_I_H__

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_NRT__
#include <thinkos/nrt.h>

#include <thinkos.h>
#include <sys/delay.h>
#include <sys/param.h>
#include <stdlib.h>
#include <vt100.h>


struct nrt_heap {
	uint8_t length;
	uint8_t size;
	uint8_t thread[];
};

struct thinkos_nrt {
	struct thinkos_krn * krn;
	/* time wait bitmap */
	uint64_t tmw_bmp;
	uint16_t wq[THINKOS_NRT_THREADS_MAX];
	struct {
		struct {
			uint8_t length;
			uint8_t size;
			uint8_t thread[THINKOS_NRT_THREADS_MAX];
		} heap;
	} clk;
};

extern struct thinkos_nrt thinkos_nrt;

static inline int32_t clk_heap_key(struct thinkos_krn * krn,
								   struct thinkos_nrt * nrt, int i) {
	unsigned int thread = nrt->clk.heap.thread[i];
	return krn->clk.th_tmr[thread] - krn->clk.time;
}

static inline int32_t clk_key(struct thinkos_krn * krn, uint8_t thread) {
	return krn->clk.th_tmr[thread] - krn->clk.time;
}

#ifdef __cplusplus
extern "C" {
#endif

/* Insert a key/value pair into the heap, maintaining the heap property, 
   i.e., the minimum value is always at the top. */
int clk_heap_insert_min(struct thinkos_krn * krn, struct nrt_heap * heap, 
						uint32_t tmo, uint8_t thread_id);

/* Get the minimum value from the heap */
int heap_minimum(struct nrt_heap * heap);

/* Remove the minimum key from the heap and reorder so the next 
 minimum will be at the top. */
int clk_heap_delete_min(struct thinkos_krn * krn, struct nrt_heap * heap);

/* Remove the minimum key from the heap and reorder so the next 
 minimum will be at the top. Returns the value and the associated key */
int clk_heap_extract_min(struct thinkos_krn * krn, 
						 struct nrt_heap * heap);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_NRT_I_H__ */

