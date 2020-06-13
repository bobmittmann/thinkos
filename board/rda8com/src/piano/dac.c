/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 *PCM
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
 * @file dac.c
 * @brief 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stm32f.h>
#include <thinkos.h>
#include <fixpt.h>
#include <string.h>
#include <sys/param.h>

#include "board.h"
#include "dac.h"

#define SHRT_MAX 32767

#define DAC2_GPIO STM32_GPIOA, 5
#define DAC2_DMA_CHAN STM32_DMA_CHANNEL4

const int16_t dac_silence[DAC_FRAME_SIZE] = { 0, }; 

/* 16 bit pcm encoding */
int silence_pcm_encode(void * arg, float pcm[], unsigned int len, uint32_t clk)
{
	unsigned int i;

	for (i = 0; i < len; ++i) {
		pcm[i] = 0;
	}

	return len;
}

int silence_reset(void * arg, uint32_t clk)
{
	return 0;
}

static const struct dac_stream_op silence_gen_op = {
	.encode = (int (*)(void *, float*, unsigned int, uint32_t))
		silence_pcm_encode,
	.reset = (int (*)(void *, uint32_t))silence_reset
};

static const struct dac_stream silence = {
	.arg = NULL,
	.op = &silence_gen_op
};

struct dac_voice {
	struct {
		void * arg;
		int (* encode)(void *, float [], unsigned int, uint32_t);
	} lvl[DAC_PIPELINE_MAX];
};

struct {
	uint16_t dma_buf[2][DAC_FRAME_SIZE];
	volatile float gain;
	volatile uint32_t clk;
	volatile uint8_t enabled;
	uint8_t flag;
	uint8_t mutex;
	volatile struct dac_voice voice[DAC_VOICES_MAX];
	struct {
		const struct dac_stream_op * op[DAC_PIPELINE_MAX];
	} voice_op[DAC_VOICES_MAX];

} dac_rt;

static inline void __dac_start(void)
{
	struct stm32f_dac *dac = STM32F_DAC1;

	dac->cr |= DAC_EN2;
}

static inline void __dac_stop(void)
{
	struct stm32f_dac *dac = STM32F_DAC1;

	dac->cr &= ~DAC_EN2;
}

static inline void __dac_timer_start(void)
{
	struct stm32f_tim *tim = STM32F_TIM7;

	tim->cr1 = TIM_URS | TIM_CEN;	/* Enable counter */
}

static inline void __dac_timer_stop(void)
{
	struct stm32f_tim *tim = STM32F_TIM7;

	tim->cr1 = TIM_URS;	
}

uint32_t dac_clock(void)
{
	return dac_rt.clk;
}

int dac_task(void *arg)
{
	struct stm32f_dma *dma = STM32F_DMA1;
	uint32_t cnt = 0;
	uint32_t clk = 0;
	uint32_t isr;

    thinkos_irq_priority_set(STM32_IRQ_DMA1_CH4, IRQ_PRIORITY_HIGHEST);

#if 0
	__dac_start();
	__dac_timer_start();
#endif

	do {
		/* Wait for first DAC DMA transfer to finish */
		thinkos_irq_wait(STM32_IRQ_DMA1_CH4);
		/* Get the status register */
		isr = dma->isr;
		/* clear the DMA interrupt flags */
		dma->ifcr = isr;
#if 0
		if (isr & DMA_TEIF4) {
			printf("ERROR!\n");
		}

		if ((isr & (DMA_TCIF4 | DMA_CHTIF4)) == 0) {
			printf("isr=0x%x\n", isr);
		}
#endif
		/* first transfer should be a half transfer */
	} while ((isr & DMA_CHTIF4) == 0);

	printf("DAC started!\n");
  
	for (;;) {
		float pcm[DAC_FRAME_SIZE];
		uint16_t * dst;
		float gain;
		int i;
		int j;

		/* prepare for next transfer */
		for (i = 0; i < DAC_FRAME_SIZE; ++i)
			pcm[i] = 0;

		/* Synthesis pipline */
		for (j = 0; j < DAC_VOICES_MAX; ++j) {
			struct dac_voice voice = dac_rt.voice[j];
			float buf[DAC_FRAME_SIZE];

			for (i = 0; i < DAC_PIPELINE_MAX; ++i) { 
				voice.lvl[i].encode(voice.lvl[i].arg, buf, DAC_FRAME_SIZE, clk);
			}

			/* Mixer */
			for (i = 0; i < DAC_FRAME_SIZE; ++i)
				pcm[i] += buf[i];
		}

		dst = dac_rt.dma_buf[cnt & 1];
		gain = dac_rt.gain;

		for (i = 0; i < DAC_FRAME_SIZE; ++i) {
			int32_t y;

			y = gain * pcm[i];
			dst[i] = __SSAT(y, 16) + 32768;
		}

//		do {
			/* Wait for DAC DMA transfer to finish */
			thinkos_irq_wait(STM32_IRQ_DMA1_CH4);
			/* Get the status register */
//			isr = dma->isr;
//			if (isr == 0) {
//				printf("!");
//			}
//		} while (isr == 0);
		/* clear the DMA transfer complete flags */
		dma->ifcr = DMA_CTEIF4 | DMA_CHTIF4 | DMA_CTCIF4 | DMA_CGIF4;

		cnt++;
		clk += DAC_FRAME_SIZE;
		dac_rt.clk = clk;

//		dma->ifcr = DMA_CTEIF4 | DMA_CHTIF4 | DMA_CTCIF4 | DMA_CGIF4;
//		if ((isr & (DMA_TCIF4 | DMA_CHTIF4)) == (DMA_TCIF4 | DMA_CHTIF4)) {
//			printf("isr=0x%x\n", isr);
//		}

#if 0
		uint32_t n;
		(void)n;

		if (isr & DMA_TCIF4) {
			n = 1;
		} else {
			n = 0;
		}

		if (n != (cnt & 1)) {
			printf("?");
		}
#endif

	}

	return 0;
}

static void __dac_dma_init(void *src, unsigned int ndt)
{
	struct stm32f_dac *dac = STM32F_DAC1;
	struct stm32f_dma *dma = STM32F_DMA1;
	uint32_t cselr;

	/* DMA clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_DMA1);

	/* DMA Disable */
	dma->ch[DAC2_DMA_CHAN].ccr = 0;
	/* Wait for the channel to be ready .. */
	while (dma->ch[DAC2_DMA_CHAN].ccr & DMA_EN) ;

	dma->ifcr = DMA_CTEIF4 | DMA_CHTIF4 | DMA_CTCIF4 | DMA_CGIF4;

	/* Channel select */
	cselr = dma->cselr & ~(DMA1_C4S_MSK);
	dma->cselr = cselr | DMA1_C4S_TIM7_UP_DAC2;

	/* Peripheral address */
	dma->ch[DAC2_DMA_CHAN].cpar = &dac->dhr12l2;
	/* Memory pointer */
	dma->ch[DAC2_DMA_CHAN].cmar = (void *)src;
	/* Number of data items to transfer */
	dma->ch[DAC2_DMA_CHAN].cndtr = ndt;
	/* Configuration for double buffer circular, 
	   half-transfer interrupt  */
	dma->ch[DAC2_DMA_CHAN].ccr = DMA_MSIZE_16 | DMA_PSIZE_16 |
	    DMA_MINC | DMA_CIRC | DMA_DIR_MTP;
	/* enable DAC DMA */
	dma->ch[DAC2_DMA_CHAN].ccr |= DMA_HTIE | DMA_TCIE;
}

static void __dac_dma_enable(void)
{
	struct stm32f_dma *dma = STM32F_DMA1;

	/* enable DAC DMA */
	dma->ch[DAC2_DMA_CHAN].ccr |= DMA_EN;
}

uint32_t dac_stack[256 + 2 * DAC_FRAME_SIZE] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf dac_thread_inf = {
	.stack_ptr = dac_stack,
	.stack_size = sizeof(dac_stack),
	.priority = 2,
	.thread_id = 2,
	.paused = 0,
	.tag = "DAC"
};

static void __dac_timer_init(uint32_t freq)
{
	struct stm32f_tim *tim = STM32F_TIM7;
	uint32_t div;
	uint32_t pre;
	uint32_t n;

	/* get the total divisior */
	div = (stm32f_tim1_hz + (freq / 2)) / freq;
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div + pre / 2) / pre;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM7);

	tim->cr1 = 0;		/* Disable the counter */
	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->cnt = 0;
	tim->egr = 0;
	tim->sr = 0;
	tim->dier = 0;
	tim->cr2 = TIM_MMS_UPDATE;
	tim->cr1 = TIM_ARPE | TIM_URS;
}

int dac_init(void)
{
	struct stm32f_dac *dac = STM32F_DAC1;
	uint16_t * pcm;
	int j;
	int i;

	if (dac_rt.mutex != 0) {
		return -1;
	}

	/* DAC clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_DAC1);
	/* DAC disable */
	dac->cr = 0;

	/* DAC channel 2 initial value */
	dac->dhr12l2 = 0;

	/* Ramp up to middle value */
	pcm = dac_rt.dma_buf[0];
	for (i = 0; i < 2 * DAC_FRAME_SIZE; ++i) {
		pcm[i] = (32768 * 2 * DAC_FRAME_SIZE) / i;
	}

	/* DAC configure with TIM7 as trigger */
	dac->cr = DAC_TSEL2_TIM7 | DAC_TEN2 | DAC_DMAEN2;

	__dac_dma_init(dac_rt.dma_buf[0], 2 * DAC_FRAME_SIZE);

	__dac_dma_enable();

	__dac_timer_init(DAC_SAMPLE_RATE);

	dac_rt.flag = thinkos_flag_alloc();
	dac_rt.mutex = thinkos_mutex_alloc();
	dac_rt.enabled = 0;
	dac_rt.gain = 0.25 * 32767;

	for (j = 0; j < DAC_VOICES_MAX; ++j) { 
		for (i = 0; i < DAC_PIPELINE_MAX; ++i) { 
			dac_rt.voice[j].lvl[i].encode = silence.op->encode; 
		}
	}

	thinkos_thread_create_inf((int (*)(void *))dac_task, (void *)NULL,
				  &dac_thread_inf);

	return 0;
}

void dac_start(void)
{
	if (!dac_rt.enabled) {
		__dac_start();
		__dac_timer_start();
		dac_rt.enabled = 1;
	}
}

void dac_stop(void)
{
	if (dac_rt.enabled) {
		__dac_timer_stop();
		__dac_stop();
		dac_rt.enabled = 0;
	}
}

void dac_gain_set(float gain)
{
	dac_rt.gain = gain * 32767;
}

int dac_voice_op_set(int vid, int lid, 
	void * arg, const struct dac_stream_op * op)
{
	unsigned int vj = vid - 1;
	unsigned int li = lid - 1;

	if (vj >= DAC_VOICES_MAX)
		return -1;

	if (li >= DAC_PIPELINE_MAX)
		return -1;

//	printf("DAC op set: vid=%d lid=%d\n", vid, lid);

	dac_rt.voice[vj].lvl[li].arg = arg;
	dac_rt.voice[vj].lvl[li].encode = op->encode;
	dac_rt.voice_op[vj].op[li] = op;

	return 0;
}

