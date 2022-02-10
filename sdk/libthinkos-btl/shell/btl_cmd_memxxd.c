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
 * @file btl_cmd_memxxd.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#if 0

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
	krn_fmt_hex8(s, val >> 8);
	krn_fmt_hex8(s, val);

	return 4;
}

int krn_fmt_hex32(char * s, uint32_t val)
{
	krn_fmt_hex8(s, val >> 8);
	krn_fmt_hex8(s, val);

	return 8;
}

int krn_fmt_line_hex32(char * s, uint32_t  addr, const void * buf, int count)
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
		n = count + j;
	} else {
		j = 0;
		n = count;
	}

	cp = s;
	cp += krn_fmt_hex32(cp, base);
	__thinkos_memcpy(cp, ": ", 2);
	cp += 2;

	p = (uint8_t *)buf; 
	for (i = 0; i < n; i += sizeof(uint32_t)) {
		if (i < j) {
			__thinkos_memcpy(cp, "________ ", 9);
			cp += 9;
		} else {
			val = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
			cp += krn_fmt_hex32(cp, val);
			p += sizeof(uint32_t);
		}
	}

	for (; i < 16; i += 4) {
		__thinkos_memcpy(cp, "         ", 9);
		cp += 9; 
	}

	__thinkos_memcpy(cp, ": ", 2);
	cp += 2;

	p = (uint8_t *)buf; 
	for (i = 0; i < n; i++) {
		if (i == 8)
			*cp++ = ' ';
		if (i < j) {
			*cp++ = '.';
		} else {
			c = *p++;
			*cp++ = ((c < ' ') || (c > 126)) ? '.' : c;
		}
	}

	__thinkos_memcpy(cp, "\r\n", 3);
	cp += 3;

	return cp - s;
}

int cmd_memxxd(FILE *f, int argc, char ** argv)
{
	uint8_t buf[16];
	char ln[80];
	unsigned int base;
	unsigned int n;
	unsigned int addr;
	unsigned int count;

	if (argc < 2)
		return -1;

	if (argc > 3)
		return -1;

	addr = strtoul(argv[1], NULL, 0);

	if (argc == 3)
		count = strtoul(argv[2], NULL, 0);
	else
		count = 64;

	n = (count + (sizeof(uint32_t) - 1)) & ~(sizeof(uint32_t) - 1);

	addr &= ~(sizeof(uint32_t) - 1);
	base = addr & ~(16 - 1);
	if (base < addr) {
		n = (base + 16) - addr;
	} else {
		n = 16;
	}

	do {
		if (count < n)
			n = count;

		memcpy(buf, (void *)addr, n);

		krn_fmt_line_hex32(ln, addr, buf, n);
		addr += n;
		count -= n;
		n = 16;
	} while (count > 0);


	return 0;
}

#endif

