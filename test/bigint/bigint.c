/* 
 * File:	 usb-test.c
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

#include <stdint.h>

static inline uint64_t u64m(uint32_t x, uint32_t y)
{
	return x * y;
}

static inline uint64_t u64mac(uint64_t acc, uint32_t x, uint32_t y)
{
	return acc + x * y;
}

void u64mul(uint32_t z[], uint32_t x[], uint32_t y[])
{
	uint64_t t;

	t = u64m(x[0], y[0]);
	z[0] = t;
	z[1] = x[0] * y[1] + x[1] * y[0] + (t >> 32);
}

/*
                        Y3   Y2   Y1   Y0
                        X3   X2   X1   X0
   ---------------------------------------
                       X0Y3 X0Y2 X0Y1 X0Y0
                  X1Y3 X1Y2 X1Y1 X1Y0
             X2Y3 X2Y2 X2Y1 X2Y0
        X3Y3 X3Y2 X3Y1 X3Y0
    C7   C6   C5   C4   C3   C2   C1
   ---------------------------------------
    Z7   Z6   Z5   Z4   Z3   Z2   Z1   Z0
 */
void u128mul(uint32_t z[], uint32_t x[], uint32_t y[])
{
	uint64_t t;
	uint32_t c;

	t = u64m(x[0], y[0]);
	z[0] = t;
	c = t >> 32;
	t = u64mac(c, x[0], y[1]);
	t = u64mac(t, x[1], y[0]);
	z[1] = t;
	c = t >> 32;
	t = u64mac(c, x[0], y[2]);
	t = u64mac(t, x[1], y[1]);
	t = u64mac(t, x[2], y[0]);
	z[2] = t;
	c = t >> 32;
	t = u64mac(c, x[0], y[3]);
	t = u64mac(t, x[1], y[2]);
	t = u64mac(t, x[2], y[1]);
	t = u64mac(t, x[3], y[0]);
	z[3] = t >> 32;
}


void u256mul(uint32_t z[], uint32_t x[], uint32_t y[])
{
	uint64_t t;

	t = u64m(x[0], y[0]);
	z[0] = t;
	t = u64mac(t >> 32, x[0], y[1]);
	t = u64mac(t, x[1], y[0]);
	z[1] = t;
	t = u64mac(t >> 32, x[1], y[1]);
	t = u64mac(t, x[1], y[2]);
	t = u64mac(t, x[2], y[1]);
	z[2] = t;
	t = u64mac(t >> 32, x[2], y[2]);
	t = u64mac(t, x[1], y[2]);
	t = u64mac(t, x[2], y[1]);


	z[3] = t >> 32;
}


void u512mul(uint32_t z[], uint32_t x[], uint32_t y[])
{
}

void u1024mul(uint32_t z[], uint32_t x[], uint32_t y[])
{
}

