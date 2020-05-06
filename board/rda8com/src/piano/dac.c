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
#include "mad.h"

#define SHRT_MAX 32767

#ifndef DAC_FRAME_SIZE
#define DAC_FRAME_SIZE 64
#endif

#ifndef DAC_SAMPLERATE
#define DAC_SAMPLERATE 22050
#endif

#define DAC2_GPIO STM32_GPIOA, 5
#define DAC2_DMA_CHAN STM32_DMA_CHANNEL4

const int16_t dac_silence[DAC_FRAME_SIZE] = { 0, }; 

struct {
	uint16_t dma_buf[2][DAC_FRAME_SIZE];
	volatile int32_t gain;
	volatile uint32_t clk;
	int16_t * volatile src_ptr;
	volatile uint8_t enabled;
	uint8_t flag;
} dac_rt;

int dac_task(void *arg)
{
	struct stm32f_dma *dma = STM32F_DMA1;

	for (;;) {
		uint16_t * dst;
		int16_t * src;
		uint32_t isr;
		int32_t gain;
		int i;

		/* Wait for ADC convertion to finish */
		thinkos_irq_wait(STM32_IRQ_DMA1_CH4);
		isr = dma->isr;
		if (isr & DMA_TEIF4) {
			/* clear the DMA transfer error flag */
			dma->ifcr = DMA_CTEIF4;
		}
		if ((isr & (DMA_TCIF4 | DMA_CHTIF4)) == 0)
			continue;


		/* clear the DMA transfer complete flags */
		dma->ifcr = DMA_TCIF4 | DMA_CHTIF4;

		if (isr & DMA_HTIF4) {
			dst = dac_rt.dma_buf[0];
		} else {
			dst = dac_rt.dma_buf[1];
		}

		src = dac_rt.src_ptr;
		gain = dac_rt.gain;

		for (i = 0; i < DAC_FRAME_SIZE; ++i) {
			int32_t xi;
			int32_t yi;

			xi = src[i];

			yi = Q15_MUL(xi, gain);
			dst[i] = __SSAT((int32_t)yi, 16) + 32768;
		}
		
		dac_rt.clk += DAC_FRAME_SIZE;
		thinkos_flag_give(dac_rt.flag);
	}

	return 0;
}

void dac_dma_init(void *src, unsigned int ndt)
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

	/* Channel select */
	cselr = dma->cselr & ~(DMA1_C4S_MSK);
	dma->cselr = cselr | DMA1_C4S_TIM7_UP_DAC2;

	/* Peripheral address */
	dma->ch[DAC2_DMA_CHAN].cpar = &dac->dhr12l2;
//      dma->ch[DAC2_DMA_CHAN].cpar = &dac->dhr12r2;
	/* Memory pointer */
	dma->ch[DAC2_DMA_CHAN].cmar = (void *)src;
	/* Number of data items to transfer */
	dma->ch[DAC2_DMA_CHAN].cndtr = ndt;
	/* Configuration for double buffer circular, 
	   half-transfer interrupt  */
	dma->ch[DAC2_DMA_CHAN].ccr = DMA_MSIZE_16 | DMA_PSIZE_16 |
	    DMA_MINC | DMA_CIRC | DMA_DIR_MTP;
	/* enable DAC DMA */
	dma->ch[DAC2_DMA_CHAN].ccr |= DMA_HTIE | DMA_TCIE | DMA_TEIE | DMA_EN;
}

uint32_t dac_stack[64] __attribute__ ((aligned(8), section(".stack")));

const struct thinkos_thread_inf dac_thread_inf = {
	.stack_ptr = dac_stack,
	.stack_size = sizeof(dac_stack),
	.priority = 2,
	.thread_id = 2,
	.paused = 0,
	.tag = "DAC"
};

static void tim7_init(uint32_t freq)
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


//	printf("tim1_clk_hz=%dHz\n", stm32f_tim1_hz);
//	printf("TIM7: freq=%dHz div=%d ->  %dHz", freq, div,
//	    stm32f_tim1_hz / pre / div);

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM7);

	tim->cr1 = 0;		/* Disable the counter */
	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->cnt = 0;
	tim->egr = 0;
	tim->dier = TIM_UIE;	/* Update interrupt enable */
	tim->ccmr1 = 0;
	tim->ccr1 = 0;
	tim->cr2 = TIM_MMS_UPDATE;
	tim->sr = 0;
	tim->cr1 = TIM_URS | TIM_CEN;	/* Enable counter */
}

void dac_init(void)
{
	struct stm32f_dac *dac = STM32F_DAC1;

	/* DAC clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_DAC1);
	/* DAC disable */
	dac->cr = 0;

	/* DAC channel 2 initial value */
	dac->dhr12l2 = 32768;

	dac_dma_init(dac_rt.dma_buf[0], 2 * DAC_FRAME_SIZE);

	tim7_init(DAC_SAMPLERATE);

	/* DAC configure with Timer 7 as trigger */
	dac->cr = DAC_TSEL2_TIM7 | DAC_TEN2 | DAC_DMAEN2 | DAC_EN2;

	dac_rt.flag = thinkos_flag_alloc();
	dac_rt.enabled = 0;
	dac_rt.src_ptr = (int16_t *)dac_silence;
	dac_rt.gain = Q15(0.08);

	thinkos_thread_create_inf((void *)dac_task, (void *)NULL,
				  &dac_thread_inf);
}

void dac_start(void)
{
	struct stm32f_dac *dac = STM32F_DAC1;

	dac->cr |= DAC_EN2;
}

void dac_stop(void)
{
	struct stm32f_dac *dac = STM32F_DAC1;

	dac->cr &= ~DAC_EN2;
}

void dac_gain_set(int32_t gain)
{
	dac_rt.gain = gain;
}

