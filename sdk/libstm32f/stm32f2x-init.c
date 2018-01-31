/* 
 * File:	stm32f-init.c
 * Author:  Robinson Mittmann (bobmittmann@gmail.com)
 * Target:  jtagtool3
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

#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>

/* -------------------------------------------------------------------- 
   Default options 
*/
#ifndef STM32_ENABLE_HSI
#define STM32_ENABLE_HSI 1
#endif

#ifndef STM32_ENABLE_HSE
#define STM32_ENABLE_HSE 0
#endif

#ifndef STM32_ENABLE_PLL
#define STM32_ENABLE_PLL 1
#endif

#ifndef STM32_ENABLE_PLLSAI
#define STM32_ENABLE_PLLSAI 1
#endif

#ifndef STM32_ENABLE_PLLI2S
#define STM32_ENABLE_PLLI2S 0
#endif

#ifndef STM32_HCLK_HZ 
  #if defined(STM32F4X)
    #define STM32_HCLK_HZ    168000000
  #else
    #define STM32_HCLK_HZ    120000000
  #endif
#endif

#ifndef STM32_HSE_HZ 
  #if defined(STM32F4X)
    #define STM32_HSE_HZ     25000000
  #elif defined(STM32F2X)
    #define STM32_HSE_HZ     24000000
  #else
    #define STM32_HSE_HZ     12000000
  #endif
#endif

#ifndef STM32_MSI_HZ 
#define STM32_MSI_HZ          4000000
#endif

#define STM32_HSI_HZ         16000000

#ifndef STM32_APB1_PRE
#define STM32_APB1_PRE   1
#endif

#ifndef STM32_APB2_PRE
#define STM32_APB2_PRE   1
#endif

#ifndef STM32_AHB_PRE
#define STM32_AHB_PRE    1
#endif

#ifndef STM32_MCO_PRE
#define STM32_MCO_PRE    1
#endif

#if defined(STM32F446)
  #define PLLR 2
#else
  #define PLLR 0
#endif

#if (STM32_HCLK_HZ == 168000000)
#if (STM32_HSE_HZ == 25000000)
	/* F_HSE = 25 MHz
	   F_VCO = 335.9375 MHz (F_HSE * 215 / 16)
	   F_MAIN = 167.96875 MHz (F_VCO / 2)
	   F_USB = 47.991 MHz (F_VCO / 7) */
  #define PLLQ 7
  #define PLLP 2
  #define PLLN 215
  #define PLLM 16
#elif (STM32_HSE_HZ == 16000000)
	/* F_HSE = 16 MHz
	   F_VCO = 336 MHz (F_HSE * 42)
	   F_MAIN = 168 MHz (F_VCO / 2)
	   F_USB = 48 MHz (F_VCO / 7)*/
  #define PLLQ 7
  #define PLLP 2
  #define PLLN 168
  #define PLLM 8
#elif (STM32_HSE_HZ == 12000000)
	/* F_HSE = 12 MHz
	   F_VCO = 336 MHz (F_HSE * 28)
	   F_MAIN = 168 MHz (F_VCO / 2)
	   F_USB = 48 MHz (F_VCO / 7)*/
  #define PLLQ 7
  #define PLLP 2
  #define PLLN 112
  #define PLLM 4
    /* F_VCO = 389454513
	   F_I2S = 169333333 */
  #define PLLI2SR 3
  #define PLLI2SN 127
  #define PLLI2SM 9
  #define PLLI2SQ 8
  #define PLLI2SP 2
   /* F_VCO = 389454513
      F_I2S = 169333333 */
  #define PLLSAIN 127
  #define PLLSAIM 9
  #define PLLSAIQ 3
  #define PLLSAIP 2
#elif (STM32_HSE_HZ == 8000000)
	/* F_HSE = 8 MHz
	   F_VCO = 336 MHz (F_HSE * 42)
	   F_MAIN = 168 MHz (F_VCO / 2)
	   F_USB = 48 MHz (F_VCO / 7)*/
  #define PLLQ 7
  #define PLLP 2
  #define PLLN 168
  #define PLLM 4
#else
#error "STM32_HSE_HZ invalid!"
#endif

#elif (STM32_HCLK_HZ == 120000000)
#if (STM32_HSE_HZ == 24000000)
	/* F_HSE = 24 MHz
	   F_VCO = 240 MHz
	   F_MAIN = 120 MHz
	   F_USB = 48 MHz */
  #define PLLQ 5
  #define PLLP 2
  #define PLLN 120
  #define PLLM 12
#elif (STM32_HSE_HZ == 12000000)
	/* F_HSE = 24 MHz
	   F_VCO = 240 MHz
	   F_MAIN = 120 MHz
	   F_USB = 48 MHz */
  #define PLLQ 5
  #define PLLP 2
  #define PLLN 240
  #define PLLM 12
  /* Target I2S frequecny = 44100 * 32 * X */
#else
#error "STM32_HSE_HZ invalid!"
#endif

#elif (STM32_HCLK_HZ == 192000000)
#if (STM32_HSE_HZ == 25000000)
	/* F_HSE = 25 MHz
	   F_VCO = 383.9286 MHz (F_HSE * 215 / 14)
	   F_MAIN = 191.964 MHz (F_VCO / 2)
	   F_USB = 47.991 MHz (F_VCO / 7) */
  #define PLLQ 8
  #define PLLP 2
  #define PLLN 215
  #define PLLM 14
#else
#error "STM32_HSE_HZ invalid!"
#endif

#elif (STM32_HCLK_HZ == 180000000)
#if (STM32_HSE_HZ == 25000000)
	/* F_HSE = 25 MHz
	   F_VCO = 360 MHz (F_HSE * 216 / 15)
	   F_MAIN = 180 MHz (F_VCO / 2)
	   F_USB = 45 MHz (F_VCO / 8) */
  #define PLLQ 8
  #define PLLP 2
  #define PLLN 216
  #define PLLM 15
#else
#error "STM32_HSE_HZ invalid!"
#endif

#endif

  /* Target I2S frequecny = 44100 * 32 * X */
#define VCO_HZ ((STM32_HCLK_HZ) * (PLLP))
#define USB_HZ 48000000

/* Validate the PLL configuration */
#if ((STM32_HSE_HZ * PLLN) / PLLM) > ((VCO_HZ) + (VCO_HZ) / 1000)
#error "invalid PLL configuration (err > 0.1 %)!"
#endif

#if ((STM32_HSE_HZ * PLLN) / PLLM) < ((VCO_HZ) - (VCO_HZ) / 1000)
#error "invalid PLL configuration (err < -0.1 %)!"
#endif

#if (VCO_HZ / PLLQ) > ((USB_HZ) + (USB_HZ) / 1000) 
#error "invalid PLL configuration (err > 0.1 %)!"
#endif

#if (VCO_HZ / PLLQ) < ((USB_HZ) - (USB_HZ) / 1000) 
#error "invalid PLL configuration (err < -0.1 %)!"
#endif

#if (PLLN > 432)
#error "invalid PLLN!"
#endif

#if (PLLM > 63)
#error "invalid PLLM!"
#endif


#if defined(STM32F446)
#if (PLLI2SN < 50)
#error "invalid PLLI2SN!"
#endif

#if (PLLI2SN > 432)
#error "invalid PLLI2SN!"
#endif

#if (PLLI2SM > 63)
#error "invalid PLLI2SM!"
#endif

#if (PLLI2SM < 2)
#error "invalid PLLI2SM!"
#endif

#if (PLLI2SR < 2)
#error "invalid PLLI2SR!"
#endif

#endif

#define __VCO_HZ (((uint64_t)STM32_HSE_HZ * PLLN) / PLLM)
#define __HCLK_HZ (__VCO_HZ / PLLP)

#if STM32_ENABLE_PLLSAI
  /* Enable the SAI PLL, the SAI PLL is PLLSAI1CLK */
  #define __VCOSAI_HZ (((uint64_t)STM32_HSE_HZ * PLLSAIN) / PLLSAIM)
  #define __SAI_HZ (__VCOSAI_HZ / PLLSAIQ)
#else
  #if STM32_ENABLE_PLL
    /* The SAI PLL is disabled, the SAI PLL is PLLSAI2CLK */
    #define __SAI_HZ (__VCO_HZ / PLLPDIV)
  #else
    #define __SAI_HZ STM32_HSI_HZ
  #endif
#endif

#if STM32_ENABLE_PLLI2S
#define __VCOI2S_HZ (((uint64_t)STM32_HSE_HZ * PLLI2SN) / PLLI2SM)
#define __I2S_HZ (__VCOI2S_HZ / PLLI2SR)
#endif

const uint32_t stm32f_ahb_hz  = __HCLK_HZ;
const uint32_t stm32f_apb1_hz = __HCLK_HZ / 4;
const uint32_t stm32f_tim1_hz = __HCLK_HZ / 2;
const uint32_t stm32f_apb2_hz = __HCLK_HZ / 2;
const uint32_t stm32f_tim2_hz = __HCLK_HZ;
const uint32_t stm32f_sai_hz = __SAI_HZ;
const uint32_t stm32f_hsi_hz = STM32_HSI_HZ;
#if STM32_ENABLE_PLL
const uint32_t stm32f_vco_hz = __VCO_HZ;
#endif
#if STM32_ENABLE_PLLSAI
const uint32_t stm32f_vcosai_hz = __VCOSAI_HZ;
#endif

#if STM32_ENABLE_PLLI2S
const uint32_t stm32f_i2s_hz = __I2S_HZ;
#endif

#ifndef THINKAPP

/* This constant is used to calibrate the systick timer */
const uint32_t cm3_systick_load_1ms = ((__HCLK_HZ / 8) / 1000) - 1;

const uint32_t sysclk_hz[] = {
	[SYSCLK_STM32_AHB] = __HCLK_HZ,
	[SYSCLK_STM32_APB1] = __HCLK_HZ / 4,
	[SYSCLK_STM32_APB2] = __HCLK_HZ / 2,
	[SYSCLK_STM32_TIM1] = __HCLK_HZ / 2,
	[SYSCLK_STM32_TIM2] = __HCLK_HZ,
};

void __attribute__((section(".init"))) _init(void)
{
	struct stm32_rcc * rcc = STM32_RCC;
	struct stm32_flash * flash = STM32_FLASH;
	uint32_t sr;
	uint32_t cr;
	uint32_t pll;
	uint32_t cfg;
	int again;

	/* Make sure we are using the internal oscillator */
	rcc->cfgr = RCC_PPRE2_1 | RCC_PPRE1_1 | RCC_HPRE_1 | RCC_SW_HSI;

#if defined(STM32F446)
	rcc->sscgr = 0;
	rcc->pllsaicfgr = 0;
	rcc->ckgatenr = 0xffffffff;
	rcc->dckcfgr2 = 0;
#endif

	/* Enable external oscillator */
	cr = rcc->cr;
	cr |= RCC_HSEON;
	rcc->cr = cr;;

	for (again = 8192; ; again--) {
		cr = rcc->cr;
		if (cr & RCC_HSERDY)
			break;
		if (again == 0) {
			/* external clock startup fail! */
			return;
		}

	}

	pll = RCC_PLLR(PLLR) | RCC_PLLQ(PLLQ) | RCC_PLLSRC_HSE | RCC_PLLP(PLLP) | 
		RCC_PLLN(PLLN) | RCC_PLLM(PLLM);

	rcc->pllcfgr = pll;

	/* enable PLL */
	cr |= RCC_PLLON;
	rcc->cr = cr;;

	for (again = 8192; ; again--) {
		cr = rcc->cr;
		if (cr & RCC_PLLRDY)
			break;
		if (again == 0) {
			/* PLL lock fail */
			return;
		}
	}

	/* switch to external clock */
	cfg = RCC_MCO2_SYSCLK | RCC_MCO2PRE_2 /* Clock output 2 */
		| RCC_PPRE2_2 /* APB high speed prescaler : 60|84MHz */
		| RCC_PPRE1_4 /* APB low speed prescaler : 30|42MHz */
		| RCC_HPRE_1 /* AHB prescaler : 120|168MHz */ 
		| RCC_SW_HSE;
	
	rcc->cfgr = cfg;

	for (again = 4096; ; again--) {
		sr = flash->sr;
		if ((sr & FLASH_BSY) == 0)
			break;
		if (again == 0) {
			/* flash not ready! */
			return;
		}
	}

	/* adjust flash wait states and enable caches */
	flash->acr = FLASH_DCEN | FLASH_ICEN | FLASH_PRFTEN | 
		FLASH_LATENCY(STM32_HCLK_HZ / 30000000);

	if (flash->cr & FLASH_LOCK) {
		/* unlock flash write */
		flash->keyr = FLASH_KEY1;
		flash->keyr = FLASH_KEY2;
	}

	/* switch to pll oscillator */
	rcc->cfgr = (cfg & ~RCC_SW) | RCC_SW_PLL;

#ifdef CM3_RAM_VECTORS
	/* Remap the VECTOR table to SRAM 0x20000000  */
	CM3_SCB->vtor = 0x20000000; /* Vector Table Offset */
#endif

#if STM32_ENABLE_PLLI2S
	/* configure IS2 PLL */
	rcc->plli2scfgr =  RCC_PLLI2SR(PLLI2SR) | RCC_PLLI2SQ(PLLI2SQ) | 
		RCC_PLLI2SP(PLLI2SP) | RCC_PLLI2SN(PLLI2SN) | RCC_PLLI2SM(PLLI2SM);

	/* enable I2SPLL */
	cr |= RCC_PLLI2SON;
	rcc->cr = cr;;

	for (again = 8192; ; again--) {
		cr = rcc->cr;
		if (cr & RCC_PLLI2SRDY)
			break;
		if (again == 0) {
			/* PLL lock fail */
			return;
		}
	}

	/* configure SAI PLL */
	rcc->pllsaicfgr =  RCC_PLLSAIQ(PLLSAIQ) | RCC_PLLSAIP(PLLSAIP) |
		RCC_PLLSAIN(PLLSAIN) | RCC_PLLSAIM(PLLSAIM);

	/* enable SAISPLL */
	cr |= RCC_PLLSAION;
	rcc->cr = cr;;

	for (again = 8192; ; again--) {
		cr = rcc->cr;
		if (cr & RCC_PLLSAIRDY)
			break;
		if (again == 0) {
			/* PLL lock fail */
			return;
		}
	}

	rcc->dckcfgr = I2S2SRC_PLLI2S_R | I2S1SRC_PLLI2S_R;
#endif

}

#endif /* THINKAPP */

