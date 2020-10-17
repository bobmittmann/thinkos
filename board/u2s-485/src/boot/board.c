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
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <trace.h>
#include <vt100.h>

#include "board.h"
#include "version.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

/* Bootloader board description  
   Bootloader and debugger, memory description  
 */

const struct thinkos_mem_desc sram_desc = {
	.tag = "RAM",
	.base = 0,
	.cnt = 2,
	.blk = {
		{.tag = "BOOT", 0x20000000, M_RO, SZ_1K, 2},	/* Bootloader: 2KiB */
		{.tag = "APP", 0x20000800, M_RW, SZ_1K, 8},	/* Application: 8KiB */
		}
};

const struct thinkos_mem_desc flash_desc = {
	.tag = "FLASH",
	.base = 0x08000000,
	.cnt = 2,
	.blk = {
		{.tag = "BOOT", 0x00000000, M_RO, SZ_2K, 8},	/* Bootloader: 16 KiB */
		{.tag = "APP", 0x00002000, M_RW, SZ_2K, 8},	/* Application: 16 KiB */
		}
};

struct thinkos_mem_map mem_map = {
	.tag = "MEM",
	.cnt = 2,
	.desc = {
		 &flash_desc,
		 &sram_desc
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

extern const struct flash_dev stm32f1x_flash_dev;

const struct thinkos_flash_desc board_flash_desc = {
	.mem = (struct mem_desc *)&flash_desc,
	.dev = &stm32f1x_flash_dev
};


void __puts(const char *s)
{
	int rem = __thinkos_strlen(s, 64);

	while (rem) {
		int n = thinkos_console_write(s, rem);
		s += n;
		rem -= n;
	}
}

void usb_vbus(bool on)
{
	if (on)
		stm32_gpio_mode(USB_FS_VBUS, OUTPUT, PUSH_PULL | SPEED_LOW);
	else
		stm32_gpio_mode(USB_FS_VBUS, INPUT, 0);
}

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

#ifndef SOFTRESET_DISABLE_DMA
#define SOFTRESET_DISABLE_DMA 0
#endif

void board_on_softreset(void)
{
//	struct stm32_rcc *rcc = STM32_RCC;
#if SOFTRESET_DISABLE_DMA
	struct stm32f_dma *dma;
	unsigned int j;
#endif

	DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_BRI "^^^^ Soft Reset ^^^^" VT_POP);

	/* disable all peripherals clock sources except USB_FS, 
	   GPIOA and GPIOB */

#if SOFTRESET_DISABLE_DMA
	dma = STM32F_DMA1;
	for (j = 0; j < 7; ++j) {
		dma->ch[j].ccr = 0;
		while (dma->ch[j].ccr & DMA_EN);
	}
	dma = STM32F_DMA2;
	for (j = 0; j < 7; ++j) {
		dma->ch[j].ccr = 0;
		while (dma->ch[j].ccr & DMA_EN);
	}
#endif
#if 0
	/* Reset all peripherals except USB_FS, GPIOA, GPIOB, FLASH and RTC */
	rcc->ahbrstr = ~((1 << RCC_FLASH) | (1 << RCC_RTC));
	rcc->ahb2rstr = ~((1 << RCC_GPIOA) | (1 << RCC_GPIOB));
	rcc->ahb3rstr = ~(0);
	rcc->apb1rstr1 = ~((1 << RCC_USBFS) | (1 << RCC_PWR));
	rcc->apb1rstr2 = ~(0);
	rcc->apb2rstr = ~(0);

	rcc->ahb1rstr = 0;
	rcc->ahb2rstr = 0;
	rcc->ahb3rstr = 0;
	rcc->apb1rstr1 = 0;
	rcc->apb1rstr2 = 0;
	rcc->apb2rstr = 0;

	/* enable peripherals clock sources for FLASH, RTC, SPI1, USB_FS, 
	   GPIOA and GPIOB */
	rcc->ahb1enr = (1 << RCC_FLASH) | (RCC_RTC);
	rcc->ahb2enr = (1 << RCC_GPIOA) | (1 << RCC_GPIOB);
	rcc->ahb3enr = 0;
	rcc->apb1enr1 = (1 << RCC_USBFS) | (1 << RCC_PWR);
	rcc->apb1enr2 = 0;
	rcc->apb2enr = (1 << RCC_SPI1);
#endif
	/* initialize IO's */
	io_init();

	/* Adjust USB interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_USB_HP, MONITOR_PRIORITY);
	cm3_irq_pri_set(STM32F_IRQ_USB_LP, MONITOR_PRIORITY);
	/* Enable USB interrupt */
	cm3_irq_enable(STM32F_IRQ_USB_HP);
	cm3_irq_enable(STM32F_IRQ_USB_LP);
}

int board_init(void)
{
	board_on_softreset();

#if (THINKOS_FLASH_MEM_MAX > 0)
	thinkos_flash_drv_init(0, &board_flash_desc);
#endif

	return 0;
}

/* ----------------------------------------------------------------------------
 * Preboot: this task runs once at power up only.
 * It's used to delay booting up the application ... 
 * ----------------------------------------------------------------------------
 */

#define PREBOOT_TIME_SEC 5

int board_preboot_task(void *ptr)
{
	uint32_t tick;

	/* Time window autoboot */
	for (tick = 0; tick < 4*PREBOOT_TIME_SEC; ++tick) {
		thinkos_sleep(250);

		__puts(".");

		switch (tick & 0x3) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		}
	}

	thinkos_sleep(250);

	__puts(".\r\n");

	return 0;
}

int board_configure_task(void *ptr)
{
	return 0;
}

int board_selftest_task(void *ptr)
{
	return 0;
}

/* ----------------------------------------------------------------------------
 * This function runs as the main thread's task when the bootloader 
 * fails to run the application ... 
 * ----------------------------------------------------------------------------
 */

int board_default_task(void *ptr)
{
	uint32_t tick;

	for (tick = 0;; ++tick) {
		thinkos_sleep(128);
	}
}

const struct monitor_comm * board_comm_init(void)
{
	DCC_LOG(LOG_TRACE, "USB comm init");

	return usb_comm_init(&stm32f_usb_fs_dev);
}



/* Bootloader board description  */
const struct thinkos_board this_board = {
	.name = "RDA8COM",
	.desc = "Digital Audio Comm Interface",
	.hw = {
	       .tag = "RDA8",
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
			.start_addr = 0x08010000,
			.block_size = (96 * 2) * 1024,
			.magic = &thinkos_10_app_magic},
	.init = board_init,
	.softreset = board_on_softreset,
	.upgrade = NULL,
	.preboot_task = board_preboot_task,
	.configure_task = board_configure_task,
	.selftest_task = board_selftest_task,
	.default_task = board_default_task,
	.monitor_comm_init = board_comm_init,
	.memory = &mem_map
};

int main(int argc, char ** argv)
{
	return thinkos_boot(&this_board);
}

