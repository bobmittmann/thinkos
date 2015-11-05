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
 * @file fire_alaram.c
 * @brief Fire Alarm Application
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <sys/console.h>
#include <thinkos.h>
#include <stdio.h>
#include <stdlib.h>

#include "ui.h"
#include "net.h"

void fa_main(void)
{
	int i;

	for (;;) {
		for (i = 1; i <= 8; ++i)
			ui_led_blink(i);

		thinkos_sleep(4000);

		ui_led_off(1);
		ui_led_off(2);
		ui_led_off(3);
		ui_led_off(4);

		thinkos_sleep(4000);
	}	
}

static int fa_ui_input_task(void * arg)
{
	int key;

	for (;;) {
		/* Wait for the Ack Button press ... */
		key = ui_key_get();
		if (key != UI_KEY_ACK)
			continue;

		/* Process the input */
		ui_led_on(1);
		ui_led_off(2);
		ui_led_on(3);
		ui_led_off(4);
		ui_led_on(5);
		ui_led_off(6);
		ui_led_on(7);
		ui_led_off(8);
	}

	return 0;
}

uint32_t fa_ui_input[64];

const struct thinkos_thread_inf fa_ui_input_inf = {
		.stack_ptr = fa_ui_input,
		.stack_size = sizeof(fa_ui_input),
		.priority = 8,
		.thread_id = 2,
		.paused = false,
		.tag = "UI_INP"
};

int fa_init(void)
{
	thinkos_thread_create_inf(fa_ui_input_task, NULL, &fa_ui_input_inf);
	return 0;
}

static void stdio_init(void)
{
	FILE * f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

int main(int argc, char ** argv)
{
	/* Initialize GPIO pins */
	gpio_init();

	/* Initialize the stdio streams so we can use printf() */
	stdio_init();

	/* Initialize the User Interface */
	ui_init();

	/* Initialize the network link layer */
	netlnk_init();

	/* Initialize the network transport layer */
//	netmsg_init();

	/* Initialize the fire alarm module */
	fa_init();

	/* Run the fire alarm application */
	fa_main();

	return 0;
}

