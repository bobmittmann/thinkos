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
 * @file boot.c
 * @brief YARD-ICE bootloader main
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include "monitor-i.h"

#include <sys/stm32f.h>
#include <sys/delay.h>

#include <sys/dcclog.h>

#include "board.h"
#include "version.h"


void io_init(void)
{
	struct stm32_rcc * rcc = STM32_RCC;

	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);

	/* Enable Alternate Functions IO clock */
	rcc->apb2enr |= RCC_AFIOEN;

	/* UART */
	stm32_gpio_mode(USART2_TX, ALT_FUNC, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(USART2_RX, INPUT, PULL_UP);

	/* RS 485 */
	stm32_gpio_mode(RS485_RXEN, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(RS485_RXEN);

	stm32_gpio_mode(RS485_TXEN, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(RS485_TXEN);

	/* USB */
	stm32_gpio_mode(USB_FS_VBUS, INPUT, 0);
	stm32_gpio_set(USB_FS_VBUS);

}

void board_reset(void)
{
	/* disable all peripherals clock sources except USB_FS, 
	   GPIOA and GPIOB */

	/* initialize IO's */
	io_init();

	/* Adjust USB interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_USB_HP, MONITOR_PRIORITY);
	cm3_irq_pri_set(STM32F_IRQ_USB_LP, MONITOR_PRIORITY);
	/* Enable USB interrupt */
	cm3_irq_enable(STM32F_IRQ_USB_HP);
	cm3_irq_enable(STM32F_IRQ_USB_LP);
}


void board_init(void)
{
	board_reset();
}


void main(int argc, char ** argv)
{
	const struct monitor_comm * comm;
	uint32_t flags = 0;

	DCC_LOG_INIT();

#ifndef UDELAY_FACTOR 
	thinkos_krn_udelay_calibrate();
#endif

#if DEBUG
	DCC_LOG_CONNECT();
	mdelay(100);
	DCC_LOG(LOG_INFO, "______________________________________________________");
	DCC_LOG(LOG_INFO, "_________________ ! Board start up ! _________________");
	DCC_LOG(LOG_INFO, "______________________________________________________");
	mdelay(100);
#endif
	DCC_LOG1(LOG_TRACE, "udelay_factor=%d.", udelay_factor);

	thinkos_krn_init(THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED, NULL, NULL);

	board_init();

	comm = usb_comm_init(&stm32f_usb_fs_dev);

	board_reset();

	/* starts/restarts monitor with autoboot enabled */
	thinkos_krn_monitor_init(comm, monitor_task, (void *)flags);

	DCC_LOG(LOG_TRACE, "10. thinkos_abort()");
	thinkos_abort();

	DCC_LOG(LOG_ERROR, "11. unreachable code reched!!!");
}

