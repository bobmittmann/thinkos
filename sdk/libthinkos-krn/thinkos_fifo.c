/* 
 * thinkos_fifo.c
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

#define __THINKOS_FIFO__
#include <thinkos/fifo.h>
#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif


int __krn_fifo8_putc(struct thinkos_fifo8 * fifo, int c)
{
	uint32_t head = fifo->head;
	uint32_t tail = fifo->tail;
	uint32_t size = fifo->size;

	if ((int32_t)(head - tail) <= size) {
		uint32_t pos = head & (size - 1);
		fifo->buf[pos] = c;
		fifo->head = head + 1;
		return 1;
	}

	return 0;
}

int __krn_fifo8_getc(struct thinkos_fifo8 * fifo)
{
	uint32_t head = fifo->head;
	uint32_t tail = fifo->tail;
	uint32_t size = fifo->size;
	int c = 0;

	if ((int32_t)(head - tail) > 0) {
		uint32_t pos = tail & (size - 1);
		c = fifo->buf[pos];
		fifo->tail = tail + 1;
	}

	return c;
}



int __krn_fifo_putc(struct thinkos_fifo * fifo, int c)
{
	uint32_t head = fifo->head;
	uint32_t tail = fifo->tail;
	uint32_t size = fifo->size;

	if ((int32_t)(head - tail) <= size) {
		uint32_t pos = head & (size - 1);
		fifo->buf[pos] = c;
		fifo->head = head + 1;
		return 1;
	}

	return 0;
}

int __krn_fifo_getc(struct thinkos_fifo * fifo)
{
	uint32_t head = fifo->head;
	uint32_t tail = fifo->tail;
	uint32_t size = fifo->size;
	int c = 0;

	if ((int32_t)(head - tail) > 0) {
		uint32_t pos = tail & (size - 1);
		c = fifo->buf[pos];
		fifo->tail = tail + 1;
		return 1;
	}

	return c;
}


/* Write into fifo from.  */
ssize_t __krn_fifo_write(struct thinkos_fifo * fifo, 
						 const uint8_t * buf, size_t len)
{
	uint8_t * cp = (uint8_t *)buf;
	/* fifo->head is declared as volatile, 
	   for performance reasons we read it only once at 
	   the beginning and write it back at the end. */
	uint32_t head = fifo->head;
	uint32_t tail = fifo->tail;
	uint32_t size = fifo->size;
	uint32_t max;
	uint32_t pos;
	int cnt;

	/* get the maximum number of chars we can write into buffer */
	if ((max = (tail - head) + size) == 0) {
		return 0;
	}
	/* cnt is the number of chars we will write to the buffer,
	   it should be the minimum of max and len */
	cnt = MIN(max, len);

#if (THINKOS_ENABLE_SANITY_CHECK)
	__thinkos_mem_usr_rd_chk((uint32_t)cp, cnt); 
#endif

	/* get the head position in the buffer */
	pos = head & (size - 1);
	/* check whether to wrap around or on not */
	if ((pos + cnt) > size) {
		/* we need to perform two writes */
		int n;
		/* get the number of chars from tail pos until the end of buffer */
		n = size - pos;
		__thinkos_memcpy(&fifo->buf[pos], cp, n);
		/* the remaining chars are at the beginning of the buffer */
		__thinkos_memcpy(&fifo->buf[0], cp + n, cnt - n);
	} else {
		__thinkos_memcpy(&fifo->buf[pos], cp, cnt);
	}

	fifo->head = head + cnt;

	return cnt;
}

/* Read from fifo.  */
ssize_t __krn_fifo_read(struct thinkos_fifo * fifo, uint8_t * buf, size_t len)
{
	/* fifo->tail is declared as volatile, 
	   for performance reasons we read it only once at 
	   the beginning and write it back at the end. */
	uint32_t head = fifo->head;
	uint32_t tail = fifo->tail;
	uint32_t size = fifo->size;
	unsigned int max;
	int cnt;
	int pos;

	/* get the maximum number of chars we can read from the buffer */
	if ((max = head - tail) == 0)
		return 0;

	/* cnt is the number of chars we will read from the buffer,
	   it should be the minimum of max and len */
	cnt = MIN(max, len);
	/* get the tail position in the buffer */
	pos = tail & (size - 1);
	/* check whether to wrap around or on not */
	if ((pos + cnt) > size) {
		/* we need to perform two reads */
		int n;
		/* get the number of chars from tail pos until the end of buffer */
		n = size - pos;
		__thinkos_memcpy(buf, &fifo->buf[pos], n);
		/* the remaining chars are at the beginning of the buffer */
		__thinkos_memcpy(buf + n, &fifo->buf[0], cnt - n);
	} else {
		__thinkos_memcpy(buf, &fifo->buf[pos], cnt);
	}

	fifo->tail = tail + cnt;

	return cnt;
}

