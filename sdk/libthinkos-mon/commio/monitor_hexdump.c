/* 
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

/** 
 * @file monitor_hexdump.h
 * @brief ThinkOS monitor example
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 


#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/param.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/dcclog.h>

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

int long2hex_le(char * s, unsigned long val);
int char2hex(char * s, int c);

void monitor_hexdump(const struct monitor_comm * comm, 
					const struct thinkos_mem_desc * mem,
					uint32_t addr, unsigned int size)
{
	char buf[14 + 16 * 3 + 18];
	unsigned int rem = size;
	uint8_t * cmp = (uint8_t *)-1;
	bool eq = false;

	while (rem) {
		unsigned int n = rem < 16 ? rem : 16;
		char * cp = buf;
		unsigned int i;
		uint8_t src[16];

		n = monitor_mem_read(mem, addr, src, n);
		if (n <= 0)
			break;

		DCC_LOG2(LOG_TRACE, "addr=0x%08x n=%d", addr, n);

		if (cmp != (uint8_t *)-1) {
			for (i = 0; i < n; ++i) {
				if (src[i] != cmp[i]) {
					eq = false;
					goto dump_line;
				}
			}

			if (!eq) {
				monitor_comm_send(comm, " ...\r\n", 6);
				eq = true;
			}
		} else {	

dump_line:
			cp += long2hex_le(cp, addr);
			*cp++ = ':';

			for (i = 0; i < n; ++i) {
				*cp++ = ' ';
				cp += char2hex(cp, src[i]);
			}

			*cp++ = ' ';

			for (i = 0; i < n; i++) {
				unsigned int c = src[i];
				if (i == 8)
					*cp++ = ' ';
				*cp++ = ((c < ' ') || (c > 126)) ? '.' : c;
			}

			*cp++ = '\r';
			*cp++ = '\n';

			monitor_comm_send(comm, buf, cp - buf);
		}

		cmp = (uint8_t *)buf;
		for (i = 0; i < n; i++)
			cmp[i] = src[i];

		addr += n;
		rem -= n;
	}
}

