/*
   Copyright(C) 2011 Robinson Mittmann.
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <strings.h>

/* FLoating point number layout:

   32 bits (IEEE 754 single-precision)
       sign | 0x80000000 | 1
   exponent | 0x7f800000 | 8
   fraction | 0x007fffff | 23
       bias | 127

   64 bits (IEEE 754 double-precision)
       sign | 0x80000000 00000000 | 1
   exponent | 0x7ff00000 00000000 | 8
   fraction | 0x000fffff ffffffff | 52
       bias | 1023

   −1^sign * (1 + fraction) * 2^(exponent − bias). 
*/

/* This is a fast conversion routine that doesn't perform 
   rounding. It's intended to be used on embedded systems that support
   float only math. In this cases the compiler will promote some
   floats to double, Ex in variadic functions. In such circumstances
   its safe to just downconvert withouth checking for boundaries.
 */

#define DOUBLE2UINT64(D) ({ union { double d; uint64_t u; } a; a.d = (D); a.u;})
#define UINT322FLOAT(U) ({ union { float f; uint32_t u; } a; a.u = (U); a.f;})

float double2float(double val) 
{
	uint64_t x = DOUBLE2UINT64(val);
	uint32_t y;
	int exp;

	/* get the sign and exponent */
    y = (uint32_t)(x >> 32);
	/* adjust the exponent */
	exp = ((y >> 20) & 0x7ff) - 1023;
	/* recombine sign and exponent */
	y = (y & 0x80000000) | (((exp + 127) & 0xff) << 23);
	/* adjust the fraction */
	y |= ((uint32_t)(x >> 29)) & 0x007fffff;

	return UINT322FLOAT(y);
}

float __double2float(double val) 
{
	uint64_t x = DOUBLE2UINT64(val);
	uint32_t y;

	y = ((uint32_t)(x >> 32) & 0x80000000) | 
		((((int32_t)((uint32_t)(x >> 52) & 0x7ff) - 896) & 0xff) << 23) |
		(((uint32_t)(x >> 29)) & 0x007fffff);

	return UINT322FLOAT(y);
}

