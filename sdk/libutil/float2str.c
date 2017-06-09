/* 
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
 * @file uint2dec.c
 * @brief YARD-ICE libc
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

int uint2dec(char * s, unsigned int val);

int float2str(char * s, float val, unsigned int dec)
{
	unsigned int x;
	char * cp = s;
	int n;

	if (val < 0) {
		*cp++ = '-';
		val = - val;
	}

	x = val;
	cp += uint2dec(cp, x);

	if (dec) { 
		val -= x;
		*cp++ = '.';
		do {
			val *= 10;
			x = val;
			val -= x;
			*cp++ = x + '0';
			dec--;
		} while (dec);
	}

	*cp = '\0';
	n = cp - s;

	return n;
}

