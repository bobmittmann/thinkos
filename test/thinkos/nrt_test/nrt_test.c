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
 * @file nrt_test.c
 * @brief application test
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <arch/cortex-m3.h>
#include <sys/dcclog.h>
#include <stdlib.h>
#include <thinkos.h>
#include <sys/stm32f.h>

#define IO_LED1A           STM32_GPIOE, 9
#define IO_LED1B           STM32_GPIOE, 10
#define IO_LED1C           STM32_GPIOE, 11
#define IO_LED1D           STM32_GPIOE, 12


static inline void __led_on(struct stm32_gpio *__gpio, int __pin) {
	stm32_gpio_set(__gpio, __pin);
}

static inline void __led_off(struct stm32_gpio *__gpio, int __pin) {
	stm32_gpio_clr(__gpio, __pin);
}

void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOE);

	/* - LEDs ---------------------------------------------------------*/
	stm32_gpio_clr(IO_LED1A);
	stm32_gpio_mode(IO_LED1A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1B);
	stm32_gpio_mode(IO_LED1B, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1C);
	stm32_gpio_mode(IO_LED1C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1D);
	stm32_gpio_mode(IO_LED1D, OUTPUT, PUSH_PULL | SPEED_LOW);
}

int nrt_task1(void * arg)
{
	int j;

	DCC_LOG(LOG_TRACE, "...");

	for (j = 0; j < 200000000; ++j) {
		thinkos_sleep(200);
		__led_on(IO_LED1A);
		thinkos_sleep(200);
		__led_off(IO_LED1A);
	}

	return 0;
}

int nrt_task2(void * arg)
{
	int j;

	DCC_LOG(LOG_TRACE, "...");

	for (j = 0; j < 200000000; ++j) {
		thinkos_sleep(210);
		__led_on(IO_LED1C);
		thinkos_sleep(210);
		__led_off(IO_LED1C);
	}

	return 0;
}

uint32_t nrt_stack1[256];
uint32_t nrt_stack2[256];

int main(int argc, char ** argv)
{
	DCC_LOG_INIT();
	DCC_LOG_CONNECT();

	thinkos_krn_init(THINKOS_OPT_ID(1));

	io_init();

	thinkos_nrt_init();

	DCC_LOG(LOG_TRACE, "...");

	thinkos_thread_create(nrt_task1, NULL, nrt_stack1, 
						  1024 | THINKOS_OPT_ID(7));
	thinkos_thread_create(nrt_task2, NULL, nrt_stack2,
						  1024 | THINKOS_OPT_ID(8));

	for(;;) {
		DCC_LOG(LOG_TRACE, "Sleep...");
		thinkos_sleep(10000);
		DCC_LOG(LOG_TRACE, "Wakeup...");
	};

	return 0;
}


