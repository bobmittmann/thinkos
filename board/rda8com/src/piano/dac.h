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

#ifndef __DAC_H__
#define __DAC_H__

#ifndef DAC_FRAME_SIZE
#define DAC_FRAME_SIZE 128
#endif

#ifndef DAC_SAMPLERATE
#define DAC_SAMPLERATE 44100
#endif

struct dac_stream_op {
	int (* encode)(void *, float pcm[], unsigned int len);
	int (* reset)(void *);
};

struct dac_stream {
	void * arg;
	const struct dac_stream_op op;
};

#ifdef __cplusplus
extern "C" {
#endif

void dac_init(void);

void dac_pcm8_play(const uint8_t pcm[], unsigned int len);

int dac_mp3_play(const uint8_t data[], unsigned int len);

void dac_start(void);

void dac_stop(void);

void dac_gain_set(float gain);

void dac_stream_play(const struct dac_stream * s, float t);

void dac_stream_reset(const struct dac_stream * s);

void dac_stream_set(int id, const struct dac_stream * s);

#ifdef __cplusplus
}
#endif
#endif /* __DAC_H__ */

