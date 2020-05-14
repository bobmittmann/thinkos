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
	unsigned int j;
	unsigned int n;
	float dt;

	/* FIXME: dynamic voice allocation */
	if ((n = cfg->note_cnt) > ADDSYNTH_INSTRUMENT_VOICE_MAX) {
		printf("cfg->note_cnt > ADDSYNTH_INSTRUMENT_VOICE_MAX\n");
		return -1;
	}

	thinkos_mutex_lock(instr->mutex);

	dt = (float)1.0 / DAC_SAMPLE_RATE;
	instr->dt = dt;

	//printf("cfg->note_cnt=%d,  dt=%f\n", cfg->note_cnt, (double)dt);

	for (j = 0; j < ADDSYNTH_INSTR_KEY_CNT; ++j) {
		instr->key_lut[j] = -1;
	}

	for (j = 0; j < cfg->note_cnt; ++j) {
		const struct addsynth_note_cfg * note = &cfg->note[j];
		struct addsynth_voice * voice;
		uint32_t idx = j;
		uint32_t key;

		if (idx > ADDSYNTH_INSTRUMENT_VOICE_MAX) {
			printf("idx > ADDSYNTH_INSTRUMENT_VOICE_MAX\n");
			break;
		}

		key = note->midi - ADDSYNTH_INSTR_KEY_FIRST;
		if (key > ADDSYNTH_INSTR_KEY_CNT) {
			printf("key > ADDSYNTH_INSTR_KEY_CNT)\n\r");
			break;
		}

		printf("Config note: midi=%2d idx=%2d key=%2d\n", note->midi, idx, key);
		thinkos_sleep(10);
		instr->key_lut[key] = idx;
		voice = &instr->voice[idx];

		voice->key = key;
		voice->idx = idx;
		voice->code = note->midi;
		strcpy(voice->tag, note->tag);

		addsynth_voice_osc_set(voice, dt, note->osc, ADDSYNTH_OSC_MAX);
		addsynth_voice_env_set(voice, dt, note->env);

		dac_voice_op_set(idx + 1, 1, voice, &addsynth_osc_op);
		dac_voice_op_set(idx + 1, 2, &voice->env.exp, &exp_envelope_op);

		addsynth_voice_env_config(voice, cfg->envelope);
	}

	for (; j < ADDSYNTH_INSTRUMENT_VOICE_MAX; ++j) {
		/* unused voices */
	}


	instr->envelope = cfg->envelope;
	instr->cfg = cfg;

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

		//printf("ON voice->idx=%d\n", voice->idx);
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

#define INSTR_POLL_ITV_MS 125  /* 62.5 Hz */

enum {
	ENV_IDLE = 0,
	ENV_DELAY = 1,
	ENV_ATTACK = 2,
	ENV_HOLD = 3,
	ENV_DECAY = 4,
	ENV_SUSTAIN = 5,
	ENV_GUARD = 6,
	ENV_RELEASE = 7,
} env_state;

#define INSTR_TMR_CNT (ADDSYNTH_INSTRUMENT_VOICE_MAX + 1)
#define INSTR_ENV_CNT ADDSYNTH_INSTRUMENT_VOICE_MAX

int addsynth_instr_task(struct addsynth_instrument * instr)
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
		int32_t guard_itv_ms;
		int32_t delay_itv_ms;
		int32_t attack_itv_ms;
		int32_t hold_itv_ms;
		int32_t decay_itv_ms;
		int32_t release_itv_ms;
		int32_t min = INSTR_POLL_ITV_MS;
		int32_t key_down = 0;
		int32_t key_up = 0;
		uint32_t bmp;
		int32_t diff;
		struct addsynth_voice * voice;
		int32_t prev;

		(void)guard_itv_ms;
		(void)delay_itv_ms;
		(void)attack_itv_ms;
		(void)hold_itv_ms;
		(void)decay_itv_ms;

		thinkos_mutex_unlock(instr->mutex);

		/* periodic task, wait for alarm clock */
		thinkos_flag_timedtake(instr->flag,  min);
		clk = thinkos_clock();

		thinkos_mutex_lock(instr->mutex);

		guard_itv_ms = instr->envelope.guard_itv_ms;
		delay_itv_ms = instr->envelope.delay_itv_ms;
		attack_itv_ms = instr->envelope.attack_itv_ms;
		hold_itv_ms = instr->envelope.hold_itv_ms;
		decay_itv_ms = instr->envelope.decay_itv_ms;
		release_itv_ms = instr->envelope.release_itv_ms;

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

					voice = &instr->voice[j];

					switch (env_state[j]) {
					case ENV_IDLE:
						/* disable timer */
						tmr_en &= ~(1 << j);
						printf("[%d] idle tmr!!!!\n", j + 1);
						break;
					case ENV_DELAY:
						env_state[j] = ENV_ATTACK;
//						printf("[%d] ATTACK\n", j + 1);
						tmr_clk[j] += attack_itv_ms;
						addsynth_voice_attack(voice, clk);
						break;
					case ENV_ATTACK:
						env_state[j] = ENV_HOLD;
//						printf("[%d] HOLD\n", j + 1);
						tmr_clk[j] += hold_itv_ms;
						addsynth_voice_hold(voice, clk);
						break;
					case ENV_HOLD:
						env_state[j] = ENV_GUARD;
//						printf("[%d] GUARD %d ms\n", j + 1,  guard_itv_ms);
						tmr_clk[j] += guard_itv_ms;
						addsynth_voice_decay(voice, clk);
						break;
					case ENV_GUARD:
						env_state[j] = ENV_DECAY;
//						printf("[%d] DECAY %d ms\n", j + 1, decay_itv_ms);
						tmr_clk[j] += decay_itv_ms;
						break;
					case ENV_DECAY:
						if (key_stat & (1 << j)) {
							env_state[j] = ENV_SUSTAIN;
//							printf("[%d] SUSTAIN\n", j + 1);
							addsynth_voice_sustain(voice, clk);
							/* disable timer */
							tmr_en &= ~(1 << j);
						} else {
							env_state[j] = ENV_RELEASE;
//							printf("[%d] RELEASE\n", j + 1);
							tmr_clk[j] = clk + release_itv_ms;
							addsynth_voice_release(voice, clk);
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
						addsynth_voice_off(voice, clk);
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
				int32_t diff;

				bmp &= ~(0x80000000 >> j);  

				voice = &instr->voice[j];

				switch (env_state[j]) {
				case ENV_IDLE:
				case ENV_DECAY:
				case ENV_SUSTAIN:
				case ENV_RELEASE:
					env_state[j] = ENV_DELAY;
//					printf("[%d] DELAY\n", j + 1);
					tmr_clk[j] = clk + delay_itv_ms;
					tmr_en |= (1 << j);
					diff = clk - tmr_clk[j];
					if (diff < min) 
						min = diff;

					printf(" %s", voice->tag);
					addsynth_voice_on(voice, clk);
					break;
				case ENV_DELAY:
					break;
				case ENV_ATTACK:
					break;
				case ENV_HOLD:
					break;
				case ENV_GUARD:
					/* schedule timer blocker */
//					printf("[%d] ++guard\n", j + 1);
					tmr_clk[j] = clk + guard_itv_ms;
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
				int32_t diff;

				bmp &= ~(0x80000000 >> j);  

				voice = &instr->voice[j];

				switch (env_state[j]) {
				case ENV_IDLE:
					printf("[%d] IDLE up ???\n", j + 1);
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
//					printf("[%d] RELEASE\n", j + 1);
					tmr_clk[j] = clk + release_itv_ms;
					tmr_en |= (1 << j);
					diff = clk - tmr_clk[j];
					if (diff < min) 
						min = diff;
					addsynth_voice_release(voice, clk);
					break;
				case ENV_RELEASE:
					break;
				}

			}
		}

		/* update next alarm */
		clk += min;
	}

	return 0;
}

uint32_t addsynth_instr_stack[512] __attribute__ ((aligned(8), section(".stack")));

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
	if (instr->mutex != 0)
		return -1;

	instr->mutex = thinkos_mutex_alloc();
	instr->flag = thinkos_flag_alloc();

	/* default values */
	instr->envelope.guard_itv_ms = 5;
	instr->envelope.delay_itv_ms = 5;
	instr->envelope.attack_itv_ms = 5;
	instr->envelope.hold_itv_ms = 5;
	instr->envelope.decay_itv_ms = 5;
	instr->envelope.release_itv_ms = 5;
	instr->envelope.sustain_lvl = 0.5;

	thinkos_thread_create_inf((int (*)(void *))addsynth_instr_task, 
							  (void *)instr,
							  &addsynth_instr_thread_inf);
	return 0;
}

struct addsynth_instrument addsynth_instrument_singleton;

struct addsynth_instrument * addsynth_instrument_getinstance(void)
{
	return & addsynth_instrument_singleton;
}

