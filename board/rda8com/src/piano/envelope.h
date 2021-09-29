/* 
 * File:	 envelope.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2020 Bob Mittmann. All Rights Reserved.
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

#ifndef __ENVELOPE_H__
#define __ENVELOPE_H__

#include <stdint.h>
/* 
   DAHDSR (delay, attack, hold, decay, sustain, release) envelopes. 
 */

struct envelope_cfg {
	uint32_t id; 
	uint16_t guard_itv_ms;
	uint16_t delay_itv_ms;
	uint16_t attack_itv_ms;
	uint16_t hold_itv_ms;
	uint16_t decay_itv_ms;
	uint16_t release_itv_ms;
	uint16_t supress_itv_ms;
	float sustain_lvl;
};


struct exp_envelope {
	int32_t id;
	uint32_t clk;
	float c2attack;
	float c1hold;
	float c1decay;
	float c1release;
	float c1supress;
	float e1;
	float e2;
	float c1;
	float c2;
};

#ifdef __cplusplus
extern "C" {
#endif

int exp_envelope_on(struct exp_envelope * env, uint32_t clk);
int exp_envelope_attack(struct exp_envelope * env, uint32_t clk);
int exp_envelope_hold(struct exp_envelope * env, uint32_t clk);
int exp_envelope_decay(struct exp_envelope * env, uint32_t clk);
int exp_envelope_sustain(struct exp_envelope * env, uint32_t clk);
int exp_envelope_supress(struct exp_envelope * env, uint32_t clk);
int exp_envelope_release(struct exp_envelope * env, uint32_t clk);
int exp_envelope_off(struct exp_envelope * env, uint32_t clk);

int exp_envelope_config(struct exp_envelope *env, float dt, 
						const struct envelope_cfg * cfg);

int exp_envelope_pcm_encode(struct exp_envelope *env, float pcm[], 
                            unsigned int len, uint32_t clk);

int exp_envelope_reset(struct exp_envelope *env, uint32_t clk);

#ifdef __cplusplus
}
#endif
#endif /* __ENVELOPE_H__ */

