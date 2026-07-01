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

#include <sys/stm32f.h>
#include <sys/delay.h>
#include <sys/dcclog.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>

#include "board.h"
#include "version.h"

void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOD);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOE);

	/* - Amplifier power --------------------------------------------------*/
	stm32_gpio_clr(IO_TRIG);
	stm32_gpio_mode(IO_TRIG, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_PWR1);
	stm32_gpio_mode(IO_PWR1, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_PWR2);
	stm32_gpio_mode(IO_PWR2, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_PWR3);
	stm32_gpio_mode(IO_PWR3, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_PWR4);
	stm32_gpio_mode(IO_PWR4, OUTPUT, PUSH_PULL | SPEED_LOW);

	/* Pulse trigger to force power down */
	stm32_gpio_set(IO_TRIG);
	udelay(10);
	stm32_gpio_clr(IO_TRIG);

	/* - USB ----------------------------------------------------*/
	stm32_gpio_af(OTG_FS_DP, GPIO_AF10);
	stm32_gpio_af(OTG_FS_DM, GPIO_AF10);
	stm32_gpio_af(OTG_FS_VBUS, GPIO_AF10);

	stm32_gpio_mode(OTG_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_VBUS, ALT_FUNC, SPEED_LOW);

	/* - LEDs ---------------------------------------------------------*/
	stm32_gpio_clr(IO_LED1A);
	stm32_gpio_mode(IO_LED1A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1B);
	stm32_gpio_mode(IO_LED1B, OUTPUT, PUSH_PULL | SPEED_LOW);
	/*Trouble LED */
	stm32_gpio_clr(IO_LED1C);
	stm32_gpio_mode(IO_LED1C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1D);
	stm32_gpio_mode(IO_LED1D, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED2A);
	stm32_gpio_mode(IO_LED2A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED2B);
	stm32_gpio_mode(IO_LED2B, OUTPUT, PUSH_PULL | SPEED_LOW);
	/*Trouble LED */
	stm32_gpio_clr(IO_LED2C);
	stm32_gpio_mode(IO_LED2C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED2D);
	stm32_gpio_mode(IO_LED2D, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED3A);
	stm32_gpio_mode(IO_LED3A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED3B);
	stm32_gpio_mode(IO_LED3B, OUTPUT, PUSH_PULL | SPEED_LOW);
	/*Trouble LED */
	stm32_gpio_clr(IO_LED3C);
	stm32_gpio_mode(IO_LED3C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED3D);
	stm32_gpio_mode(IO_LED3D, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED4A);
	stm32_gpio_mode(IO_LED4A, OUTPUT, PUSH_PULL | SPEED_LOW);
	/*Trouble LED */
	stm32_gpio_set(IO_LED4B);
	stm32_gpio_mode(IO_LED4B, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED4C);
	stm32_gpio_mode(IO_LED4C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED4D);
	stm32_gpio_mode(IO_LED4D, OUTPUT, PUSH_PULL | SPEED_LOW);

	/* - Relays ---------------------------------------------------------*/
	stm32_gpio_mode(IO_BKP1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_BKP1);

	stm32_gpio_mode(IO_BKP2, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_BKP2);

	stm32_gpio_mode(IO_BKP3, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_BKP3);

	stm32_gpio_mode(IO_BKP4, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_BKP4);

	stm32_gpio_mode(IO_LINB1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_LINB1);

	stm32_gpio_mode(IO_LINB2, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_LINB2);

	stm32_gpio_mode(IO_LINB3, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_LINB3);

	stm32_gpio_mode(IO_LINB4, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_set(IO_LINB4);

	/* - I2S ---------------------------------------------------------*/
	stm32_gpio_af(IO_I2S2_SD, GPIO_AF5);
	stm32_gpio_af(IO_I2S2_SCLK, GPIO_AF5);
	stm32_gpio_af(IO_I2S2_FS, GPIO_AF5);
	stm32_gpio_af(IO_I2S2_MCLK, GPIO_AF5);
	stm32_gpio_mode(IO_I2S2_SD, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S2_SCLK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S2_FS, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S2_MCLK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_af(IO_I2S3_SD, GPIO_AF6);
	stm32_gpio_af(IO_I2S3_SCLK, GPIO_AF6);
	stm32_gpio_af(IO_I2S3_FS, GPIO_AF6);
	stm32_gpio_af(IO_I2S3_MCLK, GPIO_AF6);
	stm32_gpio_mode(IO_I2S3_SD, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S3_SCLK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S3_FS, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_I2S3_MCLK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);

	/* - Fault ---------------------------------------------------------*/
	/* Waits before programming the fault lines as fault 2 is connected
	   to the JTRST preventing the JTAG to work */

	stm32_gpio_mode(IO_FAULT1, INPUT, SPEED_LOW);
#ifndef DEBUG
	DCC_LOG(LOG_WARNIGN, "Skiping FAULT2 pin config!");
//	stm32_gpio_mode(IO_FAULT2, INPUT, SPEED_LOW);
#endif
	stm32_gpio_mode(IO_FAULT3, INPUT, SPEED_LOW);
	stm32_gpio_mode(IO_FAULT4, INPUT, SPEED_LOW);

	/* Enble FPU */
	CM3_SCB->cpacr |= CP11_SET(3) | CP10_SET(3);
}

int board_init(void)
{
	int opt = 0;

	DCC_LOG1(LOG_TRACE, "clk[AHB]=%d", stm32f_ahb_hz);
	DCC_LOG1(LOG_TRACE, "clk[APB1]=%d", stm32f_apb1_hz);
	DCC_LOG1(LOG_TRACE, "clk[TIM1]=%d", stm32f_tim1_hz);
	DCC_LOG1(LOG_TRACE, "clk[APB2]=%d", stm32f_apb2_hz);
	DCC_LOG1(LOG_TRACE, "clk[TIM2]=%d", stm32f_tim2_hz);

	DCC_LOG(LOG_MSG, "io_init()");
	io_init();

#if 0
	DCC_LOG(LOG_TRACE, "leds_on()");
	__led_on(IO_LED1A);
	__led_on(IO_LED2A);
	__led_on(IO_LED3A);
	__led_on(IO_LED4A);

	udelay(250000);

	__led_off(IO_LED1A);
	__led_off(IO_LED2A);
	__led_off(IO_LED3A);
	__led_off(IO_LED4A);

	__led_on(IO_LED1B);
	__led_on(IO_LED2B);
	__led_on(IO_LED3B);
	__led_on(IO_LED4B);

	udelay(250000);

	__led_off(IO_LED1B);
	__led_off(IO_LED2B);
	__led_off(IO_LED3B);
	__led_off(IO_LED4B);

	__led_on(IO_LED1C);
	__led_on(IO_LED2C);
	__led_on(IO_LED3C);
	__led_on(IO_LED4C);

	udelay(250000);

	__led_off(IO_LED1C);
	__led_off(IO_LED2C);
	__led_off(IO_LED3C);
	__led_off(IO_LED4C);

	__led_on(IO_LED1D);
	__led_on(IO_LED2D);
	__led_on(IO_LED3D);
	__led_on(IO_LED4D);

	udelay(250000);

	__led_off(IO_LED1D);
	__led_off(IO_LED2D);
	__led_off(IO_LED3D);
	__led_off(IO_LED4D);

	udelay(250000);
#endif

	if (stm32_gpio_stat(IO_JTMS) == 0) {
		/* Force loading monitor */
		opt |= BOOT_OPT_DBGCOMM;
		opt |= BOOT_OPT_CONSOLE;
		opt |= BOOT_OPT_MONITOR;

		/* Flash all RED leds */
		__led_on(IO_LED1D);
		__led_on(IO_LED2D);
		__led_on(IO_LED3D);
		__led_on(IO_LED4D);
		thinkos_sleep(250000);
		__led_off(IO_LED1D);
		__led_off(IO_LED2D);
		__led_off(IO_LED3D);
		__led_off(IO_LED4D);
		thinkos_sleep(250000);
		__led_on(IO_LED1D);
		__led_on(IO_LED2D);
		__led_on(IO_LED3D);
		__led_on(IO_LED4D);
		thinkos_sleep(250000);
		__led_off(IO_LED1D);
		__led_off(IO_LED2D);
		__led_off(IO_LED3D);
		__led_off(IO_LED4D);
		thinkos_sleep(250000);
		__led_on(IO_LED1D);
		__led_on(IO_LED2D);
		__led_on(IO_LED3D);
		__led_on(IO_LED4D);
		thinkos_sleep(250000);
		__led_off(IO_LED1D);
		__led_off(IO_LED2D);
		__led_off(IO_LED3D);
		__led_off(IO_LED4D);
		thinkos_sleep(250000);
	} else {
		opt |= BOOT_OPT_APPRUN;
	}

	/* set the interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);

	DCC_XXD(LOG_TRACE, "vectors", (void *)0, 32);

	(void)opt;
	return 1;
}

bool board_autoboot(uint32_t tick)
{
	switch (tick & 0x3) {
	case 0:
		__led_off(IO_LED4A);
		__led_on(IO_LED1A);
		break;
	case 1:
		__led_off(IO_LED1A);
		__led_on(IO_LED2A);
		break;
	case 2:
		__led_off(IO_LED2A);
		__led_on(IO_LED3A);
		break;
	case 3:
		__led_off(IO_LED3A);
		__led_on(IO_LED4A);
		break;
	}

	/* Time window autoboot */
	return (tick == 40) ? true : false;
}

void board_on_comm_init(void)
{
	/* set the interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);
}

void board_on_softreset(void)
{
	struct stm32_rcc * rcc = STM32_RCC;

	DCC_LOG1(LOG_TRACE, "AHB1ENR=0x%08x", rcc->ahb1enr);
	DCC_LOG1(LOG_TRACE, "AHB2ENR=0x%08x", rcc->ahb2enr);
	DCC_LOG1(LOG_TRACE, "AHB3ENR=0x%08x", rcc->ahb3enr);
	DCC_LOG1(LOG_TRACE, "APB1ENR=0x%08x", rcc->apb1enr);
	DCC_LOG1(LOG_TRACE, "APB2ENR=0x%08x", rcc->apb2enr);

	/* Reset all peripherals except USB_OTG and GPIOA */
	rcc->ahb1rstr = ~(1 << RCC_GPIOA); 
	rcc->ahb2rstr = ~(1 << RCC_OTGFS);
	rcc->ahb3rstr = ~(0);
	rcc->apb1rstr = ~(0);
	rcc->apb2rstr = ~(0);


	rcc->ahb1rstr = 0;
	rcc->ahb2rstr = 0;
	rcc->ahb3rstr = 0;
	rcc->apb1rstr = 0;
	rcc->apb2rstr = 0;

	/* disable all peripherals clock sources except USB_OTG and GPIOA */
	rcc->ahb1enr = (1 << RCC_GPIOA); 
	rcc->ahb2enr = (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr = 0;

	/* reinitialize IO's */
	io_init();
}

void app_default(void * arg);

void board_on_error(int code)
{
	uint32_t tick;

	for (tick = 0;; ++tick) {
		thinkos_sleep(125);
		if (tick & 1) {
			__led_on(IO_LED1D);
			__led_on(IO_LED2D);
			__led_off(IO_LED3D);
			__led_off(IO_LED4D);
		} else {
			__led_on(IO_LED3D);
			__led_on(IO_LED4D);
			__led_off(IO_LED1D);
			__led_off(IO_LED2D);
		}

#if 0
		switch (tick & 0xf) {
		case 0:
			__led_off(IO_LED4D);
			__led_on(IO_LED1A);
			break;
		case 1:
			__led_off(IO_LED1A);
			__led_on(IO_LED1B);
			break;
		case 2:
			__led_off(IO_LED1B);
			__led_on(IO_LED1C);
			break;
		case 3:
			__led_off(IO_LED1C);
			__led_on(IO_LED1D);
			break;
		case 4:
			__led_off(IO_LED1D);
			__led_on(IO_LED2A);
			break;
		case 5:
			__led_off(IO_LED2A);
			__led_on(IO_LED2B);
			break;
		case 6:
			__led_off(IO_LED2B);
			__led_on(IO_LED2C);
			break;
		case 7:
			__led_off(IO_LED2C);
			__led_on(IO_LED2D);
			break;
		case 8:
			__led_off(IO_LED2D);
			__led_on(IO_LED3A);
			break;
		case 9:
			__led_off(IO_LED3A);
			__led_on(IO_LED3B);
			break;
		case 10:
			__led_off(IO_LED3B);
			__led_on(IO_LED3C);
			break;
		case 11:
			__led_off(IO_LED3C);
			__led_on(IO_LED3D);
			break;
		case 12:
			__led_off(IO_LED3D);
			__led_on(IO_LED4A);
			break;
		case 13:
			__led_off(IO_LED4A);
			__led_on(IO_LED4B);
			break;
		case 14:
			__led_off(IO_LED4B);
			__led_on(IO_LED4C);
			break;
		case 15:
			__led_off(IO_LED4C);
			__led_on(IO_LED4D);
			break;
		}
#endif
	}
}

void board_on_appload(void)
{
	DCC_LOG(LOG_TRACE, "........");
}

struct board_cfg {
	uint32_t magic;
	uint32_t mstp_addr;
	uint32_t ip_addr;
};

bool board_configure(struct dmon_comm * comm)
{
	return true;
}

void board_selftest(struct dmon_comm * comm)
{
}

extern int __heap_base;
const int * heap_base_ = &__heap_base;

extern const uint8_t otg_xflash_pic[];
extern const unsigned int sizeof_otg_xflash_pic;

const struct magic_blk bootldr_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 10
	},
	.rec = {
		{  0xfff00000, 0x10000000 },
		{  0xffff0000, 0x08000000 },
		{  0xffff0000, 0x08000000 },
		{  0xffff0000, 0x08000000 },

		{  0xffff0000, 0x08000000 },
		{  0xffff0000, 0x08000000 },
		{  0xffff0000, 0x08000000 },
		{  0xffff0000, 0x08000000 },

		{  0xffffffff, 0x10010000 },
		{  0xffff0000, 0x08000000 }
	}
};

const struct magic_blk thinkos_10_app_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 3 
	},
	.rec = {
		{ 0xffffffff, 0x0a0de004 },
		{ 0xffffffff, 0x6e696854 },
		{ 0xffffffff, 0x00534f6b },
		{ 0x00000000, 0x00000000 },
		{ 0x00000000, 0x00000000 }
	}
};

void board_upgrade(struct dmon_comm * comm)
{
	uint32_t * xflash_code = (uint32_t *)(0x20001000);
	int (* xflash_ram)(uint32_t, uint32_t, const struct magic_blk *) = 
		((void *)xflash_code) + 1;

	cm3_cpsid_f();
	__thinkos_memcpy(xflash_code, otg_xflash_pic, sizeof_otg_xflash_pic);
	xflash_ram(0, 65536, &bootldr_magic);
}

#ifndef ENABLE_PRIPHERAL_MEM
#define  ENABLE_PRIPHERAL_MEM 1
#endif

const struct mem_desc sram_desc = {
	.name = "RAM",
	.blk = {
		{ 0x20000000, BLK_RO, SZ_4K,   1 }, /* Bootloader: 4KiB */
		{ 0x20001000, BLK_RW, SZ_4K,  27 }, /* Application: 108KiB */
		{ 0x2001c000, BLK_RW, SZ_16K,  1 }, /* SRAM 2: 16KiB */

		{ 0x22000000, BLK_RO, SZ_512K, 1 }, /* Bootloader - bitband */
		{ 0x22080000, BLK_RW, SZ_512K, 27}, /* Application - bitband */
		{ 0x22e00000, BLK_RW, SZ_2M,   1 }, /* SRAM 2 - bitband */

#ifdef ENABLE_PRIPHERAL_MEM
		{ 0x40000000, BLK_RO, SZ_1M,   1 },  /* Peripheral */
		{ 0x42000000, BLK_RO, SZ_128M, 1 },  /* Peripheral - bitband */
#endif

		{ 0x00000000, 0, 0, 0 }
	}
}; 

const struct mem_desc flash_desc = {
	.name = "FLASH",
	.blk = {
		{ 0x08000000, BLK_RO, SZ_16K,  3 }, /* Bootloader */
		{ 0x0800c000, BLK_RW, SZ_16K,  1 }, /* Application */
		{ 0x08010000, BLK_RW, SZ_64K,  1 }, /* Application */
		{ 0x08020000, BLK_RW, SZ_128K, 3 }, /* Application */
		{ 0x00000000, 0, 0, 0 }
	}
}; 

const struct thinkos_board this_board = {
	.name = "RDA420-70",
	.hw_ver = {
		.major = 0,
		.minor = 1,
	},
	.sw_ver = {
		.major = VERSION_MAJOR,
		.minor = VERSION_MINOR,
		.build = VERSION_BUILD
	},
	.memory = {
		.ram = &sram_desc,
		.flash = &flash_desc
	},
	.application = {
		.start_addr = 0x08020000,
		.block_size = (3 * 128) * 1024,
		.magic = &thinkos_10_app_magic 
	},
	.init = board_init,
	.softreset = board_on_softreset,
	.autoboot = board_autoboot,
	.configure = board_configure,
	.upgrade = board_upgrade,
	.selftest = board_selftest,
	.on_appload = board_on_appload,
	.on_error = board_on_error,
	.on_comm_init = board_on_comm_init
};

#if 0

#include <sys/console.h>
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

#endif

void board_test(void * arg)
{
	DCC_LOG1(LOG_TRACE, "arg=%p", arg);

#if 0
	stdio_init();
#endif

	for (;;) {
		__led_on(IO_LED1A);

		thinkos_sleep(66);

		__led_off(IO_LED1A);

		__led_on(IO_LED1B);

		thinkos_sleep(67);

		__led_off(IO_LED1B);

		DCC_LOG(LOG_TRACE, "tick");
	}	
}

void app_default(void * arg) __attribute__((weak, alias("board_test")));

