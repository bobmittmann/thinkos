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

#if 0
void __puts(char *s)
{
	int rem = __thinkos_strlen(s, 64);

	while (rem) {
		int n = thinkos_console_write(s, rem);
		s += n;
		rem -= n;
	}
}
#endif

void io_init(void)
{
	struct stm32_gpio * gpioa = STM32_GPIOA;
	struct stm32_gpio * gpiob = STM32_GPIOB;

	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);

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

#ifdef PB_874_R02
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
	/* 5. Wait until the regulator become ready */

	/* - CRC --------------------------------------------------------------- */
	stm32_clk_enable(STM32_RCC, STM32_CLK_CRC);

	/* - USB OTG FS -------------------------------------------------------- */
	stm32_gpio_af(OTG_FS_DP, GPIO_AF10);
	stm32_gpio_af(OTG_FS_DM, GPIO_AF10);
	//stm32_gpio_af(OTG_FS_VBUS, GPIO_AF10);

	stm32_gpio_mode(OTG_FS_DP, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(OTG_FS_DM, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	/* XXX: something wrong with this configuration!!! Set as input instead ...
	 */
	stm32_gpio_mode(OTG_FS_VBUS, INPUT, SPEED_LOW);

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

#ifdef PB_874_R02
	stm32_gpio_clr(IO_LEDTB1);
	stm32_gpio_mode(IO_LEDTB1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_LEDTB2);
	stm32_gpio_mode(IO_LEDTB2, OUTPUT, PUSH_PULL | SPEED_LOW);
#endif

#if !THINKOS_ENABLE_FPU
	/* This code is used when the OS is not handling the FPU context.
	   In which case only one thread is allowed to use the floating
	   point registers. */
	{
		uint32_t ctrl;

		/* Configure FPU */
		/* Floating Point Context Control (0xf34) */
		ctrl = cm3_control_get();
		cm3_control_set(ctrl & ~CONTROL_FPCA);
		/* Disable FPU Access */
		CM3_SCB->cpacr &= ~(CP11_SET(3) | CP10_SET(3));
		/* Clear the FP context automatic save and restore */
		CM3_SCB->fpccr &= ~(SCB_FPCCR_ASPEN | SCB_FPCCR_LSPEN);
		/* Enable FPU */
		CM3_SCB->cpacr |= CP11_SET(3) | CP10_SET(3);
	}
#endif
}

int board_init(void)
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

	DCC_LOG(LOG_MSG, "io_init()");
	io_init();

	DCC_LOG(LOG_WARNING, "/!\\ NVIC interrupts enabled");
	/* set the interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);

	(void)opt;
	return 1;
}

void board_on_softreset(void)
{
	struct stm32_rcc *rcc = STM32_RCC;

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
}


int board_init(void)
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

	DCC_LOG(LOG_MSG, "io_init()");
	io_init();

	DCC_LOG(LOG_WARNING, "/!\\ NVIC interrupts enabled");
	/* set the interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);

	(void)opt;
	return 1;
}

void board_on_softreset(void)
{
	struct stm32_rcc *rcc = STM32_RCC;

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
}


/* This function runs as the main thread's task when the bootloader 
   fails to run the application ... */
int board_default_task(void *ptr)
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

	return 0;
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

int board_preboot_task(void * ptr)
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

int board_configure_task(void *ptr)
{
	DCC_LOG(LOG_TRACE, "board configuration");

	return 0;
}

int board_selftest_task(void *ptr)
{
	DCC_LOG(LOG_TRACE, "board self test");
#if 0
	uint32_t crc;

	thinkos_sleep(500);

	__puts("\r\n\r\n== " VERSION_NAME " " VERSION_NUM " "
	       VERSION_DATE " ==\r\n");

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
	DCC_LOG2(LOG_TRACE, "rtcbkp=%p crc=0x%08x", &trace_ctl,
		 trace_ctl.crc32);
#endif
	return 0;
}

/* -----------------------------------------------------------------------
   Bootloader self upload.

   The self upload works by loading a small code (codelet) in RAM. 
   This simple application runs YMODEM protocol over an 
   existing USB connection erases the current bootloader and writes 
   the new one over.
   Self upgrade is dangerous as it needs to erase the bootloader flash 
   block and override it. If something goes wrong during this process
   we may ending up with a corrupted bootloader!!
 */
extern const uint8_t otg_xflash_pic[];
extern const unsigned int sizeof_otg_xflash_pic;

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

void board_upgrade(const struct dbgmon_comm *comm)
{
	uint32_t *xflash_code = (uint32_t *) (0x20001000);
	int (*xflash_ram) (uint32_t, uint32_t, const struct magic_blk *) =
	    ((void *)xflash_code) + 1;

	/* Disable all interrupts */
	cm3_cpsid_f();
	/* Copy flash upgrade codelet into RAM */
	__thinkos_memcpy(xflash_code, otg_xflash_pic, sizeof_otg_xflash_pic);
	/* Run codelet... */
	xflash_ram(0, 65536, &bootldr_magic);
}

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

const struct mem_desc flash_desc = {
	.tag = "FLASH",
	.blk = {
		{"BOOT", 0x08000000, M_RO, SZ_16K, 3},	/* Bootloader    :  48KiB */
		{"CFG", 0x0800c000, M_RW, SZ_16K, 1},	/* Configuration :  16KiB */
		{"APP", 0x08010000, M_RW, SZ_64K, 1},	/* Application   :  ...   */
		{"APP", 0x08020000, M_RW, SZ_128K, 3},	/* Application   : 448KiB */
		{"", 0x00000000, 0, 0, 0}
		}
};

const struct mem_desc sram_desc = {
	.tag = "RAM",
	.blk = {
		{"BOOT", 0x20000000, M_RO, SZ_4K, 1},	/* Bootloader: 4KiB */
		{"APP", 0x20001000, M_RW, SZ_4K, 27},	/* Application: 108KiB */
		{"STACK", 0x2001c000, M_RW, SZ_16K, 1},	/* SRAM 2: 16KiB */

#if ENABLE_BITBAND_MEM
		{"", 0x22000000, M_RO, SZ_512K, 1},	/* Bootloader - bitband */
		{"", 0x22080000, M_RW, SZ_512K, 27},	/* Application - bitband */
		{"", 0x22e00000, M_RW, SZ_2M, 1},	/* SRAM 2 - bitband */
#endif
		{"", 0x00000000, 0, 0, 0}
		}
};

const struct mem_desc peripheral_desc = {
	.tag = "PERIPH",
	.blk = {
#if ENABLE_PRIPHERAL_MEM
		{"", 0x40000000, M_RO, SZ_4K, 36},	/* Peripheral - 1 */
#endif
		{"RTC", 0x40002800, M_RW, SZ_1K, 1},	/* RTC - 1K */
#if ENABLE_PRIPHERAL_MEM
		{"", 0x40002c00, M_RW, SZ_1K, 1},	/* WWDG - 1K */
		{"", 0x40003000, M_RW, SZ_1K, 1},	/* IWD - 1K */
		{"", 0x40003800, M_RW, SZ_1K, 1},	/* SPI2 - 1K */
		{"", 0x40003c00, M_RW, SZ_1K, 1},	/* SPI3 - 1K */
		{"", 0x40004000, M_RW, SZ_1K, 1},	/* SPDIF-RX - 1K */
#endif
		{"BACKUP", 0x40024000, M_RW, SZ_1K, 4},	/* Backup SRAM - 4K */
#if ENABLE_PRIPHERAL_MEM
		{"", 0x40026000, M_RO, SZ_4K, 1},	/* DMA1 */
		{"", 0x40026400, M_RO, SZ_4K, 1},	/* DMA2 */
		{"", 0x40040000, M_RO, SZ_4K, 64},	/* USB_OTG_HS */
		{"", 0x50000000, M_RO, SZ_4K, 64},	/* USB_OTG_FS */
		{"", 0x50050000, M_RO, SZ_1K, 1},	/* DCMI */
		{"", 0xA0000000, M_RO, SZ_4K, 1},	/* FMC registers */
		{"", 0xA0010000, M_RO, SZ_4K, 1},	/* QUADSPI registers */
#endif
#if ENABLE_PRIPHERAL_MEM && ENABLE_BITBAND_MEM
		{"", 0x42000000, M_RO, SZ_128M, 1},	/* Peripheral - bitband */
#endif
		{"", 0x00000000, 0, 0, 0}
		}
};

/* Application signature */
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
	.memory = {
		   .cnt = 3,
		   .flash = &flash_desc,
		   .ram = &sram_desc,
		   .periph = &peripheral_desc},
	.application = {
			.tag = "",
			.start_addr = 0x08020000,
			.block_size = (3 * 128) * 1024,
			.magic = &thinkos_10_app_magic},
	.init = board_init,
	.softreset = board_on_softreset,
	.upgrade = board_upgrade,
	.preboot_task = board_preboot_task,
	.configure_task = board_configure_task,
	.selftest_task = board_selftest_task,
	.default_task = board_default_task
};
