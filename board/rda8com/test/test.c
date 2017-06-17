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
#include <sys/param.h>
#include <sys/delay.h>
#include <sys/console.h>
#include <thinkos.h>
#include <stdio.h>

void stdio_init(void)
{
	FILE * f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

unsigned int second;
unsigned int minute;
unsigned int hour;

void test(void)
{
	int i;

	for (i = 0; i < 20000000; ++i) {
		asm volatile( "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"

					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n" 

					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n" 

					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n" 

					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n"
					  "nop\n" :);
	}
}

int main(int argc, char ** argv)
{
	uint32_t clk;
	int32_t dt;

	stdio_init();

	printf("1. Initializing ... \n");

	printf("2. test start 1 billion instrucions... \n");
	clk = thinkos_clock();
	test();
	dt = thinkos_clock() - clk;
	printf("3. test done: %d ms.\n", dt);

	second = 0;
	minute = 0;
	hour = 0;
	for (;;) {
		clk += 1000;
		thinkos_alarm(clk);
		if (++second == 60) {
			second = 0;
			if (++minute == 60) {
				minute = 0;
				hour++;
			}
		}
		printf("%4d:%02d:%02d tick...\n", hour, minute, second);
	}

	return 0;
}



