/* 
 * File:	stm32l1-init.c
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
#define STM32_ENABLE_PLL 0
#endif

/* Set default values for system clocks */
#ifndef STM32_HCLK_HZ 
#define STM32_HCLK_HZ 80000000
#endif

#ifndef STM32_HSE_HZ
#define STM32_HSE_HZ 8000000
#endif

#ifndef STM32_HSI_HZ
#define STM32_HSI_HZ 16000000
#endif

#define STM32_APB1_PRE 1
#define STM32_APB2_PRE 1
#define STM32_MCO_DIV 1

/* --------------------------------------------------------------------
   Precalculated PLL configuartion
*/

#if (STM32_HCLK_HZ == 16000000) && (STM32_HSE_HZ == 8000000)
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

#if (STM32_ENABLE_PLL)
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
#define __MCO_HZ 	(__HCLK_HZ) / (STM32_MCO_DIV)

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


#if STM32_APB1_HZ == HCLK_HZ
#define STM32_TIM1_HZ STM32_APB1_HZ
#else
#define STM32_TIM1_HZ (2 * STM32_APB1_HZ)
#endif

#if STM32_APB2_HZ == HCLK_HZ
#define STM32_TIM2_HZ STM32_APB2_HZ
#else
#define STM32_TIM2_HZ (2 * STM32_APB2_HZ)
#endif
/* -------------------------------------------------------------------- 
   Constants to be used by the libraries 
*/

const uint32_t stm32f_apb1_hz = __APB1_HZ;
const uint32_t stm32f_apb2_hz = __APB2_HZ;
//const uint32_t stm32f_ahb_hz  = __AHB_HZ;
const uint32_t stm32f_mco_hz  = __MCO_HZ;
const uint32_t stm32f_tim1_hz = __TIM1_HZ;
const uint32_t stm32f_tim2_hz = __TIM2_HZ;
const uint32_t stm32f_sai_hz = __SAI_HZ;
const uint32_t stm32f_hse_hz = STM32_HSE_HZ;
const uint32_t stm32f_hsi_hz = STM32_HSI_HZ;
#if STM32_ENABLE_PLL
const uint32_t stm32f_vco_hz = __VCO_HZ;
#endif

#ifndef THINKAPP

const uint32_t sysclk_hz[] = {
	[SYSCLK_STM32_APB1] = __APB1_HZ,
	[SYSCLK_STM32_APB2] = __APB2_HZ,
	[SYSCLK_STM32_TIM1] = __TIM1_HZ,
	[SYSCLK_STM32_TIM2] = __TIM2_HZ,
};

/* This constant is used to calibrate the systick timer */
const uint32_t cm3_systick_load_1ms = ((__HCLK_HZ / 8) / 1000) - 1;

void __attribute__((section(".init"))) _init(void)
{
	struct stm32_rcc * rcc = STM32_RCC;
	struct stm32_pwr * pwr = STM32_PWR;
	struct stm32_flash * flash = STM32_FLASH;
	uint32_t rcc_cr;
	uint32_t csr;
	uint32_t acr;
	int again;
#if STM32_ENABLE_PLL
	uint32_t cfg;
#endif

	rcc->cr = rcc_cr = RCC_MSION;
	rcc->cfgr = RCC_PPRE2_1 | RCC_PPRE1_1 | RCC_HPRE_1 | RCC_SW_MSI;

	/*******************************************************************
	 * Wait for voltage rises to 3.1 volts enabling full power
	 * operation.
	 *******************************************************************/
	pwr->cr = PWR_VOS_1_5V | PWR_PLS_3_1V | PWR_PVDE;
	for (again = 8192; ; again--) {
		if (((csr = pwr->csr) & PWR_PVDO) == 0)
			break;
	}

	/*******************************************************************
	 * Adjust core voltage regulator
	 *******************************************************************/
	pwr->cr = PWR_VOS_1_8V;
	/* wait for voltage to stabilize */
	for (again = 8192; ; again--) {
		if (((csr = pwr->csr) & PWR_VOSF) == 0)
			break;
	}

#if STM32_ENABLE_HSI
	/*******************************************************************
	 * Enable internal oscillator 
	 *******************************************************************/
	rcc_cr |= RCC_HSION;
	rcc->cr = rcc_cr;

	for (again = 8192; ; again--) {
		if ((rcc_cr = rcc->cr) & RCC_HSIRDY)
			break;
	}
#endif

#if STM32_ENABLE_HSE
	/*******************************************************************
	 * Enable external oscillator 
	 *******************************************************************/
	rcc_cr |= RCC_HSEON;
	rcc->cr = rcc_cr;

	for (again = 8192; ; again--) {
		if ((rcc_cr = rcc->cr) & RCC_HSERDY)
			break;
	}
#endif


#if STM32_ENABLE_PLL
	/*******************************************************************
	 * Configure PLL
	 *******************************************************************/
#if STM32_ENABLE_HSI
	/* F_HSI = 16 MHz
	   VCOCLK = 64 MHz
	   PLLCLK = 32 MHz
	   SYSCLK = 32 MHz
	   PCLK1 = 8 MHz
	   PCLK2 = 8 MHz */
	cfg = RCC_PLLDIV_2 | RCC_PLLMUL_4 | RCC_PLLSRC_HSI | 
		RCC_PPRE2_1 | RCC_PPRE1_1 | RCC_HPRE_2 | RCC_SW_MSI;
#else

	/* F_HSE = 8 MHz
	   VCOCLK = 32 MHz
	   PLLCLK = 64 MHz
	   SYSCLK = 32 MHz
	   PCLK1 = 8 MHz
	   PCLK2 = 8 MHz */
	cfg = RCC_PLLDIV_2 | RCC_PLLMUL_8 | RCC_PLLSRC_HSE | 
		RCC_PPRE2_2 | RCC_PPRE1_2 | RCC_HPRE_2 | RCC_SW_MSI;
#endif
	rcc->cfgr = cfg;

	/* enable PLL */
	rcc_cr |= RCC_PLLON;
	rcc->cr = rcc_cr;

	for (again = 8192; ; again--) {
		if ((rcc_cr = rcc->cr) & RCC_PLLRDY)
			break;
	}
#endif /* STM32_ENABLE_PLL */

	/*******************************************************************
	 * Configure flash access and wait states 
	 *******************************************************************/
	flash->acr = FLASH_ACC64;
	
	for (again = 8192; ; again--) {
		acr = flash->acr;
		if (acr & FLASH_ACC64)
			break;
	}
	
#if STM32_ENABLE_PLL
	flash->acr = FLASH_ACC64 | FLASH_PRFTEN | FLASH_LATENCY;
#else
//	flash->acr = FLASH_ACC64 | FLASH_PRFTEN;
	flash->acr = FLASH_ACC64 | FLASH_PRFTEN | FLASH_LATENCY;
#endif

	for (again = 8192; ; again--) {
		acr = flash->acr;
		if (acr & FLASH_LATENCY)
			break;
	}

#if STM32_ENABLE_PLL
	/* switch to PLL oscillator */
	rcc->cfgr = (cfg & ~RCC_SW) | RCC_SW_PLL;
	rcc->cr = rcc_cr & ~RCC_MSION;
#elif STM32_ENABLE_HSI
	/* select HSI as system clock */
	rcc->cfgr = RCC_PPRE2_1 | RCC_PPRE1_1 | RCC_HPRE_1 | RCC_SW_HSI;
	rcc->cr = rcc_cr & ~RCC_MSION;
#elif STM32_ENABLE_HSE
	/* select HSE as system clock */
	rcc->cfgr = RCC_PPRE2_1 | RCC_PPRE1_1 | RCC_HPRE_1 | RCC_SW_HSE;
	rcc->cr = rcc_cr & ~RCC_MSION;
#endif

}

#endif /* THINKAPP */

