/* 
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
 * @file rbit.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <stdint.h>

uint32_t __rbit(uint32_t x)
{
	/* swap odd and even bits */
	x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
	/* swap consecutive pairs */
	x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
	/* swap nibbles ... */
	x = ((x >> 4) & 0x0F0F0F0F) | ((x & 0x0F0F0F0F) << 4);
	/* swap bytes */
	x = ((x >> 8) & 0x00FF00FF) | ((x & 0x00FF00FF) << 8);
	/* swap 2-byte long pairs */
	x = ( x >> 16             ) | ( x               << 16);

	return x;
}

