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
#define __THINKOS_FLASH__
#include <thinkos/flash.h>
#define __THINKOS_DEBUG__
#include <thinkos/debug.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <trace.h>
#include <vt100.h>

#include "board.h"
#include "version.h"

#ifndef COMM_NTXEN_ISR_ENABLE
#define COMM_NTXEN_ISR_ENABLE 0
#endif

#ifndef COMM_NSEL_ISR_ENABLE
#define COMM_NSEL_ISR_ENABLE 0
#endif

#ifndef BOARD_TRACE_ENABLE
#define BOARD_TRACE_ENABLE 1
#endif

#define FLASH_BLK_BOOT 0
#define FLASH_BLK_APP  1

#define RAM_BLK_STACK 0
#define RAM_BLK_BOOT  1
#define RAM_BLK_APP   2

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

const struct thinkos_mem_desc flash_desc = {
	.tag = "FLASH",
	.base = 0x08000000,
	.cnt = 3,
	.blk = {
		/* Bootloader    :  64KiB */
		{.tag = "BOOT", .off = 0x00000000, M_RO, SZ_16K, 4},
		/* Configuration :  64KiB */
		{.tag = "CFG", .off = 0x00010000, M_RW, SZ_64K, 1},	
		/* Application   : 384KiB */
		{.tag = "APP", .off = 0x00020000, M_RW, SZ_128K, 3}
	}
};

const struct thinkos_mem_desc sram_desc = {
	.tag = "RAM",
	.base = 0,
	.cnt =  3,
	.blk = {
		{.tag = "BOOT", 0x20000000, M_RO, SZ_4K, 1}, /* Bootloader: 4KiB */
		{.tag = "APP", 0x20001000, M_RW, SZ_4K, 27}, /* Application: 108KiB */
		{.tag = "STACK", 0x2001c000, M_RW, SZ_16K, 1},	/* SRAM 2: 16KiB */
	}
};

const struct thinkos_mem_desc peripheral_desc = {
	.tag = "PERIPH",
	.base = 0,
	.cnt = 2,
	.blk = {
		/* RTC - 1K */
		{.tag = "RTC", 0x40002800, M_RW, SZ_1K, 1},	
		/* Backup SRAM - 4K */
		{.tag = "BACKUP", 0x40024000, M_RW, SZ_1K, 4},	
	}
};

const struct thinkos_mem_map mem_map = {
	.tag = "SOC",
	.cnt = 3,
	.desc = {
		 &flash_desc,
		 &sram_desc,
		 &peripheral_desc
	}
};

extern const struct flash_dev stm32f4x_flash_dev;

const struct thinkos_flash_desc board_flash_desc = {
	.mem = (struct thinkos_mem_desc *)&flash_desc,
	.dev = &stm32f4x_flash_dev
};

#pragma GCC diagnostic pop

void __puts(const char *s)
{
	int rem = __thinkos_strlen(s, 64);

	while (rem) {
		int n = thinkos_console_write(s, rem);
		s += n;
		rem -= n;
	}
}

#if 0
/* 32 bits aligned access only */
uint32_t crc32(void *buf, unsigned int len)
{
	struct stm32_crc *crc = STM32_CRC;
	uint32_t *src = (uint32_t *) buf;
	uint32_t *mrk = src + (len >> 2);

	crc->cr = CRC_RESET;
	while (src != mrk)
		crc->dr = *src++;

	if (len & 3)
		crc->dr = *src & (0xffffffff >> ((4 - (len & 3)) * 8));

	return crc->dr;
}
#endif

/* 
 * This is the low level board support for the Rio amplifier with
 * 4 channels at 25W.
 *
 */

void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOD);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOE);
	/* SYSCFG - System configuration controller clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_SYSCFG);

#ifndef PB_874_R01
	/* - Booster power ----------------------------------------------------- */
	stm32_gpio_clr(IO_PWON);
	stm32_gpio_mode(IO_PWON, OUTPUT, PUSH_PULL | SPEED_LOW);
#endif

	/* - Amplifier power --------------------------------------------------- */
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

	/* - Clock and backup SRAM --------------------------------------------- */
	/* 1. Enable the power interface clock by setting the PWREN bits in 
	   the RCC APB1 peripheral clock enable register (RCC_APB1ENR) */
	stm32_clk_enable(STM32_RCC, STM32_CLK_PWR);
	/* 2. Set the DBP bit in the PWR power control register (PWR_CR) 
	   to enable access to the backup domain */
	STM32_PWR->cr |= PWR_DBP;
	/* 3. Enable the backup SRAM clock by setting BKPSRAMEN bit in the RCC 
	   AHB1 peripheral clock register (RCC_AHB1ENR) */
	stm32_clk_enable(STM32_RCC, STM32_CLK_BKPSRAM);
	/* 4. Enable the backup regulator */
	STM32_PWR->csr |= PWR_BRE;

	/* - CRC --------------------------------------------------------------- */
	stm32_clk_enable(STM32_RCC, STM32_CLK_CRC);

	/* - USB OTG FS -------------------------------------------------------- */
	stm32_gpio_af(OTG_FS_DP, GPIO_AF10);
	stm32_gpio_af(OTG_FS_DM, GPIO_AF10);

	stm32_gpio_mode(OTG_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_VBUS, INPUT, SPEED_LOW);

	/* COMM (Remote Operation) ----------------------------------------- */
	stm32_gpio_mode(IO_COMM_TX, OUTPUT, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_set(IO_COMM_TX);
	stm32_gpio_mode(IO_COMM_RX, INPUT, SPEED_HIGH);
	stm32_gpio_set(IO_COMM_RX);

	stm32_gpio_set(IO_COMM_NTXEN);
	stm32_gpio_mode(IO_COMM_NTXEN, INPUT, PULL_UP | SPEED_HIGH);
	stm32_gpio_set(IO_COMM_NSEL);
	stm32_gpio_mode(IO_COMM_NSEL, INPUT, PULL_UP | SPEED_HIGH);
#if 0
	stm32_gpio_af(IO_COMM_TX, GPIO_AF7);
	stm32_gpio_mode(IO_COMM_TX, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_af(IO_COMM_RX, GPIO_AF7);
	stm32_gpio_mode(IO_COMM_RX, ALT_FUNC, SPEED_HIGH);

	/* Raise interrupt on falling edge */
	stm32f_exti_init(IO_COMM_NTXEN, 0);
	stm32f_exti_init(IO_COMM_NSEL, 0);
#endif
	/* - LEDs -------------------------------------------------------------- */
	stm32_gpio_clr(IO_LED1A);
	stm32_gpio_mode(IO_LED1A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1B);
	stm32_gpio_mode(IO_LED1B, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1C);
	stm32_gpio_mode(IO_LED1C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED1D);
	stm32_gpio_mode(IO_LED1D, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED2A);
	stm32_gpio_mode(IO_LED2A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED2B);
	stm32_gpio_mode(IO_LED2B, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED2C);
	stm32_gpio_mode(IO_LED2C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED2D);
	stm32_gpio_mode(IO_LED2D, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED3A);
	stm32_gpio_mode(IO_LED3A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED3B);
	stm32_gpio_mode(IO_LED3B, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED3C);
	stm32_gpio_mode(IO_LED3C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED3D);
	stm32_gpio_mode(IO_LED3D, OUTPUT, PUSH_PULL | SPEED_LOW);

	stm32_gpio_clr(IO_LED4A);
	stm32_gpio_mode(IO_LED4A, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED4B);
	stm32_gpio_mode(IO_LED4B, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED4C);
	stm32_gpio_mode(IO_LED4C, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LED4D);
	stm32_gpio_mode(IO_LED4D, OUTPUT, PUSH_PULL | SPEED_LOW);

#ifndef PB_874_R01
	stm32_gpio_clr(IO_LEDTB1);
	stm32_gpio_mode(IO_LEDTB1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LEDTB2);
	stm32_gpio_mode(IO_LEDTB2, OUTPUT, PUSH_PULL | SPEED_LOW);
#endif
}

int board_boot_init(void)
{
	struct stm32_gpio *gpioa = STM32_GPIOA;
	struct stm32_gpio *gpiob = STM32_GPIOB;
	struct stm32_rcc *rcc = STM32_RCC;
	int opt = 0;

	/* avoid compiler's unused variable warning when debug is disabled */
	(void)gpioa;
	(void)gpiob;
	(void)rcc;

	DCC_LOG1(LOG_TRACE, "AHB1ENR=0x%08x", rcc->ahb1enr);
	DCC_LOG1(LOG_TRACE, "AHB2ENR=0x%08x", rcc->ahb2enr);
	DCC_LOG1(LOG_TRACE, "AHB3ENR=0x%08x", rcc->ahb3enr);
	DCC_LOG1(LOG_TRACE, "APB1ENR=0x%08x", rcc->apb1enr);
	DCC_LOG1(LOG_TRACE, "APB2ENR=0x%08x", rcc->apb2enr);

	DCC_LOG1(LOG_TRACE, "clk[AHB]=%d", stm32f_ahb_hz);
	DCC_LOG1(LOG_TRACE, "clk[APB1]=%d", stm32f_apb1_hz);
	DCC_LOG1(LOG_TRACE, "clk[TIM1]=%d", stm32f_tim1_hz);
	DCC_LOG1(LOG_TRACE, "clk[APB2]=%d", stm32f_apb2_hz);
	DCC_LOG1(LOG_TRACE, "clk[TIM2]=%d", stm32f_tim2_hz);

	/* Port A */
	DCC_LOG1(LOG_TRACE, "gpioa->afrl=0x%08x", gpioa->afrl);
	DCC_LOG1(LOG_TRACE, "gpioa->afrh=0x%08x", gpioa->afrh);
	DCC_LOG1(LOG_TRACE, "gpioa->moder=0x%08x", gpioa->moder);
	DCC_LOG1(LOG_TRACE, "gpioa->otyper=0x%08x", gpioa->otyper);
	DCC_LOG1(LOG_TRACE, "gpioa->ospeedr=0x%08x", gpioa->ospeedr);
	DCC_LOG1(LOG_TRACE, "gpioa->pupdr=0x%08x", gpioa->pupdr);
	DCC_LOG1(LOG_TRACE, "gpioa->odr=0x%08x", gpioa->odr);

	/* Port B */
	DCC_LOG1(LOG_TRACE, "gpiob->afrl=0x%08x", gpiob->afrl);
	DCC_LOG1(LOG_TRACE, "gpiob->afrh=0x%08x", gpiob->afrh);
	DCC_LOG1(LOG_TRACE, "gpiob->moder=0x%08x", gpiob->moder);
	DCC_LOG1(LOG_TRACE, "gpiob->otyper=0x%08x", gpiob->otyper);
	DCC_LOG1(LOG_TRACE, "gpiob->ospeedr=0x%08x", gpiob->ospeedr);
	DCC_LOG1(LOG_TRACE, "gpiob->pupdr=0x%08x", gpiob->pupdr);
	DCC_LOG1(LOG_TRACE, "gpiob->odr=0x%08x", gpiob->odr);

	io_init();

	DCC_LOG(LOG_WARNING, "/!\\ NVIC interrupts enabled");
	/* set the interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);
#if 0
	cm3_irq_pri_set(STM32F_IRQ_EXTI3, MONITOR_PRIORITY);
	cm3_irq_enable(STM32F_IRQ_EXTI3);

	cm3_irq_pri_set(STM32F_IRQ_EXTI9_5, MONITOR_PRIORITY);
	cm3_irq_enable(STM32F_IRQ_EXTI9_5);
#endif
	DCC_LOG(LOG_TRACE, "......................");

	(void)opt;
	return 1;
}

#if COMM_NSEL_ISR_ENABLE
void stm32f_exti9_5_isr(void)
{
	struct stm32_exti *exti = STM32_EXTI;

	exti->pr = EXTI_COMM_NSEL;

	if (exti->ftsr & EXTI_COMM_NSEL)
		exti->rtsr = EXTI_COMM_NSEL;
	else
		exti->ftsr = EXTI_COMM_NSEL;

	DCC_LOG(LOG_TRACE, ".");
}
#endif

#if COMM_NTXEN_ISR_ENABLE
void ptp_uart_init(unsigned int baudrate)
{
	struct stm32_usart *uart = STM32_USART2;
	uint32_t f_pclk;
	uint32_t div;
	uint32_t f;
	uint32_t m;

	/* Enable peripheral clock */
	stm32_clk_enable(STM32_RCC, STM32_CLK_USART2);

	/* Disable peripheral */
	uart->cr1 = 0;
	uart->cr2 = 0;
	uart->cr3 = 0;
	/* Clear pending interrupts */
	uart->sr = 0;

	/* Configure baud rate */
	f_pclk = stm32_clk_hz(STM32_CLK_USART2);
	div = f_pclk / baudrate;
	m = div >> 4;
	f = div & 0x0f;
	uart->brr = (m << 4) | f;

	/* Configure DMA transfer on RX and TX */
//	uart->cr3 = USART_DMAT | USART_DMAR;
	/* 1 stop bit */
	uart->cr2 = USART_STOP_1;
	/* 8 bits, enable UART */
	uart->cr1 = USART_M8 | USART_UE;
}

void stm32f_exti3_isr(void)
{
	struct stm32_exti *exti = STM32_EXTI;
	struct stm32_usart *uart = STM32_USART2;
	uint32_t cr1;

	exti->pr = EXTI_COMM_NTXEN;

	cr1 = uart->cr1;
	if (exti->ftsr & EXTI_COMM_NTXEN) {
		exti->ftsr &= ~EXTI_COMM_NTXEN;
		exti->rtsr |= EXTI_COMM_NTXEN;
		/* enable transmmiter and disable receiver */
		uart->cr1 = (cr1 & ~USART_RE) | USART_TE;
		DCC_LOG(LOG_JABBER, "-");
	} else {
		exti->rtsr &= ~EXTI_COMM_NTXEN;
		exti->ftsr |= EXTI_COMM_NTXEN;
		/* enable receiver and disable transmmiter */
		uart->cr1 = (cr1 & ~USART_TE) | USART_RE;
		DCC_LOG(LOG_JABBER, "+");
	}

}
#endif

void board_on_softreset(void)
{
	struct stm32_rcc *rcc = STM32_RCC;
	struct stm32f_dma * dma;
	unsigned int j;

	DCC_LOG1(LOG_TRACE, "AHB1ENR=0x%08x", rcc->ahb1enr);
	DCC_LOG1(LOG_TRACE, "AHB2ENR=0x%08x", rcc->ahb2enr);
	DCC_LOG1(LOG_TRACE, "AHB3ENR=0x%08x", rcc->ahb3enr);
	DCC_LOG1(LOG_TRACE, "APB1ENR=0x%08x", rcc->apb1enr);
	DCC_LOG1(LOG_TRACE, "APB2ENR=0x%08x", rcc->apb2enr);

	/* Disable all DMA channels */
	dma = STM32F_DMA1;
	for (j = 0; j < 8; ++j) {
		dma->s[j].cr = 0;
		while (dma->s[j].cr & DMA_EN);
	}
	dma = STM32F_DMA2;
	for (j = 0; j < 8; ++j) {
		dma->s[j].cr = 0;
		while (dma->s[j].cr & DMA_EN);
	}

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

	rcc->ahb1enr = (1 << RCC_GPIOA);
	rcc->ahb2enr = (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr = 0;

	/* disable all peripherals clock sources except USB_OTG and GPIOA */
	rcc->ahb1enr = (1 << RCC_GPIOA);
	rcc->ahb2enr = (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr = 0;

	/* reinitialize IO's */
	io_init();

	DCC_LOG(LOG_WARNING, "/!\\ NVIC interrupts enabled");
	/* set the interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);

#if COMM_NTXEN_ISR_ENABLE
	ptp_uart_init(38400);

	cm3_irq_pri_set(STM32F_IRQ_EXTI3, MONITOR_PRIORITY);
	cm3_irq_enable(STM32F_IRQ_EXTI3);
#endif

#if COMM_NSEL_ISR_ENABLE
	cm3_irq_pri_set(STM32F_IRQ_EXTI9_5, MONITOR_PRIORITY);
	cm3_irq_enable(STM32F_IRQ_EXTI9_5);
#endif
}

uint32_t board_cyccnt;

/* This function runs as the main thread's task when the bootloader 
   fails to run the application ... */
void __attribute__((noreturn)) board_default_task(void *ptr)
{
	uint32_t tick;

	for (tick = 0;; ++tick) {
		thinkos_sleep(250);
		if (tick & 1) {
			__led_on(IO_LED1C);
			__led_on(IO_LED2C);
			__led_off(IO_LED3C);
			__led_off(IO_LED4C);
		} else {
			__led_on(IO_LED3C);
			__led_on(IO_LED4C);
			__led_off(IO_LED1C);
			__led_off(IO_LED2C);
		}
	}
}

#define LOG_MAX 1024

#ifndef TRACE_RING_SIZE
#define TRACE_RING_SIZE 1024
#endif

#ifndef TRACE_STRING_MAX
#define TRACE_STRING_MAX 64
#endif

#ifndef TRACE_TIMER
#if defined(STM32F_TIM5)
#define TRACE_TIMER STM32F_TIM5
#elif defined(STM32F_TIM2)
#define TRACE_TIMER STM32F_TIM2
#endif
#endif

struct trace_ctl {
	uint32_t crc32;
	unsigned int mutex;
	uint64_t tm;
	volatile uint32_t head;
	volatile uint32_t tail;
	volatile uint32_t print_pos;
	volatile uint32_t print_tm;
};

struct trace_ring {
	struct {
		union {
			const struct trace_ref *ref;
			uint32_t ts;
			uint32_t val;
		};
	} buf[TRACE_RING_SIZE];
};

struct trace_ctl trace_ctl __attribute__ ((section(".bkpctl.trace")));
struct trace_ring trace_ring __attribute__ ((section(".bkpdat.trace")));

/* ----------------------------------------------------------------------------
 * Preboot: this task runs once at power up only.
 * It's used to delay booting up the application ... 
 * ----------------------------------------------------------------------------
 */

int board_integrity_check(void)
{
	uint32_t tick;

	/* Time window autoboot */
	for (tick = 0; tick < 8; ++tick) {
		thinkos_sleep(250);

		switch (tick & 0x3) {
		case 0:
			__led_off(IO_LED4C);
			__led_on(IO_LED1C);
			break;
		case 1:
			__led_off(IO_LED1C);
			__led_on(IO_LED2C);
			break;
		case 2:
			__led_off(IO_LED2C);
			__led_on(IO_LED3C);
			break;
		case 3:
			__led_off(IO_LED3C);
			__led_on(IO_LED4C);
			break;
		}
	}
#if 0
	if (stm32_gpio_stat(IO_JTMS) == 0)
		return false;
#endif

	return 0;
}


#if 0
#if BOARD_TRACE_ENABLE
static void trace_print_all(void)
{
	struct trace_iterator trace_it;
	char msg[80];

	thinkos_krn_trace_tail(&trace_it);

	while (trace_krn_getnext(&trace_it) >= 0) {
		struct trace_entry * trace = &trace_it.entry;;

		trace_fmt(trace, msg, sizeof(msg));
		__puts("  ");
		__puts(trace->ref->func);
		__puts(": ");
		__puts(msg);
		__puts("\r\n");
	}
}
#endif
#endif

int board_selftest_task(void *ptr)
{
	DCC_LOG(LOG_TRACE, "board self test");
#if 0

	__puts("\r\n");
	__puts("-- begin traces ------------------------------------------------");
	__puts("\r\n\r\n");

	__puts("\r\n\r\n");

	thinkos_sleep(board_cyccnt);
	uint32_t crc;


	/* check for valid data on the backup area */
	crc = crc32(&trace_ring, sizeof(trace_ring));
	if (crc != trace_ctl.crc32) {
		__thinkos_memset32(&trace_ring, 0, sizeof(trace_ring));
		__puts("Trace ring CRC error!\r\n");
		trace_ctl.crc32 = crc;
	} else {
		__puts("Trace ring CRC match.\r\n");
	}
	DCC_LOG2(LOG_TRACE, "log=%p crc=0x%08x", &trace_ring, crc);
#endif

#if BOARD_TRACE_ENABLE
	DCC_LOG2(LOG_TRACE, "rtcbkp=%p crc=0x%08x", &trace_ctl,
		 trace_ctl.crc32);

//	trace_print_all();

	__puts("\r\n");
	__puts("-- end traces --------------------------------------------------");
	__puts("\r\n\r\n");
#endif

	thinkos_sleep(500);

	return 0;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
/* Bootloader signature */
const struct magic_blk bootldr_magic = {
	.hdr = {
		.pos = 0,
		.cnt = 10},
	.rec = {
		{0xfff00000, 0x20020000},
		{0xffff0000, 0x08000000},
		{0xffff0000, 0x08000000},
		{0xffff0000, 0x08000000},

		{0xffff0000, 0x08000000},
		{0xffff0000, 0x08000000},
		{0xffff0000, 0x08000000},
		{0xffff0000, 0x08000000},

		{0xfff00000, 0x20020000},
		{0xffff0000, 0x08000000}
		}
};
#pragma GCC diagnostic pop

/* -----------------------------------------------------------------------
   Bootloader and debugger, memory description  
 */

#ifndef ENABLE_PRIPHERAL_MEM
/* Allow read-only access from the debugger
   to the peripheral memory block */
#define ENABLE_PRIPHERAL_MEM 0
#endif

#ifndef ENABLE_BITBAND_MEM
#define ENABLE_BITBAND_MEM 0
#endif

/* Bootloader board description  */
const struct thinkos_board this_board = {
	.name = "QAD6425",
	.desc = "Rio CLass-D Amplifier 4x25W",
	.hw = {
	       .tag = "PB-847",
	       .ver = {.major = 1,.minor = 1}
	       },
	.sw = {
	       .tag = "ThinkOS",
	       .ver = {
		       .major = VERSION_MAJOR,
		       .minor = VERSION_MINOR,
		       .build = VERSION_BUILD}
	       },
	.on_softreset = board_on_softreset,
	.memory = &mem_map
};

void standby_monitor_task(const struct monitor_comm * comm, void * arg);
void boot_monitor_task(const struct monitor_comm * comm, void * arg);

void __attribute((noreturn)) main(int argc, char ** argv)
{
	struct thinkos_rt * krn = &thinkos_rt;
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
    thinkos_krn_init(krn, THINKOS_OPT_PRIORITY(0) | THINKOS_OPT_ID(0) |
                     THINKOS_OPT_PRIVILEGED |
                     THINKOS_OPT_STACK_SIZE(32768), NULL);

#if DEBUG
    mdelay(125);
#endif
    DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR
            "* 2. board_init()." VT_POP);
    board_boot_init();

#if DEBUG
    mdelay(125);
#endif
    DCC_LOG(LOG_TRACE, VT_PSH VT_BRI VT_FGR
            "* 3. usb_comm_init()." VT_POP);

	comm = usb_comm_init(&stm32f_otg_fs_dev);

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



