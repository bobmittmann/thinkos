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

#include <sys/stm32f.h>
#include <sys/halt.h>

#ifdef CONFIG_H
#include "config.h"
#endif

#ifndef STM32_ENABLE_HSI
#define STM32_ENABLE_HSI 1
#endif

#ifndef STM32_ENABLE_HSE
#define STM32_ENABLE_HSE 0
#endif

#ifndef STM32_ENABLE_PLL
#define STM32_ENABLE_PLL 1
#endif

#if defined(STM32L4X)

#ifndef HSE_HZ
  #define HSE_HZ 8000000
#endif

#ifndef HSI_HZ
  #define HSI_HZ 16000000
#endif

#ifndef PLL_HZ
  #define PLL_HZ 32000000
#endif

#ifndef HCLK_HZ
  #if STM32_ENABLE_PLL
    #define HCLK_HZ PLL_HZ
  #elif STM32_ENABLE_HSI
    #define HCLK_HZ HSI_HZ
  #elif STM32_ENABLE_HSE
    #define HCLK_HZ HSE_HZ
  #else
	#error "HCLK_HZ undefined!"
  #endif
#endif


#ifndef STM32_APB1_HZ
  #if STM32_ENABLE_PLL
    #define STM32_APB1_HZ (HCLK_HZ / 2)
  #else 
    #define STM32_APB1_HZ HCLK_HZ
  #endif
#endif

#ifndef STM32_APB2_HZ
  #if STM32_ENABLE_PLL
    #define STM32_APB2_HZ (HCLK_HZ / 2)
  #else 
    #define STM32_APB2_HZ HCLK_HZ
  #endif
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

#endif

#if (HCLK_HZ == 80000000)
#if (HSE_HZ == 11289600)
	/* F_HSE = 11.2896 MHz
	   F_VCO = 321.7535 MHz
	   F_MAIN = 80.4384 MHz */
  #define PLLN 57
  #define PLLM 2
  #define PLLR 4
  #define PLLP 4
  #define PLLQ 8
#elif (HSE_HZ == 16000000)
#elif (HSE_HZ == 12000000)
#elif (HSE_HZ == 8000000)
#else
#error "HSE_HZ invalid!"
#endif

#endif

#define __VCO_HZ (((uint64_t)HSE_HZ * PLLN) / PLLM)
#define __HCLK_HZ (__VCO_HZ / PLLR)

//#define __VCOI2S_HZ (((uint64_t)HSE_HZ * PLLI2SN) / PLLI2SM)
//#define __I2S_HZ (__VCOI2S_HZ / PLLI2SR)

const uint32_t stm32f_ahb_hz  = __HCLK_HZ;
const uint32_t stm32f_apb1_hz = __HCLK_HZ / 4;
const uint32_t stm32f_tim1_hz = __HCLK_HZ / 2;
const uint32_t stm32f_apb2_hz = __HCLK_HZ / 2;
const uint32_t stm32f_tim2_hz = __HCLK_HZ;

//const uint32_t stm32f_sai_hz = __I2S_HZ;

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
	uint32_t rcc_cr;
	uint32_t cfg;
	int again;
#if STM32_ENABLE_PLL
	uint32_t pll;
#endif
	uint32_t ws;

	rcc->cr = rcc_cr = RCC_MSION;
	rcc->cfgr = RCC_PPRE2_1 | RCC_PPRE1_1 | RCC_HPRE_1 | RCC_SW_MSI;

#if STM32_ENABLE_HSI
	/*******************************************************************
	 * Enable internal oscillator 
	 *******************************************************************/
	rcc_cr |= RCC_HSION;
	rcc->cr = rcc_cr;

	for (again = 8192; ; again--) {
		if ((rcc_cr = rcc->cr) & RCC_HSIRDY)
			break;
		if (again == 0)
			halt(); /* internal clock startup failed! */
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
		if (again == 0)
			halt(); /* external clock startup failed! */
	}
#endif


#if STM32_ENABLE_PLL
	/*******************************************************************
	 * Configure PLL
	 *******************************************************************/

	pll = RCC_PLLPDIV(PLLP) | 
		RCC_PLLR(PLLR) | RCC_PLLREN | 
		RCC_PLLQ(PLLQ) | RCC_PLLQEN |
		RCC_PLLN(PLLN) | RCC_PLLM(PLLM);
#if STM32_ENABLE_HSI
	pll |= RCC_PLLSRC_HSI;
#else
	pll |= RCC_PLLSRC_HSE;
#endif

	rcc->pllcfgr = pll;

	/* enable PLL */
	rcc_cr |= RCC_PLLON;
	rcc->cr = rcc_cr;;

	for (again = 8192; ; again--) {
		if ((rcc_cr = rcc->cr) & RCC_PLLRDY)
			break;
		if (again == 0)
			halt(); /* PLL lock failed! */
	}
#endif /* STM32_ENABLE_PLL */

	cfg = RCC_MCOPRE_1 | RCC_MCO_PLL | 
		RCC_PPRE2_1 | RCC_PPRE1_1 | RCC_HPRE_1 | RCC_SW_MSI;
	rcc->cfgr = cfg; 

	/*******************************************************************
	 * Configure flash access and wait states 
	 *******************************************************************/
	
	for (again = 4096; ; again--) {
		if ((flash->sr & FLASH_BSY) == 0)
			break;
		if (again == 0)
			halt(); /* flash not ready! */
	}

	ws = __HCLK_HZ / 30000000;
	/* adjust flash wait states and enable caches */
	flash->acr = FLASH_DCEN | FLASH_ICEN | FLASH_PRFTEN | FLASH_LATENCY(ws);


	if (flash->cr & FLASH_LOCK) {
		/* unlock flash write */
		flash->keyr = FLASH_KEY1;
		flash->keyr = FLASH_KEY2;
	}

#if STM32_ENABLE_PLL
	/* switch to pll oscillator */
	rcc->cfgr = (cfg & ~RCC_SW) | RCC_SW_PLL;
	while ((rcc->cfgr & RCC_SWS_MASK) != RCC_SWS_PLL);
#elif STM32_ENABLE_HSI
	/* select HSI as system clock */
	rcc->cfgr = (cfg & ~RCC_SW_MASK) | RCC_SW_HSI;
#elif STM32_ENABLE_HSE
	/* select HSE as system clock */
	rcc->cfgr = (cfg & ~RCC_SW_MASK) | RCC_SW_HSE;
#endif

	rcc->cr = rcc_cr & ~RCC_MSION;


	/*******************************************************************
	 * Configure USB and SAI clocks
	 *******************************************************************/

	/* Enable HSI48 internal oscillator  */
	rcc->crrcr |= RCC_HSI48ON;
	for (again = 8192; ; again--) {
		if (rcc->crrcr & RCC_HSI48RDY)
			break;
		if (again == 0)
			halt(); /* HSI48 internal clock startup failed! */
	}

    /* HSI48 clock selected as 48 MHz clock */
	rcc->ccipr = RCC_CLK48SEL_HSI48;

#ifdef CM3_RAM_VECTORS
	/* Remap the VECTOR table to SRAM 0x20000000  */
	CM3_SCB->vtor = 0x20000000; /* Vector Table Offset */
#endif
}

#endif /* THINKAPP */

