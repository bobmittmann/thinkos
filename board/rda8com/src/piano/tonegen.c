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
#include <fixpt.h>
#include <stdio.h>
#include <sys/stm32f.h>

int tonegen_reset(struct tonegen *tone)
{
	float e0 = tone->env.e0 * tone->a;
	tone->osc.p = 0;
	tone->env.e1 = e0;
	tone->env.e2 = e0;
	tone->env.c3 = 1.0;

	return 0;
}

int tonegen_release(struct tonegen *tone)
{
	tone->env.c3 = 0.5;

	return 0;
}

int tonegen_env_set(struct tonegen *tone, uint32_t k1, uint32_t k2)
{
	float t = tone->t;
	float c1;
	float c2;
	float e1 = (float)1.0;
	float e2 = (float)1.0;
	float x1 = (float)0.0;
	float x0 = (float)0.0;

	c1 = (float)1.0 - k1 * t;
	c2 = (float)1.0 - k2 * t;

	x1 = e1 - e2;
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


/* Conversion form float to fixed point Q1.31 */
static inline int32_t f2q31(float x) {
	return (int32_t)((float)x * (float)(2147483648.));
}

int tonegen_set(struct tonegen *tone, float freq, 
				float ampl, uint32_t k1, uint32_t k2)
{
	float t = tone->t;
	float w;

	/* Normalized frequency */
	w = 2 * freq * t;

	tone->osc.dp = f2q31(w);

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


#define LOG2_N 5
#define MASK_N 31

static const float qtab[][2] = {
	{0x0.000000p+0, 0x1.000000p+0}, /*   0 */
	{0x1.8f8b84p-3, 0x1.f6297cp-1}, /*   1 */
	{0x1.87de2ap-2, 0x1.d906bcp-1}, /*   2 */
	{0x1.1c73b4p-1, 0x1.a9b662p-1}, /*   3 */
	{0x1.6a09e6p-1, 0x1.6a09e6p-1}, /*   4 */
	{0x1.a9b662p-1, 0x1.1c73b4p-1}, /*   5 */
	{0x1.d906bcp-1, 0x1.87de2ap-2}, /*   6 */
	{0x1.f6297cp-1, 0x1.8f8b84p-3}, /*   7 */
	{0x1.000000p+0, 0x1.1a6264p-54}, /*   8 */
	{0x1.f6297cp-1, -0x1.8f8b84p-3}, /*   9 */
	{0x1.d906bcp-1, -0x1.87de2ap-2}, /*  10 */
	{0x1.a9b662p-1, -0x1.1c73b4p-1}, /*  11 */
	{0x1.6a09e6p-1, -0x1.6a09e6p-1}, /*  12 */
	{0x1.1c73b4p-1, -0x1.a9b662p-1}, /*  13 */
	{0x1.87de2ap-2, -0x1.d906bcp-1}, /*  14 */
	{0x1.8f8b84p-3, -0x1.f6297cp-1}, /*  15 */
	{0x1.1a6264p-53, -0x1.000000p+0}, /*  16 */
	{-0x1.8f8b84p-3, -0x1.f6297cp-1}, /*  17 */
	{-0x1.87de2ap-2, -0x1.d906bcp-1}, /*  18 */
	{-0x1.1c73b4p-1, -0x1.a9b662p-1}, /*  19 */
	{-0x1.6a09e6p-1, -0x1.6a09e6p-1}, /*  20 */
	{-0x1.a9b662p-1, -0x1.1c73b4p-1}, /*  21 */
	{-0x1.d906bcp-1, -0x1.87de2ap-2}, /*  22 */
	{-0x1.f6297cp-1, -0x1.8f8b84p-3}, /*  23 */
	{-0x1.000000p+0, -0x1.a79394p-53}, /*  24 */
	{-0x1.f6297cp-1, 0x1.8f8b84p-3}, /*  25 */
	{-0x1.d906bcp-1, 0x1.87de2ap-2}, /*  26 */
	{-0x1.a9b662p-1, 0x1.1c73b4p-1}, /*  27 */
	{-0x1.6a09e6p-1, 0x1.6a09e6p-1}, /*  28 */
	{-0x1.1c73b4p-1, 0x1.a9b662p-1}, /*  29 */
	{-0x1.87de2ap-2, 0x1.d906bcp-1}, /*  30 */
	{-0x1.8f8b84p-3, 0x1.f6297cp-1}, /*  31 */
	{-0x1.1a6264p-52, 0x1.000000p+0}, /*  32 */
	{0x1.8f8b84p-3, 0x1.f6297cp-1}, /*  33 */
};

#define DX ((int32_t)(1 << (32 - LOG2_N)))

static const float ftab[][2] = {
	{0x0.000000p+0, 0x1.000000p+0}, /*   0 */
	{0x1.921f10p-8, 0x1.fffd88p-1}, /*   1 */
	{0x1.921d20p-7, 0x1.fff622p-1}, /*   2 */
	{0x1.2d936cp-6, 0x1.ffe9ccp-1}, /*   3 */
	{0x1.921560p-6, 0x1.ffd886p-1}, /*   4 */
	{0x1.f69374p-6, 0x1.ffc252p-1}, /*   5 */
	{0x1.2d8658p-5, 0x1.ffa72ep-1}, /*   6 */
	{0x1.5fc00ep-5, 0x1.ff871ep-1}, /*   7 */
	{0x1.91f660p-5, 0x1.ff621ep-1}, /*   8 */
	{0x1.c428d2p-5, 0x1.ff3830p-1}, /*   9 */
	{0x1.f656e8p-5, 0x1.ff0956p-1}, /*  10 */
	{0x1.144014p-4, 0x1.fed58ep-1}, /*  11 */
	{0x1.2d520ap-4, 0x1.fe9cdap-1}, /*  12 */
	{0x1.466118p-4, 0x1.fe5f3ap-1}, /*  13 */
	{0x1.5f6d00p-4, 0x1.fe1cb0p-1}, /*  14 */
	{0x1.787586p-4, 0x1.fdd53ap-1}, /*  15 */
	{0x1.917a6cp-4, 0x1.fd88dap-1}, /*  16 */
	{0x1.aa7b72p-4, 0x1.fd3792p-1}, /*  17 */
	{0x1.c3785cp-4, 0x1.fce160p-1}, /*  18 */
	{0x1.dc70ecp-4, 0x1.fc8646p-1}, /*  19 */
	{0x1.f564e6p-4, 0x1.fc2648p-1}, /*  20 */
	{0x1.072a04p-3, 0x1.fbc162p-1}, /*  21 */
	{0x1.139f0cp-3, 0x1.fb5798p-1}, /*  22 */
	{0x1.20116ep-3, 0x1.fae8e8p-1}, /*  23 */
	{0x1.2c8106p-3, 0x1.fa7558p-1}, /*  24 */
	{0x1.38edbcp-3, 0x1.f9fce6p-1}, /*  25 */
	{0x1.45576cp-3, 0x1.f97f92p-1}, /*  26 */
	{0x1.51bdf8p-3, 0x1.f8fd60p-1}, /*  27 */
	{0x1.5e2144p-3, 0x1.f87650p-1}, /*  28 */
	{0x1.6a8130p-3, 0x1.f7ea62p-1}, /*  29 */
	{0x1.76dd9ep-3, 0x1.f7599ap-1}, /*  30 */
	{0x1.83366ep-3, 0x1.f6c3f8p-1}, /*  31 */
	{0x1.8f8b84p-3, 0x1.f6297cp-1}, /*  32 */
};

#define DFX ((int32_t)(1 << (32 - 2*LOG2_N)))

/* Circular interpolation version, using the trigonometric 
   identity: sin(a + b) = sin(a)*cos(b) + cos(a)*sin(b) 
*/
static inline float __fsinq31(int32_t x)
{
	int32_t x0;
	int32_t fx;
	int32_t ffx;
	float sin_x0;
	float cos_x0;
	float sin_fx0;
	float sin_fx1;
	float sin_fx;
	float cos_fx0;
	float cos_fx1;
	float cos_fx;
	float qfx;
	float y;
	int32_t i;

	i = x >> (32 - LOG2_N);

	x0 = i << (32 - LOG2_N);
	sin_x0 = qtab[i][0];
	cos_x0 = qtab[i][1];

	fx = (x - x0);
	i = fx >> (32 - 2*LOG2_N);

	ffx = fx - (i << (32 - 2*LOG2_N));
	qfx = ffx * (float)(1.0 / DFX);

	sin_fx0 = ftab[i][0];
	cos_fx0 = ftab[i][1];
	sin_fx1 = ftab[i + 1][0];
	cos_fx1 = ftab[i + 1][1];
	sin_fx = sin_fx0 + (sin_fx1 - sin_fx0) * qfx;
	cos_fx = cos_fx0 + (cos_fx1 - cos_fx0) * qfx;

	y = sin_x0 * cos_fx + cos_x0 * sin_fx;

	return y;
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

		y = __fsinq31(p) * (e1 - e2);
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

