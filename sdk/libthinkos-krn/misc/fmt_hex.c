/* 
 * fmt_hex.c
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

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif


int krn_fmt_hex8(char * s, uint32_t val)
{
	int c;

	c = (val >> 4) & 0x0f;
	s[0] = c + ((c < 10) ? '0' : ('a' - 10));
	c = val & 0x0f;
	s[1] = c + ((c < 10) ? '0' : ('a' - 10));

	return 2;
}

int krn_fmt_hex16(char * s, uint32_t val)
{
	s += krn_fmt_hex8(s, val >> 8);
	s += krn_fmt_hex8(s, val);
	*s = '\0';

	return 4;
}

int krn_fmt_hex32(char * s, uint32_t val)
{
	s += krn_fmt_hex16(s, val >> 16);
	s += krn_fmt_hex16(s, val);
	*s = '\0';

	return 8;
}


int krn_fmt_line_hex32(char * s, uint32_t  addr, 
					   const void * __buf, int __cnt)
{
	uint32_t base;
	uint8_t * p;
	uint32_t val;
	char * cp;
	int i;
	int j;
	int n;
	int c;

	/* 32bits addr alignement */
	addr &= ~(sizeof(uint32_t) - 1);
	/* 16 bytes base alignment */
	base = addr & ~(16 - 1);

	if (base < addr) {
		j = (addr - base);
		n = __cnt + j;
	} else {
		j = 0;
		n = __cnt;
	}

	cp = s;
	cp += krn_fmt_hex32(cp, base);
	*cp++ = ':';
	*cp++ = ' ';

	p = (uint8_t *)__buf; 
	for (i = 0; i < n; i += sizeof(uint32_t)) {
		if (i < j) {
			__thinkos_memcpy(cp, "________ ", 9);
			cp += 9; 
		} else {
			val = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
			p += sizeof(uint32_t);
			cp += krn_fmt_hex32(cp, val);
			*cp++ = ' ';
		}
	}

	for (; i < 16; i += sizeof(uint32_t)) {
		__thinkos_memcpy(cp, "         ", 9);
		cp += 9; 
	}

	*cp++ = ' ';

	p = (uint8_t *)__buf; 
	for (i = 0; i < n; i++) {
		if (i == 8) {
			c = ' ';
		} else if (i < j) {
			c = '.';
		} else {
			c = *p++;
			c = ((c < ' ') || (c > 126)) ? '.' : c;
		}
		*cp++ = c;
	}

	*cp = '\0';

	return cp - s;
}

