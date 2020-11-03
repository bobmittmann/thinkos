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
		{.tag = "STACK",  0x10000000, M_RW, SZ_64K, 1}, /*  CCM - Main Stack */
		{.tag = "KERN",   0x20000000, M_RO, SZ_4K, 1},	 /* Bootloader: 4KiB */
		{.tag = "DATA",   0x20001000, M_RW, SZ_4K, 27}, /* App: 108KiB */
		{.tag = "SRAM2",  0x2001c000, M_RW, SZ_16K,  1 }, /* SRAM 2: 16KiB */
		{.tag = "SRAM3",  0x20020000, M_RW, SZ_64K,  1 }, /* SRAM 3: 64KiB */
		{.tag = "", 0x00000000, 0, 0, 0}
		}
};

const struct thinkos_mem_desc flash_mem = {
	.tag = "FLASH",
	.base = 0x08000000,
	.cnt = 3,
	.blk = {
		{.tag = "BOOT", 0x00000000, M_RO, SZ_16K, 4}, /* Bootloader: 64 KiB */
		{.tag = "CONF", 0x00010000, M_RW, SZ_64K, 1}, /* Config: 64 KiB */
		{.tag = "APP",  0x00020000, M_RW, SZ_128K, 7}, /* Application:  */
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


const struct thinkos_mem_map mem_map = {
	.tag = "MEM",
	.cnt = 3,
	.desc = {
		 &flash_mem,
		 &sram_mem,
		 &peripheral_mem
	}
};

const struct magic_blk thinkos_10_app_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 3},
	.rec = {
		{0xffffffff, 0x0a0de004},
		{0xffffffff, 0x6e696854},
		{0xffffffff, 0x00534f6b},
		{0x00000000, 0x00000000}
		}
};

#pragma GCC diagnostic pop

extern const struct flash_dev stm32f4x_flash_dev;

const struct thinkos_flash_desc board_flash_desc = {
	.mem = (const struct mem_desc *)&flash_mem,
	.dev = &stm32f4x_flash_dev
};


extern const uint8_t otg_xflash_pic[];
extern const unsigned int sizeof_otg_xflash_pic;

/* Receives a file using YMODEM protocol and writes into Flash. */
static int yflash(uint32_t blk_offs, uint32_t blk_size,
		   const struct magic_blk * magic)
{
	uintptr_t yflash_code = (uintptr_t)(0x20001000);
	int (* yflash_ram)(uint32_t, uint32_t, const struct magic_blk *);
	uintptr_t thumb;
	int ret;

	cm3_primask_set(1);
	__thinkos_memcpy((void *)yflash_code, otg_xflash_pic, 
					 sizeof_otg_xflash_pic);

	thumb = yflash_code | 0x00000001; /* thumb call */
	yflash_ram = (int (*)(uint32_t, uint32_t, const struct magic_blk *))thumb;
	ret = yflash_ram(blk_offs, blk_size, magic);

	return ret;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
static const struct magic_blk bootloader_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 2
	},
	.rec = {
		{  0xffff0000, 0x10010000 },
		{  0xffff0000, 0x08000000 },
	}
};
#pragma GCC diagnostic pop

void bootloader_yflash(const struct monitor_comm * comm)
{

	stm32_gpio_clr(IO_LED1);
	stm32_gpio_clr(IO_LED2);
	stm32_gpio_clr(IO_LED3);
	stm32_gpio_clr(IO_LED4);

	yflash(0, 32768, &bootloader_magic);
}


void __puts(const char *s)
{
	int rem = __thinkos_strlen(s, 64);

	while (rem) {
		int n = thinkos_console_write(s, rem);
		s += n;
		rem -= n;
	}
}

void io_init(void)
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
}

void board_on_softreset(void)
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
		(1 << RCC_GPIOB) | (1 << RCC_GPIOC); 
	rcc->ahb2enr |= (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr = 0;

	/* Reset all peripherals except USB_OTG and GPIOA */
	rcc->ahb1rstr = ~((1 << RCC_CCMDATARAM) | (1 << RCC_GPIOA) |
					  (1 << RCC_GPIOB) | (1 << RCC_GPIOC)); 
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
	rcc->ahb1enr = (1 << RCC_CCMDATARAM) | (1 << RCC_GPIOA) | 
		(1 << RCC_GPIOB) | (1 << RCC_GPIOC); 
	rcc->ahb2enr = (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr = 0;

	/* reinitialize IO's */
	io_init();

	/* Adjust USB OTG FS interrupts priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);

	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);
}

int board_init(void)
{
	board_on_softreset();

#if (THINKOS_FLASH_MEM_MAX > 0)
	thinkos_flash_drv_init(0, &board_flash_desc);
#endif

	stm32_gpio_set(IO_LED3);
	stm32_gpio_set(IO_LED4);

	return 0;
}

/* ----------------------------------------------------------------------------
 * Preboot: this task runs once at power up only.
 * It's used to delay booting up the application ... 
 * ----------------------------------------------------------------------------
 */

#define PREBOOT_TIME_SEC 1

int board_preboot_task(void *ptr)
{
	uint32_t tick;

	__puts("- board preboot\r\n");

	/* Time window autoboot */
	for (tick = 0; tick < 4*PREBOOT_TIME_SEC; ++tick) {
		thinkos_sleep(250);

		__puts(".");

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
	}

	thinkos_sleep(250);
	stm32_gpio_clr(IO_LED1);
	stm32_gpio_clr(IO_LED2);

	return 0;
}

int board_configure_task(void *ptr)
{
	__puts("- board configuration\r\n");
	return 0;
}

int board_selftest_task(void *ptr)
{
	__puts("- board self test\r\n");
	return 0;
}

void write_fault(void)
{
	volatile uint32_t * ptr = (uint32_t *)(0x0);
	uint32_t x = 0;
	int i;

	for (i = 0; i < (16 << 4); ++i) {
		*ptr = x;
		ptr += 0x10000000 / (2 << 4);
	}
}



/* ----------------------------------------------------------------------------
 * This function runs as the main thread's task when the bootloader 
 * fails to run the application ... 
 * ----------------------------------------------------------------------------
 */

int board_default_task(void *ptr)
{
	uint32_t tick;

#if 0
	DCC_LOG1(LOG_TRACE, "ptr=0x%08x", ptr);
	__puts("- board default\r\n");
#endif

	for (tick = 0; tick < 10000000; ++tick) {
		thinkos_sleep(1000);

		switch (tick & 0x7) {
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
		case 4:
			stm32_gpio_clr(IO_LED3);
			break;
		case 5:
			stm32_gpio_clr(IO_LED4);
			break;
		case 6:
			stm32_gpio_set(IO_LED3);
			break;
		case 7:
			stm32_gpio_set(IO_LED4);
			break;
		}
	
//		__puts("- tick\r\n");
	}

#if 0
	int x;
	x = thinkos_flash_mem_open("BOOT");
	thinkos_flash_mem_close(x);

	char buf[128];

	__puts("- FLASH test\r\n");
	x = thinkos_flash_mem_open("BOOT");
	thinkos_flash_mem_read(x, 0, buf, 14);
	thinkos_flash_mem_close(x);

	__puts(buf);

	x = thinkos_flash_mem_open("BOOT");
	thinkos_flash_mem_write(x, 0, "Hello world!", 14);
	thinkos_flash_mem_close(x);


	x = thinkos_flash_mem_open("CONF");
	thinkos_flash_mem_write(x, 0, "Hello world!", 16);
	thinkos_flash_mem_read(x, 0, buf, 16);
	thinkos_flash_mem_close(x);

	__puts(buf);

	x = thinkos_flash_mem_open("CONF");
	thinkos_flash_mem_erase(x, 0, 16);
	thinkos_flash_mem_close(x);
#endif
	return 0;
}

const struct monitor_comm * board_comm_init(void)
{
	DCC_LOG(LOG_TRACE, "USB comm init");

	return usb_comm_init(&stm32f_otg_fs_dev);
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
	.application = {
			.tag = "",
			.start_addr = 0x08020000,
			.block_size = (128 * 3) * 1024,
			.magic = &thinkos_10_app_magic},
	.init = board_init,
	.softreset = board_on_softreset,
//	.upgrade = bootloader_yflash,
	.preboot_task = board_preboot_task,
	.configure_task = board_configure_task,
	.selftest_task = board_selftest_task,
	.default_task = board_default_task,
	.monitor_comm_init = board_comm_init,
	.memory = &mem_map
};


void standby_monitor_task(const struct monitor_comm * comm, void * arg);
void boot_monitor_task(const struct monitor_comm * comm, void * arg);

void __attribute((noreturn)) main(int argc, char ** argv)
{
	thinkos_boot(&this_board, &standby_monitor_task);
}


