/* 


 * File:	 exp_envelope.c
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

#include <stdio.h>
#include "dac.h"
#include "envelope.h"
#include "math.h"

static inline uint32_t dac_ms2samples(uint32_t ms)
{
	uint32_t x;

	x = ((DAC_SAMPLE_RATE * ms) + DAC_FRAME_SIZE - 1) / (DAC_FRAME_SIZE * 1000);
	return x * DAC_FRAME_SIZE;
}

static float __powfu32(float x, uint32_t n) 
{
	float y = 1;

	while (n > 0) {
		if (n % 2 == 1) 
			y *= x;
		x *= x;
		n /= 2;
	}

	return y;
}

int exp_envelope_on(struct exp_envelope * env, uint32_t clk)
{
	env->e1 = 1.0;
	env->e2 = 1.0;
	env->c1 = 1.0;
	env->c2 = 1.0;

	return 0;
}


int exp_envelope_reset(struct exp_envelope *env, uint32_t clk)
{
	env->e1 = 1.0;
	env->e2 = 1.0;

	return 0;
}

int exp_envelope_start(struct exp_envelope *env)
{

	return 0;
}

int exp_envelope_config(struct exp_envelope *env, float dt, 
						const struct envelope_cfg * cfg)
{
	float sustain_lvl;
	float e1;
	float e2;
	float c1;
	float c2;
	uint32_t nattack;
	uint32_t nhold;
	uint32_t ndecay;
	uint32_t nrelease;
	uint32_t n;
	float c2attack;
	float c1hold;
	float c1decay;
	float c1release;
	float b;
	float y;
/*
	c2 = ((e1.*c1^dn - yn) / e2) .^ (1 / dn)
	c1 = ((e2.*c2^dn + yn) / e1) .^ (1 / dn)

*/
	nattack = dac_ms2samples(cfg->attack_itv_ms);
	nhold = dac_ms2samples(cfg->hold_itv_ms);
	ndecay = dac_ms2samples(cfg->guard_itv_ms + cfg->decay_itv_ms);
	nrelease = dac_ms2samples(cfg->release_itv_ms);
	sustain_lvl = cfg->sustain_lvl;
/*
	printf("  sustain_lvl=%f\n", (double)sustain_lvl); 
	printf("attack=%d c1hold=%d\n", 
		   cfg->attack_itv_ms, cfg->hold_itv_ms);
*/
	e1 = 1;
	e2 = 1;
	c1 = 1;
	c2 = 1;
	y = 0.99;
	n = nattack;
	if ((n = nattack) > 0) {
		b = ((float)1.0 - y) / e2;
		c2 = powf(b, (float)1.0 / n);
	} else {
		c2 = .125;
	}	
	c2attack = c2;
	e1 = __powfu32(c1, n);
	e2 = __powfu32(c2, n);
/*
	printf("1.\n"); 
*/
	if ((n = nhold) > 0) {
		b = (e2 * __powfu32(c2, n) + y) / e1;
		c1 = powf(b, (float)1.0 / n);
	}
	c1hold = c1;
	e1 = __powfu32(c1, n);
	e2 = __powfu32(c2, n);

//	printf("2. c2=%f e1=%f e2=%f\n", (double)c2, (double)e1, (double)e2);
	if ((n = ndecay) > 0) {
		y = sustain_lvl;
		b = (e2 * __powfu32(c2, n) + y) / e1;
		c1 = powf(b, (float)1.0 / n);
	} else {
		c1 = .125;
	}
	c1decay = c1;
	e1 = __powfu32(c1, n);
	e2 = __powfu32(c2, n);

//	printf("3. c1=%f c2=%f e1=%f e2=%f\n", (double)c1, (double)c2,
//		   (double)e1, (double)e2);
	y = 0.01;
	if ((n = nrelease) > 0) {
		b = (e2 * __powfu32(c2, n) + y) / e1;
//		printf("5. b=%f\n", (double)b); 
		c1 = powf(b, (float)1.0 / n);
	} else {
		c1 = .5;
	}
	c1release = c1;

//	printf("attack=%d hold=%d ndecay%d nrelease=%d\n", 
//		   nattack, nhold, ndecay, nrelease);
//	printf("c2attack=%10.8f c1hold=%10.8f c1decay=%10.8f c1release=%10.8f\n", 
//		   (double)c2attack, (double)c1hold, 
//		   (double)c1decay, (double)c1release);
//	thinkos_sleep(10);

	env->c2attack = c2attack;
	env->c1hold = c1hold;
	env->c1decay = c1decay;
	env->c1release = c1release;

	return 0;
};

int exp_envelope_attack(struct exp_envelope *env, uint32_t clk)
{
	env->c2 = env->c2attack;

	return 0;
}

int exp_envelope_hold(struct exp_envelope *env, uint32_t clk)
{
	env->c1 = env->c1hold;

	return 0;
}

int exp_envelope_decay(struct exp_envelope * env, uint32_t clk)
{
	env->c1 = env->c1decay;

	return 0;
}

int exp_envelope_sustain(struct exp_envelope * env, uint32_t clk)
{
	env->c1 = 1;

	return 0;
}

int exp_envelope_release(struct exp_envelope * env, uint32_t clk)
{
	env->c1 = env->c1release;

	return 0;
}

int exp_envelope_off(struct exp_envelope * env, uint32_t clk)
{
	env->c1 = 0;

	return 0;
}

int exp_envelope_init(struct exp_envelope *env)
{

	env->c1 = 1;
	env->c2 = 1;
	env->e1 = 1;
	env->e2 = 1;

	return 0;
}

int exp_envelope_pcm_encode(struct exp_envelope *env, float pcm[], 
						unsigned int len, uint32_t clk)
{
	unsigned int i;
	float e1;
	float e2;
	float c1;
	float c2;

	e1 = env->e1;
	e2 = env->e2;
	c1 = env->c1;
	c2 = env->c2;

	for (i = 0; i < len; ++i) {
		float y;
		
		/* Apply the envelope */
		y = (e1 - e2);
		pcm[i] *= y;

		/* Compute the envelope */
		e1 *= c1;
		e2 *= c2;
	}

	env->e1 = e1;
	env->e2 = e2;

	return len;
}

