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
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <trace.h>
#include <vt100.h>

#include "board.h"
#include "version.h"

int fpga_configure(void);
void boot_monitor_task(const struct monitor_comm * comm, void * arg);
int boot_console_shell(const char * msg, const char * prompt);
int flash_app_exec(const char * tag);

/* The terminal emulator may take some time to process the reset */
static const char vt_reset[] = VT100_RESET;

int __console_shell_task(void * parm)
{
	return boot_console_shell("\r\nQCC6000 bootloader\r\n", 
						   "boot# ");
}

#ifndef BOOT_ENABLE_JTAG
#define BOOT_ENABLE_JTAG DEBUG
#endif

static void __led1_on(void)
{
#if (HWREV == 2)
	stm32_gpio_set(IO_3);
#endif
}

static void __led1_off(void)
{
#if (HWREV == 2)
	stm32_gpio_clr(IO_3);
#endif
}

static bool __dbg_jmp(void)
{
#if (BOOT_ENABLE_JTAG)
	return true;
#else
  #if (HWREV == 2)
	return stm32_gpio_stat(IO_DBG_JMP) ? false : true;
  #else
	return true;
  #endif
#endif
}


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
	.base = 0x00000000,
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
#define FLASH_DIAG 1
#define FLASH_APP  2

const struct thinkos_mem_desc flash_desc = {
	.tag = "flash",
	.base = 0x08000000,
	.cnt = 3,
	.blk = {
		/* Bootloader: 40 KiB */
		[FLASH_BOOT] = {.tag = "boot", .off= 0x00000000, M_RO, SZ_2K, 20}, 
		/* Alternate application: 24 KiB */
		[FLASH_DIAG] = {.tag = "diag", .off = 0x0000a000, M_RW, SZ_2K, 12},  
		/* Application: 192 KiB */
		[FLASH_APP] = {.tag = "app", .off = 0x00010000, M_RW, SZ_2K, 96},  
	}
};

#define PERIPH_RTC 0

const struct thinkos_mem_desc peripheral_desc = {
	.tag = "periph",
	.base = 0x40000000,
	.cnt = 1,
	.blk = {
		/* RTC - 1K */
		[PERIPH_RTC]    =  {.tag = "rtc",    .off = 0x00002800, M_RW, SZ_1K, 1}
	}
};

#define MEM_FLASH  0
#define MEM_SRAM   1
#define MEM_PERIPH 2

const struct thinkos_mem_map mem_map = {
	.tag = "MEM",
	.cnt = 3,
	.desc = {
		[MEM_FLASH] = &flash_desc,
		[MEM_SRAM] = &sram_desc,
		[MEM_PERIPH] = &peripheral_desc
	}
};

const struct thinkos_flash_desc board_flash_desc = {
	.mem = &flash_desc,
	.dev = &stm32l4x_flash_dev
};

#pragma GCC diagnostic pop

void io_debug(void)
{
	struct stm32_gpio *gpioa = STM32_GPIOA;

	gpioa->afrh = GPIO_AFRH_SET(8, GPIO_AF0) /* SPI SCK */
		| GPIO_AFRH_SET(11, GPIO_AF10) | GPIO_AFRH_SET(12, GPIO_AF10) /* USB */
		| GPIO_AFRH_SET(13, GPIO_AF0) | GPIO_AFRH_SET(14, GPIO_AF0) /* JTAG */
	    | GPIO_AFRH_SET(15, GPIO_AF0) /* JTAG */
		;

	gpioa->moder = GPIO_MODE_ALT_FUNC(1) /* SPI SCK */
		| GPIO_MODE_ALT_FUNC(8) /* MCO */
		| GPIO_MODE_ALT_FUNC(11) | GPIO_MODE_ALT_FUNC(12) /* USB */
	    | GPIO_MODE_ALT_FUNC(13) | GPIO_MODE_ALT_FUNC(14) /* JTAG */
	    | GPIO_MODE_ALT_FUNC(15)	/* JTAG */
		| GPIO_MODE_OUTPUT(4) /* CSEL */
	    | GPIO_MODE_OUTPUT(9) /* CRESET */
	    | GPIO_MODE_INPUT(10) /* CDONE */
		;

	gpioa->otyper = GPIO_PUSH_PULL(1) /* SPI SCK */
	    | GPIO_PUSH_PULL(8) /* MCO */
	    | GPIO_PUSH_PULL(11) | GPIO_MODE_OUTPUT(12) /* USB */
		| GPIO_OPEN_DRAIN(4) /* CSEL */
	    | GPIO_PUSH_PULL(9) /* CRESET */
		;

	gpioa->ospeedr = GPIO_OSPEED_HIGH(1) /* SPI SCK */
	    | GPIO_OSPEED_HIGH(8) /* MCO */
	    | GPIO_OSPEED_HIGH(11) | GPIO_OSPEED_HIGH(12) /* USB */
		| GPIO_OSPEED_HIGH(4) /* CSEL */
	    | GPIO_OSPEED_LOW(9) /* CRESET */
		;

	gpioa->pupdr = GPIO_PULL_UP(13) /* JTAG */
	    | GPIO_PULL_UP(15)          /* JTAG */
		| GPIO_PULL_DOWN(14)        /* JTAG */
	    | GPIO_PULL_UP(10)          /* CDONE */
		;

	gpioa->odr = GPIO_SET(4)   /* CSEL */
	    | GPIO_SET(9)          /* CRESET */
		;
}

static void io_reset(struct stm32_rcc *rcc)
{
	struct stm32_gpio *gpioa = STM32_GPIOA;
	struct stm32_gpio *gpiob = STM32_GPIOB;
#if (HWREV) == 2
	struct stm32_gpio *gpioh = STM32_GPIOH;
#endif

	/* enable peripherals clock sources for FLASH, RTC, CRC, USB_FS, 
	   GPIOA and GPIOB */
	rcc->ahb1enr = (1 << RCC_FLASH) | (1 << RCC_CRC);
	rcc->ahb2enr = (1 << RCC_GPIOA) | (1 << RCC_GPIOB)
#if (HWREV) == 2
		| (1 << RCC_GPIOH)
#endif
		;
	rcc->ahb3enr = 0;
	rcc->apb1enr1 = (1 << RCC_USBFS) | (1 << RCC_PWR) | (1 << RCC_RTC);
	rcc->apb1enr2 = 0;
	rcc->apb2enr = (1 << RCC_SPI1);

#if 0 
	gpiob->afrl = 0;
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


	/* Port B */
	gpiob->afrh = 0;
	/* Port B */
	gpiob->afrl = GPIO_AFRL_SET(5, GPIO_AF5);	/* SPI */

	gpiob->moder = GPIO_MODE_ALT_FUNC(3) /* JTAG */
	    | GPIO_MODE_ALT_FUNC(4) /* JTAG */
		| GPIO_MODE_ALT_FUNC(5)
	    | GPIO_MODE_OUTPUT(8) | GPIO_MODE_OUTPUT(12) /* IO */
		;

	gpiob->otyper = GPIO_PUSH_PULL(5) | GPIO_PUSH_PULL(8) | 
		GPIO_PUSH_PULL(12); /* IO */

	gpiob->ospeedr = GPIO_OSPEED_HIGH(3) | /* JTAG  */
		GPIO_OSPEED_HIGH(5) |
		GPIO_OSPEED_LOW(8) | GPIO_OSPEED_LOW(12); /* IO */

	gpiob->odr = GPIO_SET(8) | GPIO_SET(12);	/* IO */


#else

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
	    | GPIO_MODE_ALT_FUNC(13) 
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
		| GPIO_OPEN_DRAIN(4) /* CSEL */
	    | GPIO_PUSH_PULL(9) /* CRESET */
		;

	gpioa->ospeedr = GPIO_OSPEED_HIGH(1) 
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
	    | GPIO_MODE_OUTPUT(8) | GPIO_MODE_OUTPUT(12) /* IO */
		;

	gpiob->otyper = GPIO_PUSH_PULL(5) /* SPI */
#if (BOOT_ENABLE_JTAG)
#else
	    | GPIO_MODE_INPUT(4) /* SPI */
#endif
	    | GPIO_PUSH_PULL(8) | GPIO_PUSH_PULL(12) /* IO */
		;

	gpiob->ospeedr = GPIO_OSPEED_HIGH(5) /* SPI */
	    | GPIO_OSPEED_HIGH(3) /* JTAG */
#if (BOOT_ENABLE_JTAG)
#else
		| GPIO_OSPEED_HIGH(4) /* SPI*/
#endif
	    | GPIO_OSPEED_LOW(8) | GPIO_OSPEED_LOW(12);	/* IO */

	gpiob->odr = GPIO_SET(8) | GPIO_SET(12);	/* IO */

#endif /* DEBUG */

#if (HWREV) == 2
	/* Port H */
	gpioh->afrl = 0;
	gpioh->afrh = 0;
	gpioh->moder = GPIO_MODE_OUTPUT(3); /* LED */
	gpioh->otyper = GPIO_PUSH_PULL(3);	/* LED */
	gpioh->ospeedr = GPIO_OSPEED_LOW(3); /* LED */
#endif

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
	DCC_LOG1(LOG_INFO, "ahb1enr =0x%08x", rcc->ahb1enr);
	DCC_LOG1(LOG_INFO, "ahb2enr =0x%08x", rcc->ahb2enr);
	DCC_LOG1(LOG_INFO, "ahb3enr =0x%08x", rcc->ahb3enr);
	DCC_LOG1(LOG_INFO, "apb1enr1=0x%08x", rcc->apb1enr1);
	DCC_LOG1(LOG_INFO, "apb1enr2=0x%08x", rcc->apb1enr2);
	DCC_LOG1(LOG_INFO, "apb2enr =0x%08x", rcc->apb2enr);

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
	/* Reset all peripherals except USB_FS, GPIOA, FLASH, RCC and RTC */
	rcc->ahb1rstr = ~(1 << RCC_FLASH);
	rcc->ahb2rstr = ~((1 << RCC_GPIOA) | (1 << RCC_GPIOB));
	rcc->ahb3rstr = ~(0);
	rcc->apb1rstr1 = ~((1 << RCC_USBFS) | (1 << RCC_PWR) | (1 << RCC_RTC));
	rcc->apb1rstr2 = ~(0);
	rcc->apb2rstr = ~((1 << RCC_SPI1));

	udelay(8192);

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
	return monitor_thread_create(comm, C_TASK(__console_shell_task), 
								 C_ARG(NULL), true);
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
	.memory = &mem_map,
	.app = { 
		.mem = MEM_FLASH, 
		.blk = FLASH_APP 
	},
	.softreset = board_on_softreset,
	.on_break = board_on_break
};



/* ----------------------------------------------------------------------------
 * Preboot: this task runs once at power up only.
 * It's used to delay booting up the application ... 
 * ----------------------------------------------------------------------------
 */

#define  PREBOOT_TIME_SEC 5

bool board_integrity_check(void)
{
	uint32_t clk;
	uint32_t tick;
	uint32_t tmo;
	uint32_t itv;
	bool term = false;

	DCC_LOG(LOG_TRACE, VT_PSH VT_FMG VT_BRI "Preboot start..." VT_POP);

	clk = thinkos_clock();
	/* Synchronize the clock rate */
	itv = 1000 - (clk % 1000);
	tmo = clk + (PREBOOT_TIME_SEC * 1000);

	/* Time window autoboot */
	for (tick = 0; (int32_t)(clk - tmo) < 0; ++tick) {
		clk += itv;
		thinkos_alarm(clk);

		if (__dbg_jmp()) {
			itv = 250;
		} else {
			itv = 125;
		} 

		switch (tick & 0x3) {
		case 0:
			__led1_on();
			break;
		case 1:
			break;
		case 2:
			__led1_off();
			break;
		case 3:
			break;
		}
		
		if (thinkos_console_is_connected()) {
			int c;

			if (!term ) {
				term = true;
				krn_console_puts(vt_reset);
				thinkos_sleep(100);
				krn_console_puts("Press Esc to abort ..");
			}
			if ((c = krn_console_getc(0)) == '\033') {
				krn_console_puts("\r\n");
				return false;
			}
			krn_console_putc('.');
		}
	}

	return true;
}

void _exit(void)
{
	thinkos_abort();
}

int board_init(void)
{
	struct stm32_rcc *rcc = STM32_RCC;

	io_reset(rcc);

	return 0;
}

void main(int argc, char ** argv)
{
	struct thinkos_rt * krn = &thinkos_rt;
	const struct monitor_comm * comm;

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


	if (thinkos_krn_init(krn, THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
					 THINKOS_OPT_PRIVILEGED | THINKOS_OPT_STACK_SIZE(32768), 
					 &mem_map) < 0) {
		DCC_LOG(LOG_ERROR, 	
				VT_PSH VT_BRI VT_FRD "thinkos_krn_init() failed!" VT_POP);
		for(;;);
	}

#if !(THINKOS_ENABLE_PRIVILEGED_THREAD)
	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 5. thinkos_krn_irq_on()." VT_POP);
	/* enable interrupts */
	thinkos_krn_irq_on();

	/* Wait a bit for the hardware to initialize ... */
	thinkos_sleep(125);
#endif

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR "* 2. board_init()." VT_POP);
	board_init();

	fpga_configure();

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 3. thinkos_flash_drv_init()." VT_POP);
	thinkos_flash_drv_init(0, &board_flash_desc);

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 3. usb_comm_init()." VT_POP);
	comm = usb_comm_init(&stm32f_usb_fs_dev);

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 4. thinkos_krn_monitor_init()." VT_POP);
	thinkos_krn_monitor_init(comm, boot_monitor_task, (void *)&this_board);

	DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
			"* 6. board_integrity_check()..." VT_POP);

	if (board_integrity_check()) {
		DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR 
				 "* 7. thinkos_app_exec(\"app\")..." VT_POP);
		flash_app_exec("app");
		DCC_LOG(LOG_ERROR, VT_PSH VT_BRI VT_FRD
				"**** thinkos_app_exec() failed." VT_POP);
	}
}

