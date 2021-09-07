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

/* Write into fifo from.  */
ssize_t __krn_fifo_write(struct thinkos_fifo * fifo, const uint8_t * buf, size_t len)
{
	uint8_t * cp = (uint8_t *)buf;
	uint32_t head;
	uint32_t tail;
	uint32_t max;
	uint32_t pos;
	int cnt;

	/* fifo->head is declared as volatile, 
	   for performance reasons we read it only once at 
	   the beginning and write it back at the end. */
	head = fifo->head;
	tail = fifo->tail;
	/* get the maximum number of chars we can write into buffer */
	if ((max = (tail - head) + FIFO_LEN(fifo)) == 0) {
		return 0;
	}
	/* cnt is the number of chars we will write to the buffer,
	   it should be the minimum of max and len */
	cnt = MIN(max, len);

#if (THINKOS_ENABLE_SANITY_CHECK)
	__thinkos_mem_usr_rd_chk((uint32_t)cp, cnt); 
#endif

	/* get the head position in the buffer */
	pos = head & (FIFO_LEN(fifo) - 1);
	/* check whether to wrap around or on not */
	if ((pos + cnt) > FIFO_LEN(fifo)) {
		/* we need to perform two writes */
		int n;
		/* get the number of chars from tail pos until the end of buffer */
		n = FIFO_LEN(fifo) - pos;
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
	unsigned int max;
	uint32_t tail;
	int cnt;
	int pos;

	/* fifo->tail is declared as volatile, 
	   for performance reasons we read it only once at 
	   the beginning and write it back at the end. */
	tail = fifo->tail;
	/* get the maximum number of chars we can read from the buffer */
	if ((max = fifo->head - tail) == 0)
		return 0;

	/* cnt is the number of chars we will read from the buffer,
	   it should be the minimum of max and len */
	cnt = MIN(max, len);
	/* get the tail position in the buffer */
	pos = tail & (FIFO_LEN(fifo) - 1);
	/* check whether to wrap around or on not */
	if ((pos + cnt) > FIFO_LEN(fifo)) {
		/* we need to perform two reads */
		int n;
		/* get the number of chars from tail pos until the end of buffer */
		n = FIFO_LEN(fifo) - pos;
		__thinkos_memcpy(buf, &fifo->buf[pos], n);
		/* the remaining chars are at the beginning of the buffer */
		__thinkos_memcpy(buf + n, &fifo->buf[0], cnt - n);
	} else {
		__thinkos_memcpy(buf, &fifo->buf[pos], cnt);
	}

	fifo->tail = tail + cnt;

	return cnt;
}

