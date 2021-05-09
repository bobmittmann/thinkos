/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 *
 * This file is part of the YARD-ICE.
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
 * @file stm32f-dac.c
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 


#include <sys/stm32f.h>
#include <stdio.h>
#include <stdlib.h>
#include <thinkos.h>
#include <fixpt.h>

#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>

#include "jitbuf.h"
#include "g711.h"
#include "board.h"

#define DAC1_GPIO STM32_GPIOA
#define DAC1_PORT 4

#define DAC2_GPIO STM32_GPIOA
#define DAC2_PORT 5

#define DAC1_DMA_STRM 5
#define DAC1_DMA_CHAN 7

#define SAMPLE_RATE 11025

void wave_set(uint8_t * wave, unsigned int len)
{
	struct stm32f_dma * dma = STM32F_DMA1;

	/* disable DMA */
	dma->s[DAC1_DMA_STRM].cr &= ~DMA_EN;	
	/* Wait for the channel to be ready .. */
	while (dma->s[DAC1_DMA_STRM].cr & DMA_EN); 
	/* Memory address */
	dma->s[DAC1_DMA_STRM].m0ar = wave;
	/* Number of data items to transfer */
	dma->s[DAC1_DMA_STRM].ndtr = len;
}

void wave_play(void)
{
	struct stm32f_dma * dma = STM32F_DMA1;
	/* enable DMA */
	dma->s[DAC1_DMA_STRM].cr |= DMA_EN;	
}

void wave_pause(void)
{
	struct stm32f_dma * dma = STM32F_DMA1;
	/* disable DMA */
	dma->s[DAC1_DMA_STRM].cr &= ~DMA_EN;	
}


struct sndbuf sndbuf_zero = {
		.ref = 0,
		.data = { [0 ... SNDBUF_LEN - 1] = (1 << 15) }
};

void stm32_dma1_stream5_isr(void);

void stm32f_dac_init(unsigned int freq)
{
	struct stm32f_dac * dac = STM32F_DAC;
	struct stm32f_tim * tim = STM32F_TIM2;
	struct stm32f_dma * dma = STM32F_DMA1;
	uint32_t div;
	uint32_t pre;
	uint32_t n;

	/* I/O pins config */
//	stm32_gpio_clock_en(DAC2_GPIO);
	stm32_gpio_mode(DAC2_GPIO, DAC2_PORT, ANALOG, 0);
	stm32_gpio_mode(DAC1_GPIO, DAC1_PORT, ANALOG, 0);

	/* DAC clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_DAC);
	/* DAC disable */
	dac->cr = 0;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM2);
	/* Timer disable */
	tim->cr1 = TIM_URS | TIM_CEN;

	/* DMA clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_DMA1);
	/* DMA Disable */
	dma->s[DAC1_DMA_STRM].cr = 0;
	/* Wait for the channel to be ready .. */
	while (dma->s[DAC1_DMA_STRM].cr & DMA_EN); 

	/* DAC configure */
	dac->cr = DAC_EN2 | DAC_EN1 | DAC_TSEL1_TIMER2 | DAC_TEN1 | DAC_DMAEN1;
	/* DAC channel 2 initial value */
	dac->dhr12r2 = 2048;

	/* get the total divisior */
	div = ((stm32f_tim1_hz) + (freq / 2)) / freq;
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div + pre / 2) / pre;

	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->cnt = 0;
	tim->egr = 0;
	tim->dier = TIM_UIE; /* Update interrupt enable */
	tim->ccmr1 = TIM_OC1M_PWM_MODE1;
	tim->ccr1 = tim->arr - 2;
	tim->cr2 = TIM_MMS_OC1REF;
	tim->cr1 = TIM_URS | TIM_CEN; /* Enable counter */

	/*  DMA Configuration */
	/* Peripheral address */
//	dma->s[DAC1_DMA_STRM].par = &dac->dhr12r1;

	dma->s[DAC1_DMA_STRM].par = &dac->dhr12l1;

	/* Memory address */
	dma->s[DAC1_DMA_STRM].m0ar = (void *)sndbuf_zero.data;
	dma->s[DAC1_DMA_STRM].m1ar = (void *)sndbuf_zero.data;
	/* Number of data items to transfer */
	dma->s[DAC1_DMA_STRM].ndtr = SNDBUF_LEN;

#if 0
	/* Configuration single buffer circular */
	dma->s[DAC1_DMA_STRM].cr = DMA_CHSEL_SET(DAC1_DMA_CHAN) | 
		DMA_MBURST_1 | DMA_PBURST_1 | 
		DMA_MSIZE_8 | DMA_PSIZE_8 | DMA_MINC | 
		DMA_CIRC | DMA_DIR_MTP;
#endif

	/* Configuration for double buffer circular */
	dma->s[DAC1_DMA_STRM].cr = DMA_CHSEL_SET(DAC1_DMA_CHAN) |
		DMA_MBURST_1 | DMA_PBURST_1 | DMA_MSIZE_16 | DMA_PSIZE_16 |
		DMA_CT_M0AR | DMA_DBM | DMA_CIRC | DMA_MINC | DMA_DIR_MTP |
		DMA_TCIE | DMA_TEIE | DMA_DMEIE;

	dma->s[DAC1_DMA_STRM].fcr = DMA_FEIE | DMA_DMDIS | DMA_FTH_FULL;

	/* Enable DMA interrupt */
//	thinkos_irq_register(STM32F_IRQ_DMA1_STREAM5, 0xff, stm32_dma1_stream5_isr);
}


struct iir2_filt {
    int32_t y[3];
    int16_t x[3];
};

struct audio_player {
	volatile unsigned int dma_cnt;
	volatile unsigned int clk;
	unsigned int sps;
	struct jitbuf jbuf;
	struct sndbuf * pcm[2];
	int32_t gain;
	int32_t offs;
	struct iir2_filt filt;
};

struct line_supervisor {
	int32_t offs;
	volatile uint32_t supv;
	volatile uint32_t ccr2;
	volatile uint32_t ccr1;
};

struct audio_player audio;

struct line_supervisor line;

void stm32_dma1_stream5_isr(void)
{
	struct stm32f_dma * dma = STM32F_DMA1;

    if (dma->hisr & DMA_CFEIF5) {
        WARN("DMA_CFEIF5");
        dma->hifcr = DMA_CFEIF5;
    }

    if (dma->hisr & DMA_TEIF5) {
        WARN("DMA_TEIF5");
        dma->hifcr = DMA_CTEIF5;
    }

	if ((dma->hisr & DMA_TCIF5) == 0) {
        WARN("!= DMA_TCIF5");
		return;
	}

	/* clear the DMA transfer complete flag */
	dma->hifcr = DMA_CTCIF5;

	if (dma->s[DAC1_DMA_STRM].cr & DMA_CT) {
		if (audio.pcm[0] != (struct sndbuf *)&sndbuf_zero)
			sndbuf_free(audio.pcm[0]);
		if ((audio.pcm[0] = jitbuf_dequeue(&audio.jbuf)) == NULL)
			audio.pcm[0] = (struct sndbuf *)&sndbuf_zero;
		dma->s[DAC1_DMA_STRM].m0ar = audio.pcm[0]->data;
	} else {
		if (audio.pcm[1] != (struct sndbuf *)&sndbuf_zero)
			sndbuf_free(audio.pcm[1]);
		if ((audio.pcm[1] = jitbuf_dequeue(&audio.jbuf)) == NULL)
			audio.pcm[1] = (struct sndbuf *)&sndbuf_zero;
		dma->s[DAC1_DMA_STRM].m1ar = audio.pcm[1]->data;
	}

	audio.dma_cnt++;
}

int audio_task(void * arg)
{
	struct stm32f_dma * dma = STM32F_DMA1;
	unsigned int samples;
	unsigned int time = 0;

	INF("Audio player started (thread %d).", thinkos_thread_self());

	audio.dma_cnt = 0;
	audio.clk = thinkos_clock();
	audio.pcm[0] = (struct sndbuf *)&sndbuf_zero;
	audio.pcm[1] = (struct sndbuf *)&sndbuf_zero;

	dma->s[DAC1_DMA_STRM].m0ar = audio.pcm[0]->data;
	dma->s[DAC1_DMA_STRM].m1ar = audio.pcm[1]->data;
	dma->s[DAC1_DMA_STRM].ndtr = SNDBUF_LEN;

#if 0
	dma->s[DAC1_DMA_STRM].m1ar = (void *)wave_a4s;
	dma->s[DAC1_DMA_STRM].m0ar = (void *)wave_a4s;
	/* Number of data items to transfer */
	dma->s[DAC1_DMA_STRM].ndtr = sizeof(wave_a4s) / 2;
#endif
	/* enable DMA */
	dma->s[DAC1_DMA_STRM].cr |= DMA_EN;

	for (;;) {
		thinkos_irq_wait(STM32F_IRQ_DMA1_STREAM5);
		stm32_dma1_stream5_isr();

		audio.clk += 1000;
//		thinkos_alarm(audio.clk);
		time++;
		samples = audio.dma_cnt * SNDBUF_LEN;
		audio.sps = (samples + time / 2) / time;
	}

	return 0;
}

void stm32_dac2_set(uint32_t val)
{
	struct stm32f_dac * dac = STM32F_DAC;
	/* DAC channel 2 initial value */
	dac->dhr12r2 = val;
}

uint32_t audio_stack[1024];

const struct thinkos_thread_inf audio_inf = {
	.stack_ptr = audio_stack,
	.stack_size = sizeof(audio_stack),
	.priority = 32,
	.thread_id = 2,
	.paused = 0,
	.tag = "AUDIO"
};

#define AUDIO_DAC_OVERSAMPLE 2

#define STREAM_SAMPLE_RATE 11025
#define STREAM_CLOCK_RATE 11025

#define AUDIO_SAMPLE_RATE (STREAM_SAMPLE_RATE * AUDIO_DAC_OVERSAMPLE)

struct audio_player * audio_init(void)
{
	sndbuf_pool_init();

	audio.gain = Q15(0.125);
	audio.offs = 0;

	stm32f_dac_init(AUDIO_SAMPLE_RATE);

	jitbuf_init(&audio.jbuf, STREAM_CLOCK_RATE, AUDIO_SAMPLE_RATE,
			50, &sndbuf_zero);

	thinkos_thread_create_inf((void *)audio_task, (void *)NULL, &audio_inf);

	return &audio;
}

void audio_gain_set(int32_t gain)
{
	if (gain > Q15(8.0))
		gain = Q15(8.0);
	else if (gain < Q15(-8.0))
		gain = Q15(-8.0);

	audio.gain = gain;
}

int32_t audio_gain_get(void)
{
	return audio.gain;
}

void audio_offset_set(int32_t offs)
{
	int32_t diff;

	if (offs >= Q15(1.0))
		offs = Q15(1.0) - 1;
	else if (offs < Q15(-1.0))
		offs = Q15(-1.0);

	INF("Audio DC offset=%d!", offs);

	if ((diff = (offs - audio.offs)) != 0) {
		int i;

		audio.offs = offs;
		for (i = 0; i < SNDBUF_LEN; ++i)
			sndbuf_zero.data[i] += diff;
	}
}

int32_t audio_offset_get(void)
{
	return audio.offs;
}

void line_offset_set(int32_t offs)
{
	char s[16];

	if (offs > 2047)
		offs = 2047;
	else if (offs < -2048)
		offs = -2048;

	DBG("Speaker line DC offset=%d!", offs);

	if (offs != audio.offs) {
		line.offs = offs;

		sprintf(s, "%d", offs);
		setenv("OFFSET", s, 1);
		/* DAC channel 2 initial value */
		stm32_dac2_set(2048 - offs);
	}
}

int32_t line_offset_get(void)
{
	return line.offs;
}

uint32_t line_supv_get(void)
{
	uint32_t val;
	float vl = 10;
	float vs = 2.5;
	float cs = .000001;
	float ib = .000028;
	float dt;
	float r;

	dt = (float)(line.supv * 10 / 8) / 1000000;
	r = (float)(.05*vl*dt)/(cs*vs - ib*dt);

	val = r;

//	printf("Speaker line=%d val=%d", line.supv, val);
//	printf(" CCR1=%d", line.ccr1);
//	printf(" CCR2=%d\n", line.ccr2);

//	printf("Speaker line=%d CCR1=%d CCR2=%d val=%d\n", 
//		line.supv, line.ccr1, line.ccr2, val);

	printf("Speaker line dT=%dms R=%d\n", (int)(dt * 1000), val);
//		line.supv, line.ccr1, line.ccr2, val);

	return val;
}

#define LINE_TMR_FREQ 100000

void stm32_tim1_cc_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM1;
	uint32_t ccr1;
	uint32_t ccr2;

	/* Clear timer interrupt flags */
	tim->sr = 0;

	ccr1 = tim->ccr1;
	ccr2 = tim->ccr2;

	line.ccr1 = ccr1;
	line.ccr2 = ccr2;
	line.supv = (line.supv * 7) / 8 + ccr1;
//	INF_I("TIM1 CCR1=%d CCR2=%d", ccr1, ccr2);
}

void line_supv_init(void)
{
	struct stm32f_tim * tim = STM32F_TIM1;
	uint32_t div;
	char * env;

	stm32_gpio_mode(IO_AMP_LSPV, ALT_FUNC, SPEED_HIGH);
	stm32_gpio_af(IO_AMP_LSPV, GPIO_AF1);

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM1);
	/* Disable counter */
	tim->cr1 = 0; 
	/* get the total divisior */
	div = (stm32f_tim2_hz + (LINE_TMR_FREQ / 2)) / LINE_TMR_FREQ;
	/* Timer configuration */
	tim->psc = div - 1;
	tim->cnt = 0;
	tim->ccer = 0;
	
	/* • Select the active input for TIMx_CCR1: write the CC1S bits to 
	   01 in the TIMx_CCMR1 register (TI1 selected). */
	/* • Select the active input for TIMx_CCR2: write the CC2S bits to 10 
	   in the TIMx_CCMR1 register (TI1 selected). */
	tim->ccmr1 = TIM_IC1F_4 | TIM_CC1S_TI1 | TIM_CC2S_TI1; 
	/* • Select the active polarity for TI1FP1 (used both for capture in 
	   TIMx_CCR1 and counter clear): write the CC1P and CC1NP bits to ‘0’ 
	   (active on rising edge). */
	tim->ccer &= ~(TIM_CC1NP | TIM_CC1P);
   	/* • Select the active polarity for TI1FP2 (used for capture in 
	   TIMx_CCR2): write the CC2P and CC2NP bits to ‘1’ (active on 
	   falling edge). */
	tim->ccer |= (TIM_CC2NP | TIM_CC2P);
   	/* • Select the valid trigger input: write the TS bits to 101 in the 
	   TIMx_SMCR register (TI1FP1 selected). */
   	/* • Configure the slave mode controller in reset mode: write the SMS 
	   bits to 100 in the TIMx_SMCR register. */
	tim->smcr = TIM_TS_SET(5) | TIM_SMS_SET(4);  

   	/* • Enable the captures: write the CC1E and CC2E bits to ‘1’ in the 
	   TIMx_CCER register. */
	tim->ccer |= TIM_CC1E | TIM_CC2E;  

	tim->dier = TIM_UIE; /* Update interrupt enable */
//	tim->dier = TIM_CC2IE; /* Capture/Compare interrupt enable */

//	thinkos_irq_register(STM32F_IRQ_TIM1_CC, IRQ_PRIORITY_VERY_HIGH, 
//						 stm32_tim1_cc_isr);

	thinkos_irq_register(STM32F_IRQ_TIM1_UP, IRQ_PRIORITY_VERY_HIGH, 
						 stm32_tim1_cc_isr);

	tim->cr1 = TIM_CEN; /* Enable counter */

	INFS_I("TIM1 Init!");

	if ((env = getenv("OFFSET")) != NULL) {
		line.offs = atoi(env);
		stm32_dac2_set(2048 - line.offs);
	}

}

// Second order high pass filter
// ---------------------------------------------------------------------------
//

//
// These values are for a cut off frequency of 240Hz.
// This will remove any DC and 50/60Hz coupling on the
// analog side.
//
#define ACU_LP_A0 14585
#define ACU_LP_A1 -29170
#define ACU_LP_A2 14585
#define ACU_LP_B0 16384
#define ACU_LP_B1 -29033
#define ACU_LP_B2 12923

void filt_hp240hz(struct iir2_filt * filt, 
				  const int16_t in[], int16_t out[], unsigned int len)
{
	int i;

	for (i = 0; i < len; ++i) {
		/* Shift the old samples */
		filt->x[2] = filt->x[1];
		filt->x[1] = filt->x[0];
		filt->y[2] = filt->y[1];
		filt->y[1] = filt->y[0];
		/* Calculate the new output */
		filt->x[0] = in[i];
		filt->y[0] = ACU_LP_A0 * filt->x[0];
		filt->y[0] += ACU_LP_A1 * filt->x[1] - ACU_LP_B1 * filt->y[1];
		filt->y[0] += ACU_LP_A2 * filt->x[2] - ACU_LP_B2 * filt->y[2];
		/* Round and scale down */
		filt->y[0] = (filt->y[0] + 8192) / 16384;
		/* Saturate */
		out[i] = Q15_SAT(filt->y[0]);
	}
}

void audio_alaw_enqueue(uint32_t ts, uint8_t * data, unsigned int samples)
{
#if (STREAM_SAMPLE_RATE != AUDIO_SAMPLE_RATE)
	static int32_t y0 = 0;
	static int32_t y1 = 0;
	static int32_t y2 = 0;
#endif
	int16_t * dst;
	uint8_t * src;
	int32_t gain = audio.gain;
	int32_t offs = audio.offs;
	struct iir2_filt * filt = &audio.filt;
	jitbuf_t * jb = &audio.jbuf;
	struct sndbuf * pcm;
	int rem;
	int i;
	int n;
	int32_t y;

	rem = samples;
	src = (uint8_t *)data;
	while (rem > 0) {
		if ((pcm = sndbuf_alloc()) == NULL)
			return;

		dst = (int16_t *)pcm->data;
#if (STREAM_SAMPLE_RATE == AUDIO_SAMPLE_RATE)
		n = rem > SNDBUF_LEN ? SNDBUF_LEN : rem;

		for (i = 0; i < n; ++i) {
			y = alaw2linear(src[i]);
			y = Q15_MUL(y, gain);
			/* Filter */
			/* Shift the old samples */
			filt->x[2] = filt->x[1];
			filt->x[1] = filt->x[0];
			filt->y[2] = filt->y[1];
			filt->y[1] = filt->y[0];
			/* Calculate the new output */
			filt->x[0] = y;
			y = ACU_LP_A0 * y;
			y += ACU_LP_A1 * filt->x[1] - ACU_LP_B1 * filt->y[1];
			y += ACU_LP_A2 * filt->x[2] - ACU_LP_B2 * filt->y[2];
			/* Round and scale down */
			y = (y + 8192) / 16384;
			filt->y[0] = y;
			/* OffSet */
			y += offs; 
			/* Saturate */
			y = Q15_SAT(y);
			y += 32768;
			dst[i] = y;
		}
#else
		n = rem > (SNDBUF_LEN / 2) ? (SNDBUF_LEN / 2) : rem;
		/* Newton's quadratic interpolation */
		for (i = 0; i < n; ++i) {
			y0 = y1;
			y1 = y2;
			y2 = alaw2linear(src[i]);
			y2 = Q15_MUL(y2, gain);

			/* Filter */
			/* Shift the old samples */
			filt->x[2] = filt->x[1];
			filt->x[1] = filt->x[0];
			filt->y[2] = filt->y[1];
			filt->y[1] = filt->y[0];
			/* Calculate the new output */
			filt->x[0] = y2;
			y2 = ACU_LP_A0 * y2;
			y2 += ACU_LP_A1 * filt->x[1] - ACU_LP_B1 * filt->y[1];
			y2 += ACU_LP_A2 * filt->x[2] - ACU_LP_B2 * filt->y[2];
			/* Round and scale down */
			y2 = (y2 + 8192) / 16384;
			filt->y[0] = y2;
			/* OffSet */
			y2 += offs; 
			y2 = Q15_SAT(y2);
			y2 += 32768;
			y = (6*y1 + 3*y2 - y0) / 8;
			dst[i * 2] = y2;
			dst[i * 2 + 1] = y;
		}
#endif

		jitbuf_enqueue(jb, pcm, ts);
		sndbuf_free(pcm);

		ts += n;
		rem -= n;
		src += n;
	}
}
