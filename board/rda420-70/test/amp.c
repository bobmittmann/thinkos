/* 
 * File:	 usb-test.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <trace.h>
#include <thinkos.h>
#include <assert.h>
#include <io.h>

#include "board.h"

/* Hardware dependent (Low level) io assignemnts */ 

const struct gpio_pin pwr_gpio[] = {
	{IO_PWR1},
	{IO_PWR2},
	{IO_PWR3},
	{IO_PWR4}
};

const struct gpio_pin fault_gpio[] = {
	{IO_FAULT1},
	{IO_FAULT2},
	{IO_FAULT3},
	{IO_FAULT4}
};

struct amp_ckt_def {
	struct gpio_pin io_pwr;
	struct gpio_pin io_fault;
	uint8_t tbl_led;
};

struct amp_ckt_obj {
	bool pwr_switch;
	bool pwr_fault;
};

#define AMP_CKTS 4

const struct amp_ckt_def amp_ckt_def_tab[AMP_CKTS] = {
	{
		.io_pwr = {IO_PWR1},
		.io_fault = {IO_FAULT1},
		.tbl_led = LED1C
	},
	{
		.io_pwr = {IO_PWR2},
		.io_fault = {IO_FAULT2},
		.tbl_led = LED2C
	},
	{
		.io_pwr = {IO_PWR3},
		.io_fault = {IO_FAULT3},
		.tbl_led = LED3C
	},
	{
		.io_pwr = {IO_PWR4},
		.io_fault = {IO_FAULT4},
		.tbl_led = LED4C
	}
};

struct {
	struct amp_ckt_obj ckt_obj[AMP_CKTS ];
} amp;

int amp_power(unsigned int ckt, bool on)
{
	const struct amp_ckt_def * def;
	const struct gpio_pin * p;
	struct amp_ckt_obj * obj;

	if ((ckt > 4) || (ckt < 1))
		return -1;

	def = &amp_ckt_def_tab[ckt - 1];
	obj = &amp.ckt_obj[ckt - 1];

	p = &def->io_pwr;
	if (on) {
		stm32_gpio_set(p->gpio, p->pin);
		obj->pwr_switch = true;
	} else {
		stm32_gpio_clr(p->gpio, p->pin);
		obj->pwr_switch = false;
	}

	thinkos_sleep(1);
	stm32_gpio_set(IO_TRIG);
	thinkos_sleep(1);
	stm32_gpio_clr(IO_TRIG);

	return 0;
}

void amp_supv(void)
{
	int i;

	for (i = 0; i <= AMP_CKTS; ++i) {
		const struct amp_ckt_def * def;
		const struct gpio_pin * p;
		struct amp_ckt_obj * obj;

		def = &amp_ckt_def_tab[i];
		obj = &amp.ckt_obj[i];

		p = &def->io_fault;
		if (!stm32_gpio_stat(p->gpio, p->pin)) {
			if (!obj->pwr_fault) {
				io_set_rate(def->tbl_led, RATE_120BPM);
				obj->pwr_fault = true;
				WARN("/!\ CKT %d power fault!\n", i + 1);
			}
		} else {
			if (obj->pwr_fault) {
				io_set_rate(def->tbl_led, RATE_OFF);
				obj->pwr_fault = false;
				WARN("CKT %d power fault restored.\n", i + 1);
			}
		}
	}
}

void amp_init(void)
{
	struct amp_ckt_obj * obj;
	int i;

	for (i = 0; i <= AMP_CKTS; ++i) {
		obj = &amp.ckt_obj[i];
		obj->pwr_fault = false;
		obj->pwr_switch = false;
	}
}

