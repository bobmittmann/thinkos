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
#include "lattice.h"
#include "version.h"

#define DAC1_DMA_STREAM  5
#define DAC2_DMA_STREAM  6
#define DAC_DMA_CHANNEL  7

#define AUDIO_DMA_STREAM DAC2_DMA_STREAM
#define AUDIO_SAMPLE_RATE 12000

#if 0
/* over=15 err=0.001582 freq=1046.512 */
static const uint8_t wave_c4[] = /* 1046.502 Hz */
{
	   128,     194,     241,     255,     231,     178,     110,     47,
	     8,       4,      37,      96,     165,     223,     253,    247,
	   206,     142,      74,      22,       1,      17,      66,    133,
	   198,     243,     254,     229,     173,     105,      43,      6,
	     5,      40,     100,     169,     226,     254,     245,    202,
	   137,      70,      20,       1,      20,      70,     137,    202,
	   245,     254,     226,     169,     100,      40,       5,      6,
	    43,     105,     173,     229,     254,     243,     198,    133,
	    66,      17,       1,      22,      74,     142,     206,    247,
	   253,     223,     165,      96,      37,       4,       8,     47,
	   110,     178,     231,     255,     241,     194,     128,     62,
	    15,       1,      25,      78,     146,     209,     248,    252,
	   219,     160,      91,      33,       3,       9,      50,    114,
	   182,     234,     255,     239,     190,     123,      58,     13,
	     2,      27,      83,     151,     213,     250,     251,    216,
	   156,      87,      30,       2,      11,      54,     119,    186,
	   236,     255,     236,     186,     119,      54,      11,      2,
	    30,      87,     156,     216,     251,     250,     213,    151,
	    83,      27,       2,      13,      58,     123,     190,    239,
	   255,     234,     182,     114,      50,       9,       3,     33,
	    91,     160,     219,     252,     248,     209,     146,     78,
	    25,       1,      15,      62, 
};

void wave_set(const uint8_t * wave, unsigned int len)
{
	struct stm32f_dma * dma = STM32F_DMA1;

	/* disable DMA */
	dma->s[AUDIO_DMA_STREAM].cr &= ~DMA_EN;	
	/* Wait for the channel to be ready .. */
	while (dma->s[AUDIO_DMA_STREAM].cr & DMA_EN); 
	/* Memory address */
	dma->s[AUDIO_DMA_STREAM].m0ar = (void *)wave;
	/* Number of data items to transfer */
	dma->s[AUDIO_DMA_STREAM].ndtr = len;
}

void wave_play(void)
{
	struct stm32f_dma * dma = STM32F_DMA1;
	/* enable DMA */
	dma->s[AUDIO_DMA_STREAM].cr |= DMA_EN;	
}

void wave_pause(void)
{
	struct stm32f_dma * dma = STM32F_DMA1;
	/* disable DMA */
	dma->s[AUDIO_DMA_STREAM].cr &= ~DMA_EN;	
}

void wave_play_init(const uint8_t * wave, unsigned int len)
{
	struct stm32f_dac * dac = STM32F_DAC;
	struct stm32f_tim * tim2 = STM32F_TIM2;
	struct stm32f_dma * dma = STM32F_DMA1;
	uint32_t freq = AUDIO_SAMPLE_RATE;
	uint32_t div;
	uint32_t pre;
	uint32_t n;

	/* DAC clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_DAC);
	/* DAC configure */
	dac->cr = DAC_EN2 | DAC_TSEL2_TIMER2 | DAC_TEN2 | DAC_DMAEN2;
	/* DAC channel 2 initial value */
	dac->dhr8r2 = 128;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM2);
	/* get the total divisior */
	div = ((stm32f_tim1_hz) + (freq / 2)) / freq;
	/* get the minimum pre scaler */
	pre = (div / 65536) + 1;
	/* get the reload register value */
	n = (div + pre / 2) / pre;
	tim2->psc = pre - 1;
	tim2->arr = n - 1;
	tim2->cnt = 0;
	tim2->egr = 0; /* Update generation */
	tim2->cr2 = TIM_MMS_OC1REF;
	tim2->ccmr1 = TIM_OC1M_PWM_MODE1;
	tim2->ccr1 = tim2->arr - 2;
	tim2->cr1 = TIM_URS | TIM_CEN; /* Enable counter */

	/* DMA clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_DMA1);
	/* DMA Disable */
	dma->s[AUDIO_DMA_STREAM].cr = 0;
	/* Wait for the channel to be ready .. */
	while (dma->s[AUDIO_DMA_STREAM].cr & DMA_EN); 
	/*  DMA Configuration */
	/* Peripheral address */
	dma->s[AUDIO_DMA_STREAM].par = &dac->dhr8r2;
	/* Memory address */
	dma->s[AUDIO_DMA_STREAM].m0ar = (void *)wave;
	/* Number of data items to transfer */
	dma->s[AUDIO_DMA_STREAM].ndtr = len;
	/* Configuration single buffer circular */
	dma->s[AUDIO_DMA_STREAM].cr = DMA_CHSEL_SET(DAC_DMA_CHANNEL) | 
		DMA_MBURST_1 | DMA_PBURST_1 | 
		DMA_MSIZE_8 | DMA_PSIZE_8 | DMA_MINC | 
		DMA_CIRC | DMA_DIR_MTP;
}

#endif

extern const uint8_t ice40lp384_bin[];
extern const unsigned int sizeof_ice40lp384_bin;

static void io_init(void)
{
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOA);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOB);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOC);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOD);
	stm32_clk_enable(STM32_RCC, STM32_CLK_GPIOE);

	/* - LEDs ---------------------------------------------------------*/
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
#if 0
	/* - DACs ----------------------------------------------------*/
	stm32_gpio_mode(IO_DAC1, ANALOG, 0);
	stm32_gpio_mode(IO_DAC2, ANALOG, 0);

	/* - Relays ----------------------------------------------------*/
	stm32_gpio_mode(IO_RELAY1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(IO_RELAY2, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_RELAY1);
	stm32_gpio_clr(IO_RELAY2);

    /* - CODEC * ------------------------------------------------------ */
	stm32_gpio_mode(IO_CODEC_DEM0, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(IO_CODEC_DEM1, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(IO_CODEC_RST, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_clr(IO_CODEC_DEM0);
	stm32_gpio_clr(IO_CODEC_DEM1);
	stm32_gpio_clr(IO_CODEC_RST);

	stm32_gpio_mode(IO_CODEC_I2S_DIN, ALT_FUNC, SPEED_HIGH);
	stm32_gpio_mode(IO_CODEC_I2S_DOUT, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_CODEC_I2S_SCLK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	/* Warning: this pin is shared with the JTAG TDI signal !!! */
//	stm32_gpio_mode(IO_CODEC_I2S_FS, ALT_FUNC, PUSH_PULL | SPEED_HIGH);
	stm32_gpio_mode(IO_CODEC_I2S_MCLK, ALT_FUNC, PUSH_PULL | SPEED_HIGH);

	/* - Amplifier ----------------------------------------------------*/
	stm32_gpio_mode(IO_AMP_ASPV, ANALOG, 0);
	stm32_gpio_mode(IO_AMP_POL, OUTPUT, PUSH_PULL | SPEED_LOW);
	stm32_gpio_mode(IO_AMP_LSPV, INPUT, PULL_UP);
#endif
}

bool board_init(void)
{
//	int i;

	DCC_LOG1(LOG_MSG, "clk[AHB]=%d", stm32f_ahb_hz);
	DCC_LOG1(LOG_MSG, "clk[APB1]=%d", stm32f_apb1_hz);
	DCC_LOG1(LOG_MSG, "clk[TIM1]=%d", stm32f_tim1_hz);
	DCC_LOG1(LOG_MSG, "clk[APB2]=%d", stm32f_apb2_hz);
	DCC_LOG1(LOG_MSG, "clk[TIM2]=%d", stm32f_tim2_hz);

	DCC_LOG(LOG_MSG, "io_init()");
	io_init();

	/* set the interrupt priority */
	cm3_irq_pri_set(STM32F_IRQ_OTG_FS, MONITOR_PRIORITY);
	/* Enable USB OTG FS interrupts */
	cm3_irq_enable(STM32F_IRQ_OTG_FS);

#if 0
	DCC_LOG(LOG_MSG, "wave_play_init()");
	wave_play_init(wave_c4, sizeof(wave_c4));
	wave_play();

	for (i = 0; i < 3; ++i) {
		__led_on(IO_LED1);
		__led_on(IO_LED2);
		udelay(100000);
		__led_off(IO_LED1);
		__led_off(IO_LED2);
		udelay(100000);
	}

	wave_pause();
#endif

	return true;
}

void board_softreset(void)
{
	struct stm32_rcc * rcc = STM32_RCC;
#if 0
	struct stm32f_dma * dma;
	int i;

	/* disable DMA */
	dma = STM32F_DMA1;
	for(i = 0; i < 8; ++i) {
		dma->s[i].cr = 0;
		while (dma->s[i].cr & DMA_EN); 
	}

	dma = STM32F_DMA2;
	for(i = 0; i < 8; ++i) {
		dma->s[i].cr = 0;
		while (dma->s[i].cr & DMA_EN); 
	}
#endif

	DCC_LOG1(LOG_INFO, "AHB1ENR=0x%08x", rcc->ahb1enr);
	DCC_LOG1(LOG_INFO, "AHB2ENR=0x%08x", rcc->ahb2enr);
	DCC_LOG1(LOG_INFO, "AHB3ENR=0x%08x", rcc->ahb3enr);
	DCC_LOG1(LOG_INFO, "APB1ENR=0x%08x", rcc->apb1enr);
	DCC_LOG1(LOG_INFO, "APB2ENR=0x%08x", rcc->apb2enr);

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
	rcc->ahb1enr = (1 << RCC_CCMDATARAM) | (1 << RCC_GPIOA); 
	rcc->ahb2enr = (1 << RCC_OTGFS);
	rcc->ahb3enr = 0;
	rcc->apb1enr = 0;
	rcc->apb2enr = 0;

	/* reinitialize IO's */
	io_init();
}

void app_default(void * arg);

bool board_autoboot(uint32_t tick)
{
	if (tick & 1) {
		__led_off(IO_LED1B);
		__led_on(IO_LED1A);
	} else {
		__led_off(IO_LED1A);
		__led_on(IO_LED1B);
	}

	/* Time window autoboot */
	return (tick == 40) ? true : false;
}

void board_on_appload(void)
{
#if 0
	wave_pause();
#endif
	__led_off(IO_LED1A);
	__led_off(IO_LED1B);
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

struct magic {
	struct {
		uint16_t pos;
		uint16_t cnt;
	} hdr;
	struct {
	    uint32_t mask;
		uint32_t comp;
	} rec[];
};

const struct magic thinkos_magic = {
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

/*
 c0 ff 00 10
 49 00 00 08
 a5 1a 00 08
 bd 23 00 08

 49 23 00 08
 61 22 00 08
 d5 22 00 08
 ed 5c 00 08

 00 00 01 10
 19 60 00 08
 b1 65 00 08
 21 51 00 08
 89 31 00 08
 d5 6f 00 08
 91 4f 00 08 
 61 50 00 08
*/

void board_upgrade(struct dmon_comm * comm)
{
	uint32_t * xflash_code = (uint32_t *)(0x20001000);
	int (* xflash_ram)(uint32_t, uint32_t, const struct magic *) = 
		((void *)xflash_code) + 1;

	cm3_cpsid_f();
	__thinkos_memcpy(xflash_code, otg_xflash_pic, sizeof_otg_xflash_pic);
	xflash_ram(0, 65536, &thinkos_magic);
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
		.block_size = (3 * 128) * 1024
	},
	.init = board_init,
	.softreset = board_softreset,
	.autoboot = board_autoboot,
	.configure = board_configure,
	.upgrade = board_upgrade,
	.selftest = board_selftest,
	.on_appload = board_on_appload
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
	}	
}

void app_default(void * arg) __attribute__((weak, alias("board_test")));

