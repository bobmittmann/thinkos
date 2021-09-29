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

void shell_init(void);

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

void select_silence(void)
{
	struct addsynth_instrument * instr;
	instr = addsynth_instrument_getinstance();

	dac_gain_set(0);

	keyboard_config(&keyboard_piano_cfg);
	addsynth_instr_config(instr, &addsynth_null_cfg);
}

void select_xilophone(void)
{
	struct addsynth_instrument * instr;
	instr = addsynth_instrument_getinstance();

	keyboard_config(&keyboard_xilophone_cfg);
	addsynth_instr_config(instr, &addsynth_xilophone_cfg);
	dac_gain_set(0.75);
}

void select_piano(void)
{
	struct addsynth_instrument * instr;
	instr = addsynth_instrument_getinstance();

	keyboard_config(&keyboard_piano_cfg);
	addsynth_instr_config(instr, &addsynth_piano_cfg);
	dac_gain_set(0.75);
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

	io_init();

	spidrv_master_init(200000);

	dac_init();
	dac_gain_set(0);

	keyboard_init();

	instr = addsynth_instrument_getinstance();
	addsynth_instr_init(instr);

	dac_start();

	shell_init();

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

		case KBD_EV_SWITCH_ON:
			if (arg == 0)
				select_piano();
			else
				select_xilophone();
//			printf("<%2d> SWITCH ON\r\n", arg);
			break;

		case KBD_EV_SWITCH_OFF:
			select_silence();
//			printf("<%2d> SWITCH OFF\r\n", arg);
			break;

		case KBD_EV_TIMEOUT:
	//		printf("timeout\r\n");
//			cpu_usage_report(stdout);
			break;
		}
	}

	return 0;
}

