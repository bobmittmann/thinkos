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

   HSE: High Speed External oscillator. When this clock is enabled
   it will be the reference for the PLL otherwise the internal 
   high speed oscillator HSI will be used.

   PLLSAI (Serial Audio Interface) - this PLL is  not present in all 
   chips.

   PLLI2S (Inter-IC Sound) - Not present in all chips.
*/

/* Enable HSE oscillator */
#ifndef STM32_ENABLE_HSE
#define STM32_ENABLE_HSE    1
#endif

/* Enable main PLL */
#ifndef STM32_ENABLE_PLL
#define STM32_ENABLE_PLL    1
#endif

/* Use HSE for main PLL input. */
#ifndef STM32_PLL_CKIN_HSE
#define STM32_PLL_CKIN_HSE  1
#endif

/* Use HSI for main PLL input. */
#ifndef STM32_PLL_CKIN_HSI
#define STM32_PLL_CKIN_HSI  0
#endif

#ifndef STM32_ENABLE_PLLSAI
#define STM32_ENABLE_PLLSAI 0
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

#if (STM32_HCLK_HZ == 168000000) && (STM32_HSE_HZ == 25000000)
	/* F_HSE = 25 MHz
	   F_VCO = 335.9375 MHz (F_HSE * 215 / 16)
	   F_MAIN = 167.96875 MHz (F_VCO / 2)
	   F_USB = 47.991 MHz (F_VCO / 7) */
  #define PLLQ 7
  #define PLLP 2
  #define PLLN 215
  #define PLLM 16
#endif

#if (STM32_HCLK_HZ == 168000000) && (STM32_HSE_HZ == 16000000)
	/* F_HSE = 16 MHz
	   F_VCO = 336 MHz (F_HSE * 42)
	   F_MAIN = 168 MHz (F_VCO / 2)
	   F_USB = 48 MHz (F_VCO / 7)*/
  #define PLLQ 7
  #define PLLP 2
  #define PLLN 168
  #define PLLM 8
#endif

#if (STM32_HCLK_HZ == 168000000) && (STM32_HSE_HZ == 12000000)
	/* F_HSE = 12 MHz
	   F_VCO = 336 MHz (F_HSE * 112 / 4)
	   F_MAIN = 168 MHz (F_VCO / 2)
	   F_USB = 48 MHz (F_VCO / 7) */
  #define PLLQ 7
  #define PLLP 2
  #define PLLN 112
  #define PLLM 4
  #if defined(STM32F446)
    #define PLLR 2
  #else
    #define PLLR 0
  #endif
    /* F_VCO = 389454513
	   F_I2S = 169333333 */
  #define PLLI2SR 3
  #define PLLI2SN 127
  #define PLLI2SM 9
  #define PLLI2SQ 8
  #define PLLI2SP 2
#endif

#if (STM32_HCLK_HZ == 168000000) && (STM32_HSE_HZ == 8000000)
	/* F_HSE = 8 MHz
	   F_VCO = 336 MHz (F_HSE * 42)
	   F_MAIN = 168 MHz (F_VCO / 2)
	   F_USB = 48 MHz (F_VCO / 7)*/
  #define PLLQ 7
  #define PLLP 2
  #define PLLN 168
  #define PLLM 4
  #define PLLR 0
#endif

#if (STM32_HCLK_HZ == 120000000) && (STM32_HSE_HZ == 24000000)
	/* F_HSE = 24 MHz
	   F_VCO = 240 MHz
	   F_MAIN = 120 MHz
	   F_USB = 48 MHz */
  #define PLLQ 5
  #define PLLP 2
  #define PLLN 120
  #define PLLM 12
  #define PLLR 0
#endif

#if (STM32_HCLK_HZ == 120000000) && (STM32_HSE_HZ == 12000000)
	/* F_HSE = 24 MHz
	   F_VCO = 240 MHz
	   F_MAIN = 120 MHz
	   F_USB = 48 MHz */
  #define PLLQ 5
  #define PLLP 2
  #define PLLN 240
  #define PLLM 12
  #define PLLR 0
#endif

#if (STM32_HCLK_HZ == 192000000) && (STM32_HSE_HZ == 25000000)
	/* F_HSE = 25 MHz
	   F_VCO = 383.9286 MHz (F_HSE * 215 / 14)
	   F_MAIN = 191.964 MHz (F_VCO / 2)
	   F_USB = 47.991 MHz (F_VCO / 7) */
  #define PLLQ 8
  #define PLLP 2
  #define PLLN 215
  #define PLLM 14
  #define PLLR 0
#endif

#if (STM32_HCLK_HZ == 180006400) && (STM32_HSE_HZ == 11289600)
  /* PLLVCO = 360.012800 MHz */
  #define PLLM         9
  #define PLLN       287
  #define PLLP         2 /*     PLLCLK -> 180.006400 MHz */
  #define PLLR         2 /*       PLLR ->   0.000002 MHz */
  #define PLLQ         8 /* PLL48M1CLK ->  45.001600 MHz */
  #define PLLPDIV      4 /* PLLSAI2CLK ->  90.003200 MHz */

  /* PLLI2SVCO = 180.633600 MHz */
  #define PLLI2SM      6
  #define PLLI2SN     96
  #define PLLI2SQ      2 /* PLLSAI3CLK ->  90.316800 MHz */
  #define PLLI2SP      8 /*  SPDIF2CLK ->  22.579200 MHz */
  #define PLLI2SR      2 /* PLLI2S1CLK ->  90.316800 MHz */

  /* PLLSAIVCO = 191.923200 MHz */
  #define PLLSAIM      6
  #define PLLSAIN    102
  #define PLLSAIQ      2 /* PLLSAI2CLK ->  95.961600 MHz */
  #define PLLSAIP      4 /* PLL48M2CLK ->  47.980800 MHz */
#endif

#if STM32_PLL_CKIN_HSE && !STM32_ENABLE_HSE
#error "invalid PLL HSE input clock configuration!"
#endif

#if STM32_PLL_CKIN_HSE
  #define __PLL_CKIN_HZ (STM32_HSE_HZ)
#elif STM32_PLL_CKIN_HSI
  #define __PLL_CKIN_HZ (STM32_HSI_HZ)
#else 
#error "invalid PLL input clock configuration!"
#endif

#define VCO_HZ ((STM32_HCLK_HZ) * (PLLP))
#define USB_HZ 48000000

/* Validate the PLL configuration */
#if ((__PLL_CKIN_HZ * PLLN) / PLLM) > ((VCO_HZ) + (VCO_HZ) / 1000)
#error "invalid PLL configuration (err > 0.1 %)!"
#endif

#if ((__PLL_CKIN_HZ * PLLN) / PLLM) < ((VCO_HZ) - (VCO_HZ) / 1000)
#error "invalid PLL configuration (err < -0.1 %)!"
#endif

/* FIXME: multible USB sources */
#if !defined(STM32F446)
#if (VCO_HZ / PLLQ) > ((USB_HZ) + (USB_HZ) / 1000) 
#error "invalid PLL configuration (err > 0.1 %)!"
#endif

#if (VCO_HZ / PLLQ) < ((USB_HZ) - (USB_HZ) / 1000) 
#error "invalid PLL configuration (err < -0.1 %)!"
#endif
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

#endif /* STM32F446 */

#if STM32_ENABLE_PLL
  #define __VCO_HZ (((uint64_t)(__PLL_CKIN_HZ) * PLLN) / PLLM)
  #define __HCLK_HZ (__VCO_HZ / PLLP)
#elif STM32_ENABLE_HSE
  #define __HCLK_HZ STM32_HSE_HZ
#else
  #define __HCLK_HZ STM32_HSI_HZ
#endif

#if STM32_ENABLE_PLLSAI
  /* Enable the SAI PLL, the SAI PLL is PLLSAI1CLK */
  #define __VCOSAI_HZ (((uint64_t)__PLL_CKIN_HZ * PLLSAIN) / PLLSAIM)
  #define __SAI_HZ (__VCOSAI_HZ / PLLSAIQ)
#else
  #if STM32_ENABLE_PLL
    /* The SAI PLL is disabled, the SAI PLL is PLLSAI2CLK */
	/* FIXME: PLLPDIV undefined
    #define __SAI_HZ (__VCO_HZ / PLLPDIV)
 	*/
  #else
    #define __SAI_HZ __PLL_CKIN_HZ 
  #endif
#endif

#if STM32_ENABLE_PLLI2S
  #define __VCOI2S_HZ (((uint64_t)__PLL_CKIN_HZ * PLLI2SN) / PLLI2SM)
  #define __I2S_HZ (__VCOI2S_HZ / PLLI2SR)
#else
  #if STM32_ENABLE_PLL
    /* The SAI PLL is disabled, the SAI PLL is PLLSAI2CLK */
	/* FIXME: PLLPDIV undefined
    #define __SAI_HZ (__VCO_HZ / PLLPDIV)
 	*/
  #else
    #define __I2S_HZ __PLL_CKIN_HZ 
  #endif
#endif

const uint32_t stm32f_ahb_hz  = __HCLK_HZ;
const uint32_t stm32f_apb1_hz = __HCLK_HZ / 4;
const uint32_t stm32f_tim1_hz = __HCLK_HZ / 2;
const uint32_t stm32f_apb2_hz = __HCLK_HZ / 2;
const uint32_t stm32f_tim2_hz = __HCLK_HZ;
const uint32_t stm32f_hse_hz = STM32_HSE_HZ;
const uint32_t stm32f_hsi_hz = STM32_HSI_HZ;
#ifdef __VCO_HZ
const uint32_t stm32f_vco_hz = __VCO_HZ;
#endif
#ifdef  __SAI_HZ
const uint32_t stm32f_sai_hz = __SAI_HZ;
#endif
#ifdef __VCOSAI_HZ
const uint32_t stm32f_vcosai_hz = __VCOSAI_HZ;
#endif
#ifdef __I2S_HZ
const uint32_t stm32f_i2s_hz = __I2S_HZ;
#endif
#ifdef __VCOI2S_HZ
const uint32_t stm32f_vcoi2s_hz = __VCOI2S_HZ;
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
	uint32_t cr = 0;
	uint32_t cfg;

	/* Make sure we are using the internal oscillator */
	rcc->cr = RCC_HSION;
	while (((cr = rcc->cr) & RCC_HSIRDY) == 0);
	cfg = RCC_PPRE2_1 | RCC_PPRE1_1 | RCC_HPRE_1 | RCC_SW_HSI;
	rcc->cfgr = cfg;

#if defined(STM32F446)
	rcc->sscgr = 0;
	rcc->pllsaicfgr = 0;
	rcc->ckgatenr = 0xffffffff;
	rcc->dckcfgr2 = RCC_CK48MSEL_PLLSAI_P;
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
#if STM32_PLL_CKIN_HSE
	rcc->pllcfgr = RCC_PLLR(PLLR) | RCC_PLLQ(PLLQ) | RCC_PLLSRC_HSE | 
		RCC_PLLP(PLLP) | RCC_PLLN(PLLN) | RCC_PLLM(PLLM);
#else 
	rcc->pllcfgr = RCC_PLLR(PLLR) | RCC_PLLQ(PLLQ) | RCC_PLLSRC_HSI | 
		RCC_PLLP(PLLP) | RCC_PLLN(PLLN) | RCC_PLLM(PLLM);
#endif
	/* enable PLL */
	cr |= RCC_PLLON;
	rcc->cr = cr;
	while (((cr = rcc->cr) & RCC_PLLRDY) == 0);
#endif

#if STM32_PLL_CKIN_HSE
	/* switch to external clock */
	cfg = RCC_MCO2_SYSCLK | RCC_MCO2PRE_2 /* Clock output 2 */
		| RCC_PPRE2_2 /* APB high speed prescaler : 60|84MHz */
		| RCC_PPRE1_4 /* APB low speed prescaler : 30|42MHz */
		| RCC_HPRE_1 /* AHB prescaler : 120|168MHz */ 
		| RCC_SW_HSE;
	rcc->cfgr = cfg;
#else
	/* keep internal clock and update prescalers */
	cfg = RCC_MCO2_SYSCLK | RCC_MCO2PRE_2 /* Clock output 2 */
		| RCC_PPRE2_2 /* APB high speed prescaler : 60|84MHz */
		| RCC_PPRE1_4 /* APB low speed prescaler : 30|42MHz */
		| RCC_HPRE_1 /* AHB prescaler : 120|168MHz */ 
		| RCC_SW_HSI;
	rcc->cfgr = cfg;
#endif

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
	rcc->cfgr = (cfg & ~RCC_SW) | RCC_SW_PLL;
#elif STM32_ENABLE_HSE
	/* select HSE as system clock */
	rcc->cfgr = (cfg & ~RCC_SW) | RCC_SW_HSE;
#else
	/* select HSI as system clock */
	rcc->cfgr = (cfg & ~RCC_SW) | RCC_SW_HSI;
#endif

#if STM32_ENABLE_PLLI2S
	/* configure IS2 PLL */
	rcc->plli2scfgr =  RCC_PLLI2SR(PLLI2SR) | RCC_PLLI2SQ(PLLI2SQ) | 
		RCC_PLLI2SP(PLLI2SP) | RCC_PLLI2SN(PLLI2SN) | RCC_PLLI2SM(PLLI2SM);

	/* enable I2SPLL */
	cr |= RCC_PLLI2SON;
	rcc->cr = cr;;

	while (((cr = rcc->cr) & RCC_PLLI2SRDY) == 0);

	rcc->dckcfgr = I2S2SRC_PLLI2S_R | I2S1SRC_PLLI2S_R;
#endif

#if STM32_ENABLE_PLLSAI
	/* configure SAI PLL */
	rcc->pllsaicfgr =  RCC_PLLSAIQ(PLLSAIQ) | RCC_PLLSAIP(PLLSAIP) |
		RCC_PLLSAIN(PLLSAIN) | RCC_PLLSAIM(PLLSAIM);

	/* enable SAISPLL */
	cr |= RCC_PLLSAION;
	rcc->cr = cr;;

	while (((cr = rcc->cr) & RCC_PLLSAIRDY) == 0);
#endif

#ifdef CM3_RAM_VECTORS
	/* Remap the VECTOR table to SRAM 0x20000000  */
	CM3_SCB->vtor = 0x20000000; /* Vector Table Offset */
#endif

}

#endif /* THINKAPP */

