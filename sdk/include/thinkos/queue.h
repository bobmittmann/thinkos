/* 
 * thikos_queue.h
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

#ifndef __THINKOS_QUEUE_H__
#define __THINKOS_QUEUE_H__

#ifndef __THINKOS_QUEUE__
#error "Only use this file on privileged code"
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#ifndef __ASSEMBLER__


/*
 * Unlocked queue small (1 writer 1 reader)
 */

struct qs {
	volatile uint8_t head;
	volatile uint8_t tail;
	uint8_t flag : 3;
	uint8_t signo : 5; 
	uint8_t mask : 3;
	uint8_t ln2sz : 5; 
};


void __qs_enqueue(struct qs * q, uint32_t val) 
{
}

void __qs_dequeue(struct qs * q, uint32_t * val) 
{
}

int __qs_wr8(struct qs * q, uint8_t * ring, const uint8_t * buf, uint32_t len) 
{
	uint32_t head;
	uint32_t tail;
	uint32_t mask;
	int size;
	int used;
	int free;
	int pos;
	int cnt;

	head = q->head;
	tail = q->tail;
	size = (1 << q->ln2sz);
	mask = 0xff >> (7 - q->ln2sz);
	used = ((head - tail) & mask);
	free = size - used;
	
	if (free == 0) {
		DCC_LOG2(LOG_TRACE, "fifo full: head=%u tail=%u", head, tail);
		return 0;
	}
	/* cnt is the number of chars we will write to the buffer,
	   it should be the minimum of max and len */
	cnt = MIN(free, len);

	/* get the head position in the buffer */
	mask >>= 1;
	pos = head & mask;

	/* check whether to wrap around or on not */
	if ((pos + cnt) > size) {
		/* we need to perform two writes */
		int n;
		int m;
		/* get the number of chars from tail pos until the end of buffer */
		n = size - pos;
		/* the remaining chars are at the beginning of the buffer */
		m = cnt - n;
		__thinkos_memcpy(ring + pos, buf, n);
		__thinkos_memcpy(ring, buf + n, m);
	} else {
		__thinkos_memcpy(ring + pos, buf, cnt);
	}

	q->head = head + cnt;

	return cnt;
}

int __qs_rd8(struct qs * q, uint8_t * ring, uint8_t * buf, uint32_t len)
{
	uint32_t head;
	uint32_t tail;
	uint32_t mask;
	int size;
	int used;
	int pos;
	int cnt;

	head = q->head;
	tail = q->tail;
	size = (1 << q->ln2sz);
	mask = 0xff >> (7 - q->ln2sz);
	used = ((head - tail) & mask);
	
	if (used == 0) {
		DCC_LOG2(LOG_TRACE, "fifo empty: head=%u tail=%u", head, tail);
		return 0;
	}
	/* cnt is the number of chars we will write to the buffer,
	   it should be the minimum of max and len */
	cnt = MIN(used, len);

	/* get the tail position in the buffer */
	mask >>= 1;
	pos = tail & mask;

	/* check whether to wrap around or on not */
	if ((pos + cnt) > size) {
		/* we need to perform two writes */
		int n;
		int m;
		/* get the number of chars from tail pos until the end of buffer */
		n = size - pos;
		/* the remaining chars are at the beginning of the buffer */
		m = cnt - n;
		__thinkos_memcpy(buf, ring + pos, n);
		__thinkos_memcpy(buf + n, ring, m);
	} else {
		__thinkos_memcpy(buf, ring + pos, cnt);
	}

	q->tail = tail + cnt;

	return cnt;
}




/* get a pointer to the next available position in the
   queue. The number of contiguous available octets is returned in
   'pfree'. */ 
uint8_t * __qs_rd8_ptr(struct qs * q, uint8_t * ring, uint32_t * pfree) 
{
	uint32_t head;
	uint32_t tail;
	uint32_t mask;
	int size;
	int cnt;
	int pos;

	head = q->head;
	tail = q->tail;
	size = (1 << q->ln2sz);
	mask = 0xff >> (7 - q->ln2sz);
	cnt = ((head - tail) & mask);

	/* get the tail position in the buffer */
	pos = tail & (mask >> 1);

	/* check whether to wrap around or on not */
	if ((pos + cnt) > size)
		cnt = size - pos;

	*pfree = cnt;

	/* get the reading pointer */
	return ring + pos;
}

void __qs_rd8_commit(struct qs * q, unsigned int cnt) 
{
	uint32_t tail = q->tail;

	q->tail = tail + cnt;
}


#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_QUEUE_H__ */

