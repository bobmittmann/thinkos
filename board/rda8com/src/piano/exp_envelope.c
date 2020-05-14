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
#include "envelope.h"

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
						struct exp_envelope_cfg cfg)
{
	float c1;
	float c2;
	float e1 = (float)1.0;
	float e2 = (float)1.0;
	float x1 = (float)0.0;
	float x0 = (float)0.0;

	env->id = cfg.id;
	c1 = (float)1.0 - cfg.k1 * dt;
	c2 = (float)1.0 - cfg.k2 * dt;

	x1 = e1 - e2;
	/* find max */
	do {
		x0 = x1;
		x1 = e1 - e2;
		e1 *= c1;
		e2 *= c2;
	} while (x1 >= x0);

	env->c1 = c1;
	env->c2 = c2;

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

