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
#include <sys/serial.h>
#include <sys/i2s.h>
#include <sys/param.h>
#include <sys/delay.h>
#include <sys/console.h>

#include <sys/dcclog.h>
#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#include <thinkos.h>
#include <math.h>
#include <assert.h>
#include <trace.h>

#include "io.h"
#include "pcm.h"
#include "amp.h"

//#define SAMPLERATE 44100
#define SAMPLERATE 22050

void supervisor_init(void);
void io_init(void);

int stm32f_get_esn(void * arg)
{
	uint64_t * esn = (uint64_t *)arg;
	*esn = *((uint64_t *)STM32F_UID);
	return 0;
}

static uint32_t calc_num_samples(float freq, float rate)
{
	uint32_t periods;
	uint32_t periods_min;
	uint32_t samples;
	float err;
	float err_min;
	float x;
	float x_min;

	if (freq == 0) {
		samples = 2;
		periods = 1;
		err = 0;
	} else {
		periods = 1;

		x = (periods * rate) / freq;

		err = x - floor(x);
		periods_min = periods;
		err_min = err;
		x_min = x;
		samples = floor(x);

		while (samples < (rate / 2)) {
			periods++;
			x = ((float)(periods * rate) / freq);
			err = x - floor(x);
	
			DCC_LOG2(LOG_TRACE, "err=%d err_min=%d", 
					 (int)(err*1000000), (int)(err_min*1000000));

			if ((err - err_min) < 0.0001) {
				periods_min = periods;
				err_min = err;
				x_min = x;
			}

			if (err < 0.0001)
				break;

			samples = floor(x);
		}

		periods = periods_min;
		err = err_min;
		x = x_min;
		samples = floor(x);
	}

	DCC_LOG1(LOG_TRACE, "periods=%d", periods);

	return samples;
}

struct pcm16 * tonegen(unsigned int samplerate, float freq, float ampl)
{
	struct pcm16 * pcm;
	uint32_t nsamples;
	int i;

	/* number of samples */
	nsamples = calc_num_samples(freq, samplerate);
	if (nsamples < 1024) {
		uint32_t m = (1024 + nsamples - 1) / nsamples;
		nsamples *= m;
	}

	DCC_LOG1(LOG_TRACE, "nsamples=%d", nsamples);

	/* create a PCM buffer */
	pcm = pcm16_create(nsamples * 2, samplerate);

	DCC_LOG1(LOG_TRACE, "pcm=%p", pcm);

	/* generate a sine wave */
	pcm16_sin(pcm, freq, ampl);

	/* Expand to left/rigth */

	for (i = nsamples - 1; i >= 0; --i) {
		int16_t dat;

		dat = pcm->sample[i];

		pcm->sample[i * 2] = dat;
		pcm->sample[i * 2 + 1] = dat;
	}

	return pcm;
}


struct i2splayer {
	struct i2s_dev * i2s;
	struct pcm16 * pcm;
	int16_t offs[2];
};

void __attribute__((noreturn)) i2s_task(struct i2splayer * p)
{
	struct i2s_dev * i2s = p->i2s;
	struct pcm16 * pcm = p->pcm;
	unsigned int i;

//	i2s_setbuf(i2s, dat, dat, len);
	i2s_enable(i2s);

	for (i = 0; ; ++i) {
		int16_t * dat = pcm->sample;
		unsigned int rem = pcm->len;

		while (rem) {
			unsigned int n;

			n = i2s_send(i2s, dat, rem);
			rem -= n;
			dat += n;
//			if (rem)
//				DCC_LOG1(LOG_TRACE, "%d", n);
		}

//		DCC_LOG1(LOG_TRACE, "%d", i);
//		thinkos_sleep(1000);
	}
}

uint32_t i2s1_stack[1024];

const struct thinkos_thread_inf i2s1_thread_inf = {
	.stack_ptr = i2s1_stack, 
	.stack_size = sizeof(i2s1_stack), 
	.priority = 1,
	.thread_id = 2, 
	.paused = 0,
	.tag = "I2S1"
};

uint32_t i2s2_stack[1024];

const struct thinkos_thread_inf i2s2_thread_inf = {
	.stack_ptr = i2s2_stack, 
	.stack_size = sizeof(i2s2_stack), 
	.priority = 1,
	.thread_id = 1, 
	.paused = 0,
	.tag = "I2S2"
};

struct i2splayer player[2];

void i2s_init(void)
{
	struct i2s_dev * i2s1;
	struct i2s_dev * i2s2 = NULL;

	i2s1 = stm32_spi2_i2s_init(SAMPLERATE, 
							   I2S_TX_EN | I2S_MCK_EN | I2S_16BITS);
	i2s2 = stm32_spi3_i2s_init(SAMPLERATE, 
							   I2S_TX_EN | I2S_MCK_EN | I2S_16BITS);

	player[0].i2s = i2s1;
	player[0].pcm = tonegen(SAMPLERATE, 1000, 0.90);
	player[0].offs[0] = 0;
	player[0].offs[1] = 0;

	player[1].i2s = i2s2;
	player[1].pcm = tonegen(SAMPLERATE, 1000, 0.90);
	player[1].offs[0] = 0;
	player[1].offs[1] = 0;

	thinkos_thread_create_inf((void *)i2s_task, (void *)&player[0], 
							  &i2s1_thread_inf);

	thinkos_thread_create_inf((void *)i2s_task, (void *)&player[1], 
							  &i2s2_thread_inf);
}

void amp_offset(unsigned int ckt, int offs)
{
	struct i2splayer * p;
	int diff;
	int sel;
	int i;
	int n;

	assert(ckt <= 4);
	assert(ckt >= 1);

	if (offs < -10000)
		offs = -10000;

	if (offs > 10000)
		offs = 10000;

	if ((ckt == 1) || (ckt == 2))
		p = &player[0];
	else
		p = &player[1];


	if ((ckt == 1) || (ckt == 3))
		sel = 0;
	else 
		sel = 1;

	diff = offs - p->offs[sel];
	n = p->pcm->len;
	for (i = sel; i < n; i += 2)
		p->pcm->sample[i] += diff; 
}

/* -------------------------------------------------------------------------
 * Stdio
 * ------------------------------------------------------------------------- */

void shell_task(void * arg);

uint32_t stdio_shell_stack[512];

const struct thinkos_thread_inf stdio_shell_inf = {
	.stack_ptr = stdio_shell_stack,
	.stack_size = sizeof(stdio_shell_stack),
	.priority = 32,
	.thread_id = 31,
	.paused = false,
	.tag = "SHELL"
};

void stdio_shell_start(void)
{
	FILE * f = stdout;
	thinkos_thread_create_inf((void *)shell_task, (void *)f, &stdio_shell_inf);
}

void stdio_init(void)
{
	FILE * f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

unsigned int second;
unsigned int minute;
unsigned int hour;

int main(int argc, char ** argv)
{

	uint32_t clk;

	thinkos_udelay_factor(&udelay_factor);

	stdio_init();

	printf("1. Initializing debug tracing ... \n");
	trace_init();

	printf("2. Initializing IO... \n");
//	io_init();

	printf("3. Starting IO driver... \n");
	iodrv_init();

	printf("4. Starting amplifier ...\n");
	amp_init();

	printf("5. Starting supervisor...\n");
	supervisor_init();

	printf("6. Initializing audio subsystem ... \n");
//	line_supv_init();

	printf("7. Starting shell... \n");
	stdio_shell_start();

/*	io_set_rate(LED1C, RATE_20BPM);
	io_set_rate(LED2C, RATE_20BPM);
	io_set_rate(LED3C, RATE_20BPM);
	io_set_rate(LED4C, RATE_20BPM); 

	io_set_rate(LED1D, RATE_120BPM);
	io_set_rate(LED2D, RATE_120BPM);
	io_set_rate(LED3D, RATE_120BPM);
	io_set_rate(LED4D, RATE_120BPM);

	io_set_rate(LED1B, RATE_TEMPORAL3);
	io_set_rate(LED2B, RATE_TEMPORAL3);
	io_set_rate(LED3B, RATE_TEMPORAL3);
	io_set_rate(LED4B, RATE_TEMPORAL3); */

	printf("7. Blink some leds ... \n");
	io_set_rate(LED1A, RATE_120BPM);
	io_set_rate(LED2A, RATE_120BPM);
	io_set_rate(LED3A, RATE_120BPM);
	io_set_rate(LED4A, RATE_120BPM);

	printf("7. Wait a bit ... \n");
	DCC_LOG(LOG_TRACE, "wait a bit...");
	thinkos_sleep(2000);

	DCC_LOG(LOG_TRACE, "5. i2s_init()");
#if 0
	i2s_init();

	thinkos_sleep(2000);
	amp_offset(1, 2000);
	thinkos_sleep(2000);
	amp_offset(2, -2000);

	thinkos_sleep(2000);
	amp_offset(3, 2000);
	thinkos_sleep(2000);
	amp_offset(4, -2000);
#endif

	thinkos_sleep(2000);
	printf("7. power up amp ... \n");
	amp_power(1, true);
/*	thinkos_sleep(2000);
	amp_power(2, true);
	thinkos_sleep(2000);
	amp_power(3, true);
	thinkos_sleep(2000);
	amp_power(4, true); */

	clk = thinkos_clock();
	second = 0;
	minute = 0;
	hour = 0;
	for (;;) {
		clk += 1000;
		thinkos_alarm(clk);
		if (++second == 60) {
			second = 0;
			if (++minute == 60) {
				minute = 0;
				hour++;
			}
		}
//		INF("One second tick mark. :) ...");
//		printf("%4d:%02d:%02d tick...\n", hour, minute, second);
	}

	return 0;
}



