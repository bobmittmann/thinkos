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

static void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);

	/* - USB ----------------------------------------------------*/
	stm32_gpio_af(USB_FS_DP, GPIO_AF10);
	stm32_gpio_af(USB_FS_DM, GPIO_AF10);
	stm32_gpio_af(USB_FS_VBUS, GPIO_AF10);

	stm32_gpio_mode(USB_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(USB_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(USB_FS_VBUS, ALT_FUNC, SPEED_LOW);
//	stm32_gpio_mode(USB_FS_VBUS, INPUT, SPEED_LOW);

	stm32_gpio_af(IO_MCO, GPIO_AF0);
	stm32_gpio_mode(IO_MCO, ALT_FUNC, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_set(IO_DAC1);
	stm32_gpio_mode(IO_DAC1, OUTPUT, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_set(IO_MODE);
	stm32_gpio_mode(IO_MODE, OUTPUT, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_set(IO_SW0);
	stm32_gpio_mode(IO_SW0, OUTPUT, PULL_UP | SPEED_LOW);

	stm32_gpio_set(IO_SW1);
	stm32_gpio_mode(IO_SW1, OUTPUT, PULL_UP | SPEED_LOW);

	stm32_gpio_set(IO_SW2);
	stm32_gpio_mode(IO_SW2, OUTPUT, PULL_UP | SPEED_LOW);

	stm32_gpio_set(IO_SW3);
	stm32_gpio_mode(IO_SW3, OUTPUT, PULL_UP | SPEED_LOW);
}

int board_init(void)
{
	io_init();

	/* Adjust USB interrupts priority */
	cm3_irq_pri_set(STM32_IRQ_USB_FS, MONITOR_PRIORITY);

	/* Enable USB interrupts */
	cm3_irq_enable(STM32_IRQ_USB_FS);

	return true;
}

void board_on_softreset(void)
{
#if 1
	struct stm32_rcc * rcc = STM32_RCC;
	/* Reset all peripherals except USB_OTG and GPIOA */
	rcc->ahb1rstr = ~(0); 
	rcc->ahb2rstr = ~(1 << RCC_GPIOA); 
	rcc->apb1rstr1 = ~((1 << RCC_USBFS) | (1 << RCC_PWR));
	rcc->apb1rstr2 = ~(0);
	rcc->apb2rstr = ~(0);

	rcc->ahb1rstr = 0;
	rcc->ahb2rstr = 0; 
	rcc->apb1rstr1 = 0;
	rcc->apb1rstr2 = 0;
	rcc->apb2rstr = 0;

	/* disable all peripherals clock sources except USB_OTG, GPIOA */
	rcc->ahb1enr = 0;
	rcc->ahb2enr = (1 << RCC_GPIOA); 
	rcc->apb1enr1 = (1 << RCC_USBFS) | (1 << RCC_PWR);
	rcc->apb1enr2 = 0;
	rcc->apb2enr = 0;

	DCC_LOG(LOG_TRACE, "^^^^ Soft Reset ^^^^");
#endif
	/* reinitialize IO's */
	io_init();

	/* restore USB interrupt priority */
	cm3_irq_pri_set(STM32_IRQ_USB_FS, MONITOR_PRIORITY);
}

bool board_autoboot(uint32_t tick)
{
	/* Time window autoboot */
	return (tick == 40) ? true : false;
}


bool board_configure(struct dmon_comm * comm)
{
	return true;
}

void board_selftest(struct dmon_comm * comm)
{
}

void board_upgrade(struct dmon_comm * comm)
{
}

void board_on_appload(void)
{
}

void board_on_error(int code)
{
}

void board_on_comm_init(void)
{
}


const struct mem_desc sram_desc = {
	.name = "RAM",
	.blk = {
		{ 0x10000000, BLK_RW, SZ_8K,  1 }, /*  CCM - Main Stack */
		{ 0x20000000, BLK_RO, SZ_4K,  1 },  /* Bootloader: 4KiB */
		{ 0x20001000, BLK_RW, SZ_4K,  11 }, /* Application: 44KiB */
		{ 0x00000000, 0, 0, 0 }
	}
}; 

const struct mem_desc flash_desc = {
	.name = "FLASH",
	.blk = {
		{ 0x08000000, BLK_RO, SZ_2K,  24 }, /* Bootloader: 48 KiB */
		{ 0x0800c000, BLK_RW, SZ_2K, 104 }, /* Application:  */
		{ 0x00000000, 0, 0, 0 }
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

const struct thinkos_board this_board = {
	.name = "RDA8COM",
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
		.start_addr = 0x0800c000,
		.block_size = (104 * 4) * 1024,
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

