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
#include <fixpt.h>

#include "io.h"
#include "pcm.h"
#include "amp.h"

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>
#define SAMPLERATE 44100
//#define SAMPLERATE 22050

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

	/* number of samples */
	nsamples = calc_num_samples(freq, samplerate);
	if (nsamples < 1024) {
		uint32_t m = (1024 + nsamples - 1) / nsamples;
		nsamples *= m;
	}

	DCC_LOG1(LOG_TRACE, "nsamples=%d", nsamples);

	/* create a PCM buffer */
	pcm = pcm16_create(nsamples, samplerate);

	DCC_LOG1(LOG_TRACE, "pcm=%p", pcm);

	/* generate a sine wave */
	pcm16_sin(pcm, freq, ampl);

#if 0
	/* Expand to left/rigth */
	for (i = nsamples - 1; i >= 0; --i) {
		int16_t dat;

		dat = pcm->sample[i];

		pcm->sample[i * 2] = dat;
		pcm->sample[i * 2 + 1] = dat;
	}
#endif

	return pcm;
}

#if 0
void __attribute__((noreturn)) i2s_task(struct i2splayer * p)
{
	struct i2s_dev * i2s = p->i2s;
	struct pcm16 * pcm = p->pcm;
	unsigned int i;

	for (i = 0; ; ++i) {
		int16_t * dat = pcm->sample;
		unsigned int rem = pcm->len;

		INFS("I2s send");
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
#endif

struct iir4 {
    float a[5];
    float b[5];
    float y[5]; /* output samples */
    float x[5]; /* input samples */
};

struct iir4_coef {
    float a[5];
    float b[5];
};

void iir4_init(struct iir4 * filt, const struct iir4_coef * coef) 
{
	int i;

	for (i = 0; i < 5; ++i) {
       filt->a[i] = coef->a[i];
       filt->b[i] = coef->b[i];
	}
}

static inline float iir4_filt(struct iir4 * filt, float x) 
{
    int i;

    /* shift the old samples */
    for (i = 4; i > 0; --i) {
       filt->x[i] = filt->x[i - 1];
       filt->y[i] = filt->y[i - 1];
    }

    /* Calculate the new output */
    filt->x[0] = x;
    filt->y[0] = filt->a[0] * filt->x[0];

    for(i = 1; i <= 4; ++i)
        filt->y[0] += filt->a[i] * filt->x[i] - filt->b[i] * filt->y[i];
    
    return filt->y[0];
}


#define AUDIO_FRAME_SIZE 512

struct i2splayer {
	struct i2s_dev * i2s;
	struct pcm16 * pcm;
	int16_t offs[2];
	int16_t buf[2][AUDIO_FRAME_SIZE * 2];
	struct iir4 filt[2];
};

void __attribute__((noreturn)) i2s_task(struct i2splayer * p)
{
	struct i2s_dev * i2s = p->i2s;
	struct pcm16 * pcm = p->pcm;
//	struct iir4 * filt[2];
	int16_t * buf;
	int16_t * dat;
	unsigned int rem;
	unsigned int cnt;

//	filt[0] = &p->filt[0];
//	filt[1] = &p->filt[1];
	rem = pcm->len;
	dat = pcm->sample;
	for (; ;) {
		unsigned int n;
		unsigned int i;

		//INFS("i2s_getbuf()");
		buf = i2s_getbuf(i2s);
		cnt = AUDIO_FRAME_SIZE;

		while (cnt > 0) {
			n = MIN(rem, cnt);

			/* left channel */
			for (i = 0; i < n; ++i) {
/*				float x;
				float y;

				x = Q15F(dat[i]);
				y = iir4_filt(filt[0], x); 
				buf[i * 2] = Q15(y); */
				buf[i * 2] = dat[i];
			}

			/* right channel */
			for (i = 0; i < n; ++i) {
/*				float x;
				float y;

				x = Q15F(dat[i]);
				y = iir4_filt(filt[1], x); 
				buf[i * 2 + 1] = Q15(y); */
				buf[i * 2 + 1] = dat[i];
			}

			buf += n * 2;
			dat += n;
			rem -= n;
			cnt -= n;

			if (rem == 0) {
//				INFS("I2s send");
				rem = pcm->len;
				dat = pcm->sample;
			}
		}

//		DCC_LOG1(LOG_TRACE, "%d", i);
//		thinkos_sleep(1000);
	}
}

uint32_t i2s1_stack[1024];

const struct thinkos_thread_inf i2s1_thread_inf = {
	.stack_ptr = i2s1_stack, 
	.stack_size = sizeof(i2s1_stack), 
	.priority = 8,
	.thread_id = 8, 
	.paused = 0,
	.tag = "I2S1"
};

uint32_t i2s2_stack[1024];

const struct thinkos_thread_inf i2s2_thread_inf = {
	.stack_ptr = i2s2_stack, 
	.stack_size = sizeof(i2s2_stack), 
	.priority = 8,
	.thread_id = 9, 
	.paused = 0,
	.tag = "I2S2"
};


/*
Filter type: High Pass
Filter model: Butterworth
Filter order: 4
Sampling Frequency: 44 KHz
Cut Frequency: 0.200000 KHz
Coefficents Quantization: float

Z domain Zeros
z = 1.000000 + j 0.000000
z = 1.000000 + j 0.000000
z = 1.000000 + j 0.000000
z = 1.000000 + j 0.000000

Z domain Poles
z = 0.973953 + j -0.010622
z = 0.973953 + j 0.010622
z = 0.988817 + j -0.026042
z = 0.988817 + j 0.026042
***************************************************************/
const struct iir4_coef iir_hp200 = {
    .a = {
        0.96353492152938180000,
        -3.85413968611752720000,
        5.78120952917629080000,
        -3.85413968611752720000,
        0.96353492152938180000
    },

    .b  = {
        1.00000000000000000000,
        -3.92553975070105480000,
        5.77937889715614440000,
        -3.78207900848596790000,
        0.92824049740994874000
    }
};

struct i2splayer player[2];

void i2s_init(void)
{
	struct i2s_dev * i2s1;
	struct i2s_dev * i2s2;
	struct pcm16 * tone;

	tone = tonegen(SAMPLERATE, 1000, 0.90);

	i2s1 = stm32_spi2_i2s_init(SAMPLERATE, 
							   I2S_TX_EN | I2S_MCK_EN | I2S_16BITS);
	i2s2 = stm32_spi3_i2s_init(SAMPLERATE, 
							   I2S_TX_EN | I2S_MCK_EN | I2S_16BITS);

	player[0].i2s = i2s1;
	player[0].pcm = tone;
	player[0].offs[0] = 0;
	player[0].offs[1] = 0;
	i2s_setbuf(i2s1, player[0].buf[0], player[0].buf[1], 
			   AUDIO_FRAME_SIZE * 2);

	iir4_init(&player[0].filt[0], &iir_hp200);
	iir4_init(&player[0].filt[1], &iir_hp200);

	player[1].i2s = i2s2;
	player[1].pcm = tone;
	player[1].offs[0] = 0;
	player[1].offs[1] = 0;
	i2s_setbuf(i2s2, player[1].buf[0], player[1].buf[1], 
			   AUDIO_FRAME_SIZE * 2);

	iir4_init(&player[1].filt[0], &iir_hp200);
	iir4_init(&player[1].filt[1], &iir_hp200);

	thinkos_thread_create_inf((void *)i2s_task, (void *)&player[0], 
							  &i2s1_thread_inf);

	thinkos_thread_create_inf((void *)i2s_task, (void *)&player[1], 
							  &i2s2_thread_inf);

	i2s_enable(i2s1);
	i2s_enable(i2s2);
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
	.priority = 1,
	.thread_id = 1,
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
	thinkos_sleep(1000);

	DCC_LOG(LOG_TRACE, "5. i2s_init()");
	i2s_init();
#if 0

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



