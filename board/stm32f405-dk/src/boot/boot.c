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

#ifdef CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdbool.h>

#include <sys/stm32f.h>
#include <sys/delay.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#include <thinkos.h>

#include <sys/dcclog.h>
#include <sys/console.h>

#include "board.h"
#include "version.h"

void monitor_task(struct dmon_comm * comm, void * param);

int app_main(int argc, char *argv[]);

const char * const argv[] = { "thinkos_app" };

void __attribute__((noreturn)) board_app_task(void * param)
{
	int argc = 1;

	console_write(NULL, "\r\nThinkOS app loader...\r\n\r\n", 27);

	for (;;) {
		app_main(argc, (char **)argv);
		thinkos_sleep(500);
	}
}


#if 0
static inline void __stm32_gpio_af(struct stm32_gpio * gpio, int pin, int af)
{
	uint32_t tmp;

	if (pin < 8) {
		tmp = gpio->afrl;
		tmp &= ~GPIO_AFRL_MASK(pin);
		tmp |= GPIO_AFRL_SET(pin, af);
		gpio->afrl = tmp;
	} else {
		tmp = gpio->afrh;
		tmp &= ~GPIO_AFRH_MASK(pin);
		tmp |= GPIO_AFRH_SET(pin, af);
		gpio->afrh = tmp;
	}

	/* select alternate function pin mode */
	tmp = (gpio->moder & ~GPIO_MODE_MASK(pin)) | GPIO_MODE_ALT_FUNC(pin);
	gpio->moder = tmp;
}
#endif

static void io_init(void)
{
	struct stm32_gpio * gpioa = STM32_GPIOA;
	struct stm32_gpio * gpiob = STM32_GPIOB;

	/* select alternate functions to USB and SPI1 pins ... */
	/* Port A */
	gpioa->afrl = 0;
	gpioa->afrh = 
		GPIO_AFRH_SET(8, GPIO_AF10) | /* USB */
		GPIO_AFRH_SET(11, GPIO_AF10) | GPIO_AFRH_SET(12, GPIO_AF10) | /* USB */
		GPIO_AFRH_SET(13, GPIO_AF0) | GPIO_AFRH_SET(14, GPIO_AF0) | /* JTAG */
		GPIO_AFRH_SET(15, GPIO_AF0); /* JTAG */

	gpioa->moder = GPIO_MODE_ALT_FUNC(8) | /* USB */
		GPIO_MODE_ALT_FUNC(11) | GPIO_MODE_ALT_FUNC(12) | /* USB */
		GPIO_MODE_ALT_FUNC(13) | GPIO_MODE_ALT_FUNC(14) | /* JTAG */
		GPIO_MODE_ALT_FUNC(15); /* JTAG */

	gpioa->otyper = GPIO_PUSH_PULL(8) | /* MCO */
		GPIO_PUSH_PULL(11) | GPIO_MODE_OUTPUT(12); /* USB */

	gpioa->ospeedr = GPIO_OSPEED_HIGH(8) | /* USB */
		GPIO_OSPEED_HIGH(11) | GPIO_OSPEED_HIGH(12); /* USB */

	gpioa->pupdr = GPIO_PULL_UP(13) | GPIO_PULL_DOWN(14) | /* JTAG */
		GPIO_PULL_UP(15); /* JTAG */
#if 0
	gpioa->dor = GPIO_SET(4) | /* SPI SS */
		GPIO_SET(9); /* CRESET */
#endif
	gpioa->odr = 0;

	/* Port B */
	gpiob->afrl = GPIO_AFRL_SET(5, GPIO_AF5) | /* SPI */
		GPIO_AFRL_SET(3, GPIO_AF0) | GPIO_AFRL_SET(4, GPIO_AF0); /* JTAG */
	gpiob->afrh = 0;
	
	gpiob->moder = GPIO_MODE_ALT_FUNC(5) | /* SPI */
		GPIO_MODE_ALT_FUNC(3) | GPIO_MODE_ALT_FUNC(4); /* JTAG */

	gpiob->otyper = GPIO_PUSH_PULL(5); /* SPI */

	gpiob->ospeedr = GPIO_OSPEED_HIGH(5) | /* SPI */
		GPIO_OSPEED_LOW(8) | GPIO_OSPEED_LOW(12); /* IO */

	gpiob->pupdr = GPIO_PULL_UP(4);/* JTAG */
}

void board_on_softreset(void)
{
	struct stm32_rcc * rcc = STM32_RCC;
//	struct stm32f_spi * spi = ICE40_SPI;

	DCC_LOG(LOG_TRACE, "^^^^ Soft Reset ^^^^");

	/* disable all peripherals clock sources except USB_FS, 
	   GPIOA and GPIOB */
	DCC_LOG1(LOG_TRACE, "ahb1enr=0x%08x", rcc->ahb1enr);
	DCC_LOG1(LOG_TRACE, "ahb2enr=0x%08x", rcc->ahb2enr);
	DCC_LOG1(LOG_TRACE, "apb1enr=0x%08x", rcc->apb1enr);
	DCC_LOG1(LOG_TRACE, "apb2enr=0x%08x", rcc->apb2enr);

	/* Reset all peripherals except USB_FS, GPIOA and FLASH */
#if 0
	rcc->ahb1rstr = ~(1 << RCC_FLASH); 
	rcc->ahb2rstr = ~((1 << RCC_GPIOA) | (1 << RCC_GPIOB)); 
	rcc->ahb3rstr = ~(0);
	rcc->apb1rstr1 = ~((1 << RCC_USBFS) | (1 << RCC_PWR) | (1 << RCC_RTCAPB));
	rcc->apb1rstr2 = ~(0);
	rcc->apb2rstr = ~(0);

	rcc->ahb1rstr = 0;
	rcc->ahb2rstr = 0; 
	rcc->ahb3rstr = 0;
	rcc->apb1rstr1 = 0;
	rcc->apb1rstr2 = 0;
	rcc->apb2rstr = 0;
#endif

	/* disable all peripherals clock sources except USB_FS, 
	   GPIOA and GPIOB */
//	rcc->ahb1enr = (1 << RCC_FLASH);
	rcc->ahb1enr |= (1 << RCC_CCMDATARAM) | (1 << RCC_GPIOA) | (1 << RCC_GPIOB); 
	rcc->ahb2enr |= (1 << RCC_OTGFS);
//	rcc->apb1enr = 0;
//	rcc->apb1enr = 0;
//	rcc->apb2enr = (1 << RCC_SPI1);

	/* initialize IO's */
	io_init();

	/* Adjust USB OTG FS interrupts priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);
}

int board_init(void)
{
	board_on_softreset();

	return 0;
}

#if 0
void board_init(void)
{
	struct stm32_gpio * gpio = STM32_GPIOA;
	struct stm32_rcc * rcc = STM32_RCC;
//	struct cm3_scb * scb = CM3_SCB;

	DCC_LOG(LOG_TRACE, "...");

	/* Disable exceptions */
//	scb->shcsr = 0;

	/* Reset all peripherals except USB_OTG and GPIOA */
	rcc->ahb1rstr = ~(1 << RCC_GPIOA); 
	rcc->ahb2rstr = ~(1 << RCC_OTGFS);
	rcc->apb1rstr = ~(0);
	rcc->apb2rstr = ~(0);
	/* clear reset bits */
	rcc->ahb1rstr = 0;
	rcc->ahb2rstr = 0;
	rcc->apb1rstr = 0;
	rcc->apb2rstr = 0;

	/* disable all peripherals clock sources except USB_OTG and GPIOA */
	rcc->ahb1enr |= (1 << RCC_CCMDATARAM) | (1 << RCC_GPIOA); 
	rcc->ahb2enr |= (1 << RCC_OTGFS);
	rcc->apb1enr |= 0;
	rcc->apb2enr |= 0;

	/* select alternate functions to USB pins ... */
	gpio->afrh = GPIO_AFRH_SET(9, GPIO_AF10) | GPIO_AFRH_SET(11, GPIO_AF10) | \
				 GPIO_AFRH_SET(12, GPIO_AF10);
	gpio->moder = GPIO_MODE_ALT_FUNC(9) |  GPIO_MODE_ALT_FUNC(11) | \
				  GPIO_MODE_ALT_FUNC(12) ;

	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);

	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOD);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);

	/* - Relay ------------------------------------------------------------*/
	stm32_gpio_mode(IO_RELAY, OUTPUT, SPEED_LOW);
	stm32_gpio_clr(IO_RELAY);

	/* - External Power ---------------------------------------------------*/
	stm32_gpio_mode(IO_PWR_EN, OUTPUT, SPEED_LOW);
	stm32_gpio_clr(IO_PWR_EN);
	stm32_gpio_mode(IO_PWR_MON, INPUT, SPEED_LOW | PULL_UP);
	/* - Debug UART ---------------------------------------------------*/
	stm32_gpio_mode(IO_UART5_TX, OUTPUT, SPEED_LOW);
	stm32_gpio_clr(IO_UART5_TX);
	stm32_gpio_mode(IO_UART5_TX, INPUT, SPEED_LOW | PULL_UP);

//	stm32_gpio_mode(OTG_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
//	stm32_gpio_mode(OTG_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
//	stm32_gpio_mode(OTG_FS_VBUS, ALT_FUNC, SPEED_LOW);


//	__stm32_gpio_af(OTG_FS_DP, GPIO_AF10);
//__stm32_gpio_af(OTG_FS_DM, GPIO_AF10);
//	stm32_gpio_af(OTG_FS_VBUS, GPIO_AF10);

	/* Adjust USB OTG FS interrupts priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);
}
#endif

#define MONITOR_AUTOBOOT 1
#define MONITOR_SHELL 2

#define CTRL_C  0x03 /* ETX */

void main(int argc, char ** argv)
{
	struct dmon_comm * comm;

	DCC_LOG_INIT();
	DCC_LOG_CONNECT();

	DCC_LOG(LOG_TRACE, "1. cm3_udelay_calibrate().");
	cm3_udelay_calibrate();

	DCC_LOG(LOG_TRACE, "2. thinkos_init().");
	thinkos_init(THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0));

#if THINKOS_ENABLE_MPU
	DCC_LOG(LOG_TRACE, "3. thinkos_mpu_init()");
	thinkos_mpu_init(0x1000);
#endif

	DCC_LOG(LOG_TRACE, "4. board_init().");
	board_init();

	/* Wait for the other power supply and subsystems to stabilize */
	DCC_LOG(LOG_TRACE, "5. thinkos_sleep().");
	thinkos_sleep(1000);

	DCC_LOG(LOG_TRACE, "6. thinkos_console_init()");
	thinkos_console_init();

	DCC_LOG(LOG_TRACE, "7. usb_comm_init()");
	comm = usb_comm_init(&stm32f_otg_fs_dev);

	/* Wait for the other power supply and subsystems to stabilize */
	DCC_LOG(LOG_TRACE, "5. thinkos_sleep().");
	thinkos_sleep(100);

	DCC_LOG(LOG_TRACE, "8. thinkos_dbgmon()");
	/* starts monitor with shell enabled */
	thinkos_dbgmon(monitor_task, comm, 
				   (void *)(MONITOR_SHELL | MONITOR_AUTOBOOT));

#if THINKOS_ENABLE_MPU
	DCC_LOG(LOG_TRACE, "9. thinkos_userland()");
	thinkos_userland();
#endif

	DCC_LOG(LOG_TRACE, "10. thinkos_thread_abort()");
	thinkos_thread_abort(0);

	DCC_LOG(LOG_ERROR, "11. unreachable code reched!!!");
}

