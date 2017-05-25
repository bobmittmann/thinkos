/* 
 * File:	 leds.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "board.h"

#include <sys/dcclog.h>
#include <assert.h>
#include <thinkos.h>
#include "io.h"

/* Hardware dependent (Low level) io assignemnts */ 

const struct gpio_pin led_gpio[] = {
	{IO_LED1A},
	{IO_LED1B},
	{IO_LED1C},
	{IO_LED1D},
	{IO_LED2A},
	{IO_LED2B},
	{IO_LED2C},
	{IO_LED2D},
	{IO_LED3A},
	{IO_LED3B},
	{IO_LED3C},
	{IO_LED3D},
	{IO_LED4A},
	{IO_LED4B},
	{IO_LED4C},
	{IO_LED4D}
};

struct obj_led {
	uint8_t rate;
	uint8_t tmr;
	uint8_t state;
};

#define LED_MAX 16

struct {
	struct obj_led led[LED_MAX];
} io_drv;

void led_on(unsigned int id)
{
	assert(id < LED_MAX);

	io_drv.led[id].tmr = 0;
	io_drv.led[id].state = 1;

	__led_on(led_gpio[id].gpio, led_gpio[id].pin);
}

void led_off(unsigned int id)
{
	assert(id < LED_MAX);

	io_drv.led[id].tmr = 0;
	io_drv.led[id].state = 0;

	__led_off(led_gpio[id].gpio, led_gpio[id].pin);
}

#define IO_POLL_PERIOD_MS 10

void led_flash(unsigned int id, unsigned int ms)
{
	assert(id < LED_MAX);

	io_drv.led[id].tmr = ms / IO_POLL_PERIOD_MS;
	io_drv.led[id].state = 1;

	__led_on(led_gpio[id].gpio, led_gpio[id].pin);
}

static void led_io_set(unsigned int id, bool on)
{
	assert(id < LED_MAX);

	if (on)
		__led_on(led_gpio[id].gpio, led_gpio[id].pin);
	else
		__led_off(led_gpio[id].gpio, led_gpio[id].pin);
}

static bool led_io_get(unsigned int id)
{
	return false;
}

const struct io_obj_op led_obj_op = {
	.set = led_io_set,
	.get = led_io_get
};

#if 0
void __attribute__((noreturn)) led_task(void)
{
	uint32_t clk;
	int i;

	clk = thinkos_clock();
	for (;;) {
		for (i = 0; i < LED_MAX; ++i) {
			unsigned int tmr;

			/* process led timers */
			if ((tmr = io_drv.led_tmr[0]) != 0) {
				if (--tmr == 0) 
				led_off(0);
				io_drv.led_tmr[0] = tmr;
			}

			if ((tmr = io_drv.led_tmr[1]) != 0) {
				if (--tmr == 0) 
					led_off(1);
				io_drv.led_tmr[1] = tmr;
			}
		}

		clk += IO_POLL_PERIOD_MS;
		thinkos_alarm(clk);
	}
}
#endif
