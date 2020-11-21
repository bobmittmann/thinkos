/* 
 * File:	 misc/scan_stack.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>

int __thinkos_scan_stack(void * stack, unsigned int size)
{
	uint32_t * ptr = (uint32_t *)stack;
	unsigned int i;

#if (THINKOS_ENABLE_STACK_INIT)
	for (i = 0; i < size / 4; ++i) {
		if (ptr[i] != 0xdeadbeef)
			break;
	}
#else
	for (i = 0; i < size / 4; ++i) {
		if (ptr[i] != 0)
			break;
	}
#endif

	return i * 4;
}

