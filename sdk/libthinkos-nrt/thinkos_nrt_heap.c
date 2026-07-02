/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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

/** 
 * @file nrt_test.c
 * @brief application test
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#define __THINKOS_NRT__
#include <thinkos/nrt.h>
#define __THINKOS_SVC__
#include <thinkos/svc.h>

#include "thinkos_nrt-i.h" 
#include <sys/dcclog.h>



#define HEAP_SIZE(HEAP) (HEAP)->size
#define HEAP_LENGTH(HEAP) (HEAP)->length
#define HEAP_PARENT(I) ((I) >> 1)
#define HEAP_LEFT(I) ((I) << 1)
#define HEAP_RIGHT(I) (((I) << 1) + 1)

#define HEAP_ENTRY(HEAP, I) (HEAP)->thread[I]
#define CLK_HEAP_KEY(KRN, HEAP, I) (int32_t)(KRN->clk.th_tmr[ \
											 (int)HEAP->thread[I]] - \
											 KRN->clk.time)

#define HEAP_CLK(KRN, I) (KRN)->clk.th_tmr[I]
#define HEAP_VAL(HEAP, I) (HEAP)->thread[I]

static inline void heap_exchange(struct nrt_heap * heap, int i, int j) {
    unsigned int tmp;

    tmp = HEAP_ENTRY(heap, i);
    HEAP_ENTRY(heap, i) = HEAP_ENTRY(heap, j);
    HEAP_ENTRY(heap, j) = tmp;
}


static void clk_min_heapify(struct thinkos_krn * krn, 
					 struct nrt_heap * heap, int size, int i)
{
    int l;
    int r;
    int min;

    while ((l = HEAP_LEFT(i)) <= size) {

        min = (CLK_HEAP_KEY(krn, heap, l) < CLK_HEAP_KEY(krn, heap, i)) ? l : i;

        r = HEAP_RIGHT(i);
        if ((r <= size) && (CLK_HEAP_KEY(krn, heap, r) < 
							CLK_HEAP_KEY(krn, heap, min)))
            min = r;

        if (min == i)
            break;
        heap_exchange(heap, i, min);
        i = min;
    }
}

/* Insert a key/value pair into the heap, maintaining the heap property, 
   i.e., the minimum value is always at the top. */
int clk_heap_insert_min(struct thinkos_krn * krn, struct nrt_heap * heap, 
					 uint32_t tmo, uint8_t thread_id)
{
    unsigned int i;

    if (HEAP_SIZE(heap) == HEAP_LENGTH(heap)) {
        /* overflow */
        return -1;
    }

	/* insert at the end */
    i = HEAP_SIZE(heap) + 1;
    HEAP_SIZE(heap) = i;
    HEAP_VAL(heap, i) = thread_id;
    HEAP_CLK(krn, i) = tmo;

    while ((i > 1) && (CLK_HEAP_KEY(krn, heap, HEAP_PARENT(i)) > 
					   CLK_HEAP_KEY(krn, heap, i))) {
        heap_exchange(heap, i, HEAP_PARENT(i));
        i = HEAP_PARENT(i);
    }

    return true;
}

/* Remove the minimum key from the heap and reorder so the next 
 minimum will be at the top. */
int clk_heap_delete_min(struct thinkos_krn * krn, struct nrt_heap * heap)
{
    if (HEAP_SIZE(heap) < 1)
        return -1;

    HEAP_ENTRY(heap, 1) = HEAP_ENTRY(heap, HEAP_SIZE(heap));

    HEAP_SIZE(heap) = HEAP_SIZE(heap) - 1;
    clk_min_heapify(krn, heap, HEAP_SIZE(heap), 1);

    return 0;
}

/* Remove the minimum key from the heap and reorder so the next 
 minimum will be at the top. Returns the value and the associated key */
int clk_heap_extract_min(struct thinkos_krn * krn, struct nrt_heap * heap)
{
	int val;

    if (HEAP_SIZE(heap) < 1)
        return -1;

    val = HEAP_VAL(heap, 1);

    HEAP_ENTRY(heap, 1) = HEAP_ENTRY(heap, HEAP_SIZE(heap));

    HEAP_SIZE(heap) = HEAP_SIZE(heap) - 1;
    clk_min_heapify(krn, heap, HEAP_SIZE(heap), 1);

    return val;
}


