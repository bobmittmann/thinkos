/* 
 * Copyright(c) 2004-2012 BORESTE (www.boreste.com). All Rights Reserved.
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
 * @file vfprintf.c
 * @brief YARD-ICE libc
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifdef CONFIG_H
#include "config.h"
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef PRINTF_ENABLE_LEFT_ALIGN
#define PRINTF_ENABLE_LEFT_ALIGN 1
#endif

#ifndef PRINTF_ENABLE_LARGE_PADDING
#define PRINTF_ENABLE_LARGE_PADDING 1
#endif

#ifndef PRINTF_ENABLE_FAST
#define PRINTF_ENABLE_FAST 1
#endif

#ifndef PRINTF_ENABLE_UNSIGNED
#define PRINTF_ENABLE_UNSIGNED 1
#endif

#ifndef PRINTF_ENABLE_POINTER
#define PRINTF_ENABLE_POINTER 1
#endif

#ifndef PRINTF_ENABLE_LONG
#define PRINTF_ENABLE_LONG 1
#endif

#ifndef PRINTF_ENABLE_ARG_WIDTH
#define PRINTF_ENABLE_ARG_WIDTH 1
#endif

#ifndef PRINTF_ENABLE_FLOAT
#define PRINTF_ENABLE_FLOAT 0
#endif

#ifndef PRINTF_ENABLE_HEXUP
#define PRINTF_ENABLE_HEXUP 1
#endif

int uint2dec(char * s, unsigned int val);
int uint2hex(char * s, unsigned int val);
int uint2hexup(char * s, unsigned int val);

int ull2dec(char * s, unsigned long long val);
int ull2hex(char * s, unsigned long long val);
int ull2hexup(char * s, unsigned long long val);

#if (PRINTF_ENABLE_FLOAT)

int u32f2str(char * buf, uint32_t x, int precision); 
int u64d2str(char * buf, uint64_t x, int precision); 

/* Double to uint64_t binary copy */
#define DOUBLE2UINT64(D) ({ union { double d; uint64_t u; } a; a.d = (D); a.u;})
/* Convert from double to an uint32_t encoded floating point. */
static inline uint32_t __double2u32(double val) {
	uint64_t x = DOUBLE2UINT64(val);
	return ((uint32_t)(x >> 32) & 0x80000000) + 
		(((uint32_t)((x >> 52) & 0x7f) + (uint32_t)((x >> 55) & 0x80)) << 23) +
		(((((uint32_t)(x >> 20)) & 0xffffffff) + 0x7f) >> 9);
}
#endif

#if PRINTF_ENABLE_LONG
#define BUF_LEN 22
#else
#define BUF_LEN 12
#endif

#define PERCENT 0x01
#define WIDTH 0x02
#define ZERO 0x04
#define LEFT 0x08
#define SIGN 0x10
#define LONG 0x20
#define LONG2 0x40
#define PRECISION 0x80


#if (PRINTF_ENABLE_LONG)
#undef PRINTF_ENABLE_LARGE_PADDING
#define PRINTF_ENABLE_LARGE_PADDING 1
#endif

const char __zeros[]  = {  
#if (PRINTF_ENABLE_LARGE_PADDING)
	'0', '0', '0', '0', '0', '0', '0', '0', 
	'0', '0', '0', '0', '0', '0', '0', '0', 
	'0', '0', '0', '0', 
#endif
	'0', '0', '0', '0', '0', '0', '0', '0', 
	'0', '0', '0', '0', };
	
const char __blanks[] = {
#if (PRINTF_ENABLE_LARGE_PADDING)
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
#endif
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };

int vfprintf(struct file * f, const char * fmt, va_list ap)
{
	char buf[BUF_LEN];
	char * cp;
	int flags;
	int cnt;
	int c;
	int w;
	int n;
	int r;
#if (PRINTF_ENABLE_FLOAT)
	int p;
#endif
	union {
		void * ptr;
		unsigned int n;
		int i;
		float f;
#if (PRINTF_ENABLE_LONG)
		unsigned long long ull;
		long long ll;
#endif
	} val;

	n = 0;
	w = 0;
#if (PRINTF_ENABLE_FLOAT)
	p = -1;
#endif
	cnt = 0;
#if (PRINTF_ENABLE_FAST)
	cp = (char *)fmt;
#endif
	for (flags = 0; (c = *fmt++); ) {
		if (flags == 0) {
			if (c == '%') {
				w = 0;
#if (PRINTF_ENABLE_FLOAT)
				p = -1;
#endif
				flags = PERCENT;
#if (PRINTF_ENABLE_FAST)
				if (n) {
					n = f->op->write(f->data, cp, n);
					cp = (char *)fmt;
					cnt += n;;
					n = 0;
				}
#endif
				continue;
			}

#if (PRINTF_ENABLE_FAST)
			n++;
#else
			buf[0] = c;
			cnt += f->op->write(f->data, buf, 1);
#endif
			continue;
		}

		if ((c >= '0') && (c <= '9')) {
			if (!(flags & WIDTH)) {
				flags |= WIDTH;
				if (c == '0') {
					flags |= ZERO;
					continue;
				}
			}
#if (PRINTF_ENABLE_FLOAT)
			if (flags & PRECISION)
				p = (((p << 2) + p) << 1) + (c - '0');
			else
#endif
			/* w = w * 10 + c - '0' */
			w = (((w << 2) + w) << 1) + (c - '0');
			continue;
		}

#if (PRINTF_ENABLE_FLOAT)
		if (c == '.') {
			flags |= PRECISION;
			p = 0;
			continue;
		}
#endif

#if (PRINTF_ENABLE_LEFT_ALIGN)
		if (c == '-') {
			flags |= LEFT;
			continue;
		}
#else
		if (c == '-')
			continue;
#endif

#if (PRINTF_ENABLE_ARG_WIDTH)
		if (c == '*') {
			w = va_arg(ap, int);
			continue;
		}
#endif

#if (PRINTF_ENABLE_LONG)
		if (c == 'l') {
			flags |= (flags & LONG) ? LONG2 : LONG;
			continue;
		}
#endif

		if (c == 'c') {
			c = va_arg(ap, int);
			goto print_char;
		}

		if (c == 'd') {
			cp = buf;
#if PRINTF_ENABLE_LONG
			if (flags & LONG2) {
				val.ll = va_arg(ap, long long);
				if (val.ll < 0) {
					buf[0] = '-';
					val.ll = -val.ll;
					flags |= SIGN;
					cp++;
					w--;
				}
				n = ull2dec(cp, val.ull);
			} else
#endif
			{
				val.i = va_arg(ap, int);
				if (val.i < 0) {
					buf[0] = '-';
					val.i = -val.i;
					flags |= SIGN;
					cp++;
					w--;
				}
				n = uint2dec(cp, val.n);
			}
			goto print_buf;
		}

		if (c == 'x') {
			cp = buf;
#if PRINTF_ENABLE_LONG
			if (flags & LONG2) {
				val.ull = va_arg(ap, unsigned long long);
				n = ull2hex(cp, val.ull);
			} else
#endif
			{
				val.n = va_arg(ap, unsigned int);
#if (PRINTF_ENABLE_POINTER)
hexadecimal:
#endif
				n = uint2hex(cp, val.n);
			}
			goto print_buf;
		}

#if PRINTF_ENABLE_HEXUP
		if (c == 'X') {
			cp = buf;
#if PRINTF_ENABLE_LONG
			if (flags & LONG2) {
				val.ull = va_arg(ap, unsigned long long);
				n = ull2hexup(cp, val.ull);
			} else
#endif
			{
				val.n = va_arg(ap, unsigned int);
				n = uint2hexup(cp, val.n);
			}
			goto print_buf;
		}
#endif

		if (c == 's') {
			cp = va_arg(ap, char *);
			n = 0;
			if (cp == NULL) {
				/* XXX: Null pointer! */
				goto print_buf;
			}
			while (cp[n] != '\0')
				n++;
			goto print_buf;
		}

#if (PRINTF_ENABLE_UNSIGNED)
		if (c == 'u') {
			cp = buf;
#if PRINTF_ENABLE_LONG
			if (flags & LONG2) {
				val.ull = va_arg(ap, unsigned long long);
				n = ull2dec(cp, val.ull);
			} else
#endif
			{
				val.n = va_arg(ap, unsigned int);
				n = uint2dec(cp, val.n);
			}
			goto print_buf;
		}
#endif

#if (PRINTF_ENABLE_FLOAT)
		if (c == 'f') {
			cp = buf;
			val.n = __double2u32(va_arg(ap, double));
			n = u32f2str(cp, val.n, p);
//			n = u64d2str(cp, DOUBLE2UINT64(va_arg(ap, double)), p);
			goto print_buf;
		}
#endif

#if (PRINTF_ENABLE_POINTER)
		if (c == 'p') {
			val.ptr = va_arg(ap, void *);
			w = 8;
			flags |= ZERO;
			goto hexadecimal;
		}
#endif
	
		flags = 0;
		w = 0;

print_char:
		cp = buf;
		buf[0] = c;
		n = 1;

print_buf:
#if (PRINTF_ENABLE_LEFT_ALIGN)
		if (!(flags & LEFT) && (w > n)) {
#else
		if (w > n) {
#endif
			if (flags & ZERO) {
				if (flags & SIGN) {
					flags &= ~SIGN;
					cnt += f->op->write(f->data, buf, 1);
				}
				r = f->op->write(f->data, __zeros, w - n);
			} else {
				r = f->op->write(f->data, __blanks, w - n);
			}
			cnt += r;
		}

		if (flags & SIGN) {
			cnt += f->op->write(f->data, buf, 1);
		}

		cnt += f->op->write(f->data, cp, n);

#if (PRINTF_ENABLE_LEFT_ALIGN)
		if ((flags & LEFT) && (w > n)) {
			r = f->op->write(f->data, __blanks, w - n);
			cnt += r;
		}
#endif

		flags = 0;
		w = 0;

#if (PRINTF_ENABLE_FAST)
		cp = (char *)fmt;
		n = 0;
#endif
	}

#if (PRINTF_ENABLE_FAST)
	if (n) {
		r = f->op->write(f->data, cp, n);
		cnt+= r;;
	}
#endif

	return cnt;
}

