/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the libstm32f.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file stm32f-rcc.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __STM32F0_RCC_H__
#define __STM32F0_RCC_H__

/*-------------------------------------------------------------------------
  Reset and clock control (RCC)
  ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* RCC clock configuration register */

/* STM32F10x RCC clock configuration register */
#define STM32F_RCC_CFGR 0x04

/* PLLNODIV: Do not divide PLL to MCO 
   (in STM32F303x6/8 and STM32F328x8 , STM32F303xDxE and STM32F398xE only) */
#define RCC_PLLNODIV   (1 << 31)

#define RCC_MCOPRE     (0x7 << 28)
#define RCC_MCOPRE_1   (0x0 << 28)
#define RCC_MCOPRE_2   (0x1 << 28)
#define RCC_MCOPRE_4   (0x2 << 28)
#define RCC_MCOPRE_8   (0x3 << 28)
#define RCC_MCOPRE_16  (0x4 << 28)
#define RCC_MCOPRE_32  (0x5 << 28)
#define RCC_MCOPRE_64  (0x6 << 28)
#define RCC_MCOPRE_128 (0x7 << 28)

/* Microcontroller Clock Output Flag */
#define RCC_MCOF       (1 << 28)
/* MCOF: Microcontroller Clock Output Flag
   Set and reset by hardware.
   It is reset by hardware when MCO field is written with a new value
   It is set by hardware when the switch to the new MCO source is effective. */

/* Bits 26:24 MCO: Microcontroller clock output */
#define RCC_MCO_OFF    (0x0 << 24)
#define RCC_MCO_LSI    (0x2 << 24)
#define RCC_MCO_LSE    (0x3 << 24)
#define RCC_MCO_SYSCLK (0x4 << 24)
#define RCC_MCO_HSI    (0x5 << 24)
#define RCC_MCO_HSE    (0x6 << 24)
#define RCC_MCO_PLL    (0x7 << 24)
/* Set and cleared by software.
   000: MCO output disabled, no clock on MCO
   001: Reserved
   010: LSI clock selected.
   011: LSE clock selected.
   100: System clock (SYSCLK) selected
   101: HSI clock selected
   110: HSE clock selected
   111: PLL clock divided by 2 selected
   Note: This clock output may have some truncated cycles at startup or 
   during MCO clock source switching. */

/* Bit 23 I2SSRC: I2S external clock source selection */
#define RCC_I2SSRC    (1 << 23)
/* Set and reset by software to clock I2S2 and I2S3 with an external 
   clock. This bits must be valid before enabling I2S2-3 clocks.
   0: I2S2 and I2S3 clocked by system clock
   1: I2S2 and I2S3 clocked by the external clock */

/* Bit 22 USBPRE: USB prescaler */
#define RCC_USBPRE       (1 << 22)
#define RCC_USBPRE_1     (1 << 22)
#define RCC_USBPRE_1DOT5 (0 << 22)
/* Set and reset by software to generate 48 MHz USB clock. These bits must 
   be valid before enabling USB clocks.
   0: PLL clock is divided by 1.5
   1: PLL clock is not divided */

/* Bits 21:18 PLLMUL: PLL multiplication factor */
#define RCC_PLLMUL_MASK   (0x0f << 18)
#define RCC_PLLMUL(M)     ((((M) - 2) & 0x0f) << 18)

/* Bit 17 PLLXTPRE: HSE divider for PLL input clock */
#define RCC_PLLXTPRE      (1 << 17)
/* This bits is set and cleared by software to select the HSE division 
   factor for the PLL. It can be written only when the PLL is disabled.
   Note: This bit is the same as the LSB of PREDIV in Clock 
   configuration register 2 (RCC_CFGR2) (for compatibility with 
   other STM32 products)
   0000: HSE input to PLL not divided
   0001: HSE input to PLL divided by 2 */

/* PLLSRC: PLL entry clock source (STM32F303xD/E and STM32F398xE only) */
#define RCC_PLLSRC       (3 << 15)
#define RCC_PLLSRC_HSI_2 (0 << 15)
#define RCC_PLLSRC_HSI   (1 << 15)
#define RCC_PLLSRC_HSE   (2 << 15)
/* Set and cleared by software to select PLL clock source. 
   These bits can be written only when PLL is disabled.
   00: HSI/2 used as PREDIV1 entry and PREDIV1 forced to div by 2.
   01: HSI used as PREDIV1 entry.
   10: HSE used as PREDIV1 entry.
   11: Reserved. */


#define RCC_ADCPRE   (0x3 << 14)
#define RCC_ADCPRE_2 (0x0 << 14)
#define RCC_ADCPRE_4 (0x1 << 14)
#define RCC_ADCPRE_6 (0x2 << 14)
#define RCC_ADCPRE_8 (0x3 << 14)

/* Bits 13:11 PPRE2: APB high-speed prescaler (APB2) */
#define RCC_PPRE2    (0x7 << 11)
#define RCC_PPRE2_1  (0x0 << 11)
#define RCC_PPRE2_2  (0x4 << 11)
#define RCC_PPRE2_4  (0x5 << 11)
#define RCC_PPRE2_8  (0x6 << 11)
#define RCC_PPRE2_16 (0x7 << 11)
#define RCC_PPRE2_GET(CFGR) (((CFGR) >> 11) 0x7)

/* Bits 10:8 PPRE1:APB Low-speed prescaler (APB1) */
#define RCC_PPRE1    (0x7 << 8)
#define RCC_PPRE1_1  (0x0 << 8)
#define RCC_PPRE1_2  (0x4 << 8)
#define RCC_PPRE1_4  (0x5 << 8)
#define RCC_PPRE1_8  (0x6 << 8)
#define RCC_PPRE1_16 (0x7 << 8)
#define RCC_PPRE1_GET(CFGR) (((CFGR) >> 8) 0x7)

/* ------------------------------------------------------------------------- */
/* Clock interrupt register */
#define STM32F_RCC_CIR 0x08

/* ------------------------------------------------------------------------- */
/* APB2 peripheral reset register */
#define STM32F_RCC_APB2RSTR 0x0c

/* ------------------------------------------------------------------------- */
/* APB1 peripheral reset register */
#define STM32F_RCC_APB1RSTR 0x10

/* ------------------------------------------------------------------------- */
/* AHB peripheral clock enable register */
#define STM32F_RCC_AHBENR 0x14

#define RCC_TSC   24

#define RCC_IOPF  22
#define RCC_IOPE  21
#define RCC_IOPD  20
#define RCC_IOPC  19
#define RCC_IOPB  18
#define RCC_IOPA  17

#define RCC_CRC   6

#define RCC_FLITF 4

#define RCC_SRAM  2
#define RCC_DMA2  1
#define RCC_DMA1  0

#define RCC_GPIOF  RCC_IOPF
#define RCC_GPIOE  RCC_IOPE
#define RCC_GPIOD  RCC_IOPD
#define RCC_GPIOC  RCC_IOPC
#define RCC_GPIOB  RCC_IOPB
#define RCC_GPIOA  RCC_IOPA

/* Bit 24 - TSC clock enable */
#define RCC_TSCEN (1 << RCC_TSC)
/* Bit 22 - IO port F clock enable */
#define RCC_IOPFEN (1 << RCC_IOPF)
/* Bit 21 - IO port E clock enable */
#define RCC_IOPEEN (1 << RCC_IOPE)
/* Bit 20 - IO port D clock enable */
#define RCC_IOPDEN (1 << RCC_IOPD)
/* Bit 19 - IO port C clock enable */
#define RCC_IOPCEN (1 << RCC_IOPC)
/* Bit 18 - IO port B clock enable */
#define RCC_IOPBEN (1 << RCC_IOPB)
/* Bit 17 - IO port A clock enable */
#define RCC_IOPAEN (1 << RCC_IOPA)

/* Bit 6 - CRC clock enable */
#define RCC_CRCEN (1 << RCC_CRC)

/* Bit 4 - FLITF clock enable */
#define RCC_FLITFEN (1 << RCC_FLITF)

/* Bit 2 - SRAM interface clock enable */
#define RCC_SRAMEN (1 << RCC_SRAM)
/* Bit 1 - DMA2 clock enable */
#define RCC_DMA2EN (1 << RCC_DMA2)
/* Bit 0 - DMA1 clock enable */
#define RCC_DMA1EN (1 << RCC_DMA1)

/* APB2 peripheral clock enable register */
#define STM32F_RCC_APB2ENR 0x18

#define RCC_DBGMCU 22

#define RCC_TIM17  18
#define RCC_TIM16  17
#define RCC_TIM15  16

#define RCC_USART1 14

#define RCC_SPI1   12
#define RCC_TIM1   11

#define RCC_ADC     9

#define RCC_USART8  7
#define RCC_USART7  6
#define RCC_USART6  5

#define RCC_SYSCFG  0

/* Bit 22 - DBGMCU clock enable */
#define RCC_TIM11EN (1 << RCC_DBGMCU)
/* Bit 18 - TIM17 timer clock enable */
#define RCC_TIM17EN (1 << RCC_TIM17)
/* Bit 17 - TIM16 timer clock enable */
#define RCC_TIM16EN (1 << RCC_TIM16)
/* Bit 16 - TIM15 timer clock enable */
#define RCC_TIM15EN (1 << RCC_TIM15)
/* Bit 14 - USART1 clock enable */
#define RCC_USART1EN (1 << RCC_USART1)
/* Bit 12 - SPI1 clock enable */
#define RCC_SPI1EN (1 << RCC_SPI1)
/* Bit 11 - TIM1 timer clock enable */
#define RCC_TIM1EN (1 << RCC_TIM1)
/* Bit 9 - ADC interface clock enable */
#define RCC_ADCEN (1 << RCC_ADC)

/* Bit 7 - USART8 clock enable */
#define RCC_USART8EN (1 << RCC_USART8)
/* Bit 6 - USART7 clock enable */
#define RCC_USART7EN (1 << RCC_USART7)
/* Bit 5 - USART6 clock enable */
#define RCC_USART6EN (1 << RCC_USART6)

/* Bit 0 - System Config clock enable */
#define RCC_AFIOEN (1 << RCC_SYSCFG)

/* APB1 peripheral clock enable register */
#define STM32F_RCC_APB1ENR 0x1c

#define RCC_CEC    30
#define RCC_DAC    29
#define RCC_PWR    28
#define RCC_CRS    27

#define RCC_CAN    25

#define RCC_USB    23
#define RCC_I2C2   22
#define RCC_I2C1   21
#define RCC_USART5 20
#define RCC_USART4 19
#define RCC_USART3 18
#define RCC_USART2 17

#define RCC_SPI2   14

#define RCC_WWDG   11

#define RCC_TIM14   8

#define RCC_TIM7    5
#define RCC_TIM6    4
#define RCC_TIM3    1
#define RCC_TIM2    0

/* Bit 30 - CEC  clock enable */
#define RCC_CECEN (1 << RCC_CEC)
/* Bit 29 - DAC interface clock enable */
#define RCC_DACEN (1 << RCC_DAC)
/* Bit 28 - Power interface clock enable */
#define RCC_PWREN (1 << RCC_PWR)
/* Bit 27 - CRS clock enable */
#define RCC_CRSEN (1 << RCC_CRS)
/* Bit 25 - CAN clock enable */
#define RCC_CANEN (1 << RCC_CAN)
/* Bit 23 - USB clock enable */
#define RCC_USBEN (1 << RCC_USB)
/* Bit 22 - I2C2 clock enable */
#define RCC_I2C2EN (1 << RCC_I2C2)
/* Bit 21 - I2C1 clock enable */
#define RCC_I2C1EN (1 << RCC_I2C1)
/* Bit 20 - USART5 clock enable */
#define RCC_USART5EN (1 << RCC_USART5)
/* Bit 19 - USART4 clock enable */
#define RCC_USART4EN (1 << RCC_USART4)
/* Bit 18 - USART3 clock enable */
#define RCC_USART3EN (1 << RCC_USART3)
/* Bit 17 - USART2 clock enable */
#define RCC_USART2EN (1 << RCC_USART2)
/* Bit 14 - SPI2 clock enable */
#define RCC_SPI2EN (1 << RCC_SPI2)
/* Bit 11 - Window watchdog clock enable */
#define RCC_WWDGEN (1 << RCC_WWDG)
/* Bit 8 - TIM14 timer clock enable */
#define RCC_TIM14EN (1 << RCC_TIM14)
/* Bit 5 - TIM7 timer clock enable */
#define RCC_TIM7EN (1 << RCC_TIM7)
/* Bit 4 - TIM6 timer clock enable */
#define RCC_TIM6EN (1 << RCC_TIM6)
/* Bit 1 - TIM3 timer clock enable */
#define RCC_TIM3EN (1 << RCC_TIM3)
/* Bit 0 - TIM2 timer clock enable */
#define RCC_TIM2EN (1 << RCC_TIM2)

/* ------------------------------------------------------------------------- */
/* RCC Backup domain control register */
#define STM32F_RCC_BDCR 0x20

/* [31..17] Reserved, must be kept at reset value. */

/* Bit 16 - Backup domain software reset */
#define RCC_BDRST (1 << 16)
/* Set and cleared by software.
0: Reset not activated
1: Resets the entire Backup domain */

/* Bit 15 - RTC clock enable */
#define RCC_RTCEN (1 << 15)
/* Set and cleared by software.
0: RTC clock disabled
1: RTC clock enabled */

/* [14..10] Reserved, must be kept at reset value. */

/* Bits [9..8] - RTC clock source selection */
#define RCC_RTCSEL_MSK (((1 << (1 + 1)) - 1) << 8)
#define RCC_RTCSEL_SET(VAL) (((VAL) << 8) & RTCSEL_MSK)
#define RCC_RTCSEL_GET(REG) (((REG) & RTCSEL_MSK) >> 8)

#define RCC_RTCSEL_NONE (0 << 8)
#define RCC_RTCSEL_LSE  (1 << 8)
#define RCC_RTCSEL_LSI  (2 << 8)
#define RCC_RTCSEL_HSE  (3 << 8)

/* Set by software to select the clock source for the RTC. Once the RTC clock source has been
selected, it cannot be changed anymore unless the Backup domain is reset. The BDRST bit
can be used to reset them.
00: No clock
01: LSE oscillator clock used as RTC clock
10: LSI oscillator clock used as RTC clock
11: HSE oscillator clock divided by 32 used as RTC clock */

/* [7..5] Reserved, must be kept at reset value. */


/* Bits [4..3] - LSE oscillator drive capability */
#define RCC_LSEDRV ((4 - 3) << 3)
/* Set and reset by software to modulate the LSE oscillator’s drive 
   capability. A reset of the backup domain restores the default value.
   00: ‘Xtal mode’ lower driving capability
   01: ‘Xtal mode’ medium low driving capability
   10: ‘Xtal mode’ medium high driving capability
   11: ‘Xtal mode’ higher driving capability (reset value)
   Note: The oscillator is in Xtal mode when it is not in bypass mode. */

/* Bit 2 - LSE oscillator bypass */
#define RCC_LSEBYP (1 << 2)
/* Set and cleared by software to bypass oscillator in debug mode. This 
   bit can be written only when the external 32 kHz oscillator is disabled.
   0: LSE oscillator not bypassed
   1: LSE oscillator bypassed */

/* Bit 1 - LSE oscillator ready */
#define RCC_LSERDY (1 << 1)
/* Set and cleared by hardware to indicate when the external 32 kHz 
   oscillator is stable. After the LSEON bit is cleared, LSERDY goes low 
   after 6 external low-speed oscillator clock cycles.
   0: LSE oscillator not ready
   1: LSE oscillator ready */

/* Bit 0 - LSE oscillator enable */
#define RCC_LSEON (1 << 0)
/* Set and cleared by software.
   0: LSE oscillator OFF
   1: LSE oscillator ON */

/* ------------------------------------------------------------------------- */
/* Control/status register (RCC_CSR) */
#define STMF32_RCC_CSR 0x24

/* Bit 31 - Low-power reset flag */
#define RCC_LPWRSTF (1 << 31)
/* Set by hardware when a Low-power management reset occurs.
Cleared by writing to the RMVF bit.
0: No Low-power management reset occurred
1: Low-power management reset occurred
For further information on Low-power management reset, refer to Low-power management
reset. */

/* Bit 30 - Window watchdog reset flag */
#define RCC_WWDGRSTF (1 << 30)
/* Set by hardware when a window watchdog reset occurs.
Cleared by writing to the RMVF bit.
0: No window watchdog reset occurred
1: Window watchdog reset occurred */

/* Bit 29 - Independent window watchdog reset flag */
#define RCC_IWDGRSTF (1 << 29)
/* Set by hardware when an independent watchdog reset from VDD domain occurs.
Cleared by writing to the RMVF bit.
0: No watchdog reset occurred
1: Watchdog reset occurred */

/* Bit 28 - Software reset flag */
#define RCC_SFTRSTF (1 << 28)
/* Set by hardware when a software reset occurs.
Cleared by writing to the RMVF bit.
0: No software reset occurred
1: Software reset occurred */

/* Bit 27 - POR/PDR flag */
#define RCC_PORRSTF (1 << 27)
/* Set by hardware when a POR/PDR occurs.
Cleared by writing to the RMVF bit.
0: No POR/PDR occurred
1: POR/PDR occurred */

/* Bit 26 - PIN reset flag */
#define RCC_PINRSTF (1 << 26)
/* Set by hardware when a reset from the NRST pin occurs.
Cleared by writing to the RMVF bit.
0: No reset from NRST pin occurred
1: Reset from NRST pin occurred */

/* Bit 25 - Option byte loader reset flag */
#define RCC_OBLRSTF (1 << 25)
/* Set by hardware when a reset from the OBL occurs.
Cleared by writing to the RMVF bit.
0: No reset from OBL occurred
1: Reset from OBL occurred */

/* Bit 24 - Remove reset flag */
#define RCC_RMVF (1 << 24)
/* Set by software to clear the reset flags.
0: No effect
1: Clear the reset flags */

/* [23..2] Reserved, must be kept at reset value. */


/* Bit 1 - LSI oscillator ready */
#define RCC_LSIRDY (1 << 1)
/* Set and cleared by hardware to indicate when the LSI oscillator is stable. After the LSION bit
is cleared, LSIRDY goes low after 3 LSI oscillator clock cycles.
0: LSI oscillator not ready
1: LSI oscillator ready */

/* Bit 0 - LSI oscillator enable */
#define RCC_LSION (1 << 0)
/* Set and cleared by software.
0: LSI oscillator OFF
1: LSI oscillator ON */


/* ------------------------------------------------------------------------- */
/* AHB peripheral reset register (RCC_AHBRSTR) */
#define STMF32_RCC_AHBRSTR 0x28

/* Bit 22 - I/O port F reset */
#define RCC_IOPFRST (1 << 22)
/* Set and cleared by software.
0: No effect
1: Reset I/O port F */

/* Bit 21 - I/O port E reset */
#define RCC_OPERST (1 << 21)
/* Set and cleared by software.
0: No effect
1: Reset I/O port E */

/* Bit 20 - I/O port D reset */
#define RCC_IOPDRST (1 << 20)
/* Set and cleared by software.
0: No effect
1: Reset I/O port D */

/* Bit 19 - I/O port C reset */
#define RCC_IOPCRST (1 << 19)
/* Set and cleared by software.
0: No effect
1: Reset I/O port C */

/* Bit 18 - I/O port B reset */
#define RCC_IOPBRST (1 << 18)
/* Set and cleared by software.
0: No effect
1: Reset I/O port B */

/* Bit 17 - I/O port A reset */
#define RCC_IOPARST (1 << 17)
/* Set and cleared by software.
0: No effect
1: Reset I/O port A */

#define STM32_AHB  0
#define STM32_APB1 1
#define STM32_APB2 2

#define STM32_CLK_TSC     STM32_AHB, RCC_TSC

#define STM32_CLK_IOPF    STM32_AHB, RCC_IOPF
#define STM32_CLK_IOPE    STM32_AHB, RCC_IOPE
#define STM32_CLK_IOPD    STM32_AHB, RCC_IOPD
#define STM32_CLK_IOPC    STM32_AHB, RCC_IOPC
#define STM32_CLK_IOPB    STM32_AHB, RCC_IOPB
#define STM32_CLK_IOPA    STM32_AHB, RCC_IOPA


#define STM32_CLK_GPIOF    STM32_AHB, RCC_IOPF
#define STM32_CLK_GPIOE    STM32_AHB, RCC_IOPE
#define STM32_CLK_GPIOD    STM32_AHB, RCC_IOPD
#define STM32_CLK_GPIOC    STM32_AHB, RCC_IOPC
#define STM32_CLK_GPIOB    STM32_AHB, RCC_IOPB
#define STM32_CLK_GPIOA    STM32_AHB, RCC_IOPA


#define STM32_CLK_CRC     STM32_AHB, RCC_CRC
#define STM32_CLK_FLITF   STM32_AHB, RCC_FLITF
#define STM32_CLK_SRAM    STM32_AHB, RCC_SRAM
#define STM32_CLK_DMA2    STM32_AHB, RCC_DMA2
#define STM32_CLK_DMA1    STM32_AHB, RCC_DMA1


#define STM32_CLK_CEC     STM32_APB1, RCC_CEC
#define STM32_CLK_DAC     STM32_APB1, RCC_DAC
#define STM32_CLK_PWR     STM32_APB1, RCC_PWR
#define STM32_CLK_CRS     STM32_APB1, RCC_CRS
#define STM32_CLK_CAN     STM32_APB1, RCC_CAN
#define STM32_CLK_USB     STM32_APB1, RCC_USB
#define STM32_CLK_I2C2    STM32_APB1, RCC_I2C2
#define STM32_CLK_I2C1    STM32_APB1, RCC_I2C1
#define STM32_CLK_USART5  STM32_APB1, RCC_USART5
#define STM32_CLK_USART4  STM32_APB1, RCC_USART4
#define STM32_CLK_USART3  STM32_APB1, RCC_USART3
#define STM32_CLK_USART2  STM32_APB1, RCC_USART2
#define STM32_CLK_SPI2    STM32_APB1, RCC_SPI2
#define STM32_CLK_WWDG    STM32_APB1, RCC_WWDG
#define STM32_CLK_TIM14   STM32_APB1, RCC_TIM14
#define STM32_CLK_TIM7    STM32_APB1, RCC_TIM7
#define STM32_CLK_TIM6    STM32_APB1, RCC_TIM6
#define STM32_CLK_TIM3    STM32_APB1, RCC_TIM3
#define STM32_CLK_TIM2    STM32_APB1, RCC_TIM2

#define STM32_CLK_DBGMCU  STM32_APB2, RCC_DBGMCU
#define STM32_CLK_TIM17   STM32_APB2, RCC_TIM17
#define STM32_CLK_TIM16   STM32_APB2, RCC_TIM16
#define STM32_CLK_TIM15   STM32_APB2, RCC_TIM15
#define STM32_CLK_USART1  STM32_APB2, RCC_USART1
#define STM32_CLK_SPI1    STM32_APB2, RCC_SPI1
#define STM32_CLK_TIM1    STM32_APB2, RCC_TIM1

#define STM32_CLK_ADC     STM32_APB2, RCC_ADC
#define STM32_CLK_USART8  STM32_APB2, RCC_USART8
#define STM32_CLK_USART7  STM32_APB2, RCC_USART7
#define STM32_CLK_USART6  STM32_APB2, RCC_USART6
#define STM32_CLK_SYSCFG  STM32_APB2, RCC_SYSCFG

#define CLK_BUS(_BUS, _BIT) _BUS
#define CLK_BIT(_BUS, _BIT) _BIT

#ifndef __ASSEMBLER__

#include <stdint.h>

struct stm32_rcc {
	volatile uint32_t cr; /* Control Register */
	volatile uint32_t cfgr; 
	volatile uint32_t cir; 
	volatile uint32_t apb2rstr;

	volatile uint32_t apb1rstr;
	volatile uint32_t ahbenr;
	volatile uint32_t apb2enr;
	volatile uint32_t apb1enr;

	volatile uint32_t bdcr;
	volatile uint32_t csr;
	volatile uint32_t ahbrstr;
	volatile uint32_t cfgr2;

	volatile uint32_t cfgr3;
	volatile uint32_t cr2;
};

struct stm32_clk {
	uint8_t bus:3;
	uint8_t bit:5;
} __attribute__((packed));

/*---------------------------------------------------------------------
 * Clocks
 *---------------------------------------------------------------------*/
extern const uint32_t stm32f_ahb_hz;
extern const uint32_t stm32f_apb1_hz;
extern const uint32_t stm32f_apb2_hz;
extern const uint32_t stm32f_tim1_hz;
extern const uint32_t stm32f_tim2_hz;
extern const uint32_t stm32f_hse_hz;
extern const uint32_t stm32f_hsi_hz;
extern const uint32_t stm32f_vco_hz;
extern const uint32_t stm32f_sai_hz;
extern const uint32_t stm32f_vcosai_hz;
extern const uint32_t stm32f_i2s_hz;
extern const uint32_t stm32f_vcoi2s_hz;

#ifdef __cplusplus
extern "C" {
#endif

static inline void stm32_clk_enable(struct stm32_rcc * rcc, 
									int bus, int bit) {
	uint32_t volatile * enr;

	if (bus == STM32_APB2)
		enr = &rcc->apb2enr;
	else if (bus == STM32_APB1)
		enr = &rcc->apb1enr;
	else
		enr = &rcc->ahbenr;

	*enr |= 1 << bit;
	asm volatile ("dsb" : );
}

static inline void stm32_clk_disable(struct stm32_rcc * rcc, 
									 int bus, int bit) {
	uint32_t volatile * enr;

	if (bus == STM32_APB2)
		enr = &rcc->apb2enr;
	else if (bus == STM32_APB1)
		enr = &rcc->apb1enr;
	else
		enr = &rcc->ahbenr;

	*enr &= ~(1 << bit);
	asm volatile ("dsb" : );
}

static inline void stm32_reset(struct stm32_rcc * rcc, 
									int bus, int bit) {
	uint32_t volatile * rstr;

	if (bus == STM32_APB2)
		rstr = &rcc->apb2rstr;
	else if (bus == STM32_APB1)
		rstr = &rcc->apb1rstr;
	else
		rstr = &rcc->ahbrstr;

	*rstr |= 1 << bit;
	asm volatile ("dsb" : );
	*rstr &= ~(1 << bit);
	asm volatile ("dsb" : );
}

static inline uint32_t stm32_clk_hz(int bus, int bit) {
	if (bus == STM32_APB2)
		return stm32f_apb2_hz;
	if (bus == STM32_APB1)
		return stm32f_apb1_hz;
	else
		return stm32f_ahb_hz;
}

void stm32_rcc_i2s_pll_init(void);

#ifdef __cplusplus
}
#endif
#endif /* __ASSEMBLER__ */

#endif /* __STM32F0_RCC_H__ */

