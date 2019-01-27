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
 * @file boot.c
 * @brief YARD-ICE bootloader main
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifdef CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>

#include <sys/stm32f.h>
#include <sys/delay.h>

#include <thinkos.h>

#include "board.h"
#include "version.h"

void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);

	/* - USB OTG FS -------------------------------------------------------- */
	stm32_gpio_af(OTG_FS_DP, GPIO_AF10);
	stm32_gpio_af(OTG_FS_DM, GPIO_AF10);
	stm32_gpio_af(OTG_FS_VBUS, GPIO_AF10);

	stm32_gpio_mode(OTG_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	/* XXX: something wrong with this configuration!!! Set as input instead ...
	 */
	//stm32_gpio_mode(OTG_FS_VBUS, INPUT, SPEED_LOW);
	stm32_gpio_mode(OTG_FS_VBUS, ALT_FUNC, SPEED_LOW);

	/* - LEDs -------------------------------------------------------------- */
	stm32_gpio_clr(IO_LED1);
	stm32_gpio_mode(IO_LED1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED2);
	stm32_gpio_mode(IO_LED2, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED3);
	stm32_gpio_mode(IO_LED3, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED4);
	stm32_gpio_mode(IO_LED4, OUTPUT, PUSH_PULL | SPEED_LOW);

	/* - Switch ----------------------------------------------------------- */
	stm32_gpio_mode(IO_SW1, INPUT, PULL_UP | SPEED_LOW);

	/* - USART1 ----------------------------------------------------------- */
	stm32_gpio_clr(IO_USART1_TX);
	stm32_gpio_mode(IO_USART1_TX, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_USART1_RX);
	stm32_gpio_mode(IO_USART1_RX, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_mode(IO_USART1_RX, INPUT, PULL_UP | SPEED_LOW);

	/* - I2S2 ------------------------------------------------------------- */
	stm32_gpio_mode(IO_I2S2_WS, INPUT, PULL_UP | SPEED_LOW);
	stm32_gpio_mode(IO_I2S2_CK, INPUT, PULL_UP | SPEED_LOW);
	stm32_gpio_mode(IO_I2S2EXT_SD, INPUT, PULL_UP | SPEED_LOW);
	stm32_gpio_mode(IO_I2S2_SD, INPUT, PULL_UP | SPEED_LOW);
}

volatile unsigned int pulse_width;
int pulse_flag;

int pulse_task(void * p)
{
	for(;;) {
		thinkos_flag_take(pulse_flag);
		stm32_gpio_set(IO_USART1_TX);
		__led_on(IO_LED1);
		thinkos_sleep(pulse_width);
		stm32_gpio_clr(IO_USART1_TX);
		__led_off(IO_LED1);
	}

	return 0;
}

int io_task(void * p)
{
	unsigned int w;
	uint32_t clk;
	bool sw1[2];
	bool sw2[2];

	clk = thinkos_clock();
	sw1[0] = true;
	sw2[0] = true;

	for(;;) {
		clk += 100;
		thinkos_alarm(clk);

		w = stm32_gpio_stat(IO_I2S2_SD) ? 0 : (1 << 0);
		w += stm32_gpio_stat(IO_I2S2EXT_SD) ? 0 : (1 << 1); 
		w += stm32_gpio_stat(IO_I2S2_WS) ? 0 : (1 << 2);
		w += stm32_gpio_stat(IO_I2S2_CK) ? 0 : (1 << 3);
	
		sw1[1] = sw1[0];
		sw1[0] = stm32_gpio_stat(IO_SW1) ? false : true;
		sw2[1] = sw2[0];
		sw2[0] = stm32_gpio_stat(IO_USART1_RX) ? false : true;

		if ((sw1[0] && !sw1[1]) || (sw2[0] && !sw2[1])) {
			pulse_width = w * 20 + 20;
			thinkos_flag_give(pulse_flag);
		}

	}

	return 0;
}

static void __tim3_init(uint32_t freq)
{
	struct stm32f_tim *tim = STM32F_TIM3;
	uint32_t div;
	uint32_t pre;
	uint32_t n;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM3);

	/* get the total divisor */
	div = (stm32f_tim1_hz + (freq / 2)) / freq;
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div + (pre / 2)) / pre;

	/* Disable counter */
	tim->cr1 = 0;
	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->cnt = 0;
	tim->egr = 0;
	tim->dier = TIM_UIE;	/* Update interrupt enable */
	tim->ccmr1 = TIM_OC1M_PWM_MODE1;
	tim->ccr1 = tim->arr / 2;
	tim->cr2 = TIM_MMS_OC1REF;
}

static void __adc1_init(void)
{
	static const uint8_t __adc1_chan[] = { 0, 1, 2, 3 };
	struct stm32f_adc *adc = STM32F_ADC1;

	/* ADC clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_ADC1);

	/* Disable the ADC */
	adc->cr2 = 0;

	/* Configure ADC for sequential channel, DMA transfer */
	stm32f_adc_seq_set(adc, __adc1_chan, 4);
	stm32f_adc_smp_set(adc, __adc1_chan[0], ADC_SMP_144_CYC);
	stm32f_adc_smp_set(adc, __adc1_chan[1], ADC_SMP_144_CYC);
	stm32f_adc_smp_set(adc, __adc1_chan[2], ADC_SMP_144_CYC);
	stm32f_adc_smp_set(adc, __adc1_chan[3], ADC_SMP_144_CYC);

	adc->cr1 = ADC_RES_12BIT | ADC_SCAN;
	adc->cr2 = ADC_EXTEN_RISING | ADC_EXTSEL_TIM3_TRGO | ADC_DDS;
}

int adc_task(void * p)
{
	struct stm32f_adcc *adcc = STM32F_ADCC;
	uint32_t clk;

	/* ADC Common Control */
	/*  R_AIN = ((k - 0.5)/(f_ADC * C_ADC * log(2^(N + 2)))) - R_ADC */
	adcc->ccr = ADC_TSVREFE | ADC_ADCPRE_4;
	/* PCLK2 = APB1 = 90.003200 MHz
	   ADCCLK = PCLK2/4 = 22.50 MHz */
	/* PCLK2 = APB1 = 90003200
	   ADCCLK = PCLK2/4
	   N = 12
	   R_ADC = 6000
	   C_ADC = 7e-12
	   f_ADC = ADCCLK
	   n_S = 28
	   n_S = 15
	   R_AIN = ((n_S- 0.5)/(f_ADC * C_ADC * log(2^(N + 2)))) - R_ADC
	   t_S = n_S/f_ADC
	   t_CONV = ((n_S + N + 2) / f_ADC)
	   f_SEQ = 1 / (4 * t_CONV)
	 */

	/* Configure timer for periodic conversion */
	__tim3_init(1000);

	/* ADC_IMET */
	__adc1_init();

	clk = thinkos_clock();
	for(;;) {
		clk += 100;
		thinkos_alarm(clk);
	}

	return 0;
}


uint32_t io_stack[512];
uint32_t pulse_stack[512];
uint32_t adc_stack[512];

void __attribute__((noreturn)) main(int argc, char ** argv)
{
	cm3_udelay_calibrate();

	io_init();

	thinkos_init(THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0));

	pulse_width = 0;
	pulse_flag = thinkos_flag_alloc();

	thinkos_thread_create((void *)adc_task, (void *)NULL, 
						  adc_stack, sizeof(adc_stack));

	thinkos_thread_create((void *)io_task, (void *)NULL, 
						  io_stack, sizeof(io_stack));

	thinkos_thread_create((void *)pulse_task, (void *)NULL, 
						  pulse_stack, sizeof(pulse_stack));

	for(;;) {
		__led_on(IO_LED2);
		thinkos_sleep(200);
		__led_off(IO_LED2);
		thinkos_sleep(800);
	}
}


