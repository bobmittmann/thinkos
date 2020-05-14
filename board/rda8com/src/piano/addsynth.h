/* 
 * File:	 dac.h
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

#ifndef __SYNTH_H__
#define __SYNTH_H__

#include <stdint.h>
#include "dac.h"
#include "envelope.h"
#include <midi.h>

#define ADDSYNTH_OSC_MAX 6

struct osc_cfg {
	float freq;
	float ampl;
};

union env_cfg {
	struct exp_envelope_cfg exp;
};

#define ADDSYNTH_NOTE_TAG_MAX 6

struct addsynth_note_cfg {
	int8_t midi;
	char tag[ADDSYNTH_NOTE_TAG_MAX + 1];
	union env_cfg env;
	struct osc_cfg osc[8];
};

#define ADDSYNTH_INSTRUMENT_NOTE_MAX 32

struct addsynth_envelope_cfg {
	uint16_t guard_itv_ms;
	uint16_t delay_itv_ms;
	uint16_t attack_itv_ms;
	uint16_t hold_itv_ms;
	uint16_t decay_itv_ms;
	uint16_t release_itv_ms;
	float sustain_lvl;
};

#define ADDSYNTH_INSTR_TAG_MAX 15

struct addsynth_instrument_cfg {
	char tag[ADDSYNTH_INSTR_TAG_MAX + 1];
	struct addsynth_envelope_cfg envelope;
	unsigned int note_cnt;
	struct addsynth_note_cfg note[ADDSYNTH_INSTRUMENT_NOTE_MAX];
};

/* Oscillator run-time structure */
struct addsynth_osc {
	uint32_t p;
	uint32_t dp;
	float a;
};

union addsynth_envelope {
	struct exp_envelope exp;
};

/* Voice/channel/key run-time structure,

 */
struct addsynth_voice {
	uint8_t key;
	uint8_t code;
	uint8_t idx;
	char tag[ADDSYNTH_NOTE_TAG_MAX + 1];
	union addsynth_envelope env;
	struct {
		uint32_t p;
		uint32_t dp;
		float a;
	} osc[ADDSYNTH_OSC_MAX];
};

#define ADDSYNTH_INSTRUMENT_VOICE_MAX 16

#define ADDSYNTH_INSTR_KEY_FIRST    MIDI_C3
#define ADDSYNTH_INSTR_KEY_LAST     MIDI_B7
#define ADDSYNTH_INSTR_KEY_CNT      (ADDSYNTH_INSTR_KEY_LAST - \
                                     ADDSYNTH_INSTR_KEY_FIRST + 1)
struct addsynth_instrument {
	int8_t key_lut[ADDSYNTH_INSTR_KEY_CNT];
	uint8_t mutex;
	uint8_t flag;
	float dt;
	volatile uint32_t note_stat;
	struct addsynth_envelope_cfg envelope;
	const struct addsynth_instrument_cfg *cfg;
	struct addsynth_voice voice[ADDSYNTH_INSTRUMENT_VOICE_MAX];
};

extern const struct addsynth_instrument_cfg addsynth_piano_cfg;

#ifdef __cplusplus
extern "C" {
#endif

int addsynth_instr_init(struct addsynth_instrument * instr);

int addsynth_instr_config(struct addsynth_instrument * instr, 
                          const struct addsynth_instrument_cfg * cfg);

int addsynth_voice_osc_set(struct addsynth_voice * voice, float dt,
                           const struct osc_cfg cfg[], unsigned int cnt);

int addsynth_voice_env_set(struct addsynth_voice * voice, float dt,
                           union env_cfg cfg);

int addsynth_voice_pcm_encode(struct addsynth_voice  * voice, 
                              float pcm[], unsigned int len, uint32_t clk);

int addsynth_voice_reset(struct addsynth_voice  * voice, uint32_t clk);

int addsynth_voice_note_on(struct addsynth_voice * voice, 
                           int32_t vel, uint32_t clk);

int addsynth_voice_note_off(struct addsynth_voice * voice, 
							int32_t vel, uint32_t clk);

int addsynth_voice_env_config(struct addsynth_voice * voice,
							  struct addsynth_envelope_cfg cfg);
int addsynth_voice_on(struct addsynth_voice * voice, uint32_t clk);
int addsynth_voice_attack(struct addsynth_voice * voice, uint32_t clk);
int addsynth_voice_hold(struct addsynth_voice * voice, uint32_t clk);
int addsynth_voice_decay(struct addsynth_voice * voice, uint32_t clk);
int addsynth_voice_sustain(struct addsynth_voice * voice, uint32_t clk);
int addsynth_voice_release(struct addsynth_voice * voice, uint32_t clk);
int addsynth_voice_off(struct addsynth_voice * voice, uint32_t clk);

int addsynth_instr_note_on(struct addsynth_instrument * instr, 
						   int32_t code, int32_t vel);

int addsynth_instr_note_off(struct addsynth_instrument * instr, 
						   int32_t code, int32_t vel);

struct addsynth_instrument * addsynth_instrument_getinstance(void);


#ifdef __cplusplus
}
#endif
#endif /* __SYNTH_H__ */

