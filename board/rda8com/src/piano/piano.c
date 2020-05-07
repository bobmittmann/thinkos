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
 * @file usbamp.c
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "board.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fixpt.h>
#include <sys/param.h>
#include <sys/null.h>
#include <sys/console.h>
#include <thinkos.h>
#include <stdio.h>
#include "dac.h"
#include "iodrv.h"
#include "mp3lib.h"
#include "spi.h"
#include "tonegen.h"
#include "encoder.h"

float f32sin(int32_t x);

void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);


	/* Rotary Switch */
	stm32_gpio_set(IO_SW0);
	stm32_gpio_set(IO_SW1);
	stm32_gpio_set(IO_SW2);
	stm32_gpio_set(IO_SW3);
	stm32_gpio_mode(IO_SW0, INPUT, PULL_UP | SPEED_LOW);
	stm32_gpio_mode(IO_SW1, INPUT, PULL_UP | SPEED_LOW);
	stm32_gpio_mode(IO_SW2, INPUT, PULL_UP | SPEED_LOW);
	stm32_gpio_mode(IO_SW3, INPUT, PULL_UP | SPEED_LOW);

	/* DAC */
	stm32_gpio_mode(IO_DAC1, ANALOG, 0);
}

unsigned int io_sw_val(void)
{
	unsigned int addr;

	addr = (stm32_gpio_stat(IO_SW3) ? 0 : 1) +
	    (stm32_gpio_stat(IO_SW2) ? 0 : 2) +
	    (stm32_gpio_stat(IO_SW1) ? 0 : 4) +
	    (stm32_gpio_stat(IO_SW0) ? 0 : 8);

	return (addr);
}

void stdio_init(void)
{
	FILE * f;

	f = console_fopen();
//	f = null_fopen(0);
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

struct tonegen tonegen[16];

const struct dac_stream_op tonegen_op = {
		.encode = (int (*)(void *, float *, unsigned int))tonegen_pcm_encode,
		.reset = (int (*)(void *))tonegen_reset
};

const struct dac_stream tone[16] = {
	[0] = {
		.arg = &tonegen[0],
		.op = tonegen_op
	},
	[1] = {
		.arg = &tonegen[1],
		.op = tonegen_op
	},
	[2] = {
		.arg = &tonegen[2],
		.op = tonegen_op
	},
	[3] = {
		.arg = &tonegen[3],
		.op = tonegen_op
	},
	[4] = {
		.arg = &tonegen[4],
		.op = tonegen_op
	},
	[5] = {
		.arg = &tonegen[5],
		.op = tonegen_op
	},
	[6] = {
		.arg = &tonegen[6],
		.op = tonegen_op
	},
	[7] = {
		.arg = &tonegen[7],
		.op = tonegen_op
	},
	[8] = {
		.arg = &tonegen[8],
		.op = tonegen_op
	},
	[9] = {
		.arg = &tonegen[9],
		.op = tonegen_op
	},
	[10] = {
		.arg = &tonegen[10],
		.op = tonegen_op
	},
	[11] = {
		.arg = &tonegen[11],
		.op = tonegen_op
	},
	[12] = {
		.arg = &tonegen[12],
		.op = tonegen_op
	},
	[13] = {
		.arg = &tonegen[13],
		.op = tonegen_op
	},
	[14] = {
		.arg = &tonegen[14],
		.op = tonegen_op
	},
	[15] = {
		.arg = &tonegen[15],
		.op = tonegen_op
	},
};

/* White keys */
#define NOTE_C4_FREQ (float)261.6265
#define NOTE_D4_FREQ (float)293.6648
#define NOTE_E4_FREQ (float)392.6276
#define NOTE_F4_FREQ (float)349.2282

#define NOTE_G4_FREQ (float)391.9954
#define NOTE_A4_FREQ (float)440.0000
#define NOTE_B4_FREQ (float)493.8833
#define NOTE_C5_FREQ (float)523.2511

#define NOTE_D5_FREQ (float)587.3295
#define NOTE_E5_FREQ (float)659.2551
#define NOTE_F5_FREQ (float)698.4565
#define NOTE_G5_FREQ (float)783.9909

#define NOTE_A5_FREQ (float)880.0000
#define NOTE_B5_FREQ (float)987.7666
#define NOTE_C6_FREQ (float)1046.502
#define NOTE_D6_FREQ (float)1174.659

#define NOTE_E6_FREQ (float)659.2551
#define NOTE_F6_FREQ (float)698.4565
#define NOTE_G6_FREQ (float)783.9909
#define NOTE_A6_FREQ (float)880.0000

#define NOTE_B6_FREQ (float)987.7666
#define NOTE_C7_FREQ (float)1046.502
#define NOTE_D7_FREQ (float)1174.659

void note_play(int id)
{
	dac_stream_reset(&tone[id]);
	dac_stream_play(&tone[id], 0.25);
}

int play_task(void *arg)
{
	struct encoder enc0;
	struct encoder enc1;
	uint32_t k1 = 4;
	uint32_t k2 = 48;
	int32_t stat;
	int i;

	encoder_init(&enc0, k1, 1, 1024);
	encoder_init(&enc1, k2, 1, 1024);

	for (i = 0; i < 16; ++i) {
		dac_stream_set(i, &tone[i]);
		tonegen_env_set(&tonegen[i], k1, k2);
	} 

	stat = spidev_rd();

	for (;;) {
		int32_t tmp;
		int32_t diff;
		int32_t down;

		tmp = spidev_rd();

		diff = (stat ^ tmp);
		if (diff) {
			stat = tmp;

			down = diff & stat;
			for (i = 4; i < 16; ++i) {
				if (down & (1 << i)) { 
					dac_stream_reset(&tone[i]);
				}
			} 

			if (diff & (3 << 0)) { 
				k1 = encoder_decode(&enc0, stat);
			} 

			if (diff & (3 << 2)) { 
				k2 = encoder_decode(&enc1, stat >> 2);
			} 

			if (diff & (7 << 0)) { 
				for (i = 0; i < 16; ++i) {
					tonegen_env_set(&tonegen[i], k1, k2);
				} 
			}
		}
	}

	return 0;
}

uint32_t play_stack[2500] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf play_thread_inf = {
	.stack_ptr = play_stack,
	.stack_size = sizeof(play_stack),
	.priority = 16,
	.thread_id = 16,
	.paused = false,
	.tag = "PLAY"
};

void play_init(void)
{
	thinkos_thread_create_inf((int (*)(void *))play_task, (void *)NULL,
				  &play_thread_inf);
}

int main(int argc, char ** argv)
{
	int32_t sw = 0;
	float gain;
	float ampl;
	int32_t k1;
	int32_t k2;
	int i;

	stdio_init();

//	printf("Starting piano...\n");
//	for (;;) {
//		thinkos_sleep(100);
//	};

	io_init();

	spidev_init();

	dac_init();

	dac_start();

	play_init();

	for (i = 0; i < 16; ++i) {
		tonegen_init(&tonegen[i], DAC_SAMPLERATE, 1);
	}

	ampl = 0.33;
	k1 = 4;
	k2 = 64;

	tonegen_set(&tonegen[0], NOTE_C4_FREQ, ampl, k1, k2);
	tonegen_set(&tonegen[1], NOTE_D4_FREQ, ampl, k1, k2);
	tonegen_set(&tonegen[2], NOTE_E4_FREQ, ampl, k1, k2);
	tonegen_set(&tonegen[3], NOTE_F4_FREQ, ampl, k1, k2);

	tonegen_set(&tonegen[4], NOTE_G4_FREQ , ampl, k1, k2);
	tonegen_set(&tonegen[5], NOTE_A4_FREQ , ampl, k1, k2);
	tonegen_set(&tonegen[6], NOTE_B4_FREQ , ampl, k1, k2);
	tonegen_set(&tonegen[7], NOTE_C5_FREQ , ampl, k1, k2);

	tonegen_set(&tonegen[8], NOTE_D5_FREQ , ampl, k1, k2);
	tonegen_set(&tonegen[9], NOTE_E5_FREQ , ampl, k1, k2);
	tonegen_set(&tonegen[10], NOTE_F5_FREQ , ampl, k1, k2);
	tonegen_set(&tonegen[11], NOTE_G5_FREQ , ampl, k1, k2);

	tonegen_set(&tonegen[12], NOTE_A5_FREQ , ampl, k1, k2);
	tonegen_set(&tonegen[13], NOTE_B5_FREQ , ampl, k1, k2);
	tonegen_set(&tonegen[14], NOTE_C6_FREQ , ampl, k1, k2);
	tonegen_set(&tonegen[15], NOTE_D6_FREQ , ampl, k1, k2);

	for (;;) {
		int32_t tmp;

		thinkos_sleep(100);
		tmp = io_sw_val();
		if (tmp != sw) {
			sw = tmp;

			gain = ((float)sw) / 9;
			printf("gain=%f", (double)gain);
			dac_gain_set(gain);
		}
	}
	return 0;
}

