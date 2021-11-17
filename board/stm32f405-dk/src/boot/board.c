/* 
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

/** 
 * @file board.c
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 * @brief Bootloader descriptor and hardware initialization
 * 
 */

#include <sys/stm32f.h>
#include <sys/delay.h>
#include <sys/dcclog.h>
#define __THINKOS_FLASH__
#include <thinkos/flash.h>
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <trace.h>
#include <vt100.h>

#include "board.h"
#include "version.h"

/* ----------------------------------------------------------------------------
 * Memory map
 * ----------------------------------------------------------------------------
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

/* Bootloader board description  
   Bootloader and debugger, memory description  
 */

const struct thinkos_mem_desc sram_mem = {
	.tag = "RAM",
	.base = 0x00000000,
	.cnt = 5,
	.blk = {
		{.tag = "STACK",  0x10000000, M_RW, SZ_1K,  64 }, /* CCM - Main Stack */
		{.tag = "KERN",   0x20000000, M_RO, SZ_1K,   8 }, /* Bootloader: 8KiB */
		{.tag = "DATA",   0x20002000, M_RW, SZ_1K, 104 }, /* App: 104KiB */
		{.tag = "SRAM2",  0x2001c000, M_RW, SZ_1K,  16 }, /* SRAM 2: 16KiB */
		{.tag = "SRAM3",  0x20020000, M_RW, SZ_1K,  64 }, /* SRAM 3: 64KiB */
		{.tag = "", 0x00000000, 0, 0, 0}
		}
};

const struct thinkos_mem_desc flash_mem = {
	.tag = "FLASH",
	.base = 0x08000000,
	.cnt = 4,
	.blk = {
		{.tag = "BOOT", 0x00000000, M_RO, SZ_16K, 4}, /* Bootloader: 64 KiB */
		{.tag = "CONF", 0x00010000, M_RW, SZ_64K, 1}, /* Config: 64 KiB */
		{.tag = "APP",  0x00020000, M_RW, SZ_128K, 3}, /* Application: 384 KiB */
		{.tag = "EXT", 0x00080000, M_RW, SZ_128K, 4}, /* Extension: 512 KiB */
		{.tag = "", 0x00000000, 0, 0, 0}
		}
};

const struct thinkos_mem_desc peripheral_mem = {
	.tag = "PERIPH",
	.base = 0,
	.cnt = 1,
	.blk = {
		{.tag = "RTC", 0x40002800, M_RW, SZ_1K, 1}, /* RTC - 1K */
		{.tag = "", 0x00000000, 0, 0, 0}
		}
};


const struct thinkos_mem_map board_mem_map = {
	.tag = "MEM",
	.cnt = 3,
	.desc = {
		 &flash_mem,
		 &sram_mem,
		 &peripheral_mem
	}
};

const struct thinkos_flash_desc board_flash_desc = {
	.mem = &flash_mem,
	.dev = &stm32f4x_flash_dev
};

#pragma GCC diagnostic pop

static void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);

	/* - USB ----------------------------------------------------*/
	stm32_gpio_af(OTG_FS_DP, GPIO_AF10);
	stm32_gpio_af(OTG_FS_DM, GPIO_AF10);
	stm32_gpio_af(OTG_FS_VBUS, GPIO_AF10);

	stm32_gpio_mode(OTG_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_VBUS, ALT_FUNC, SPEED_LOW);

	/* - LEDs ---------------------------------------------------------*/
	stm32_gpio_clr(IO_LED1);
	stm32_gpio_mode(IO_LED1, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED2);
	stm32_gpio_mode(IO_LED2, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED3);
	stm32_gpio_mode(IO_LED3, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED4);
	stm32_gpio_mode(IO_LED4, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED4);
	stm32_gpio_mode(IO_LED4, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_mode(PUSH_BTN, INPUT, PULL_UP);

	stm32_gpio_mode(IO_I2C1_SCL, INPUT, 0);
	stm32_gpio_mode(IO_I2C1_SDA, INPUT, 0);

	stm32_gpio_clr(IO_I2S2_WS);
	stm32_gpio_mode(IO_I2S2_WS, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_I2S2_CK);
	stm32_gpio_mode(IO_I2S2_CK, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_I2S2_SDI);
	stm32_gpio_mode(IO_I2S2_WS, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_I2S2_SDO);
	stm32_gpio_mode(IO_I2S2_SDO, OUTPUT, PUSH_PULL | SPEED_LOW);

	/* Debug serial -----------------------------------------------------*/
	stm32_gpio_af(IO_UART1_TX, GPIO_AF7);
	stm32_gpio_mode(IO_UART1_TX, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_af(IO_UART1_RX, GPIO_AF7);
	stm32_gpio_mode(IO_UART1_RX, ALT_FUNC, PUSH_PULL | SPEED_HIGH);

/**************************************************************************
 * Debug serial
 **************************************************************************/
#define IO_UART1_TX       STM32_GPIOB, 6
#define IO_UART1_RX       STM32_GPIOB, 7

/**************************************************************************
 * I2C1
 **************************************************************************/
#define IO_I2C1_SCL           STM32_GPIOB, 8
#define IO_I2C1_SDA           STM32_GPIOB, 9

/**************************************************************************
 * USART3
 **************************************************************************/
#define IO_USART3_TX      STM32_GPIOB, 10
#define IO_USART3_RX      STM32_GPIOB, 11

/**************************************************************************
 * I2S2USART3
 **************************************************************************/
#define IO_I2S2_WS       STM32_GPIOB, 12
#define IO_I2S2_CK       STM32_GPIOB, 13
#define IO_I2S2_SDI      STM32_GPIOB, 14
#define IO_I2S2_SDO      STM32_GPIOB, 15
}

static void board_on_softreset(void)
{
	struct stm32_rcc * rcc = STM32_RCC;
//	struct stm32f_spi * spi = ICE40_SPI;

	/* disable all peripherals clock sources except USB_FS, 
	   GPIOA and GPIOB */
	DCC_LOG1(LOG_TRACE, "ahb1enr=0x%08x", rcc->ahb1enr);
	DCC_LOG1(LOG_TRACE, "ahb2enr=0x%08x", rcc->ahb2enr);
	DCC_LOG1(LOG_TRACE, "apb1enr=0x%08x", rcc->apb1enr);
	DCC_LOG1(LOG_TRACE, "apb2enr=0x%08x", rcc->apb2enr);

	/* Reset all peripherals except USB_FS, GPIOA and FLASH */

	/* disable all peripherals clock sources except USB_FS, 
	   GPIOA and GPIOB */
	rcc->ahb1enr |= (1 << RCC_CCMDATARAM) | (1 << RCC_GPIOA) | 
		(1 << RCC_GPIOB) | (1 << RCC_GPIOC) | (1 << RCC_CRC);
	rcc->ahb2enr |= (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr |= (1 << RCC_USART1);

	/* Reset all peripherals except USB_OTG and GPIOA */
	rcc->ahb1rstr = ~((1 << RCC_CCMDATARAM) | (1 << RCC_GPIOA) |
					  (1 << RCC_GPIOB) | (1 << RCC_GPIOC) | (1 << RCC_CRC)); 
	rcc->ahb2rstr = ~(1 << RCC_OTGFS);
	rcc->ahb3rstr = ~(0);
	rcc->apb1rstr = ~(0);
	rcc->apb2rstr = ~((1 << RCC_USART1));

	rcc->ahb1rstr = 0;
	rcc->ahb2rstr = 0;
	rcc->ahb3rstr = 0;
	rcc->apb1rstr = 0;
	rcc->apb2rstr = 0;

	/* disable all peripherals clock sources except USB_OTG and GPIOA */
	rcc->ahb1enr = (1 << RCC_CCMDATARAM) | (1 << RCC_GPIOA) | 
		(1 << RCC_GPIOB) | (1 << RCC_GPIOC) | (1 << RCC_CRC); 
	rcc->ahb2enr = (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr = (1 << RCC_USART1);

	/* reinitialize IO's */
	io_init();

	/* Adjust USB OTG FS interrupts priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);

	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);

	DCC_LOG1(LOG_TRACE, "OTG_FS IRQ=%d", STM32F_IRQ_OTG_FS);

	/* configure interrupts */
	cm3_irq_pri_set(STM32_IRQ_USART1, MONITOR_PRIORITY);
	/* enable interrupts */
	cm3_irq_enable(STM32_IRQ_USART1);

	DCC_LOG1(LOG_TRACE, "USART1 IRQ=%d", STM32_IRQ_USART1);
}

int board_init(void)
{
	stm32_gpio_mode(OTG_FS_VBUS, INPUT, 0);

	io_init();
	board_on_softreset();

	stm32_gpio_set(IO_LED3);
	stm32_gpio_set(IO_LED4);

	return 0;
}

void board_reset(void)
{
	board_on_softreset();
}

/* ----------------------------------------------------------------------------
 * Preboot: this task runs once at power up only.
 * It's used to delay booting up the application ... 
 * ----------------------------------------------------------------------------
 */

#define PREBOOT_TIME_SEC 1

bool board_integrity_check(void)
{
	uint32_t tick;

	/* Time window autoboot */
	for (tick = 0; tick < (PREBOOT_TIME_SEC * 4); ++tick) {
//		uint64_t time;

		thinkos_sleep(250);

		krn_console_puts(".");

		switch (tick & 0x3) {
		case 0:
			stm32_gpio_clr(IO_LED1);
			break;
		case 1:
			stm32_gpio_clr(IO_LED2);
			break;
		case 2:
			stm32_gpio_set(IO_LED1);
			break;
		case 3:
			stm32_gpio_set(IO_LED2);
			break;
		}

//		time = thinkos_time_realtime_get();
//		(void)time;
//		DCC_LOG1(LOG_TRACE, "time = %u", (time >> 32));
	}

	krn_console_puts("\r\n");

	thinkos_sleep(250);

	krn_console_puts("Loading application...\r\n");

	stm32_gpio_clr(IO_LED1);
	stm32_gpio_clr(IO_LED2);

	return true;
}

static int board_on_break(const struct monitor_comm * comm)
{
	struct btl_shell_env * env = btl_shell_env_getinstance();

#if 1
	/* Already initialized in main() */
	btl_shell_env_init(env, "\r\n+++\r\nThinkOS\r\n", "boot# ");
#endif

	return monitor_thread_create(comm, C_TASK(btl_console_shell), 
								 C_ARG(env), true);
}


/* Bootloader board description  */
const struct thinkos_board this_board = {
	.name = "STM32F405-DK",
	.desc = "STM32F405 Development Kit",
	.hw = {
	       .tag = "32F405DK",
	       .ver = {.major = 0,.minor = 1}
	       },
	.sw = {
	       .tag = "ThinkOS",
	       .ver = {
		       .major = VERSION_MAJOR,
		       .minor = VERSION_MINOR,
		       .build = VERSION_BUILD}
	       },
	.on_softreset = board_on_softreset,
	.on_break = board_on_break,
	.memory = &board_mem_map
};


