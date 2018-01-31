/* 
 * Copyright(C) 2013 Robinson Mittmann. All Rights Reserved.
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
 * @file stm32f-usb_fs.c
 * @brief YARD-ICE UART console
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <arch/cortex-m3.h>
#include <sys/delay.h>
#include <sys/dcclog.h>

#if defined(STM32F103) || defined(STM32F30X) || defined(STM32L4X)

#ifndef STM32_USB_FS_VBUS_CTRL
#define STM32_USB_FS_VBUS_CTRL   0
#endif

#define USB_FS_DP STM32_GPIOA,   12
#define USB_FS_DM STM32_GPIOA,   11
#define USB_FS_VBUS STM32_GPIOB, 12

void stm32f_usb_io_init(void)
{
	DCC_LOG(LOG_INFO, "Configuring USB IO pins ...");

	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
#if (STM32_USB_FS_VBUS_CTRL)
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
#endif

#if defined(STM32F3X)
	stm32_gpio_af(USB_FS_DP, GPIO_AF14);
	stm32_gpio_af(USB_FS_DM, GPIO_AF14);
#endif

	stm32_gpio_mode(USB_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(USB_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);

#if STM32_USB_FS_VBUS_ENABLE
	stm32_gpio_af(USB_FS_VBUS, GPIO_AF14);
	stm32_gpio_mode(USB_FS_VBUS, ALT_FUNC, SPEED_LOW);
#endif

#if (STM32_USB_FS_VBUS_CTRL)
	/* PB12: External Pull-up */
	stm32_gpio_mode(USB_FS_VBUS, INPUT, 0);
	stm32_gpio_set(USB_FS_VBUS);
#endif
}

void stm32f_usb_io_deinit(void)
{
	/* disabling IO pins */
	stm32_gpio_mode(USB_FS_DP, INPUT, 0);
	stm32_gpio_mode(USB_FS_DM, INPUT, 0);
}

void stm32f_usb_pullup(struct stm32f_usb * usb, bool connect)
{
#if defined(STM32L4X)
	/* Connect the internal pull up */
	if (connect) {
		DCC_LOG(LOG_TRACE, "Connecting Pull Up");
		usb->bcdr |= USB_DPPU;
	} else {
		DCC_LOG(LOG_TRACE, "Disconnecting Pull Up");
		usb->bcdr &= ~USB_DPPU;
	}
#endif
#if (STM32_USB_FS_VBUS_CTRL)
	if (connect)
		stm32_gpio_mode(USB_FS_VBUS, OUTPUT, PUSH_PULL | SPEED_LOW);
	else
		stm32_gpio_mode(USB_FS_VBUS, INPUT, SPEED_LOW);
#endif
}

void stm32f_usb_power_on(struct stm32f_usb * usb)
{
	DCC_LOG(LOG_INFO, "Enabling USB device clock...");

	stm32_clk_enable(STM32_RCC, STM32_CLK_USB);

	/* USB power ON */
	usb->cntr = USB_FRES;
	/* Wait tSTARTUP time ... */
	udelay(2);

	/* Removing the reset condition */
	usb->cntr = 0;

	/* Removing any spurious pending interrupts */
	usb->istr = 0;

}

void stm32f_usb_power_off(struct stm32f_usb * usb)
{
	usb->cntr = USB_FRES;
	/* Removing any spurious pending interrupts */
	usb->istr = 0;

	usb->cntr = USB_FRES | USB_PDWN;

	DCC_LOG(LOG_INFO, "Disabling USB device clock...");
	stm32_clk_disable(STM32_RCC, STM32_CLK_USB);
}

#endif /* STM32F103 || STM32F30X */

