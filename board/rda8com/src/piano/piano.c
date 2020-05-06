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

int play_task(void *arg)
{
	int32_t stat;

	stat = spidev_rd();

	for (;;) {
		int32_t tmp;
		int32_t down;

		tmp = spidev_rd();

		down = (stat ^ tmp) & tmp;
		stat = tmp;

		if (down & (1 << 0)) { 
			printf("0\n");
		} else if (down & (1 << 1)) {
			printf("1\n");
		} else if (down & (1 << 2)) {
			printf("2\n");
		} else if (down & (1 << 3)) {
			printf("3\n");
		} else if (down & (1 << 4)) {
			printf("4\n");
		} else if (down & (1 << 5)) {
			printf("5\n");
		} else if (down & (1 << 6)) {
			printf("6\n");
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
	thinkos_thread_create_inf((void *)play_task, (void *)NULL,
				  &play_thread_inf);
}

int main(int argc, char ** argv)
{
	int32_t sw = 0;
	int32_t gain;

	stdio_init();

	printf("Starting piano...\n");

//	for (;;) {
//		thinkos_sleep(100);
//	};

	io_init();

	spidev_init();

	dac_init();

	dac_start();

	play_init();

	for (;;) {
		int32_t tmp;

		thinkos_sleep(100);
		tmp = io_sw_val();
		if (tmp != sw) {
			sw = tmp;

			gain = (sw * 32765) / 10;
			dac_gain_set(gain);
		}
	}
	return 0;
}

