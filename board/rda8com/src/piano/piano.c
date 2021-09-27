/* 
 * Copyright(C) 2020 Robinson Mittmann. All Rights Reserved.
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
 * @file piano.c
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "board.h"

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <fixpt.h>
#include <sys/param.h>
#include <sys/delay.h>
#include <sys/null.h>
#include <sys/console.h>
#include <thinkos.h>
#include <stdio.h>
#include "dac.h"
#include "spidrv.h"
#include "encoder.h"
#include "addsynth.h"
#include "keyboard.h"

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

	/* SPI */
	stm32_gpio_mode(IO_ICE40_SPI_SS, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_af(IO_ICE40_SPI_SS, ICE40_SPI_AF);
	stm32_gpio_mode(IO_ICE40_SPI_SCK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_af(IO_ICE40_SPI_SCK, ICE40_SPI_AF);
	stm32_gpio_mode(IO_ICE40_SPI_SDI, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_af(IO_ICE40_SPI_SDI, ICE40_SPI_AF);
	stm32_gpio_mode(IO_ICE40_SPI_SDO, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_af(IO_ICE40_SPI_SDO, ICE40_SPI_AF);

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
	/* initialize STDIO */
	stderr = null_fopen(0);
	stdout = f;
	stdin = f;
}

const struct keyboard_cfg keyboard_piano_cfg = {
	.keymap_cnt = 16,
	.keymap = {
		 [0] = { .key =  1, .code = MIDI_C4},
		 [1] = { .key =  2, .code = MIDI_D4},
		 [2] = { .key =  3, .code = MIDI_E4},
		 [3] = { .key =  4, .code = MIDI_F4},
		 [4] = { .key =  5, .code = MIDI_G4},
		 [5] = { .key =  6, .code = MIDI_A4},
		 [6] = { .key =  7, .code = MIDI_B4},
		 [7] = { .key =  8, .code = MIDI_C5},
		 [8] = { .key =  9, .code = MIDI_D5},
		 [9] = { .key = 10, .code = MIDI_E5},
		[10] = { .key = 11, .code = MIDI_F5},
		[11] = { .key = 12, .code = MIDI_G5},
		[12] = { .key = 13, .code = MIDI_A5},
		[13] = { .key = 14, .code = MIDI_B5},

		[14] = { .key = 15, .code = MIDI_C6},
		[15] = { .key = 16, .code = MIDI_D6}
	}
};


const struct keyboard_cfg keyboard_xilophone_cfg = {
	.keymap_cnt = 16,
	.keymap = {
		 [0] = { .key =  1, .code = MIDI_A4},
		 [1] = { .key =  2, .code = MIDI_B4},
		 [2] = { .key =  3, .code = MIDI_C5},
		 [3] = { .key =  4, .code = MIDI_D5},
		 [4] = { .key =  5, .code = MIDI_E5},
		 [5] = { .key =  6, .code = MIDI_F5},
		 [6] = { .key =  7, .code = MIDI_G5},
		 [7] = { .key =  8, .code = MIDI_A5},
		 [8] = { .key =  9, .code = MIDI_B5},
		 [9] = { .key = 10, .code = MIDI_C6},
		[10] = { .key = 11, .code = MIDI_D6},
		[11] = { .key = 12, .code = MIDI_E6},
		[12] = { .key = 13, .code = MIDI_F6},
		[13] = { .key = 14, .code = MIDI_G6},
		[14] = { .key = 15, .code = MIDI_A6},
		[15] = { .key = 16, .code = MIDI_B6}
	}
};



uint32_t x;

uint32_t __attribute__((noinline)) test(uint32_t r0, uint32_t r1, uint32_t r2)
{
	uint32_t r3;


	r3 = r0 & 0x7;
	r0 = r0 & ~0x7;

	switch (r3) {
		case 0:
			x = r1 + r2;
		break;

		case 1:
			x = r1 - r2;
		break;

		case 2:
			x = r1 * r2;
		break;

		case 3:
			x = r1 / r2;
		break;

		case 4:
			x = 3 * r1  + r2;
		break;

		case 5:
			x = r1  + 7 * r2;
		break;

		case 6:
			x = 17 * r1  + 11 * r2;
		break;
	}

	return r0;
}


int main(int argc, char ** argv)
{
	struct addsynth_instrument * instr;

	stdio_init();

	printf("\n\r\n\r");
	printf("-------------------\r\n");
	printf("  JUJU Synthesizer \r\n");
	printf("-------------------\r\n");
	printf("\n\r");

	test(10, 11, 22);

	io_init();

	spidrv_master_init(200000);

	printf("DAC init...\n\r");
	thinkos_sleep(10);
	dac_init();

	printf("Keyboard init...\n\r");
	thinkos_sleep(10);
	keyboard_init();

	printf("Instrument init...\n\r");
	thinkos_sleep(10);
	instr = addsynth_instrument_getinstance();
	addsynth_instr_init(instr);

	printf("Keyboard config...\n\r");
	thinkos_sleep(10);
	keyboard_config(&keyboard_xilophone_cfg);

	printf("Instrument config...\n\r");
	thinkos_sleep(10);
//	addsynth_instr_config(instr, &addsynth_piano_cfg);

	addsynth_instr_config(instr, &addsynth_xilophone_cfg);

	printf("Sequencer ...\n\r");
	thinkos_sleep(10);
	dac_start();

	printf("------------------------------------------------------------\n\r");
	thinkos_sleep(10);

	keyboard_timer_set(1, 10000);
	keyboard_timer_enable(1);

	dac_gain_set(0.66);

	/* sequencer */
	for (;;) {
		int event;
		int opc;
		int arg;
		int32_t code;
		int32_t vel;

		event = keyboard_event_wait();
		(void)event;
		
		opc = KBD_EVENT_OPC(event);
		arg = KBD_EVENT_ARG(event);

		switch (opc) {
		case KBD_EV_KEY_ON:
			code = arg;
			vel = 1;
			addsynth_instr_note_on(instr, code, vel); 
//			printf("<%2d> KEY_ON\r\n", arg);
			break;
		case KBD_EV_KEY_OFF:
			code = arg;
			vel = 1;
			addsynth_instr_note_off(instr, code, vel); 
//			printf("<%2d> KEY_OFF\r\n", arg);
			break;

		case KBD_EV_SWITCH_OFF:
	//		printf("<%2d> SWITCH OFF\r\n", arg);
			break;

		case KBD_EV_TIMEOUT:
	//		printf("timeout\r\n");
//			cpu_usage_report(stdout);
			break;
		}
	}

	return 0;
}

