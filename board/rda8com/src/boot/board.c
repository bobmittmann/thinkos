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

/* ----------------------------------------------------------------------------
 * Memory map
 * ----------------------------------------------------------------------------
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"


#define RAM_BLK_STACK 0
#define RAM_BLK_BOOT  1
#define RAM_BLK_APP   2

const struct thinkos_mem_desc sram_desc = {
	.tag = "RAM",
	.base = 0,
	.cnt = 3,
	.blk = {
		{.tag = "STACK", 0x10000000, M_RW, SZ_1K, 16}, /*  CCM - Main Stack */
		{.tag = "BOOT", 0x20000000, M_RO, SZ_1K, 4},   /* Bootloader: 4KiB */
		{.tag = "APP", 0x20001000, M_RW, SZ_1K, 44},   /* Application: 44KiB */
		}
};

#define FLASH_BLK_BOOT 0
#define FLASH_BLK_APP  1

const struct thinkos_mem_desc flash_desc = {
	.tag = "FLASH",
	.base = 0x08000000,
	.cnt = 2,
	.blk = {
		/* Bootloader: 64 KiB */
		{.tag = "BOOT", .off= 0x00000000, M_RO, SZ_2K, 32}, 
		/* Application: 192 KiB */
		{.tag = "APP", .off = 0x00010000, M_RW, SZ_2K, 96},  
		}
};

const struct thinkos_mem_desc peripheral_desc = {
	.tag = "PERIPH",
	.base = 0,
	.cnt = 1,
	.blk = {
		{.tag = "RTC", 0x40002800, M_RW, SZ_1K, 1}	/* RTC - 1K */
		}
};

struct thinkos_mem_map mem_map = {
	.tag = "MEM",
	.cnt = 3,
	.desc = {
		 &flash_desc,
		 &sram_desc,
		 &peripheral_desc
	}
};

#pragma GCC diagnostic pop

extern const struct flash_dev stm32l4x_flash_dev;

const struct thinkos_flash_desc board_flash_desc = {
	.mem = (struct mem_desc *)&flash_desc,
	.dev = &stm32l4x_flash_dev
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

void io_init(void)
{
	struct stm32_gpio *gpioa = STM32_GPIOA;
	struct stm32_gpio *gpiob = STM32_GPIOB;

	/* select alternate functions to USB and SPI1 pins ... */
	/* Port A */
	gpioa->afrl = GPIO_AFRL_SET(1, GPIO_AF5); /* SPI SCK */
	gpioa->afrh = GPIO_AFRH_SET(8, GPIO_AF0) |	/* MCO */
	    GPIO_AFRH_SET(11, GPIO_AF10) | GPIO_AFRH_SET(12, GPIO_AF10) | /* USB */
	    GPIO_AFRH_SET(13, GPIO_AF0) | GPIO_AFRH_SET(14, GPIO_AF0) |	/* JTAG */
	    GPIO_AFRH_SET(15, GPIO_AF0);	/* JTAG */

	gpioa->moder = GPIO_MODE_ALT_FUNC(1) /* SPI SCK */
	    | GPIO_MODE_ALT_FUNC(8) /* MCO */
	    | GPIO_MODE_ALT_FUNC(11)| GPIO_MODE_ALT_FUNC(12) /* USB */
	    | GPIO_MODE_ALT_FUNC(13) | GPIO_MODE_ALT_FUNC(14) /* JTAG */
	    | GPIO_MODE_ALT_FUNC(15)	/* JTAG */
#if (HWREV > 0)
		| GPIO_MODE_OUTPUT(4) /* CSEL */
		| GPIO_MODE_OUTPUT(9) /* CRESET */
        | GPIO_MODE_INPUT (10) /* CDONE */
#endif
		;

	gpioa->otyper = GPIO_PUSH_PULL(1) /* SPI */
	    | GPIO_PUSH_PULL(8) /* MCO */
	    | GPIO_PUSH_PULL(11) | GPIO_MODE_OUTPUT(12) /* USB */
#if (HWREV > 0)
        | GPIO_PUSH_PULL(4) /* CSEL */
		| GPIO_PUSH_PULL(9) /* CRESET */
#endif
	;

	gpioa->ospeedr = GPIO_OSPEED_HIGH(1) /* SPI */
	    | GPIO_OSPEED_HIGH(8) /* MCO */
	    | GPIO_OSPEED_HIGH(11) | GPIO_OSPEED_HIGH(12) /* USB */
#if (HWREV > 0)
        | GPIO_OSPEED_LOW(4) /* CSEL */
		| GPIO_OSPEED_LOW(9) /* CRESET */
#endif
	;

	gpioa->pupdr = GPIO_PULL_UP(13) | GPIO_PULL_DOWN(14) /* JTAG */
	    | GPIO_PULL_UP(15) /* JTAG */
#if (HWREV > 0)
		| GPIO_PULL_UP(10) /* CDONE */
#endif
	;

	gpioa->odr = 0
#if (HWREV > 0)
		| GPIO_SET(4) /* CSEL */
		| GPIO_SET(9) /* CRESET */
#endif
	;

	/* Port B */
	gpiob->afrl = GPIO_AFRL_SET(5, GPIO_AF5) | /* SPI MOSI */
	    GPIO_AFRL_SET(3, GPIO_AF0) | GPIO_AFRL_SET(4, GPIO_AF0); /* JTAG */
	gpiob->afrh = 0;

	gpiob->moder = GPIO_MODE_ALT_FUNC(5) /* SPI MOSI */
		| GPIO_MODE_ALT_FUNC(3) | GPIO_MODE_ALT_FUNC(4) /* JTAG */
#if (HWREV == 0)
        | GPIO_MODE_INPUT (7) /* CDONE */
		| GPIO_MODE_OUTPUT(8) /* CSEL */
		| GPIO_MODE_OUTPUT(9) /* CRESET */
#endif
	;

	gpiob->otyper = GPIO_PUSH_PULL(5) /* SPI MOSI */
#if (HWREV == 0)
        | GPIO_PUSH_PULL(8) /* CSEL */
		| GPIO_PUSH_PULL(9) /* CRESET */
#endif
	;

	gpiob->ospeedr = GPIO_OSPEED_HIGH(5) /* SPI MOSI */
	    | GPIO_OSPEED_HIGH(3) | GPIO_OSPEED_HIGH(4) /* JTAG */
#if (HWREV == 0)
        | GPIO_OSPEED_LOW(8) /* CSEL */
		| GPIO_OSPEED_LOW(9) /* CRESET */
#endif
	;

	gpiob->pupdr = GPIO_PULL_UP(4) /* JTAG */
#if (HWREV == 0)
		| GPIO_PULL_UP(7)	/* CDONE */
#endif
	;

	gpiob->odr = 0
#if (HWREV == 0)
		| GPIO_SET(8) /* CSEL */
		| GPIO_SET(9) /* CRESET */
#endif
	;
}

void io_init_dbg(void)
{
	struct stm32_gpio *gpioa = STM32_GPIOA;
//	struct stm32_gpio *gpiob = STM32_GPIOB;

	gpioa->afrh = GPIO_AFRH_SET(8, GPIO_AF0)
		| GPIO_AFRH_SET(11, GPIO_AF10) | GPIO_AFRH_SET(12, GPIO_AF10); /* USB */

	gpioa->moder = GPIO_MODE_ALT_FUNC(8) /* MCO */
		| GPIO_MODE_ALT_FUNC(11)| GPIO_MODE_ALT_FUNC(12) /* USB */
	    | GPIO_MODE_ALT_FUNC(13) | GPIO_MODE_ALT_FUNC(14) /* JTAG */
	    | GPIO_MODE_ALT_FUNC(15);	/* JTAG */

	gpioa->otyper = GPIO_PUSH_PULL(8) /* MCO */
		| GPIO_PUSH_PULL(11) | GPIO_PUSH_PULL(12); /* USB */

	gpioa->ospeedr = GPIO_OSPEED_HIGH(8) /* MCO */
		| GPIO_OSPEED_HIGH(11) | GPIO_OSPEED_HIGH(12); /* USB */
}

#ifndef SOFTRESET_DISABLE_DMA
#define SOFTRESET_DISABLE_DMA 0
#endif

void board_on_softreset(void)
{
	struct stm32_rcc *rcc = STM32_RCC;
#if SOFTRESET_DISABLE_DMA
	struct stm32f_dma *dma;
	unsigned int j;
#endif

	DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_BRI "^^^^ Soft Reset ^^^^" VT_POP);

	/* disable all peripherals clock sources except USB_FS, 
	   GPIOA and GPIOB */
	DCC_LOG1(LOG_TRACE, "ahb1enr =0x%08x", rcc->ahb1enr);
	DCC_LOG1(LOG_TRACE, "ahb2enr =0x%08x", rcc->ahb2enr);
	DCC_LOG1(LOG_TRACE, "ahb3enr =0x%08x", rcc->ahb3enr);
	DCC_LOG1(LOG_TRACE, "apb1enr1=0x%08x", rcc->apb1enr1);
	DCC_LOG1(LOG_TRACE, "apb1enr2=0x%08x", rcc->apb1enr2);
	DCC_LOG1(LOG_TRACE, "apb2enr =0x%08x", rcc->apb2enr);

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

#if 1
	/* Reset all peripherals except USB_FS, GPIOA, GPIOB, FLASH and RTC */
	rcc->ahb1rstr = ~((1 << RCC_FLASH) | (1 << RCC_RTC));
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
#endif

	/* enable peripherals clock sources for FLASH, RTC, SPI1, USB_FS, 
	   GPIOA and GPIOB */
	rcc->ahb1enr = (1 << RCC_FLASH) | (RCC_RTC);
	rcc->ahb2enr = (1 << RCC_GPIOA) | (1 << RCC_GPIOB);
	rcc->ahb3enr = 0;
	rcc->apb1enr1 = (1 << RCC_USBFS) | (1 << RCC_PWR);
	rcc->apb1enr2 = 0;
	rcc->apb2enr = (1 << RCC_SPI1);

	/* initialize IO's */
	io_init_dbg();

	/* Adjust USB interrupt priority */
	cm3_irq_pri_set(STM32_IRQ_USB_FS, MONITOR_PRIORITY);
	/* Enable USB interrupt */
	cm3_irq_enable(STM32_IRQ_USB_FS);
}

int fpga_configure(void);

int board_init(void)
{

	board_on_softreset();

	DCC_LOG(LOG_TRACE, "Configuring FPGA...");
#if 0
	int ret;

	if ((ret = fpga_configure()) < 0) {
		DCC_LOG1(LOG_ERROR, VT_PSH VT_FRD VT_BRI 
				 "FPGA configuration error %d!" VT_POP, ret);
		return ret;
	}
#endif

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

int oard_default_task(void *ptr)
{
	return 0;
}

bool board_integrity_check(void)
{
	uint32_t tick;

	for (tick = 0; tick < 10; ++tick) {
		thinkos_sleep(500);
	}

	return true;
}

/* ----------------------------------------------------------------------------
 * This function runs as the main thread's task when the bootloader 
 * fails to run the application ... 
 * ----------------------------------------------------------------------------
 */

void __attribute__((noreturn)) board_default_task(void *ptr)
{
	uint32_t tick;

	DCC_LOG(LOG_WARNING, "Default task started...");

	for (tick = 0;; ++tick) {
		thinkos_sleep(128);
	}
}

const struct monitor_comm * board_comm_init(void)
{
	DCC_LOG(LOG_TRACE, "USB comm init");

	return usb_comm_init(&stm32f_usb_fs_dev);
}

/* Bootloader board description  
   Bootloader and debugger, memory description  
 */

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
	.init = board_init,
	.softreset = board_on_softreset,
	.memory = &mem_map
};

void boot_monitor_task(const struct monitor_comm * comm, void * arg);

void __attribute__((noreturn)) main(int argc, char ** argv)
{
	const struct monitor_comm * comm;
	uintptr_t app_addr;

	DCC_LOG_INIT();
	DCC_LOG_CONNECT();

#if DEBUG
	mdelay(125);
#endif
	DCC_LOG(LOG_TRACE, "\n\n" VT_PSH VT_BRI VT_FGR);
	DCC_LOG(LOG_TRACE, "*************************************************");
	DCC_LOG(LOG_TRACE, "*     RDA8COM ThinkOS Custom Bootloader         *");
	DCC_LOG(LOG_TRACE, "*************************************************"
			VT_POP "\n\n");
#if DEBUG
	mdelay(125);
#endif

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 1. thinkos_krn_init()." VT_POP);
	thinkos_krn_init(THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED |
					 THINKOS_OPT_STACK_SIZE(32768), NULL, NULL);

#if DEBUG
	mdelay(125);
#endif
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 2. board_init()." VT_POP);
	board_init();

#if DEBUG
	mdelay(125);
#endif
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 3. usb_comm_init()." VT_POP);
	comm = usb_comm_init(&stm32f_usb_fs_dev);

#if DEBUG
	mdelay(125);
#endif
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 4. thinkos_krn_monitor_init()." VT_POP);
	thinkos_krn_monitor_init(comm, boot_monitor_task, (void *)&this_board);

#if DEBUG
	mdelay(125);
#endif
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 5. thinkos_krn_irq_on()." VT_POP);
	/* enable interrupts */
	thinkos_krn_irq_on();

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 6. board_integrity_check()..." VT_POP);
	if (!board_integrity_check()) {
		DCC_LOG(LOG_ERROR, VT_PSH VT_BRI VT_FRD
				"**** board_integrity_check() failed." VT_POP);
#if DEBUG
		mdelay(10000);
#endif
		thinkos_abort();
	}

	app_addr = flash_desc.base + flash_desc.blk[FLASH_BLK_APP].off;  

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 7. thinkos_app_exec()..." VT_POP);
	thinkos_app_exec(app_addr);
	DCC_LOG(LOG_ERROR, VT_PSH VT_BRI VT_FRD
			"**** thinkos_app_exec() failed." VT_POP);
#if DEBUG
	mdelay(10000);
#endif

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 8. board_default_task()..." VT_POP);
	board_default_task((void*)&this_board);
}

