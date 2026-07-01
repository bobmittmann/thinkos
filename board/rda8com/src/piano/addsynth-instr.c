/* 
 * Copyright(C) 2013 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the usb-serial converter.
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
 * @file addsynth-instr.c
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "board.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <thinkos.h>
#include "dac.h"
#include "addsynth.h"
#include "midi.h"

const struct addsynth_instrument_cfg addsynth_null_cfg = {
	.tag = "Mute",
	.note_cnt = 0,
	.note = {
		[0] = {
			.midi = -1,
		}
	}
};


struct addsynth_voice * __voice_lookup(struct addsynth_instrument * instr,
									   int32_t code)
{
	uint32_t key = code - ADDSYNTH_INSTR_KEY_FIRST;
	uint32_t idx;

	if (key >= ADDSYNTH_INSTR_KEY_CNT)
		return NULL;

	idx = instr->key_lut[key];
	if (idx >= ADDSYNTH_INSTRUMENT_VOICE_MAX)
		return NULL;

	return &instr->voice[idx];
}

const struct dac_stream_op addsynth_osc_op = {
	.encode = (int (*)(void *, float*, unsigned int, uint32_t))
		addsynth_voice_pcm_encode,
	.reset = (int (*)(void *, uint32_t))
		addsynth_voice_reset
};

const struct dac_stream_op exp_envelope_op = {
	.encode = (int (*)(void *, float*, unsigned int, uint32_t))
		exp_envelope_pcm_encode,
	.reset = (int (*)(void *, uint32_t))
		exp_envelope_reset
};

int addsynth_instr_config(struct addsynth_instrument * instr, 
                          const struct addsynth_instrument_cfg * cfg)
{
	unsigned int dac_max;
	unsigned int j;
	unsigned int n;
	unsigned int max;
	float dt;

	/* FIXME: dynamic voice allocation */
	if ((n = cfg->note_cnt) > ADDSYNTH_INSTRUMENT_VOICE_MAX) {
		printf("cfg->note_cnt > ADDSYNTH_INSTRUMENT_VOICE_MAX\r\n");
		return -1;
	}

	thinkos_mutex_lock(instr->mutex);

	dt = (float)1.0 / DAC_SAMPLE_RATE;
	instr->dt = dt;

	//printf("cfg->note_cnt=%d,  dt=%f\n", cfg->note_cnt, (double)dt);

	for (j = 0; j < ADDSYNTH_INSTR_KEY_CNT; ++j) {
		instr->key_lut[j] = -1;
	}

	dac_max = dac_voice_max();
	max = MIN(dac_max, ADDSYNTH_INSTRUMENT_VOICE_MAX);
	max = MIN(max, cfg->note_cnt);

	for (j = 0; j < max; ++j) {
		const struct addsynth_note_cfg * note = &cfg->note[j];
		struct addsynth_voice * voice;
		uint32_t idx = j;
		uint32_t key;

		if (idx > ADDSYNTH_INSTRUMENT_VOICE_MAX) {
			printf("idx > ADDSYNTH_INSTRUMENT_VOICE_MAX\r\n");
			break;
		}

		key = note->midi - ADDSYNTH_INSTR_KEY_FIRST;
		if (key > ADDSYNTH_INSTR_KEY_CNT) {
			printf("key > ADDSYNTH_INSTR_KEY_CNT)\r\n");
			break;
		}

//		printf(" - note: midi=%2d idx=%2d key=%2d\r\n", 
//			   note->midi, idx, key);
//		thinkos_sleep(10);
		instr->key_lut[key] = idx;
		voice = &instr->voice[idx];

		voice->key = key;
		voice->idx = idx;
		voice->code = note->midi;
		strcpy(voice->tag, note->tag);
		voice->cfg = note;

		addsynth_voice_osc_set(voice, dt, note->osc, ADDSYNTH_OSC_MAX);
//		addsynth_voice_env_set(voice, dt, voice->cfg);

		dac_voice_op_set(idx + 1, 1, voice, &addsynth_osc_op);
		dac_voice_op_set(idx + 1, 2, &voice->env.exp, &exp_envelope_op);

		exp_envelope_config(&voice->env.exp, dt, &voice->cfg->env);
	}

	for (; j < ADDSYNTH_INSTRUMENT_VOICE_MAX; ++j) {
		uint32_t idx = j;

		/* unused voices */
		dac_voice_op_set(idx + 1, 1, NULL, &silence_gen_op);
		dac_voice_op_set(idx + 1, 2, NULL, &silence_gen_op);
	}

//	instr->cfg = cfg;

	thinkos_mutex_unlock(instr->mutex);

	return 0;
}

int addsynth_instr_note_on(struct addsynth_instrument * instr, 
						   int32_t code, int32_t vel)
{
	struct addsynth_voice * voice;
	uint32_t clk = dac_clock();
	int ret = 0;

	thinkos_mutex_lock(instr->mutex);

	if ((voice = __voice_lookup(instr, code)) == NULL) {
		ret = -1;
	} else {
//		printf("ON voice->idx=%d\r\n", voice->idx);
		instr->note_stat |= (1 << voice->idx);

		thinkos_flag_give(instr->flag);

		ret = addsynth_voice_note_on(voice, code, clk);
	}

	thinkos_mutex_unlock(instr->mutex);

	return ret;
}

int addsynth_instr_note_off(struct addsynth_instrument * instr, 
							int32_t code, int32_t vel)
{
	struct addsynth_voice * voice;
	uint32_t clk = dac_clock();
	int ret = 0;

	thinkos_mutex_lock(instr->mutex);

	if ((voice = __voice_lookup(instr, code) ) == NULL) {
		ret = -1;
	} else {

		//printf("OFF voice->idx=%d\n", voice->idx);
		instr->note_stat &= ~(1 << voice->idx);

		ret = addsynth_voice_note_off(voice, code, clk);
	}

	thinkos_mutex_unlock(instr->mutex);

	return ret;
}
#define INSTR_TMR_KEY1   0
#define INSTR_TMR_KEY2   1
#define INSTR_TMR_KEY3   2
#define INSTR_TMR_KEY4   3
#define INSTR_TMR_KEY5   4
#define INSTR_TMR_KEY6   5
#define INSTR_TMR_KEY7   6
#define INSTR_TMR_KEY8   7
#define INSTR_TMR_KEY9   8
#define INSTR_TMR_KEY10  10
#define INSTR_TMR_KEY11  11
#define INSTR_TMR_KEY12  12
#define INSTR_TMR_KEY13  13
#define INSTR_TMR_KEY14  14
#define INSTR_TMR_KEY15  15

#define INSTR_TMR_KEY_POLL 16

#define INSTR_KEY_CNT  16

#define INSTR_POLL_ITV_MS 16  /* 62.5 Hz */

enum {
	ENV_IDLE = 0,
	ENV_DELAY = 1,
	ENV_ATTACK = 2,
	ENV_HOLD = 3,
	ENV_DECAY = 4,
	ENV_SUSTAIN = 5,
	ENV_GUARD = 6,
	ENV_RELEASE = 7,
	ENV_SUPRESS =8
} env_state;

#define INSTR_TMR_CNT (ADDSYNTH_INSTRUMENT_VOICE_MAX + 1)
#define INSTR_ENV_CNT ADDSYNTH_INSTRUMENT_VOICE_MAX


int __attribute__ ((noreturn)) 
addsynth_instr_task(struct addsynth_instrument * instr)
{
	uint32_t tmr_clk[INSTR_TMR_CNT];
	uint8_t env_state[INSTR_ENV_CNT];
	uint32_t tmr_en;
	int32_t key_stat = 0;
	uint32_t clk;
	int j;

	/* Initialize timers */
	clk = thinkos_clock();
	for (j = 0; j < INSTR_ENV_CNT; j++) {
		tmr_clk[j] = clk;
		env_state[j] = ENV_IDLE;
	}
	/* Enable Key press poll timer */
	tmr_clk[INSTR_TMR_KEY_POLL] = clk + INSTR_POLL_ITV_MS;
	tmr_en = (1 << INSTR_TMR_KEY_POLL);

	thinkos_mutex_lock(instr->mutex);

	for (;;) {
		int32_t min = INSTR_POLL_ITV_MS;
		int32_t key_down = 0;
		int32_t key_up = 0;
		struct addsynth_voice * voice;
		uint32_t bmp;
		int32_t diff;
		int32_t prev;

		thinkos_mutex_unlock(instr->mutex);

		/* periodic task, wait for alarm clock */
		thinkos_flag_timedtake(instr->flag,  min);
		clk = thinkos_clock();

		thinkos_mutex_lock(instr->mutex);

		/* Key press polling */
		prev = key_stat;
		key_stat = instr->note_stat;
		if ((diff = (key_stat ^ prev)) != 0) {
			key_up = diff & ~key_stat;
			key_down |= diff & key_stat;
		}
		min = INSTR_POLL_ITV_MS; 

		bmp = __rbit(tmr_en);
		while ((j = __clz(bmp)) < 32) {
			bmp &= ~(0x80000000 >> j);  


			if ((diff = (int32_t)(tmr_clk[j] - clk)) <= 0) {
				if (j == INSTR_TMR_KEY_POLL) {
				} else { /* Key timeout */
					const struct addsynth_note_cfg * cfg;
					struct addsynth_envelope * env;
					int32_t itvl;

					voice = &instr->voice[j];
					env = &voice->env;
					cfg = voice->cfg;

					switch (env_state[j]) {
					case ENV_IDLE:
						/* disable timer */
						tmr_en &= ~(1 << j);
//						printf("[%d] idle tmr!!!!\r\n", j + 1);
						break;
					case ENV_DELAY:
						env_state[j] = ENV_ATTACK;
//						printf("[%d] ATTACK\n", j + 1);
						tmr_clk[j] += cfg->env.attack_itv_ms;
						exp_envelope_attack(&env->exp, clk);
						break;
					case ENV_ATTACK:
						if ((itvl = cfg->env.hold_itv_ms) > 0) {
							env_state[j] = ENV_HOLD;
//							printf("[%d] HOLD\r\n", j + 1);
							exp_envelope_hold(&env->exp, clk);
						} else {
							itvl = cfg->env.decay_itv_ms;
							env_state[j] = ENV_DECAY;
//							printf("[%d] DECAY\r\n", j + 1);
							exp_envelope_decay(&env->exp, clk);
						}	
						tmr_clk[j] = clk + itvl;
						break;
					case ENV_HOLD:
						env_state[j] = ENV_GUARD;
//						printf("[%d] GUARD %d ms\n", j + 1,  guard_itv_ms);
						tmr_clk[j] += cfg->env.guard_itv_ms;
						exp_envelope_decay(&env->exp, clk);
						break;
					case ENV_GUARD:
						env_state[j] = ENV_DECAY;
//						printf("[%d] DECAY %d ms\n", j + 1, decay_itv_ms);
//						printf("[%d] DECAY 2\r\n", j + 1);
						tmr_clk[j] += cfg->env.decay_itv_ms;
						break;
					case ENV_DECAY:
						if (key_stat & (1 << j)) {
							env_state[j] = ENV_SUSTAIN;
//							printf("[%d] SUSTAIN\r\n", j + 1);
							exp_envelope_sustain(&env->exp, clk);
							/* disable timer */
							tmr_en &= ~(1 << j);
						} else {
							env_state[j] = ENV_RELEASE;
//							printf("[%d] RELEASE\r\n", j + 1);
							tmr_clk[j] = clk + cfg->env.release_itv_ms;
							exp_envelope_release(&env->exp, clk);
						}
						break;
					case ENV_SUSTAIN:
//						printf("[%d] sustain tmr ????\n", j + 1);
						tmr_en &= ~(1 << j);
						break;
					case ENV_RELEASE:
						env_state[j] = ENV_IDLE;
//						printf("[%d] OFF\n", j + 1);
						/* disable timer */
						tmr_en &= ~(1 << j);
						exp_envelope_off(&env->exp, clk);
						break;
					}

					if ((tmr_en & (1 << j)) && 
						((diff = clk - tmr_clk[j]) < min)) {
						min = diff;
					}
				}
			} else {
				if (diff < min) 
					min = diff;
			}
		}

		if (key_down) {
			bmp = __rbit(key_down);
			while ((j = __clz(bmp)) < 32) {
				const struct addsynth_note_cfg * cfg;
				struct addsynth_envelope * env;
				int32_t itvl;

				bmp &= ~(0x80000000 >> j);  

				voice = &instr->voice[j];
				env = &voice->env;
				cfg = voice->cfg;

				switch (env_state[j]) {
				case ENV_IDLE:
				case ENV_RELEASE:
					if ((itvl = cfg->env.delay_itv_ms) > 0) {
						env_state[j] = ENV_DELAY;
						exp_envelope_on(&env->exp, clk);
//						printf("<%d> DELAY\r\n", j + 1);
					} else {
						itvl = cfg->env.attack_itv_ms;
						env_state[j] = ENV_ATTACK;
						exp_envelope_on(&env->exp, clk);
						exp_envelope_attack(&env->exp, clk);
//						printf("<%d> ATTACK\r\n", j + 1);
					}
					tmr_clk[j] = clk + itvl;
					tmr_en |= (1 << j);
					if (itvl < min) 
						min = itvl;
					break;
				case ENV_DELAY:
					break;
				case ENV_ATTACK:
					break;
				case ENV_DECAY:
					break;
				case ENV_HOLD:
					break;
				case ENV_GUARD:
					/* schedule timer blocker */
//					printf("[%d] ++guard\r\n", j + 1);
					tmr_clk[j] = clk + cfg->env.guard_itv_ms;
					tmr_en |= (1 << j);
					diff = clk - tmr_clk[j];
					if (diff < min) 
						min = diff;
					break;
				}

			}
		}

		if (key_up) {
			bmp = __rbit(key_up);
			while ((j = __clz(bmp)) < 32) {
				const struct addsynth_note_cfg * cfg;
				struct addsynth_envelope * env;
				int32_t diff;

				bmp &= ~(0x80000000 >> j);  

				voice = &instr->voice[j];
				env = &voice->env;
				cfg = voice->cfg;

				switch (env_state[j]) {
				case ENV_IDLE:
					printf(">%d< IDLE up ???\r\n", j + 1);
					break;
				case ENV_DELAY:
					break;
				case ENV_ATTACK:
					break;
				case ENV_HOLD:
					break;
				case ENV_DECAY:
					break;
				case ENV_GUARD:
					break;
				case ENV_SUSTAIN:
					env_state[j] = ENV_RELEASE;
//					printf(">%d< RELEASE\r\n", j + 1);
					tmr_clk[j] = clk + cfg->env.release_itv_ms;
					tmr_en |= (1 << j);
					diff = clk - tmr_clk[j];
					if (diff < min) 
						min = diff;
					exp_envelope_release(&env->exp, clk);
					break;
				case ENV_RELEASE:
					break;
				}

			}
		}
	}
}

uint32_t addsynth_instr_stack[512] __attribute__ ((aligned(8), 
												   section(".stack")));

const struct thinkos_thread_inf addsynth_instr_thread_inf = {
	.stack_ptr = addsynth_instr_stack,
	.stack_size = sizeof(addsynth_instr_stack),
	.priority = 6,
	.thread_id = 6,
	.paused = false,
	.tag = "INSTR"
};

int addsynth_instr_init(struct addsynth_instrument * instr)
{
	int j;

	if (instr->mutex != 0)
		return -1;

	instr->mutex = thinkos_mutex_alloc();
	instr->flag = thinkos_flag_alloc();
	instr->cfg = &addsynth_null_cfg;

	for (j = 0; j < ADDSYNTH_INSTR_KEY_CNT; ++j) {
		instr->key_lut[j] = -1;
	}

	for (j = 0; j < ADDSYNTH_INSTRUMENT_VOICE_MAX; ++j) {
		uint32_t idx = j;
		struct addsynth_voice * voice;

		voice = &instr->voice[idx];

		dac_voice_op_set(idx + 1, 1, NULL, &silence_gen_op);
		dac_voice_op_set(idx + 1, 2, NULL, &silence_gen_op);

		voice->key = 0;
		voice->idx = 0;
		voice->code = 0;
		strcpy(voice->tag, "");
		voice->cfg = NULL;
	}

	thinkos_thread_create_inf(C_TASK(addsynth_instr_task), 
							  (void *)instr,
							  &addsynth_instr_thread_inf);
	return 0;
}

struct addsynth_instrument addsynth_instrument_singleton;

struct addsynth_instrument * addsynth_instrument_getinstance(void)
{
	return & addsynth_instrument_singleton;
}

