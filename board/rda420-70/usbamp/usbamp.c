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
#include <sys/param.h>

#include <sys/dcclog.h>
#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#include <thinkos.h>

#include "io.h"

void io_init(void);

int __attribute__((noreturn)) main(int argc, char ** argv)
{
	int i;

	DCC_LOG_INIT();
	DCC_LOG_CONNECT();

	/* calibrate usecond delay loop */
	cm3_udelay_calibrate();

	DCC_LOG(LOG_TRACE, "1. io_init()");
	io_init();

	DCC_LOG(LOG_TRACE, "2. thinkos_init()");
	thinkos_init(THINKOS_OPT_PRIORITY(3) | THINKOS_OPT_ID(3));

	DCC_LOG(LOG_TRACE, "3. iodrv_init()");
	iodrv_init();

	io_set_rate(LED1C, RATE_20BPM);
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
	io_set_rate(LED4B, RATE_TEMPORAL3);

	io_set_rate(LED1A, RATE_PULSE1);
	io_set_rate(LED2A, RATE_PULSE1);
	io_set_rate(LED3A, RATE_PULSE1);
	io_set_rate(LED4A, RATE_PULSE1);

	for (i = 0; ; ++i) {
//		led_on(LED_AMBER);
		thinkos_sleep(1000);
//		led_off(LED_AMBER);
//		led_on(LED_RED);
		DCC_LOG(LOG_TRACE, "tick");
		thinkos_sleep(1000);
//		led_off(LED_RED);
		DCC_LOG(LOG_TRACE, "tack");
	}

}

