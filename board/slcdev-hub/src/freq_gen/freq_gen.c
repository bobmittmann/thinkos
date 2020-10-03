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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file mstp_test.c
 * @brief MS/TP test application
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stm32f.h>
#include <sys/delay.h>
#include <errno.h>

#include <thinkos.h>
#include <sys/console.h>
#include <sys/serial.h>

#define TRACE_LEVEL 7

#include <trace.h>
#include "board.h"

const uint8_t freqgen_pattern[] = {
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55
};

void __attribute__((noreturn)) freqgen_task(void)
{
	struct serial_dev * ser;

	ser = stm32f_uart5_serial_init(600000, SERIAL_8N1);

	for (;;) {
		serial_send(ser, freqgen_pattern, 16);
	}
}

uint32_t freqgen_stack[128];

const struct thinkos_thread_inf freqgen_inf = {
	.stack_ptr = freqgen_stack,
	.stack_size = sizeof(freqgen_stack),
	.priority = 32,
	.thread_id = 18,
	.paused = false,
	.tag = "SUPV"
};

void freqgen_init(void)
{
	thinkos_thread_create_inf((void *)freqgen_task, (void *)NULL,
							  &freqgen_inf);

	thinkos_sleep(1);
}

/* -------------------------------------------------------------------------
 * Test
 * ------------------------------------------------------------------------- */
void io_init(void)
{
	thinkos_udelay_factor(&udelay_factor);

	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOD);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOE);

    /* USART5 TX */
    stm32_gpio_mode(UART5_TX, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
    stm32_gpio_af(UART5_TX, GPIO_AF8);

    /* USART5 RX */
    stm32_gpio_mode(UART5_RX, ALT_FUNC, PULL_UP);
    stm32_gpio_af(UART5_RX, GPIO_AF8);

//    stm32_gpio_mode(UART5_TX, OUTPUT, PUSH_PULL | SPEED_LOW);
 //   stm32_gpio_set(UART5_TX);

    stm32_gpio_mode(IO_RS485_RX, ALT_FUNC, PULL_UP);
    stm32_gpio_af(IO_RS485_RX, RS485_USART_AF);

    stm32_gpio_mode(IO_RS485_TX, ALT_FUNC, PUSH_PULL | SPEED_MED);
    stm32_gpio_af(IO_RS485_TX, RS485_USART_AF);

    stm32_gpio_mode(IO_RS485_MODE, OUTPUT, PUSH_PULL | SPEED_LOW);
    stm32_gpio_set(IO_RS485_MODE);

    /* USART6_TX */
    stm32_gpio_mode(UART6_TX, ALT_FUNC, PUSH_PULL | SPEED_LOW);
    stm32_gpio_af(UART6_TX, GPIO_AF7);
    /* USART6_RX */
    stm32_gpio_mode(UART6_RX, ALT_FUNC, PULL_UP);
    stm32_gpio_af(UART6_RX, GPIO_AF7);
}

/* -------------------------------------------------------------------------
 * stdio
 * ------------------------------------------------------------------------- */

void stdio_init(void)
{
	FILE * f;

	f = console_fopen();
	/* initialize STDIO */
	stderr = f;
	stdout = f;
	stdin = f;
}

#if 0
void uart5_init(unsigned int baudrate)
{
	struct stm32_usart * uart = STM32_UART5;
	uint32_t f_pclk;
	uint32_t div;
	uint32_t cr1;
	uint32_t cr2;
	uint32_t cr3;
	uint32_t m;
	uint32_t f;

	/* Enable peripheral clock */
	stm32_clk_enable(STM32_RCC, STM32_CLK_UART5);

	/* Disable peripheral */
	uart->cr1 = 0;
	uart->cr2 = 0;
	uart->cr3 = 0;

	cr1 = 0;
	cr2 = 0;
	cr3 = 0;

	f_pclk = stm32_clk_hz(STM32_CLK_USART5);

	div = f_pclk / baudrate;

	INF("baudrate=%d p_clk=%d div=%d", baudrate, f_pclk, div);

	m = div >> 4;
	f = div & 0x0f;
	uart->brr = (m << 4) | f;

	/* 8 bits */
	cr1 |= USART_M8;
	/* 1 stop bit */
	cr2 |= USART_STOP_1;

	uart->cr1 = cr1;
	uart->cr2 = cr2;
	uart->cr3 = cr3;

	/* enable UART */
	uart->cr1 |= USART_TE;
	uart->cr3 |= USART_DMAT | USART_DMAR;

	for (;;) {
		stm32_usart_putc(STM32_UART5, 0x55);
	}
}
#endif

int main(int argc, char ** argv)
{
	unsigned int cnt = 0;

	io_init();

	stdio_init();

	printf("\n\n\nFrequency generator...\n");
/*
	stm32_usart_init(uart);
	stm32_usart_baudrate_set(uart, 550000);
	stm32_usart_mode_set(uart, SERIAL_8N1);
	stm32_usart_enable(uart); 

	for (;;) {
		stm32_usart_putc(STM32_UART5, 0x55);
	} */

	freqgen_init();
	for (;;) {
		cnt++;
		thinkos_sleep(1000);
		printf(" .%d ...\n", cnt);
	}

	return 0;
}


