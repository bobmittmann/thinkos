/* 
 * File:	 tonegen.c
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

#include "tonegen.h"
#include <stdio.h>
#include <sys/stm32f.h>

#define Q31(F)           ((int64_t)((double)(F) * (double)(2147483648.)))
#define Q15_SAT(X) __extension__({\
	int32_t __ret, __op1 = (X); \
	asm volatile ("ssat %0, %1, %2" : "=r" (__ret) :  "I" (16), "r" (__op1) ); \
	__ret; })


float f32sin(int32_t x);

int tonegen_reset(struct tonegen *tone)
{
	float e0 = tone->env.e0 * tone->a;
	tone->osc.p = 0;
	tone->env.e1 = e0;
	tone->env.e2 = e0;

	return 0;
}

int tonegen_env_set(struct tonegen *tone, uint32_t k1, uint32_t k2)
{
	float t = tone->t;
	float c1 = (float)1.0 - k1 * t;
	float c2 = (float)1.0 - (k1 + k2) * t;
	float e1 = (float)1.0;
	float e2 = (float)1.0;
	float x1 = (float)0.0;
	float x0 = (float)0.0;

	x1 = (e1 - e2);
	/* find max */
	do {
		x0 = x1;
		x1 = e1 - e2;
		e1 *= c1;
		e2 *= c2;
	} while (x1 >= x0);

	//printf("x0=%f\n", (double)x0);

	tone->env.e0 = (float)1.0 / x0;
	tone->env.c1 = c1;
	tone->env.c2 = c2;

	return 0;
}

int tonegen_set(struct tonegen *tone, float freq, 
				float ampl, uint32_t k1, uint32_t k2)
{
	float t = tone->t;
	float w;

	/* Normalized frequency */
	w = 2 * freq * t;

	tone->osc.dp = Q31(w);

	tone->env.c1 = (float)1.0 - k1 * t;
	tone->env.c2 = (float)1.0 - (k1 + k2) * t;

	tone->a = ampl;

	tone->osc.p = 0;
	tone->env.e1 = 0;
	tone->env.e2 = 0;

	return 0;
}

int tonegen_init(struct tonegen *tone, float samplerate, float ampl)
{
	tone->t = (float)1.0 / samplerate;
	tone->a = ampl;

	tone->env.c1 = 1;
	tone->env.c2 = 0;
	tone->env.e1 = ampl;
	tone->env.e2 = 0;

	tone->osc.p = 0;
	tone->osc.dp = 0;

	return 0;
}

/* 16 bit pcm encoding */
int tonegen_pcm16_encode(struct tonegen *tone, int16_t pcm[], unsigned int len)
{
	unsigned int i;
	int32_t p;
	int32_t dp;
	float e1;
	float e2;
	float c1;
	float c2;

//	printf("%d", len);

	p = tone->osc.p;
	dp = tone->osc.dp;
	e1 = tone->env.e1;
	e2 = tone->env.e2;
	c1 = tone->env.c1;
	c2 = tone->env.c2;

	for (i = 0; i < len; ++i) {
		int32_t y;

		y = f32sin(p) * (e1 - e2);
		/* Scale and store into the buffer */
		pcm[i] = __SSAT((int32_t)y, 16);
		/* Compute the new phase */
		p += dp;
		/* Compute the envelope */
		e1 *= c1;
		e2 *= c2;
	}

	tone->osc.p = p;
	tone->env.e1 = e1;
	tone->env.e2 = e2;

	return len;
}

/* float pcm encoding */
int tonegen_pcm_encode(struct tonegen *tone, float pcm[], unsigned int len)
{
	unsigned int i;
	int32_t p;
	int32_t dp;
	float e1;
	float e2;
	float c1;
	float c2;

//	printf("%d", len);

	p = tone->osc.p;
	dp = tone->osc.dp;
	e1 = tone->env.e1;
	e2 = tone->env.e2;
	c1 = tone->env.c1;
	c2 = tone->env.c2;

	for (i = 0; i < len; ++i) {
		float y;

		y = f32sin(p) * (e1 - e2);
		/* Scale and add into the buffer */
		pcm[i] += y;
		/* Compute the new phase */
		p += dp;
		/* Compute the envelope */
		e1 *= c1;
		e2 *= c2;
	}

	tone->osc.p = p;
	tone->env.e1 = e1;
	tone->env.e2 = e2;

	return len;
}

