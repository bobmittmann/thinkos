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
	{ IO_LED3 }, /* LED3 */
	{ IO_LED5 }, /* LED5 */
	{ IO_LED6 }, /* LED6 */
	{ IO_LED4 }, /* LED4 */
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

void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOD);

	stm32_gpio_mode(IO_LED3, OUTPUT, PUSH_PULL | SPEED_LOW);
	__led_on(IO_LED3);
	stm32_gpio_mode(IO_LED4, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(IO_LED5, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(IO_LED6, OUTPUT, PUSH_PULL | SPEED_LOW);
}

int main(int argc, char ** argv)
{
	int i;
	
	io_init();

	thinkos_init(THINKOS_OPT_PRIORITY(3) | THINKOS_OPT_ID(3));

	for (i = 0; ; ++i) {
		led_off((i - 2) & 0x3);
		led_on(i & 0x3);
		/* wait 100 ms */
		thinkos_sleep(100);
	}

	return 0;
}

