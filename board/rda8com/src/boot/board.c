/* board.c - bootloader descriptor and hardware initialization
 * -------
 *
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
#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_DEBUG__
#include <thinkos/debug.h>

#include <vt100.h>

#include "board.h"
#include "version.h"

int fpga_configure(void);
void boot_monitor_task(const struct monitor_comm * comm, void * arg,
					   struct thinkos_rt * krn);

void monitor_console_task(const struct monitor_comm * comm, void * arg,
						  struct thinkos_rt * krn);

#ifndef BOOT_ENABLE_JTAG
#define BOOT_ENABLE_JTAG (DEBUG)
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

/* Bootloader board description  
   Bootloader and debugger, memory description  
 */

#define RAM_STACK 0
#define RAM_BOOT  1
#define RAM_APP   2

const struct thinkos_mem_desc sram_desc = {
	.tag = "ram",
	.base = 0x10000000,
	.cnt = 3,
	.blk = {
		/*  CCM - Main Stack */
		[RAM_STACK] = {.tag = "stack", .off = 0x10000000, M_RW, SZ_1K, 16}, 
		/* Bootloader: 4KiB */
		[RAM_BOOT] = {.tag = "boot",  .off = 0x20000000, M_RO, SZ_1K, 4},   
		/* Application: 44KiB */
		[RAM_APP] = {.tag = "app",    .off = 0x20001000, M_RW, SZ_1K, 44},   
	}
};

#define FLASH_BOOT 0
#define FLASH_APP  1

const struct thinkos_mem_desc flash_desc = {
	.tag = "flash",
	.base = 0x08000000,
	.cnt = 2,
	.blk = {
		/* Bootloader: 40 KiB */
		[FLASH_BOOT] = {.tag = "boot", .off= 0x00000000, M_RO, SZ_2K, 20}, 
		/* Application: 220 KiB */
		[FLASH_APP] = {.tag = "app", .off = 0x0000a000, M_RW, SZ_2K, 108}
	}
};

#define MEM_FLASH  0
#define MEM_SRAM   1

const struct thinkos_mem_map board_mem_map = {
	.tag = "MEM",
	.cnt = 2,
	.desc = {
		[MEM_FLASH] = &flash_desc,
		[MEM_SRAM] = &sram_desc
	}
};

const struct thinkos_flash_desc board_flash_desc = {
	.mem = &flash_desc,
	.dev = &stm32l4x_flash_dev
};

#pragma GCC diagnostic pop

static void io_reset(struct stm32_rcc *rcc)
{
	struct stm32_gpio *gpioa = STM32_GPIOA;
	struct stm32_gpio *gpiob = STM32_GPIOB;
#if (HWREV) >= 2
	struct stm32_gpio *gpioh = STM32_GPIOH;
#endif

	/* enable peripherals clock sources for FLASH, RTC, CRC, USB_FS, 
	   GPIOA and GPIOB */
	rcc->ahb1enr = (1 << RCC_FLASH) | (1 << RCC_CRC);
	rcc->ahb2enr = (1 << RCC_GPIOA) | (1 << RCC_GPIOB)
#if (HWREV) >= 2
		| (1 << RCC_GPIOH)
#endif
		;
	rcc->ahb3enr = 0;
	rcc->apb1enr1 = (1 << RCC_USBFS) | (1 << RCC_PWR) | (1 << RCC_RTC);
	rcc->apb1enr2 = 0;
	rcc->apb2enr = (1 << RCC_SPI1);

	/* select alternate functions to USB and SPI1 pins ... */
	/* Port A */
#if (BOOT_ENABLE_JTAG)
	gpioa->afrl = 0;
#else
	gpioa->afrl = GPIO_AFRL_SET(1, GPIO_AF5); /* SPI SCK */
#endif
	gpioa->afrh = GPIO_AFRH_SET(8, GPIO_AF0) /* MCO */
		| GPIO_AFRH_SET(11, GPIO_AF10) | GPIO_AFRH_SET(12, GPIO_AF10) /* USB */
		;

	gpioa->moder = GPIO_MODE_ALT_FUNC(1) /* SPI SCK */
	    | GPIO_MODE_ALT_FUNC(8) /* MCO */
	    | GPIO_MODE_ALT_FUNC(11) | GPIO_MODE_ALT_FUNC(12) /* USB */
#if (BOOT_ENABLE_JTAG)
	    | GPIO_MODE_ALT_FUNC(13) /* TMS*/
#else
	    | GPIO_MODE_OUTPUT(13) /* IO_5 */
#endif
	    | GPIO_MODE_ALT_FUNC(15) /* JTAG */
		| GPIO_MODE_OUTPUT(4) /* CSEL */
	    | GPIO_MODE_OUTPUT(9) /* CRESET */
	    | GPIO_MODE_INPUT(10) /* CDONE */
#if (BOOT_ENABLE_JTAG)
		| GPIO_MODE_ALT_FUNC(14) /* JTAG */
#else
	    | GPIO_MODE_INPUT(14) /* DBG_JMP */
#endif
		;

	gpioa->otyper = GPIO_PUSH_PULL(1) /* SPI SCK */
	    | GPIO_PUSH_PULL(8) /* MCO */
	    | GPIO_PUSH_PULL(11) | GPIO_MODE_OUTPUT(12) /* USB */
#if (BOOT_ENABLE_JTAG)
#else
	    | GPIO_PUSH_PULL(13) /* IO_5 */
#endif
		| GPIO_OPEN_DRAIN(4) /* CSEL */
	    | GPIO_PUSH_PULL(9) /* CRESET */
		;

	gpioa->ospeedr = GPIO_OSPEED_HIGH(1) /* SPI SCK */
	    | GPIO_OSPEED_HIGH(8) /* MCO */
	    | GPIO_OSPEED_HIGH(11) | GPIO_OSPEED_HIGH(12) /* USB */
		| GPIO_OSPEED_HIGH(4) /* CSEL */
	    | GPIO_OSPEED_LOW(9) /* CRESET */
		;

#if (BOOT_ENABLE_JTAG)
#else
	gpioa->pupdr = GPIO_PULL_UP(13)
	    | GPIO_PULL_UP(14) /* DBG_JMP */
	    | GPIO_PULL_UP(10) /* CDONE */
		;
#endif

#if (BOOT_ENABLE_JTAG)
#else
	gpioa->odr = GPIO_SET(4) /* CSEL */
	    | GPIO_SET(9)	/* CRESET */
	    | GPIO_SET(13)	/* IO_5 */
		;
#endif

	/* Port B */
	gpiob->afrl = GPIO_AFRL_SET(5, GPIO_AF5)	/* SPI */
#if (BOOT_ENABLE_JTAG)
#else
		| GPIO_AFRL_SET(4, GPIO_AF5)	/* SPI*/
#endif
		;

	gpiob->afrh = 0;

	gpiob->moder = GPIO_MODE_ALT_FUNC(5) /* SPI */
		| GPIO_MODE_ALT_FUNC(4) /* JTAG / SPI */
	    | GPIO_MODE_ALT_FUNC(3) /* JTAG */
	    | GPIO_MODE_OUTPUT(8) | GPIO_MODE_OUTPUT(12) | GPIO_MODE_OUTPUT(14) /* IO */
		;

	gpiob->otyper = GPIO_PUSH_PULL(5) /* SPI */
#if (BOOT_ENABLE_JTAG)
#else
	    | GPIO_MODE_INPUT(4) /* SPI */
#endif
	    | GPIO_PUSH_PULL(8) | GPIO_PUSH_PULL(12) | GPIO_PUSH_PULL(14)/* IO */
		;

	gpiob->ospeedr = GPIO_OSPEED_HIGH(5) /* SPI */
	    | GPIO_OSPEED_HIGH(3) /* JTAG */
#if (BOOT_ENABLE_JTAG)
#else
		| GPIO_OSPEED_HIGH(4) /* SPI*/
#endif
	    | GPIO_OSPEED_LOW(8) | GPIO_OSPEED_LOW(12) | GPIO_OSPEED_LOW(14); /* IO */

	gpiob->odr = GPIO_SET(8) | GPIO_SET(12) | GPIO_SET(14);	/* IO */

#if (HWREV) >= 2
	/* Port H */
	gpioh->afrl = 0;
	gpioh->afrh = 0;
	gpioh->moder = GPIO_MODE_OUTPUT(3); /* LED */
	gpioh->otyper = GPIO_PUSH_PULL(3);	/* LED */
	gpioh->ospeedr = GPIO_OSPEED_LOW(3); /* LED */
#endif

}

#ifndef SOFTRESET_DISABLE_DMA
#define SOFTRESET_DISABLE_DMA 1
#endif

void board_on_softreset(void)
{
	struct stm32_rcc *rcc = STM32_RCC;
#if (SOFTRESET_DISABLE_DMA)
	struct stm32f_dma *dma;
	unsigned int j;
#endif

	DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_BRI "^^^^ Soft Reset ^^^^" VT_POP);

#if (SOFTRESET_DISABLE_DMA)
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

	/* Reset all peripherals except USB_FS, GPIOA, FLASH, RCC and RTC */
	rcc->ahb1rstr = ~(1 << RCC_FLASH);
	rcc->ahb2rstr = ~((1 << RCC_GPIOA) | (1 << RCC_GPIOB));
	rcc->ahb3rstr = ~(0);
	rcc->apb1rstr1 = ~((1 << RCC_USBFS) | (1 << RCC_PWR) | (1 << RCC_RTC));
	rcc->apb1rstr2 = ~(0);
	rcc->apb2rstr = ~((1 << RCC_SPI1));

	udelay(1024);

	rcc->ahb1rstr = 0;
	rcc->ahb2rstr = 0;
	rcc->ahb3rstr = 0;
	rcc->apb1rstr1 = 0;
	rcc->apb1rstr2 = 0;
	rcc->apb2rstr = 0;

	/* initialize IO's */
	io_reset(rcc);

	/* Adjust USB interrupt priority */
	cm3_irq_pri_set(STM32_IRQ_USB_FS, MONITOR_PRIORITY);
	/* Enable USB interrupt */
	cm3_irq_enable(STM32_IRQ_USB_FS);
}

int board_on_break(const struct monitor_comm * comm)
{
	struct btl_shell_env * env = btl_shell_env_getinstance();

	DCC_LOG(LOG_WARNING, "running btl_console_shell().");

	btl_shell_env_init(env, "\r\n+++\r\nThinkOS\r\n", "boot# ");

	__led_1_on();
//	mdelay(125);

//	__led_1_off();

//	mdelay(125);


//	__io_5_set();

//	__io_1_clr();
//	__io_2_clr();
//	__io_4_clr();

	return monitor_thread_create(comm, C_TASK(btl_console_shell), 
								 C_ARG(env), true);
}

/* Bootloader board description  */
const struct thinkos_board this_board = {
	.name = "QCC6000",
	.desc = "Rio Comm Interface",
	.hw = {
	       .tag = "PB-876",
	       .ver = {
			   .major = 0, 
			   .minor = 3}
	       },
	.sw = {
	       .tag = "ThinkOS",
	       .ver = {
		       .major = VERSION_MAJOR,
		       .minor = VERSION_MINOR,
		       .build = VERSION_BUILD}
	       },
	.memory = &board_mem_map,
	.on_softreset = board_on_softreset,
	.on_break = board_on_break
};

void thinkos_arch_release_get(struct thinkos_release * rel)
{
    __thinkos_memcpy(rel, &this_board.sw, sizeof(struct thinkos_release));
}

/* ----------------------------------------------------------------------------
 * Preboot: this task runs once at power up only.
 * It's used to delay booting up the application ... 
 * ----------------------------------------------------------------------------
 */

#define  PREBOOT_TIME_SEC 10

int board_init(void)
{
	struct stm32_rcc *rcc = STM32_RCC;
	int err;

	io_reset(rcc);

	if ((err = fpga_configure()) < 0) {
		DCC_LOG1(LOG_WARNING, "fpga_configure() error=%d!", err);
	}

	/* Adjust USB interrupt priority */
	cm3_irq_pri_set(STM32_IRQ_USB_FS, MONITOR_PRIORITY);
	/* Enable USB interrupt */
	cm3_irq_enable(STM32_IRQ_USB_FS);

	return err;
}

const char * const normal_argv[] = {
	"app"
};

void main(int argc, char ** argv)
{
	struct btl_shell_env * env = btl_shell_env_getinstance();
	struct thinkos_rt * krn = &thinkos_rt;
	const struct monitor_comm * comm;
	int ret;

#if DEBUG
	DCC_LOG_INIT();
	DCC_LOG_CONNECT();
	mdelay(125);

	DCC_LOG(LOG_TRACE, "\n\n" VT_PSH VT_BRI VT_FGR);
	DCC_LOG(LOG_TRACE, "*************************************************");
	DCC_LOG(LOG_TRACE, "*     RDA8COM ThinkOS Custom Bootloader         *");
	DCC_LOG(LOG_TRACE, "*************************************************"
			VT_POP "\n\n");
	mdelay(125);

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 1. thinkos_krn_init()." VT_POP);
	mdelay(125);
#endif

	ret = thinkos_krn_init(krn, THINKOS_OPT_PRIORITY(1) | THINKOS_OPT_ID(1) |
					 THINKOS_OPT_PRIVILEGED | THINKOS_OPT_STACK_SIZE(32768), 
					 &board_mem_map);
	(void)ret;

	ret = board_init();

	__led_1_on();
	__io_1_set();
	__io_2_set();
	__io_4_set();

	thinkos_krn_flash_drv_init(krn, 0, &board_flash_desc);

	comm = usb_comm_init(&stm32f_usb_fs_dev);

	thinkos_krn_monitor_init(krn, comm, boot_monitor_task, (void *)&this_board);
//	thinkos_krn_monitor_init(krn, comm, monitor_console_task, (void *)&this_board);
	btl_shell_env_init(env, "\r\n+++\r\nThinkOS\r\n", "boot# ");

	thinkos_sleep(1000);

	if (ret < 0) {
		if (thinkos_console_is_connected()) {
			krn_console_puts("\r\nError: ");
			krn_console_put_int(-ret);
		}
	} else { 
		ret = btl_flash_app_exec("app", 1, (uintptr_t)normal_argv);
		DCC_LOG(LOG_WARNING, "btl_flash_app_exec() faied.");
		if (ret < 0) {
			krn_console_puts("\r\nApp error: ");
			krn_console_put_int(-ret);
		}
	}
	DCC_LOG(LOG_WARNING, VT_PSH VT_FYW " /!\\ Abort /!\\ " VT_POP);

	btl_console_shell(env);
}

