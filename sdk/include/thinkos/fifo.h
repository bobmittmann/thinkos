/* 
 * File:	 /thinkos/fifo.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __THINKOS_FIFO_H__
#define __THINKOS_FIFO_H__

#ifndef __THINKOS_FIFO__
#error "Never use <thinkos/fifo.h> directly; include <thinkos.h> instead."
#endif 

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

#include <thinkos.h>

/* ----------------------------------------------------------------------------
 *  Kernel Pipes 
 * ----------------------------------------------------------------------------
 */

struct thinkos_fifo_ctl {
	volatile uint32_t head;
	volatile uint32_t tail;
	uint8_t sz_bits; /* length of the buffer is: len = (1 << sz_bits)  */
	uint8_t hi_mark; /* high water mark */
	uint8_t lo_mark; /* low water mark */ 
	uint16_t res;
};

struct thinkos_fifo {
	struct {
		volatile uint32_t head;
		volatile uint32_t tail;
		uint16_t size;
		uint16_t mark; /* water mark */
	};
	uint8_t buf[];
};

/* Up to 32768 elements */
struct thinkos_fifo16 {
	struct {
		volatile uint16_t head;
		volatile uint16_t tail;
		uint16_t size;
		uint16_t mark; /* water mark */
	};
	uint8_t buf[];
};

/* Up to 128 elements */
struct thinkos_fifo8_hdr {
	volatile uint8_t head;
	volatile uint8_t tail;
	uint8_t size;
	uint8_t mark; /* water mark */
};

struct thinkos_fifo8 {
	struct {
		volatile uint8_t head;
		volatile uint8_t tail;
		uint8_t size;
		uint8_t mark; /* water mark */
	};
	uint8_t buf[];
};

#define THINKOS_FIFO8(__PTR) (struct thinkos_fifo8 *)(__PTR)

#define THINKOS_FIFO(__PTR) (struct thinkos_fifo *)(__PTR__)

/*
Ex: 

- length    = 64
- high mark = 32
- low mark  = 8

sz_bits = 6;
hi_bits = 5;
lo_bits = 3;

 */

#define FIFO_SIZE(_P)        ((_P)->size) 
#define FIFO_HIGH(_P)       (1 << (_P)->hi_mark) 
#define FIFO_LOW(_P)        (1 << (_P)->lo_mark) 

static inline bool __krn_fifo_isfull(struct thinkos_fifo * fifo) {
	return (fifo->tail + FIFO_SIZE(fifo)) == fifo->head;
}

static inline bool __krn_fifo_isempty(struct thinkos_fifo * fifo) {
	return fifo->tail == fifo->head;
}

static inline bool __krn_fifo8_isfull(struct thinkos_fifo8 * fifo) {
	return (fifo->tail + fifo->size) == fifo->head;
}

static inline bool __krn_fifo8_isempty(struct thinkos_fifo8 * fifo) {
	return fifo->tail == fifo->head;
}

#ifdef __cplusplus
extern "C" {
#endif

ssize_t __krn_fifo_write(struct thinkos_fifo * fifo, 
						 const uint8_t * buf, size_t len);

ssize_t __krn_fifo_read(struct thinkos_fifo * fifo, 
						uint8_t * buf, size_t len);

int __krn_fifo8_putc(struct thinkos_fifo8 * fifo, int c);

int __krn_fifo8_getc(struct thinkos_fifo8 * fifo);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_FIFO_H__ */


