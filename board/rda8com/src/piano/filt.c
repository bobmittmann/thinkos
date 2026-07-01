/* 
 * File:	 filt.c
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

#include "addsynth.h"
#include <stdio.h>
#include <sys/stm32f.h>

struct filt {
	float s0;
	float s1;
	float s2;
	float s3;
};

int __attribute__((noinline)) o54d_filt(struct filt * f, 
										float y[], float x[], unsigned int len)
{
	/* 
	 * Resources usage: S5 .. S15 -> 11 floating point registers
	 * 2 x coefficients
	 * 4 x state
	 * 1 x accumulator
	 * 4 x intermediate calculations
	 * */
	unsigned int m = len / 4;
	float a1 = 0.123325;
	float a2 = 0.2523;
	float t0;
	float o1;
	float o2;
	float o3;
	float o4;
	float o5;
	float o6;
	float s0;
	float s1;
	float s2;
	float s3;
	unsigned int i;
	unsigned int j;

	/* read state */
	s0 = f->s0;
	s1 = f->s1;
	s2 = f->s2;
	s3 = f->s3;

	i = 0;
	for (j = 0; j < m; ++j) {
		t0 = s1 - x[i];
		s1 = a2 * t0 - s1;
		o2 = s1 - t0;

		t0 = x[i+1]  - s0;
		o1 = a1 * t0 - s0;
		s0 = o1 - t0;

		t0 = s1 - x[i+2];
		s1 = a2 * t0 - s1;
		o4 = s1 - t0;

		t0 = x[i+3]  - s0;
		o3 = a1 * t0 - s0;
		s0 = o3 - t0;

		t0 = s3 - (o1 + o2);
		s3 = a2 * t0 - s3;
		o5 = s3 - t0;

		t0 = (o3 + o4)  - s2;
		o6 = a1 * t0 - s2;
		s2 = o6 - t0;

		y[j] = (o5 + o6) / 4;

		i += 4;
	}

	/* write back */
	f->s0 = s0;
	f->s1 = s1;
	f->s2 = s2;
	f->s3 = s3;

	return m;
}

