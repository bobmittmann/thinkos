/* 
 * File:	stm32l4-init.c
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

#ifndef STM32_HCLK_HZ 
#define STM32_HCLK_HZ    80000000
#endif

#ifndef STM32_HSE_HZ 
#define STM32_HSE_HZ     11289600
#endif

#ifndef STM32_MSI_HZ 
#define STM32_MSI_HZ      4000000
#endif

#define STM32_HSI_HZ     16000000

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

/* -------------------------------------------------------------------- 
   Precalculated PLL configuartion
*/

#if (STM32_HCLK_HZ == 79027200) && (STM32_HSE_HZ == 11289600)
  /* PLLVCO = 158.054400 MHz */
  #define PLLM         1
  #define PLLN        14
  #define PLLR         2 /*     PLLCLK ->  79.027200 MHz */
  #define PLLQ         3 /* PLL48M1CLK ->  52.684800 MHz */
  #define PLLPDIV      7 /* PLLSAI2CLK ->  22.579200 MHz */

  /* PLLSAIVCO = 90.316800 MHz */
  #define PLLSAIN      8
  #define PLLSAIPDIV   4 /* PLLSAI2CLK ->  22.579200 MHz */
  #define PLLSAIQ      2 /* PLL48M2CLK ->  45.158400 MHz */
  #define PLLSAIR      4 /*  PLLADCCLK ->  22.579200 MHz */
#endif

#if (STM32_HCLK_HZ == 64915200) && (STM32_HSE_HZ == 11289600)
  /* PLLVCO = 259.660800 MHz */
  #define PLLM         1
  #define PLLN        23
  #define PLLR         4 /*     PLLCLK ->  64.915200 MHz */
  #define PLLQ         5 /* PLL48M1CLK ->  51.932160 MHz */
  #define PLLPDIV     11 /* PLLSAI2CLK ->  23.605527 MHz */

  /* PLLSAIVCO = 90.316800 MHz */
  #define PLLSAIN      8
  #define PLLSAIPDIV   4 /* PLLSAI2CLK ->  22.579200 MHz */
  #define PLLSAIQ      2 /* PLL48M2CLK ->  45.158400 MHz */
  #define PLLSAIR      4 /*  PLLADCCLK ->  22.579200 MHz */
#endif

/* -------------------------------------------------------------------- 
   The real clock frequencies based on PLL configuration
*/

#if STM32_ENABLE_PLL
  #define __VCO_HZ (((uint64_t)(STM32_HSE_HZ) * PLLN) / PLLM)
  #define __HCLK_HZ (__VCO_HZ / PLLR)
#else
  #define __HCLK_HZ (STM32_HCLK_HZ)
#endif

#if STM32_ENABLE_PLLSAI
  /* Enable the SAI PLL, the SAI PLL is PLLSAI1CLK */
  #define __VCOSAI_HZ (((uint64_t)STM32_HSE_HZ * PLLSAIN) / PLLM)
  #define __SAI_HZ (__VCOSAI_HZ / PLLSAIPDIV)
#else
  #if STM32_ENABLE_PLL
    /* The SAI PLL is disabled, the SAI PLL is PLLSAI2CLK */
    #define __SAI_HZ (__VCO_HZ / PLLPDIV)
  #else
    #define __SAI_HZ STM32_HSI_HZ
  #endif
#endif

#define __APB1_HZ   (__HCLK_HZ) / (STM32_APB1_PRE)
#define __APB2_HZ   (__HCLK_HZ) / (STM32_APB2_PRE)
#define __AHB_HZ 	(__HCLK_HZ) / (STM32_AHB_PRE)
#define __MCO_HZ 	(__HCLK_HZ) / (STM32_MCO_PRE)
/* The timer clock frequencies are automatically defined by hardware. There 
   are two cases:
   1. If the APB prescaler equals 1, the timer clock frequencies are 
   set to the same frequency as that of the APB domain.
   2. Otherwise, they are set to twice (×2) the frequency of the APB domain. */
#if (STM32_APB1_PRE == 1)
  #define __TIM1_HZ  (__APB1_HZ)
#else
  #define __TIM1_HZ  ((__APB1_HZ) * 2)
#endif

#if (STM32_APB2_PRE == 1)
  #define __TIM2_HZ  (__APB2_HZ)
#else
  #define __TIM2_HZ  ((__APB2_HZ) * 2)
#endif

/* -------------------------------------------------------------------- 
   Constants to be used by the libraries 
*/

const uint32_t stm32f_apb1_hz = __APB1_HZ;
const uint32_t stm32f_apb2_hz = __APB2_HZ;
const uint32_t stm32f_ahb_hz  = __AHB_HZ;
const uint32_t stm32f_mco_hz  = __MCO_HZ;
const uint32_t stm32f_tim1_hz = __TIM1_HZ;
const uint32_t stm32f_tim2_hz = __TIM2_HZ;
const uint32_t stm32f_sai_hz = __SAI_HZ;
const uint32_t stm32f_hsi_hz = STM32_HSI_HZ;
#if STM32_ENABLE_PLL
const uint32_t stm32f_vco_hz = __VCO_HZ;
#endif
#if STM32_ENABLE_PLLSAI
const uint32_t stm32f_vcosai_hz = __VCOSAI_HZ;
#endif

#ifndef THINKAPP

const uint32_t sysclk_hz[] = {
	[SYSCLK_STM32_AHB] = __AHB_HZ,
	[SYSCLK_STM32_APB1] = __APB1_HZ,
	[SYSCLK_STM32_APB2] = __APB2_HZ,
	[SYSCLK_STM32_TIM1] = __TIM1_HZ,
	[SYSCLK_STM32_TIM2] = __TIM2_HZ,
	[SYSCLK_STM32_MCO] = __MCO_HZ,
	[SYSCLK_STM32_SAI] = __SAI_HZ,
//	[SYSCLK_STM32_I2S] = __I2S_HZ
};

/* This constant is used to calibrate the systick timer */
const uint32_t cm3_systick_load_1ms = ((__HCLK_HZ / 8) / 1000) - 1;

void __attribute__((section(".init"))) _init(void)
{
	struct stm32_rcc * rcc = STM32_RCC;
	struct stm32_flash * flash = STM32_FLASH;
	uint32_t ccipr;
	uint32_t cr;
	uint32_t cfg;
#if STM32_ENABLE_PLL
	uint32_t pll;
#endif

	cr = RCC_MSION;
#if DEBUG
	rcc->cr = cr;
	rcc->cfgr = RCC_PPRE2_1 | RCC_PPRE1_1 | RCC_HPRE_1 | RCC_SW_MSI;
#endif

#if STM32_ENABLE_HSI
	/*******************************************************************
	 * Enable internal oscillator 
	 *******************************************************************/
	cr |= RCC_HSION;
	rcc->cr = cr;
	while (((cr = rcc->cr) & RCC_HSIRDY) == 0);
#endif

#if STM32_ENABLE_HSE
	/*******************************************************************
	 * Enable external oscillator 
	 *******************************************************************/
	cr |= RCC_HSEON;
	rcc->cr = cr;
	while (((cr = rcc->cr) & RCC_HSERDY) == 0);
#endif


#if STM32_ENABLE_PLL
	/*******************************************************************
	 * Configure PLL
	 *******************************************************************/
	pll = RCC_PLLPDIV(PLLPDIV) | 
		RCC_PLLR(PLLR) | RCC_PLLREN | 
		RCC_PLLQ(PLLQ) | RCC_PLLQEN |
		RCC_PLLN(PLLN) | RCC_PLLM(PLLM);
#if !STM32_ENABLE_PLLSAI
	/* Enable the PLLSAI2CLK if the SAI PLL is not enabled */
	pll |= RCC_PLLPEN;
#endif

#if STM32_ENABLE_HSI
	pll |= RCC_PLLSRC_HSI;
#else
	pll |= RCC_PLLSRC_HSE;
#endif
	rcc->pllcfgr = pll;
	/* enable PLL */
	cr |= RCC_PLLON;
	rcc->cr = cr;;
	while (((cr = rcc->cr) & RCC_PLLRDY) == 0);
#endif /* STM32_ENABLE_PLL */

	/* Configure MCO, APB and AHB clocks source and prescalers */
	cfg = RCC_MCOPRE(STM32_MCO_PRE) | RCC_MCO_PLL | 
		RCC_PPRE2(STM32_APB2_PRE) | 
		RCC_PPRE1(STM32_APB1_PRE) | 
		RCC_HPRE(STM32_AHB_PRE) | RCC_SW_MSI;
	rcc->cfgr = cfg; 

	/*******************************************************************
	 * Configure flash access and wait states 
	 *******************************************************************/
#if DEBUG	
	while ((flash->sr & FLASH_BSY) != 0);
#endif
	/* adjust flash wait states and enable caches */
	flash->acr = FLASH_DCEN | FLASH_ICEN | FLASH_PRFTEN | 
		FLASH_LATENCY(__HCLK_HZ / 30000000);

	if (flash->cr & FLASH_LOCK) {
		/* unlock flash write */
		flash->keyr = FLASH_KEY1;
		flash->keyr = FLASH_KEY2;
	}

#if STM32_ENABLE_PLL
	/* switch to pll oscillator */
	rcc->cfgr = (cfg & ~RCC_SW_MSK) | RCC_SW_PLL;
#if DEBUG
	while ((rcc->cfgr & RCC_SWS_MSK) != RCC_SWS_PLL);
#endif
	rcc->cr = cr & ~RCC_MSION;
#elif STM32_ENABLE_HSI
	/* select HSI as system clock */
	rcc->cfgr = (cfg & ~RCC_SW_MASK) | RCC_SW_HSI;
	rcc->cr = cr & ~RCC_MSION;
#elif STM32_ENABLE_HSE
	/* select HSE as system clock */
	rcc->cfgr = (cfg & ~RCC_SW_MASK) | RCC_SW_HSE;
	rcc->cr = cr & ~RCC_MSION;
#endif

#if STM32_ENABLE_PLLSAI
	/*******************************************************************
	 * Configure SAI PLL
	 *******************************************************************/
	rcc->pllsai1cfgr = RCC_PLLSAI1DIV(PLLSAIPDIV) | 
		RCC_PLLSAI1PEN | 
		RCC_PLLSAI1N(PLLSAIN);

	/* enable SAI1 PLL */
	cr |= RCC_PLLSAI1ON;
	rcc->cr = cr;;
	while (((cr = rcc->cr) & RCC_PLLSAI1RDY) == 0);
	ccipr = RCC_SAI1SEL_PLLSAI1;
#else
	ccipr = RCC_SAI1SEL_PLL;
#endif

	/*******************************************************************
	 * Configure USB clock
	 *******************************************************************/
	/* Enable HSI48 internal oscillator  */
	rcc->crrcr |= RCC_HSI48ON;
	while ((rcc->crrcr & RCC_HSI48RDY) == 0)
    /* HSI48 clock selected as 48 MHz clock */
	ccipr |= RCC_CLK48SEL_HSI48;

    /* Peripheral clock source selection */
	rcc->ccipr = ccipr; 

#ifdef CM3_RAM_VECTORS
	/* Remap the VECTOR table to SRAM 0x20000000  */
	CM3_SCB->vtor = 0x20000000; /* Vector Table Offset */
#endif
}

#endif /* THINKAPP */

