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
 * @file hello.c
 * @brief application test
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <sys/stm32f.h>
#include <sys/console.h>
#include <stdio.h>
#include <stdlib.h>
#include <thinkos.h>

#include "board.h"

/* GPIO pin description */ 
struct stm32f_io {
	struct stm32_gpio * gpio;
	uint8_t pin;
};

/* ----------------------------------------------------------------------
 * LEDs 
 * ----------------------------------------------------------------------
 */

const struct stm32f_io led_io[] = {
	{ LED3_IO }, /* LED3 */
	{ LED5_IO }, /* LED5 */
	{ LED7_IO }, /* LED7 */
	{ LED9_IO }, /* LED9 */
	{ LED10_IO }, /* LED10 */
	{ LED8_IO }, /* LED8 */
	{ LED6_IO }, /* LED6 */
	{ LED4_IO }, /* LED4 */
};

#define LED_COUNT (sizeof(led_io) / sizeof(struct stm32f_io))

void led_on(unsigned int id)
{
	__led_on(led_io[id].gpio, led_io[id].pin);
}

void led_off(unsigned int id)
{
	__led_off(led_io[id].gpio, led_io[id].pin);
}

void led_toggle(unsigned int id)
{
	__led_toggle(led_io[id].gpio, led_io[id].pin);
}

volatile uint32_t irq_count = 0 ;
volatile int tmr_sem;

void stm32_tim15_isr(void)
{
	struct stm32f_tim * tim = STM32F_TIM15;
	/* Clear timer interrupt flags */
	tim->sr = 0;
	irq_count++;
	thinkos_sem_post_i(tmr_sem);
}

/* PWM input mode (only for TIM15)
   This mode is a particular case of input capture mode. The procedure 
   is the same except:
   • Two ICx signals are mapped on the same TIx input.
   • These 2 ICx signals are active on edges with opposite polarity.
   • One of the two TIxFP signals is selected as trigger input and the 
   slave mode controller is configured in reset mode.
   For example, you can measure the period (in TIMx_CCR1 register) and the 
   duty cycle (in TIMx_CCR2 register) of the PWM applied on TI1 using the 
   following procedure (depending on CK_INT frequency and prescaler value):

   1. Select the active input for TIMx_CCR1: write the CC1S bits to 01 in 
   the TIMx_CCMR1 register (TI1 selected).
   2. Select the active polarity for TI1FP1 (used both for capture in 
   TIMx_CCR1 and counter clear): write the CC1P and CC1NP bits to ‘0’ 
   (active on rising edge).
   3. Select the active input for TIMx_CCR2: write the CC2S bits to 10 in 
   the TIMx_CCMR1 register (TI1 selected).
   4. Select the active polarity for TI1FP2 (used for capture in TIMx_CCR2): 
   write the CC2P and CC2NP bits to ‘1’ (active on falling edge).
   5. Select the valid trigger input: write the TS bits to 101 in the 
   TIMx_SMCR register (TI1FP1 selected).
   6. Configure the slave mode controller in reset mode: write the SMS 
   bits to 100 in the TIMx_SMCR register.
   7. Enable the captures: write the CC1E and CC2E bits to ‘1’ in the 
   TIMx_CCER register.
 */

void timer_init(uint32_t freq)
{
	struct stm32f_tim * tim = STM32F_TIM15;
	uint32_t div;
	uint32_t pre;
	uint32_t n;

	/* get the total divisior */
	div = ((stm32f_tim1_hz) + (freq / 2)) / freq;
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div + pre / 2) / pre;


	printf(" %s(): freq=%dHz pre=%d n=%d\n",
		   __func__, freq, pre, n);

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM15);

	/* Timer configuration */
	tim->psc = pre - 1;
	tim->arr = n - 1;
	tim->cnt = 0;
	tim->egr = 0;
	tim->dier = TIM_UIE; /* Update interrupt enable */
	/* capture compare mode */
	tim->ccmr1 = TIM_CC1S_TI1;
	tim->ccer = 0; 
	tim->ccmr1 |= TIM_CC2S_TI1;
	tim->ccer |= TIM_CC2P | TIM_CC2NP; 
	tim->smcr = TIM_TS_TI1FP1;
	tim->smcr = TIM_SMS_RESET_MODE;

	/* Enable captures */
	tim->ccer |= TIM_CC1E | TIM_CC2E; 

	tim->ccr1 = tim->arr - 2;
	tim->cr2 = TIM_MMS_OC1REF;
	tim->cr1 = TIM_URS | TIM_CEN; /* Enable counter */

	/* Enable DMA interrupt */
	thinkos_irq_register(STM32F_IRQ_TIM15, 0xff, stm32_tim15_isr);


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

int test_task(void * arg)
{
	for (;;) {
		thinkos_sleep(3000);
//		thinkos_sem_post(2);
	}

	return 0;
}

uint32_t test_stack[64];

#define GPIO_TIM15_CH1 STM32_GPIOA, 13 /* CC2 */

int capture_compare_init(void)
{
	stm32_gpio_af(GPIO_TIM15_CH1, GPIO_AF9);
	stm32_gpio_mode(GPIO_TIM15_CH1, ALT_FUNC, PULL_UP);

	timer_init(100);

	return 0;
}

int main(int argc, char ** argv)
{
	int i;

	stdio_init();

	thinkos_thread_create(test_task, NULL,
			test_stack, sizeof(test_stack));

	capture_compare_init();

	tmr_sem = thinkos_sem_alloc(0);

	for (i = 0; ; ++i) {
		thinkos_sem_wait(tmr_sem);
		led_off((i - 2) & 0x7);
		led_on(i & 0x7);
		printf("IRQ count = %d\r\n", irq_count);
	}

	return 0;
}

