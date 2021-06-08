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

/* The floating point value is calculated as: 

   −1^sign * (1 + fraction) * 2^(exponent − bias). 

   The fraction part is in range [0, 1). 
   
   For 32-bit values sign has 1 bit, exponent has 8 bits, 
   fraction has 23 bits, and bias has value 127; 
   exponent + bias is saved as an unsigned number. */

static const uint64_t __fdiv_tab[21] = {
	0LL, 
	8LL, 
	80LL, 
	800LL, 
	8000LL, 
	80000LL, 
	800000LL, 
	8000000LL, 
	80000000ULL, 
	800000000ULL, 
	8000000000ULL, 
	80000000000ULL, 
	800000000000ULL, 
	8000000000000ULL, 
	80000000000000ULL, 
	800000000000000ULL, 
	8000000000000000ULL, 
	80000000000000000ULL, 
	800000000000000000ULL, 
	8000000000000000000ULL, 
	10000000000000000000ULL
};

static const uint64_t __fmul_tab[25] = {
	1,
	5,
	25,
	125,
	625,
	3125,
	15625,
	78125,
	390625,
	1953125,
	9765625,
	48828125,
	244140625,
	1220703125,
	6103515625,
	30517578125,
	152587890625,
	762939453125,
	3814697265625,
	19073486328125,
	95367431640625,
	476837158203125,
	2384185791015625,
	11920928955078125,
	59604644775390625
};

/* Convert a IEE754 single precision float encoded as uint32_t into 
   a string */
int u32f2str(char * buf, uint32_t x, int precision) 
{
	char * cp = buf;
	uint32_t frac;
	uint64_t y;
	int sign;
	int exp;
	int i;

	if (precision > 22)
		precision = 22;

	sign = (x & 0x80000000);
	x &= 0x7fffffff;
	exp = (x >> 23) - 127; 	
	x &= 0x007fffff;
	frac = x;

	if (exp == 128) {
		if (frac == 0) {
			if (sign)
				*cp++ = '-';
inf:
			*cp++ = 'I';
			*cp++ = 'N';
			*cp++ = 'F';
		} else {
			*cp++ = 'N';
			*cp++ = 'a';
			*cp++ = 'N';
		}
	} else if (exp == -127) {
		if (frac != 0) {
			/* Subnormal number */
		}
		if (sign)
			*cp++ = '-';
zero:
		*cp++ = '0';
		if (precision > 0)
			*cp++ = '.';
		for (i = 0; i < precision; ++i)
			*cp++ = '0';
	} else {
		uint64_t dv;
		int trim = 0;
		int pos;
		int n;
		int c;

		x += (1 << 23);

		/* normal number */
		if (sign)
			*cp++ = '-';

		if (precision < 0) {
			trim = 1;
			precision = (((22 - exp) + 3) * 621) >> 11;
		}

		y = (uint64_t)x * __fmul_tab[precision];

		exp = exp - 22 + precision;

		if (exp < 1) {
			if (exp < -40)
				goto zero;
			/* Rounding */
			y >>= -exp;
			y += 1;
			y >>= 1;
		} else {
			if (exp > 41)
				goto inf;
			y <<= exp - 1;
		}

		n = 0;
		c = 0;
		i = 1;
		for (pos = 20; pos; --pos) {	
			if (trim && (pos < precision) && (y == 0))
				break;
			if (pos == precision) {
				if (n == 0)
					*cp++ = c + '0';
				*cp++ = '.';
				n++;
			}
			c = 0;
			dv = __fdiv_tab[pos];
			for (; (i); i >>= 1) {
				if (y >= dv) {
					y -= dv;
					c += i;
				}
				dv >>= 1;
			}
			if ((c) || (n)) {
				*cp++ = c + '0';
				n++;
			}
			i = 8;
		};
	}

	*cp = '\0';

	return cp - buf;

}

#define FLOAT2UINT32(F) __extension__({ union { float f; uint32_t u; } \
									  a; a.f = (F); a.u;})
	
int float2str(char * buf, float f, int precision) 
{
	uint32_t x = FLOAT2UINT32(f);

	return u32f2str(buf, x, precision);
}

