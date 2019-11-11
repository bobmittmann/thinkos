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
 * @file stm32l4-rcc.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#ifndef ILOG2
#define ILOG2(X) (((X)&(0xffff0000))?(((X)&(0xff000000))?\
(((X)&(0xf0000000))?(((X)&(0xc0000000))?((X)&(0x80000000)?31:30):\
((X)&(0x20000000)?29:28)):(((X)&(0x0c000000))?((X)&(0x08000000)?27:26):\
((X)&(0x02000000)?25:24))):(((X)&(0x00f00000))?(((X)&(0x00c00000))?\
((X)&(0x00800000)?23:22):((X)&(0x00200000)?21:20)):\
(((X)&(0x000c0000))?((X)&(0x00080000)?19:18):((X)&(0x00020000)?17:16)))):\
(((X)&(0x0000ff00))?(((X)&(0x0000f000))?(((X)&(0x0000c000))?\
((X)&(0x00008000)?15:14):((X)&(0x00002000)?13:12)):\
(((X)&(0x00000c00))?((X)&(0x00000800)?11:10):((X)&(0x00000200)?9:8))):\
(((X)&(0x000000f0))?(((X)&(0x000000c0))?((X)&(0x00000080)?7:6):\
((X)&(0x00000020)?5:4)):(((X)&(0x0000000c))?((X)&(0x00000008)?3:2):\
((X)&(0x00000002)?1:0)))))
#endif

/* ------------------------------------------------------------------------- */
/* RCC - Reset and clock control */
/* ------------------------------------------------------------------------- */

#ifndef __STM32L4_RCC_H__
#define __STM32L4_RCC_H__

/* ------------------------------------------------------------------------- */
/* Clock control register - CR */
#define STM32_RCC_CR 0x0000

/* Bits [28..31] - Reserved, must be kept at reset value. */

/* Bit 27 - SAI1 PLL clock ready flag */
#define RCC_PLLSAI1RDY (1 << 27)
/* Set by hardware to indicate that the PLLSAI1 is locked.
   0: PLLSAI1 unlocked
   1: PLLSAI1 locked */

/* Bit 26 - SAI1 PLL enable */
#define RCC_PLLSAI1ON (1 << 26)
/* Set and cleared by software to enable PLLSAI1.
   Cleared by hardware when entering Stop, Standby or Shutdown mode.
   0: PLLSAI1 OFF
   1: PLLSAI1 ON */

/* Bit 25 - Main PLL clock ready flag */
#define RCC_PLLRDY (1 << 25)
/* Set by hardware to indicate that the main PLL is locked.
   0: PLL unlocked
   1: PLL locked */

/* Bit 24 - Main PLL enable */
#define RCC_PLLON (1 << 24)
/* Set and cleared by software to enable the main PLL.
   Cleared by hardware when entering Stop, Standby or Shutdown mode. This bit
   cannot be reset if the PLL clock is used as the system clock.
   0: PLL OFF
   1: PLL ON */

/* Bits [20..23] - Reserved, must be kept at reset value. */

/* Bit 19 - Clock security system enable */
#define RCC_CSSON (1 << 19)
/* Set by software to enable the clock security system. When CSSON is set, the
   clock detector is enabled by hardware when the HSE oscillator is ready, and
   disabled by hardware if a HSE clock failure is detected. This bit is set
   only and is cleared by reset.
   0: Clock security system OFF (clock detector OFF)
   1: Clock security system ON (Clock detector ON if the HSE oscillator is
   stable, OFF if not). */

/* Bit 18 - HSE crystal oscillator bypass */
#define RCC_HSEBYP (1 << 18)
/* Set and cleared by software to bypass the oscillator with an external
   clock. The external clock must be enabled with the HSEON bit set, to be
   used by the device. The HSEBYP bit can be written only if the HSE
   oscillator is disabled.
   0: HSE crystal oscillator not bypassed
   1: HSE crystal oscillator bypassed with external clock */

/* Bit 17 - HSE clock ready flag */
#define RCC_HSERDY (1 << 17)
/* Set by hardware to indicate that the HSE oscillator is stable.
   0: HSE oscillator not ready
   1: HSE oscillator ready
   Note: Once the HSEON bit is cleared, HSERDY goes low after 6 HSE clock
   cycles. */

/* Bit 16 - HSE clock enable */
#define RCC_HSEON (1 << 16)
/* Set and cleared by software.
   Cleared by hardware to stop the HSE oscillator when entering Stop, Standby
   or Shutdown mode. This bit cannot be reset if the HSE oscillator is used
   directly or indirectly as the system clock.
   0: HSE oscillator OFF
   1: HSE oscillator ON */

/* Bits [12..15] - Reserved, must be kept at reset value. */

/* Bit 11 - HSI16 automatic start from Stop */
#define RCC_HSIASFS (1 << 11)
/* Set and cleared by software. When the system wakeup clock is MSI, this bit
   is used to wakeup the HSI16 is parallel of the system wakeup.
   0: HSI16 oscillator is not enabled by hardware when exiting Stop mode with
   MSI as wakeup clock.
   1: HSI16 oscillator is enabled by hardware when exiting Stop mode with MSI
   as wakeup clock. */

/* Bit 10 - HSI16 clock ready flag */
#define RCC_HSIRDY (1 << 10)
/* Set by hardware to indicate that HSI16 oscillator is stable. This bit is
   set only when HSI16 is enabled by software by setting HSION.
   0: HSI16 oscillator not ready
   1: HSI16 oscillator ready
   Note: Once the HSION bit is cleared, HSIRDY goes low after 6 HSI16 clock
   cycles. */

/* Bit 9 - HSI16 always enable for peripheral kernels. */
#define RCC_HSIKERON (1 << 9)
/* Set and cleared by software to force HSI16 ON even in Stop modes. The HSI16
   can only feed USARTs and I2Cs peripherals configured with HSI16 as kernel
   clock. Keeping the HSI16 ON in Stop mode allows to avoid slowing down the
   communication speed because of the HSI16 startup time. This bit has no
   effect on HSION value.
   0: No effect on HSI16 oscillator.
   1: HSI16 oscillator is forced ON even in Stop mode. */

/* Bit 8 - HSI16 clock enable */
#define RCC_HSION (1 << 8)
/* Set and cleared by software.
   Cleared by hardware to stop the HSI16 oscillator when entering Stop,
   Standby or Shutdown mode.
   Set by hardware to force the HSI16 oscillator ON when STOPWUCK=1 or HSIASFS
   = 1 when leaving Stop modes, or in case of failure of the HSE crystal
   oscillator.
   This bit is set by hardware if the HSI16 is used directly or indirectly as
   system clock.
   0: HSI16 oscillator OFF
   1: HSI16 oscillator ON */

/* Bits [4..7] - MSI clock ranges */
#define RCC_MSIRANGE_MSK (0xf << 4)
#define RCC_MSIRANGE_SET(VAL) (((VAL) << 4) & RCC_MSIRANGE_MSK)
#define RCC_MSIRANGE_GET(REG) (((REG) & RCC_MSIRANGE_MSK) >> 4)
/* These bits are configured by software to choose the frequency range of MSI
   when MSIRGSEL is set.12 frequency ranges are available:
   0000: range 0 around 100 kHz
   0001: range 1 around 200 kHz
   0010: range 2 around 400 kHz
   0011: range 3 around 800 kHz
   0100: range 4 around 1M Hz
   0101: range 5 around 2 MHz
   0110: range 6 around 4 MHz (reset value)
   0111: range 7 around 8 MHz
   1000: range 8 around 16 MHz
   1001: range 9 around 24 MHz
   1010: range 10 around 32 MHz
   1011: range 11 around 48 MHz
   others: not allowed (hardware write protection)
   Note: Warning: MSIRANGE can be modified when MSI is OFF (MSION=0) or when
   MSI is ready (MSIRDY=1). MSIRANGE must NOT be modified when MSI is ON and
   NOT ready (MSION=1 and MSIRDY=0) */

/* Bit 3 - MSI clock range selection */
#define RCC_MSIRGSEL (1 << 3)
/* Set by software to select the MSI clock range with MSIRANGE[3:0]. Write 0
   has no effect.
   After a standby or a reset MSIRGSEL is at 0 and the MSI range value is
   provided by MSISRANGE in CSR register.
   0: MSI Range is provided by MSISRANGE[3:0] in RCC_CSR register
   1: MSI Range is provided by MSIRANGE[3:0] in the RCC_CR register */

/* Bit 2 - MSI clock PLL enable */
#define RCC_MSIPLLEN (1 << 2)
/* Set and cleared by software to enable/ disable the PLL part of the MSI
   clock source.
   MSIPLLEN must be enabled after LSE is enabled (LSEON enabled) and ready
   (LSERDY set by hardware).There is a hardware protection to avoid enabling
   MSIPLLEN if LSE is not ready.
   This bit is cleared by hardware when LSE is disabled (LSEON = 0) or when
   the Clock Security System on LSE detects a LSE failure (refer to RCC_CSR
   register).
   0: MSI PLL OFF
   1: MSI PLL ON */

/* Bit 1 - MSI clock ready flag */
#define RCC_MSIRDY (1 << 1)
/* This bit is set by hardware to indicate that the MSI oscillator is stable.
   0: MSI oscillator not ready
   1: MSI oscillator ready
   Note: Once the MSION bit is cleared, MSIRDY goes low after 6 MSI clock
   cycles. */

/* Bit 0 - MSI clock enable */
#define RCC_MSION (1 << 0)
/* This bit is set and cleared by software.
   Cleared by hardware to stop the MSI oscillator when entering Stop, Standby
   or Shutdown mode.
   Set by hardware to force the MSI oscillator ON when exiting Standby or
   Shutdown mode.
   Set by hardware to force the MSI oscillator ON when STOPWUCK=0 when exiting
   from Stop modes, or in case of a failure of the HSE oscillator Set by
   hardware when used directly or indirectly as system clock.
   0: MSI oscillator OFF
   1: MSI oscillator ON */

/* ------------------------------------------------------------------------- */
/* Internal clock sources calibration register - ICSCR */
#define STM32_RCC_ICSCR 0x0004

/* Bits [29..31] - Reserved, must be kept at reset value. */

/* Bits [24..28] - HSI16 clock trimming */
#define RCC_HSITRIM_MSK (0x1f << 24)
#define RCC_HSITRIM_SET(VAL) (((VAL) << 24) & RCC_HSITRIM_MSK)
#define RCC_HSITRIM_GET(REG) (((REG) & RCC_HSITRIM_MSK) >> 24)
/* These bits provide an additional user-programmable trimming value that is
   added to the HSICAL[7:0] bits. It can be programmed to adjust to variations
   in voltage and temperature that influence the frequency of the HSI16.
   The default value is 16, which, when added to the HSICAL value, should trim
   the HSI16 to 16 MHz ± 1 %. */

/* Bits [16..23] - HSI16 clock calibration */
#define RCC_HSICAL_MSK (0xff << 16)
#define RCC_HSICAL_SET(VAL) (((VAL) << 16) & RCC_HSICAL_MSK)
#define RCC_HSICAL_GET(REG) (((REG) & RCC_HSICAL_MSK) >> 16)
/* These bits are initialized at startup with the factory-programmed HSI16
   calibration trim value.
   When HSITRIM is written, HSICAL is updated with the sum of HSITRIM and the
   factory trim value. */

/* Bits [8..15] - MSI clock trimming */
#define RCC_MSITRIM_MSK (0xff << 8)
#define RCC_MSITRIM_SET(VAL) (((VAL) << 8) & RCC_MSITRIM_MSK)
#define RCC_MSITRIM_GET(REG) (((REG) & RCC_MSITRIM_MSK) >> 8)
/* These bits provide an additional user-programmable trimming value that is
   added to the MSICAL[7:0] bits. It can be programmed to adjust to variations
   in voltage and temperature that influence the frequency of the MSI. */

/* Bits [0..7] - MSI clock calibration */
#define RCC_MSICAL_MSK (0xff << 0)
#define RCC_MSICAL_SET(VAL) (((VAL) << 0) & RCC_MSICAL_MSK)
#define RCC_MSICAL_GET(REG) (((REG) & RCC_MSICAL_MSK) >> 0)
/* These bits are initialized at startup with the factory-programmed MSI
   calibration trim value.
   When MSITRIM is written, MSICAL is updated with the sum of MSITRIM and the
   factory trim value. */

/* ------------------------------------------------------------------------- */
/* Clock configuration register - CFGR */
#define STM32_RCC_CFGR 0x0008

/* Bit 31 - Reserved, must be kept at reset value. */

/* Bits [28..30] - Microcontroller clock output prescaler */
#define RCC_MCOPRE_MSK (0x7 << 28)
#define RCC_MCOPRE_SET(VAL) (((VAL) << 28) & RCC_MCOPRE_MSK)
#define RCC_MCOPRE_GET(REG) (((REG) & RCC_MCOPRE_MSK) >> 28)
/* These bits are set and cleared by software.
   It is highly recommended to change this prescaler before MCO output is
   enabled.
   000: MCO is divided by 1
   001: MCO is divided by 2
   010: MCO is divided by 4
   011: MCO is divided by 8
   100: MCO is divided by 16
   Others: not allowed */
#define RCC_MCOPRE(N) (ILOG2(N) << 28)
#define RCC_MCOPRE_1  (0x0 << 28)
#define RCC_MCOPRE_2  (0x1 << 28)
#define RCC_MCOPRE_4  (0x2 << 28)
#define RCC_MCOPRE_8  (0x3 << 28)
#define RCC_MCOPRE_16 (0x4 << 28)

/* Bits [24..27] - Microcontroller clock output */
#define RCC_MCOSEL_MSK (0xf << 24)
#define RCC_MCOSEL_SET(VAL) (((VAL) << 24) & RCC_MCOSEL_MSK)
#define RCC_MCOSEL_GET(REG) (((REG) & RCC_MCOSEL_MSK) >> 24)
/* Set and cleared by software.
   0000: MCO output disabled, no clock on MCO
   0001: SYSCLK system clock selected
   0010: MSI clock selected.
   0011: HSI16 clock selected.
   0100: HSE clock selected
   0101: Main PLL clock selected
   0110: LSI clock selected
   0111: LSE clock selected
   1000: Internal HSI48 clock selected
   Others: Reserved
   Note: This clock output may have some truncated cycles at startup or during
   MCO clock source switching. */
#define RCC_MCO_SYSCLK (0x1 << 24)
#define RCC_MCO_MSI    (0x2 << 24)
#define RCC_MCO_HSI    (0x3 << 24)
#define RCC_MCO_HSE    (0x4 << 24)
#define RCC_MCO_PLL    (0x5 << 24)
#define RCC_MCO_LSI    (0x6 << 24)
#define RCC_MCO_LSE    (0x7 << 24)
#define RCC_MCO_HSI48  (0x8 << 24)

/* Bits [16..23] - Reserved, must be kept at reset value. */

/* Bit 15 - Wakeup from Stop and CSS backup clock selection */
#define RCC_STOPWUCK (1 << 15)
/* Set and cleared by software to select the system clock used when exiting
   Stop mode.
   The selected clock is also used as emergency clock for the Clock Security
   System on HSE.
   Warning: STOPWUCK must not be modified when the Clock Security System is
   enabled by HSECSSON in RCC_CR register and the system clock is HSE
   (SWS=”10”) or a switch on HSE is requested (SW=”10”).
   0: MSI oscillator selected as wakeup from stop clock and CSS backup clock.
   1: HSI16 oscillator selected as wakeup from stop clock and CSS backup clock */

/* Bit 14 - Reserved, must be kept at reset value. */

/* Bits [11..13] - APB high-speed prescaler (APB2) */
#define RCC_PPRE2_MSK (0x7 << 11)
#define RCC_PPRE2_SET(VAL) (((VAL) << 11) & RCC_PPRE2_MSK)
#define RCC_PPRE2_GET(REG) (((REG) & RCC_PPRE2_MSK) >> 11)
/* Set and cleared by software to control the division factor of the APB2
   clock (PCLK2).
   0xx: HCLK not divided
   100: HCLK divided by 2
   101: HCLK divided by 4
   110: HCLK divided by 8
   111: HCLK divided by 16 6.4.4 Bits 10:8 PPRE1[2:0]:APB low-speed prescaler
   (APB1) Set and cleared by software to control the division factor of the
   APB1 clock (PCLK1).
   0xx: HCLK not divided
   100: HCLK divided by 2
   101: HCLK divided by 4
   110: HCLK divided by 8
   111: HCLK divided by 16 */
#define RCC_PPRE2(N) ((ILOG2(N) + 3) << 11)
#define RCC_PPRE2_1  (0x0 << 11)
#define RCC_PPRE2_2  (0x4 << 11)
#define RCC_PPRE2_4  (0x5 << 11)
#define RCC_PPRE2_8  (0x6 << 11)
#define RCC_PPRE2_16 (0x7 << 11)

/* APB low-speed prescaler (APB1) */
#define RCC_PPRE1(N) (ILOG2(N) << 8)
#define RCC_PPRE1_1  (0x0 << 8)
#define RCC_PPRE1_2  (0x4 << 8)
#define RCC_PPRE1_4  (0x5 << 8)
#define RCC_PPRE1_8  (0x6 << 8)
#define RCC_PPRE1_16 (0x7 << 8)

/* Bits [4..7] - AHB prescaler */
#define RCC_HPRE_MSK (0xf << 4)
#define RCC_HPRE_SET(VAL) (((VAL) << 4) & RCC_HPRE_MSK)
#define RCC_HPRE_GET(REG) (((REG) & RCC_HPRE_MSK) >> 4)
/* Set and cleared by software to control the division factor of the AHB
   clock.
   Caution: Depending on the device voltage range, the software has to set
   correctly these bits to ensure that the system frequency does not exceed
   the maximum allowed frequency (for more details please refer to Section
   5.1.7: Dynamic voltage scaling management). After a write operation to
   these bits and before decreasing the voltage range, this register must be
   read to be sure that the new value has been taken into account.
   0xxx: SYSCLK not divided
   1000: SYSCLK divided by 2
   1001: SYSCLK divided by 4
   1010: SYSCLK divided by 8
   1011: SYSCLK divided by 16
   1100: SYSCLK divided by 64
   1101: SYSCLK divided by 128
   1110: SYSCLK divided by 256
   1111: SYSCLK divided by 512 */
#define RCC_HPRE(N)  ((ILOG2(N) + (((N) > 64)?6:7)) << 4)
#define RCC_HPRE_1   (0x0 << 4)
#define RCC_HPRE_2   (0x8 << 4)
#define RCC_HPRE_4   (0x9 << 4)
#define RCC_HPRE_8   (0xa << 4)
#define RCC_HPRE_16  (0xb << 4)
#define RCC_HPRE_64  (0xc << 4)
#define RCC_HPRE_128 (0xd << 4)
#define RCC_HPRE_256 (0xe << 4)
#define RCC_HPRE_512 (0xf << 4)

/* Bits [2..3] - System clock switch status */
#define RCC_SWS_MSK (0x3 << 2)
#define RCC_SWS_SET(VAL) (((VAL) << 2) & RCC_SWS_MSK)
#define RCC_SWS_GET(REG) (((REG) & RCC_SWS_MSK) >> 2)
/* Set and cleared by hardware to indicate which clock source is used as
   system clock.
   00: MSI oscillator used as system clock
   01: HSI16 oscillator used as system clock
   10: HSE used as system clock
   11: PLL used as system clock */
#define RCC_SWS_MSI (0x0 << 2)
#define RCC_SWS_HSI (0x1 << 2)
#define RCC_SWS_HSE (0x2 << 2)
#define RCC_SWS_PLL (0x3 << 2)

/* Bits [0..1] - System clock switch */
#define RCC_SW_MSK (0x3 << 0)
#define RCC_SW_SET(VAL) (((VAL) << 0) & RCC_SW_MSK)
#define RCC_SW_GET(REG) (((REG) & RCC_SW_MSK) >> 0)
/* Set and cleared by software to select system clock source (SYSCLK).
   Configured by HW to force MSI oscillator selection when exiting Standby or
   Shutdown mode.
   Configured by HW to force MSI or HSI16 oscillator selection when exiting
   Stop mode or in case of failure of the HSE oscillator, depending on
   STOPWUCK value.
   00: MSI selected as system clock
   01: HSI16 selected as system clock
   10: HSE selected as system clock
   11: PLL selected as system clock */
#define RCC_SW_MSI (0x0 << 0)
#define RCC_SW_HSI (0x1 << 0)
#define RCC_SW_HSE (0x2 << 0)
#define RCC_SW_PLL (0x3 << 0)

/* ------------------------------------------------------------------------- */
/* PLL configuration register - PLLCFGR */
#define STM32_RCC_PLLCFGR 0x000c

/* Bits [27..31] - Main PLL division factor for PLLSAI2CLK */
#define RCC_PLLPDIV_MSK (0x1f << 27)
#define RCC_PLLPDIV_SET(VAL) (((VAL) << 27) & RCC_PLLPDIV_MSK)
#define RCC_PLLPDIV_GET(REG) (((REG) & RCC_PLLPDIV_MSK) >> 27)
/* Set and cleared by software to control the SAI1 clock frequency. PLLSAI2CLK
   output clock frequency = VCO frequency / PLLPDIV.
   00000: PLLSAI2CLK is controlled by the bit PLLP
   00001: Reserved.
   00010: PLLSAI2CLK = VCO / 2 ....
   11111: PLLSAI2CLK = VCO / 31 */
#define RCC_PLLPDIV(PDIV) (((PDIV) & 0x1f) << 27)
#define RCC_PLLPDIV_OFF (0 << 27)

/* Bits [25..26] - Main PLL division factor for PLLCLK (system clock) */
#define RCC_PLLR_MSK (0x3 << 25)
#define RCC_PLLR_SET(VAL) (((VAL) << 25) & RCC_PLLR_MSK)
#define RCC_PLLR_GET(REG) (((REG) & RCC_PLLR_MSK) >> 25)
/* Set and cleared by software to control the frequency of the main PLL output
   clock PLLCLK.
   This output can be selected as system clock. These bits can be written only
   if PLL is disabled.
   PLLCLK output clock frequency = VCO frequency / PLLR with PLLR = 2, 4, 6,
   or 8
   00: PLLR = 2
   01: PLLR = 4
   10: PLLR = 6
   11: PLLR = 8
   Caution: The software has to set these bits correctly not to exceed 80 MHz
   on this domain. */
#define RCC_PLLR(R) (((((R) >> 1) - 1) & 0x3) << 25)

/* Bit 24 - Main PLL PLLCLK output enable */
#define RCC_PLLREN (1 << 24)
/* Set and reset by software to enable the PLLCLK output of the main PLL (used
   as system clock).
   This bit cannot be written when PLLCLK output of the PLL is used as System
   Clock.
   In order to save power, when the PLLCLK output of the PLL is not used, the
   value of PLLREN should be 0.
   0: PLLCLK output disable
   1: PLLCLK output enable */

/* Bit 23 - Reserved, must be kept at reset value. */

/* Bits [21..22] - Main PLL division factor for PLL48M1CLK (48 MHz clock). */
#define RCC_PLLQ_MSK (0x3 << 21)
#define RCC_PLLQ_SET(VAL) (((VAL) << 21) & RCC_PLLQ_MSK)
#define RCC_PLLQ_GET(REG) (((REG) & RCC_PLLQ_MSK) >> 21)
/* Set and cleared by software to control the frequency of the main PLL output
   clock PLL48M1CLK. This output can be selected for USB, RNG, SDMMC (48 MHz
   clock). These bits can be written only if PLL is disabled.
   PLL48M1CLK output clock frequency = VCO frequency / PLLQ with PLLQ = 2, 4,
   6, or 8
   00: PLLQ = 2
   01: PLLQ = 4
   10: PLLQ = 6
   11: PLLQ = 8
   Caution: The software has to set these bits correctly not to exceed 80 MHz
   on this domain. */
#define RCC_PLLQ(Q) (((((Q) >> 1) - 1) & 0x3) << 21)

/* Bit 20 - Main PLL PLL48M1CLK output enable */
#define RCC_PLLQEN (1 << 20)
/* Set and reset by software to enable the PLL48M1CLK output of the main PLL.
   In order to save power, when the PLL48M1CLK output of the PLL is not used,
   the value of PLLQEN should be 0.
   0: PLL48M1CLK output disable
   1: PLL48M1CLK output enable */

/* Bits [18..19] - Reserved, must be kept at reset value. */

/* Bit 17 - Main PLL division factor for PLLSAI2CLK (SAI1 clock). */
#define RCC_PLLP (1 << 17)
/* Set and cleared by software to control the frequency of the main PLL output
   clock PLLSAI2CLK. This output can be selected for SAI1. These bits can be
   written only if PLL is disabled.
   When the PLLPDIV[4:0] is set to “00000”, PLLSAI2CLK output clock frequency
   = VCO frequency / PLLP with PLLP =7, or 17
   0: PLLP = 7
   1: PLLP = 17
   Caution: The software has to set these bits correctly not to exceed 80 MHz
   on this domain. */

/* Bit 16 - Main PLL PLLSAI1CLK output enable */
#define RCC_PLLPEN (1 << 16)
/* Set and reset by software to enable the PLLSAI2CLK output of the main PLL.
   In order to save power, when the PLLSAI2CLK output of the PLL is not used,
   the value of PLLPEN should be 0.
   0: PLLSAI2CLK output disable
   1: PLLSAI2CLK output enable */

/* Bit 15 - Reserved, must be kept at reset value. */

/* Bits [..8] - */
#define RCC_PLLN_MSK (0x7f << 8)
#define RCC_PLLN(N) (((N) & 0x7f) << 8)
/* Main PLL multiplication factor for VCO Set and cleared by software to
   control the multiplication factor of the VCO. These bits can be written only 
   when the PLL is disabled. VCO output frequency = VCO input frequency x PLLN
   with 8 =< PLLN =< 86
   0000000: PLLN = 0 wrong configuration
   0000001: PLLN = 1 wrong configuration
   ... 0000111: PLLN = 7 wrong configuration
   0001000: PLLN = 8
   0001001: PLLN = 9
   ... 1010101: PLLN = 85
   1010110: PLLN = 86
   1010111:
   PLLN = 87 wrong configuration
   ... 1111111: PLLN = 127 wrong configuration
   Caution: The software has to set correctly these bits to assure that the VCO 
   output frequency is between 64 and 344 MHz */

/* Bit 7 - Reserved, must be kept at reset value. */

/* Bits [4..6] - Division factor for the main PLL and audio PLL (PLLSAI1) input clock */
#define RCC_PLLM_MSK (0x7 << 4)
#define RCC_PLLM_SET(VAL) (((VAL) << 4) & RCC_PLLM_MSK)
#define RCC_PLLM_GET(REG) (((REG) & RCC_PLLM_MSK) >> 4)
/* Set and cleared by software to divide the PLL and PLLSAI1 input clock
   before the VCO.
   These bits can be written only when all PLLs are disabled.
   VCO input frequency = PLL input clock frequency / PLLM with 1 <= PLLM <= 8
   000: PLLM = 1
   001: PLLM = 2
   010: PLLM = 3
   011: PLLM = 4
   100: PLLM = 5
   101: PLLM = 6
   110: PLLM = 7
   111: PLLM = 8
   Caution: The software has to set these bits correctly to ensure that the
   VCO input frequency ranges from 4 to 16 MHz. */
#define RCC_PLLM(M) ((((M) - 1) & 0x7) << 4)

/* Bits [2..3] - Reserved, must be kept at reset value. */

/* Bits [0..1] - Main PLL and PLLSAI1 entry clock source */
#define RCC_PLLSRC_MSK (0x3 << 0)
#define RCC_PLLSRC_SET(VAL) (((VAL) << 0) & RCC_PLLSRC_MSK)
#define RCC_PLLSRC_GET(REG) (((REG) & RCC_PLLSRC_MSK) >> 0)
/* Set and cleared by software to select PLL and PLLSAI1 clock source. These
   bits can be written only when PLL and PLLSAI1 are disabled.
   In order to save power, when no PLL is used, the value of PLLSRC should be
   00.
   00: No clock sent to PLL and PLLSAI1
   01: MSI clock selected as PLL and PLLSAI1 clock entry
   10: HSI16 clock selected as PLL and PLLSAI1 clock entry
   11: HSE clock selected as PLL and PLLSAI1 clock entry */
#define RCC_PLLSRC_NONE (0 << 0)
#define RCC_PLLSRC_MSI  (1 << 0)
#define RCC_PLLSRC_HSI  (2 << 0)
#define RCC_PLLSRC_HSE  (3 << 0)

/* ------------------------------------------------------------------------- */
/* PLLSAI1 configuration register - PLLSAI1CFGR */
#define STM32_RCC_PLLSAI1CFGR 0x0010

/* Bits [27..31] - PLLSAI1 division factor for PLLSAI1CLK */
#define RCC_PLLSAI1PDIV_MSK (0x1f << 27)
#define RCC_PLLSAI1PDIV_SET(VAL) (((VAL) << 27) & RCC_PLLSAI1PDIV_MSK)
#define RCC_PLLSAI1PDIV_GET(REG) (((REG) & RCC_PLLSAI1PDIV_MSK) >> 27)
/* Set and cleared by software to control the SAI1 clock frequency. PLLSAI1CLK
   output clock frequency = VCOSAI1 frequency / PLLPDIV.
   00000: PLLSAI1CLK is controlled by the bit PLLP
   00001: Reserved.
   00010: PLLSAI1CLK = VCOSAI1 / 2 ....
   11111: PLLSAI1CLK = VCOSAI1 / 31 */
#define RCC_PLLSAI1DIV(Q) (((Q) & 0x1f) << 27)

/* Bits [25..26] - PLLSAI1 division factor for PLLADC1CLK (ADC clock) */
#define RCC_PLLSAI1R_MSK (0x3 << 25)
#define RCC_PLLSAI1R_SET(VAL) (((VAL) << 25) & RCC_PLLSAI1R_MSK)
#define RCC_PLLSAI1R_GET(REG) (((REG) & RCC_PLLSAI1R_MSK) >> 25)
/* Set and cleared by software to control the frequency of the PLLSAI1 output
   clock PLLADC1CLK. This output can be selected as ADC clock. These bits can
   be written only if PLLSAI1 is disabled.
   PLLADC1CLK output clock frequency = VCOSAI1 frequency / PLLSAI1R with
   PLLSAI1R = 2, 4, 6, or 8
   00: PLLSAI1R = 2
   01: PLLSAI1R = 4
   10: PLLSAI1R = 6
   11: PLLSAI1R = 8 */
#define RCC_PLLSAI1R(R) (((((R) >> 1) - 1) & 0x3) << 25)

/* Bit 24 - PLLSAI1 PLLADC1CLK output enable */
#define RCC_PLLSAI1REN (1 << 24)
/* Set and reset by software to enable the PLLADC1CLK output of the PLLSAI1
   (used as clock for ADC).
   In order to save power, when the PLLADC1CLK output of the PLLSAI1 is not
   used, the value of PLLSAI1REN should be 0.
   0: PLLADC1CLK output disable
   1: PLLADC1CLK output enable */

/* Bit 23 - Reserved, must be kept at reset value. */

/* Bits [21..22] - PLLSAI1 division factor for PLL48M2CLK (48 MHz clock) */
#define RCC_PLLSAI1Q_MSK (0x3 << 21)
#define RCC_PLLSAI1Q_SET(VAL) (((VAL) << 21) & RCC_PLLSAI1Q_MSK)
#define RCC_PLLSAI1Q_GET(REG) (((REG) & RCC_PLLSAI1Q_MSK) >> 21)
/* Set and cleared by software to control the frequency of the PLLSAI1 output
   clock PLL48M2CLK. This output can be selected for USB(1), RNG, SDMMC (48
   MHz clock).
   These bits can be written only if PLLSAI1 is disabled.
   PLL48M2CLK output clock frequency = VCOSAI1 frequency / PLLQ with PLLQ = 2,
   4, 6, or
   00: PLLQ = 2
   01: PLLQ = 4
   10: PLLQ = 6
   11: PLLQ = 8
   Caution: The software has to set these bits correctly not to exceed 80 MHz
   on this domain. */
#define RCC_PLLSAI1Q(Q) (((((Q) >> 1) - 1) & 0x3) << 21)

/* Bit 20 - PLLSAI1 PLL48M2CLK output enable */
#define RCC_PLLSAI1QEN (1 << 20)
/* Set and reset by software to enable the PLL48M2CLK output of the PLLSAI1.
   In order to save power, when the PLL48M2CLK output of the PLLSAI1 is not
   used, the value of PLLSAI1QEN should be 0.
   0: PLL48M2CLK output disable
   1: PLL48M2CLK output enable */

/* Bits [18..19] - Reserved, must be kept at reset value. */

/* Bit 17 - PLLSAI1 division factor for PLLSAI1CLK (SAI1 clock). */
#define RCC_PLLSAI1P (1 << 17)
/* Set and cleared by software to control the frequency of the PLLSAI1 output
   clock PLLSAI1CLK. This output can be selected for SAI1. These bits can be
   written only if PLLSAI1 is disabled.
   When the PLLSAI1PDIV[4:0] is set to “00000”, PLLSAI1CLK output clock
   frequency = VCOSAI1 frequency / PLLSAI1P with PLLSAI1P =7, or 17
   0: PLLSAI1P = 7
   1: PLLSAI1P = 17 */
#define RCC_PLLSAI1P_7  (0 << 17)
#define RCC_PLLSAI1P_17 (1 << 17)

/* Bit 16 - PLLSAI1 PLLSAI1CLK output enable */
#define RCC_PLLSAI1PEN (1 << 16)
/* Set and reset by software to enable the PLLSAI1CLK output of the PLLSAI1.
   In order to save power, when the PLLSAI1CLK output of the PLLSAI1 is not
   used, the value of PLLSAI1PEN should be 0.
   0: PLLSAI1CLK output disable
   1: PLLSAI1CLK output enable */

/* Bit 15 - Reserved, must be kept at reset value. */

/* Bits [8..14] - PLLSAI1 multiplication factor for VCO */
#define RCC_PLLSAI1N_MSK (0x7f << 8)
#define RCC_PLLSAI1N_SET(VAL) (((VAL) << 8) & RCC_PLLSAI1N_MSK)
#define RCC_PLLSAI1N_GET(REG) (((REG) & RCC_PLLSAI1N_MSK) >> 8)
/* Set and cleared by software to control the multiplication factor of the
   VCO. These bits can be written only when the PLLSAI1 is disabled.
   VCOSAI1 output frequency = VCOSAI1 input frequency x PLLSAI1N with 8 =<
   PLLSAI1N =< 86
   0000000: PLLSAI1N = 0 wrong configuration
   0000001: PLLSAI1N = 1 wrong configuration ...
   0000111: PLLSAI1N = 7 wrong configuration
   0001000: PLLSAI1N = 8
   0001001: PLLSAI1N = 9 ...
   1010101: PLLSAI1N = 85
   1010110: PLLSAI1N = 86
   1010111: PLLSAI1N = 87 wrong configuration ...
   1111111: PLLSAI1N = 127 wrong configuration
   Caution: The software has to set correctly these bits to ensure that the
   VCO output frequency is between 64 and 344 MHz. */
#define RCC_PLLSAI1N(N) (((N) & 0x7f) << 8)

/* Bits [0..7] - Reserved, must be kept at reset value. */

/* ------------------------------------------------------------------------- */
/* Clock interrupt enable register - CIER */
#define STM32_RCC_CIER 0x0018

/* Bits [11..31] - Reserved, must be kept at reset value. */

/* Bit 10 - HSI48 ready interrupt enable */
#define RCC_HSI48RDYIE (1 << 10)
/* Set and cleared by software to enable/disable interrupt caused by the
   internal HSI48 oscillator.
   0: HSI48 ready interrupt disabled
   1: HSI48 ready interrupt enabled */

/* Bit 9 - LSE clock security system interrupt enable */
#define RCC_LSECSSIE (1 << 9)
/* Set and cleared by software to enable/disable interrupt caused by the clock
   security system on LSE.
   0: Clock security interrupt caused by LSE clock failure disabled
   1: Clock security interrupt caused by LSE clock failure enabled */

/* Bits [7..8] - Reserved, must be kept at reset value. */

/* Bit 6 - PLLSAI1 ready interrupt enable */
#define RCC_PLLSAI1RDYIE (1 << 6)
/* Set and cleared by software to enable/disable interrupt caused by PLSAI1L
   lock.
   0: PLLSAI1 lock interrupt disabled
   1: PLLSAI1 lock interrupt enabled */

/* Bit 5 - PLL ready interrupt enable */
#define RCC_PLLRDYIE (1 << 5)
/* Set and cleared by software to enable/disable interrupt caused by PLL lock.
   0: PLL lock interrupt disabled
   1: PLL lock interrupt enabled */

/* Bit 4 - HSE ready interrupt enable */
#define RCC_HSERDYIE (1 << 4)
/* Set and cleared by software to enable/disable interrupt caused by the HSE
   oscillator stabilization.
   0: HSE ready interrupt disabled
   1: HSE ready interrupt enabled */

/* Bit 3 - HSI16 ready interrupt enable */
#define RCC_HSIRDYIE (1 << 3)
/* Set and cleared by software to enable/disable interrupt caused by the HSI16
   oscillator stabilization.
   0: HSI16 ready interrupt disabled
   1: HSI16 ready interrupt enabled */

/* Bit 2 - MSI ready interrupt enable */
#define RCC_MSIRDYIE (1 << 2)
/* Set and cleared by software to enable/disable interrupt caused by the MSI
   oscillator stabilization.
   0: MSI ready interrupt disabled
   1: MSI ready interrupt enabled */

/* Bit 1 - LSE ready interrupt enable */
#define RCC_LSERDYIE (1 << 1)
/* Set and cleared by software to enable/disable interrupt caused by the LSE
   oscillator stabilization.
   0: LSE ready interrupt disabled
   1: LSE ready interrupt enabled */

/* Bit 0 - LSI ready interrupt enable */
#define RCC_LSIRDYIE (1 << 0)
/* Set and cleared by software to enable/disable interrupt caused by the LSI
   oscillator stabilization.
   0: LSI ready interrupt disabled
   1: LSI ready interrupt enabled */

/* ------------------------------------------------------------------------- */
/* Clock interrupt flag register - CIFR */
#define STM32_RCC_CIFR 0x001c

/* Bits [11..31] - Reserved, must be kept at reset value. */

/* Bit 10 - HSI48 ready interrupt flag */
#define RCC_HSI48RDYF (1 << 10)
/* Set by hardware when the HSI48 clock becomes stable and HSI48RDYIE is set
   in a response to setting the HSI48ON (refer to Clock recovery RC register
   (RCC_CRRCR)).
   Cleared by software setting the HSI48RDYC bit.
   0: No clock ready interrupt caused by the HSI48 oscillator
   1: Clock ready interrupt caused by the HSI48 oscillator */

/* Bit 9 - LSE Clock security system interrupt flag */
#define RCC_LSECSSF (1 << 9)
/* Set by hardware when a failure is detected in the LSE oscillator.
   Cleared by software setting the LSECSSC bit.
   0: No clock security interrupt caused by LSE clock failure
   1: Clock security interrupt caused by LSE clock failure */

/* Bit 8 - Clock security system interrupt flag */
#define RCC_CSSF (1 << 8)
/* Set by hardware when a failure is detected in the HSE oscillator.
   Cleared by software setting the CSSC bit.
   0: No clock security interrupt caused by HSE clock failure
   1: Clock security interrupt caused by HSE clock failure */

/* Bit 7 - Reserved, must be kept at reset value. */

/* Bit 6 - PLLSAI1 ready interrupt flag */
#define RCC_PLLSAI1RDYF (1 << 6)
/* Set by hardware when the PLLSAI1 locks and PLLSAI1RDYDIE is set.
   Cleared by software setting the PLLSAI1RDYC bit.
   0: No clock ready interrupt caused by PLLSAI1 lock
   1: Clock ready interrupt caused by PLLSAI1 lock */

/* Bit 5 - PLL ready interrupt flag */
#define RCC_PLLRDYF (1 << 5)
/* Set by hardware when the PLL locks and PLLRDYDIE is set.
   Cleared by software setting the PLLRDYC bit.
   0: No clock ready interrupt caused by PLL lock
   1: Clock ready interrupt caused by PLL lock */

/* Bit 4 - HSE ready interrupt flag */
#define RCC_HSERDYF (1 << 4)
/* Set by hardware when the HSE clock becomes stable and HSERDYDIE is set.
   Cleared by software setting the HSERDYC bit.
   0: No clock ready interrupt caused by the HSE oscillator
   1: Clock ready interrupt caused by the HSE oscillator 6.4.8 Bit 3 HSIRDYF:
   HSI16 ready interrupt flag Set by hardware when the HSI16 clock becomes
   stable and HSIRDYDIE is set in a response to setting the HSION (refer to
   Clock control register (RCC_CR)). When HSION is not set but the HSI16
   oscillator is enabled by the peripheral through a clock request, this bit
   is not set and no interrupt is generated.
   Cleared by software setting the HSIRDYC bit.
   0: No clock ready interrupt caused by the HSI16 oscillator
   1: Clock ready interrupt caused by the HSI16 oscillator */

/* Bit 2 - MSI ready interrupt flag */
#define RCC_MSIRDYF (1 << 2)
/* Set by hardware when the MSI clock becomes stable and MSIRDYDIE is set.
   Cleared by software setting the MSIRDYC bit.
   0: No clock ready interrupt caused by the MSI oscillator
   1: Clock ready interrupt caused by the MSI oscillator */

/* Bit 1 - LSE ready interrupt flag */
#define RCC_LSERDYF (1 << 1)
/* Set by hardware when the LSE clock becomes stable and LSERDYDIE is set.
   Cleared by software setting the LSERDYC bit.
   0: No clock ready interrupt caused by the LSE oscillator
   1: Clock ready interrupt caused by the LSE oscillator */

/* Bit 0 - LSI ready interrupt flag */
#define RCC_LSIRDYF (1 << 0)
/* Set by hardware when the LSI clock becomes stable and LSIRDYDIE is set.
   Cleared by software setting the LSIRDYC bit.
   0: No clock ready interrupt caused by the LSI oscillator
   1: Clock ready interrupt caused by the LSI oscillator */

/* ------------------------------------------------------------------------- */
/* Clock interrupt clear register - CICR */
#define STM32_RCC_CICR 0x0020

/* Bits [11..31] - Reserved, must be kept at reset value. */

/* Bit 10 - HSI48 oscillator ready interrupt clear */
#define RCC_HSI48RDYC (1 << 10)
/* This bit is set by software to clear the HSI48RDYF flag.
   0: No effect
   1: Clear the HSI48RDYC flag */

/* Bit 9 - LSE Clock security system interrupt clear */
#define RCC_LSECSSC (1 << 9)
/* This bit is set by software to clear the LSECSSF flag.
   0: No effect
   1: Clear LSECSSF flag 6.4.9 Bit 8 CSSC: Clock security system interrupt
   clear This bit is set by software to clear the CSSF flag.
   0: No effect
   1: Clear CSSF flag */

/* Bit 7 - Reserved, must be kept at reset value. */

/* Bit 6 - PLLSAI1 ready interrupt clear */
#define RCC_PLLSAI1RDYC (1 << 6)
/* This bit is set by software to clear the PLLSAI1RDYF flag.
   0: No effect
   1: Clear PLLSAI1RDYF flag */

/* Bit 5 - PLL ready interrupt clear */
#define RCC_PLLRDYC (1 << 5)
/* This bit is set by software to clear the PLLRDYF flag.
   0: No effect
   1: Clear PLLRDYF flag */

/* Bit 4 - HSE ready interrupt clear */
#define RCC_HSERDYC (1 << 4)
/* This bit is set by software to clear the HSERDYF flag.
   0: No effect
   1: Clear HSERDYF flag */

/* Bit 3 - HSI16 ready interrupt clear */
#define RCC_HSIRDYC (1 << 3)
/* This bit is set software to clear the HSIRDYF flag.
   0: No effect
   1: Clear HSIRDYF flag */

/* Bit 2 - MSI ready interrupt clear */
#define RCC_MSIRDYC (1 << 2)
/* This bit is set by software to clear the MSIRDYF flag.
   0: No effect
   1: MSIRDYF cleared */

/* Bit 1 - LSE ready interrupt clear */
#define RCC_LSERDYC (1 << 1)
/* This bit is set by software to clear the LSERDYF flag.
   0: No effect
   1: LSERDYF cleared */

/* Bit 0 - LSI ready interrupt clear */
#define RCC_LSIRDYC (1 << 0)
/* This bit is set by software to clear the LSIRDYF flag.
   0: No effect
   1: LSIRDYF cleared */

/* ------------------------------------------------------------------------- */
/* AHB1 peripheral reset register - AHB1RSTR */
#define STM32_RCC_AHB1RSTR 0x0028

/* Bits [17..31] - Reserved, must be kept at reset value. */

/* Bit 16 - Touch Sensing Controller reset */
#define RCC_TSCRST (1 << 16)
/* Set and cleared by software.
   0: No effect
   1: Reset TSC */

/* Bits [13..15] - Reserved, must be kept at reset value. */

/* Bit 12 - CRC reset */
#define RCC_CRCRST (1 << 12)
/* Set and cleared by software.
   0: No effect
   1: Reset CRC */

/* Bits [9..11] - Reserved, must be kept at reset value. */

/* Bit 8 - Flash memory interface reset */
#define RCC_FLASHRST (1 << 8)
/* Set and cleared by software. This bit can be activated only when the Flash
   memory is in power down mode.
   0: No effect
   1: Reset Flash memory interface */

/* Bits [2..7] - Reserved, must be kept at reset value. */

/* Bit 1 - DMA2 reset */
#define RCC_DMA2RST (1 << 1)
/* Set and cleared by software.
   0: No effect
   1: Reset DMA2 */

/* Bit 0 - DMA1 reset */
#define RCC_DMA1RST (1 << 0)
/* Set and cleared by software.
   0: No effect
   1: Reset DMA1 */

/* ------------------------------------------------------------------------- */
/* AHB2 peripheral reset register - AHB2RSTR */
#define STM32_RCC_AHB2RSTR 0x002c

/* Bits [19..31] - Reserved, must be kept at reset value. */

/* Bit 18 - Random number generator reset */
#define RCC_RNGRST (1 << 18)
/* Set and cleared by software.
   0: No effect
   1: Reset RNG */

/* Bit 17 - Reserved, must be kept at reset value. */

/* Bits [14..15] - Reserved, must be kept at reset value. */

/* Bit 13 - ADC reset */
#define RCC_ADCRST (1 << 13)
/* Set and cleared by software.
   0: No effect
   1: Reset ADC interface */

/* Bits [8..12] - Reserved, must be kept at reset value. */

/* Bit 7 - IO port H reset */
#define RCC_GPIOHRST (1 << 7)
/* Set and cleared by software.
   0: No effect
   1: Reset IO port H */

/* Bits [5..6] - Reserved, must be kept at reset value. */

/* Bit 2 - IO port C reset */
#define RCC_GPIOCRST (1 << 2)
/* Set and cleared by software.
   0: No effect
   1: Reset IO port C */

/* Bit 1 - IO port B reset */
#define RCC_GPIOBRST (1 << 1)
/* Set and cleared by software.
   0: No effect
   1: Reset IO port B */

/* Bit 0 - IO port A reset */
#define RCC_GPIOARST (1 << 0)
/* Set and cleared by software.
   0: No effect
   1: Reset IO port A 1. Available on STM32L44xxx and STM32L46xxx devices
   only.
   2. Not available on STM32L432xx and STM32L442xx devices. */

/* ------------------------------------------------------------------------- */
/* AHB3 peripheral reset register - AHB3RSTR */
#define STM32_RCC_AHB3RSTR 0x0030

/* Bits [9..31] - Reserved, must be kept at reset value. */

/* Bit 8 - QUADSPI1 memory interface reset */
#define RCC_QSPIRST (1 << 8)
/* Set and cleared by software.
   0: No effect
   1: Reset QUADSPI */

/* Bits [0..7] - Reserved, must be kept at reset value. */

/* ------------------------------------------------------------------------- */
/* APB1 peripheral reset register 1 - APB1RSTR1 */
#define STM32_RCC_APB1RSTR1 0x0038

/* Bit 31 - Low Power Timer 1 reset */
#define RCC_LPTIM1RST (1 << 31)
/* Set and cleared by software.
   0: No effect
   1: Reset LPTIM1 */

/* Bit 30 - OPAMP interface reset */
#define RCC_OPAMPRST (1 << 30)
/* Set and cleared by software.
   0: No effect
   1: Reset OPAMP interface */

/* Bit 29 - DAC1 interface reset */
#define RCC_DAC1RST (1 << 29)
/* Set and cleared by software.
   0: No effect
   1: Reset DAC1 interface */

/* Bit 28 - Power interface reset */
#define RCC_PWRRST (1 << 28)
/* Set and cleared by software.
   0: No effect
   1: Reset PWR */

/* Bit 27 - Reserved, must be kept at reset value. */

/* Bit 25 - CAN1 reset */
#define RCC_CAN1RST (1 << 25)
/* Set and reset by software.
   0: No effect
   1: Reset the CAN1 */

/* Bit 24 - CRS reset */
#define RCC_CRSRST (1 << 24)
/* Set and cleared by software.
   0: No effect
   1: Reset the CRS */

/* Bit 23 - I2C3 reset */
#define RCC_I2C3RST (1 << 23)
/* Set and reset by software.
   0: No effect
   1: Reset I2C3 Bit 22  I2C2RST(2): I2C2 reset Set and cleared by software.
   0: No effect
   1: Reset I2C2 */

/* Bit 21 - I2C1 reset */
#define RCC_I2C1RST (1 << 21)
/* Set and cleared by software.
   0: No effect
   1: Reset I2C1 */

/* Bit 20 - Reserved, must be kept at reset value. */

/* Bit 17 - USART2 reset */
#define RCC_USART2RST (1 << 17)
/* Set and cleared by software.
   0: No effect
   1: Reset USART2 */

/* Bit 16 - Reserved, must be kept at reset value. */

/* Bit 15 - SPI3 reset */
#define RCC_SPI3RST (1 << 15)
/* Set and cleared by software.
   0: No effect
   1: Reset SPI3 */

/* Bit 14 - SPI2 reset */
#define RCC_SP23RST (1 << 14)
/* Set and cleared by software.
   0: No effect
   1: Reset SPI2 */

/* Bits [10..13] - Reserved, must be kept at reset value. */

/* Bits [6..8] - Reserved, must be kept at reset value. */

/* Bit 4 - TIM6 timer reset */
#define RCC_TIM6RST (1 << 4)
/* Set and cleared by software.
   0: No effect
   1: Reset TIM6 */

/* Bits [2..3] - Reserved, must be kept at reset value. */

/* Bit 0 - TIM2 timer reset */
#define RCC_TIM2RST (1 << 0)
/* Set and cleared by software.
   0: No effect
   1: Reset TIM2 1. Available on STM32L4x2xx and STM32L4x3xx devices only.
   2. Not available on STM32L432xx and STM32L442xx devices.
   3. Available on STM32L45xxx and STM32L46xxx devices only.
   4. Available on STM32L4x3xx devices only.
   5. Available on STM32L43xxx and STM32L44xxx devices only. */

/* ------------------------------------------------------------------------- */
/* APB1 peripheral reset register 2 - APB1RSTR2 */
#define STM32_RCC_APB1RSTR2 0x003c

/* Bits [6..31] - Reserved, must be kept at reset value. */

/* Bit 5 - Low-power timer 2 reset */
#define RCC_LPTIM2RST (1 << 5)
/* Set and cleared by software.
   0: No effect
   1: Reset LPTIM2 */

/* Bits [3..4] - Reserved, must be kept at reset value. */

/* Bit 0 - Low-power UART 1 reset */
#define RCC_LPUART1RST (1 << 0)
/* Set and cleared by software.
   0: No effect
   1: Reset LPUART1 1. Available on STM32L43xxx and STM32L44xxx devices only.
   2. Available on STM32L45xxx and STM32L46xxx devices only. */

/* ------------------------------------------------------------------------- */
/* APB2 peripheral reset register - APB2RSTR */
#define STM32_RCC_APB2RSTR 0x0040

/* Bits [25..31] - Reserved, must be kept at reset value. */

/* Bits [22..23] - Reserved, must be kept at reset value. */

/* Bit 21 - Serial audio interface 1 (SAI1) reset */
#define RCC_SAI1RST (1 << 21)
/* Set and cleared by software.
   0: No effect
   1: Reset SAI1 */

/* Bits [18..20] - Reserved, must be kept at reset value. */

/* Bit 17 - TIM16 timer reset */
#define RCC_TIM16RST (1 << 17)
/* Set and cleared by software.
   0: No effect
   1: Reset TIM16 timer */

/* Bit 16 - TIM15 timer reset */
#define RCC_TIM15RST (1 << 16)
/* Set and cleared by software.
   0: No effect
   1: Reset TIM15 timer */

/* Bit 15 - Reserved, must be kept at reset value. */

/* Bit 14 - USART1 reset */
#define RCC_USART1RST (1 << 14)
/* Set and cleared by software.
   0: No effect
   1: Reset USART1 */

/* Bit 13 - Reserved, must be kept at reset value. */

/* Bit 12 - SPI1 reset */
#define RCC_SPI1RST (1 << 12)
/* Set and cleared by software.
   0: No effect
   1: Reset SPI1 */

/* Bit 11 - TIM1 timer reset */
#define RCC_TIM1RST (1 << 11)
/* Set and cleared by software.
   0: No effect
   1: Reset TIM1 timer Bit 10 SDMMC1RST(2): SDMMC reset Set and cleared by
   software.
   0: No effect
   1: Reset SDMMC */

/* Bits [1..9] - Reserved, must be kept at reset value. */

/* Bit 0 - SYSCFG + COMP + VREFBUF reset */
#define RCC_SYSCFGRST (1 << 0)
/* 0: No effect
   1: Reset SYSCFG + COMP + VREFBUF 1. Available on STM32L45xxx and
   STM32L46xxx devices only.
   2. Not available on STM32L432xx and STM32L442xx devices. */

/* ------------------------------------------------------------------------- */
/* AHB1 peripheral clock enable register - AHB1ENR */
#define STM32_RCC_AHB1ENR 0x0048

/* Bits [17..31] - Reserved, must be kept at reset value. */

/* Bit 16 - Touch Sensing Controller clock enable */
#define RCC_TSCEN (1 << 16)
/* Set and cleared by software.
   0: TSC clock disable
   1: TSC clock enable */

/* Bits [13..15] - Reserved, must be kept at reset value. */

/* Bit 12 - CRC clock enable */
#define RCC_CRCEN (1 << 12)
/* Set and cleared by software.
   0: CRC clock disable
   1: CRC clock enable */

/* Bits [9..11] - Reserved, must be kept at reset value. */

/* Bit 8 - Flash memory interface clock enable */
#define RCC_FLASHEN (1 << 8)
/* Set and cleared by software. This bit can be disabled only when the Flash
   is in power down mode.
   0: Flash memory interface clock disable
   1: Flash memory interface clock enable */

/* Bits [2..7] - Reserved, must be kept at reset value. */

/* Bit 1 - DMA2 clock enable */
#define RCC_DMA2EN (1 << 1)
/* Set and cleared by software.
   0: DMA2 clock disable
   1: DMA2 clock enable */

/* Bit 0 - DMA1 clock enable */
#define RCC_DMA1EN (1 << 0)
/* Set and cleared by software.
   0: DMA1 clock disable
   1: DMA1 clock enable */

/* ------------------------------------------------------------------------- */
/* AHB2 peripheral clock enable register - AHB2ENR */
#define STM32_RCC_AHB2ENR 0x004c

/* Bits [19..31] - Reserved, must be kept at reset value. */

/* Bit 18 - Random Number Generator clock enable */
#define RCC_RNGEN (1 << 18)
/* Set and cleared by software.
   0: Random Number Generator clock disabled
   1: Random Number Generator clock enabled */

/* Bit 17 - Reserved, must be kept at reset value. */

/* Bits [14..15] - Reserved, must be kept at reset value. */

/* Bit 13 - ADC clock enable */
#define RCC_ADCEN (1 << 13)
/* Set and cleared by software.
   0: ADC clock disabled
   1: ADC clock enabled */

/* Bits [8..12] - Reserved, must be kept at reset value. */

/* Bit 7 - IO port H clock enable */
#define RCC_GPIOHEN (1 << 7)
/* Set and cleared by software.
   0: IO port H clock disabled
   1: IO port H clock enabled */

/* Bits [5..6] - Reserved, must be kept at reset value. */

/* Bit 2 - IO port C clock enable */
#define RCC_GPIOCEN (1 << 2)
/* Set and cleared by software.
   0: IO port C clock disabled
   1: IO port C clock enabled */

/* Bit 1 - IO port B clock enable */
#define RCC_GPIOBEN (1 << 1)
/* Set and cleared by software.
   0: IO port B clock disabled
   1: IO port B clock enabled */

/* Bit 0 - IO port A clock enable */
#define RCC_GPIOAEN (1 << 0)
/* Set and cleared by software.
   0: IO port A clock disabled
   1: IO port A clock enabled 1. Available on STM32L44xxx and STM32L46xxx
   devices only.
   2. Not available on STM32L432xx and STM32L442xx devices. */

/* ------------------------------------------------------------------------- */
/* AHB3 peripheral clock enable register - AHB3ENR */
#define STM32_RCC_AHB3ENR 0x0050

/* Bits [9..31] - Reserved, must be kept at reset value. */

/* Bit 8 - Quad SPI memory interface clock enable */
#define RCC_QSPIEN (1 << 8)
/* Set and cleared by software.
   0: QUADSPI clock disable
   1: QUADSPI clock enable */

/* Bits [0..7] - Reserved, must be kept at reset value. */

/* ------------------------------------------------------------------------- */
/* APB1 peripheral clock enable register 1 - APB1ENR1 */
#define STM32_RCC_APB1ENR1 0x0058

/* Bit 31 - Low power timer 1 clock enable */
#define RCC_LPTIM1EN (1 << 31)
/* Set and cleared by software.
   0: LPTIM1 clock disabled
   1: LPTIM1 clock enabled */

/* Bit 30 - OPAMP interface clock enable */
#define RCC_OPAMPEN (1 << 30)
/* Set and cleared by software.
   0: OPAMP interface clock disabled
   1: OPAMP interface clock enabled */

/* Bit 29 - DAC1 interface clock enable */
#define RCC_DAC1EN (1 << 29)
/* Set and cleared by software.
   0: DAC1 interface clock disabled
   1: DAC1 interface clock enabled */

/* Bit 28 - Power interface clock enable */
#define RCC_PWREN (1 << 28)
/* Set and cleared by software.
   0: Power interface clock disabled
   1: Power interface clock enabled */

/* Bit 27 - Reserved, must be kept at reset value. */

/* Bit 25 - CAN1 clock enable */
#define RCC_CAN1EN (1 << 25)
/* Set and cleared by software.
   0: CAN1 clock disabled
   1: CAN1 clock enabled */

/* Bit 24 - CRS clock enable */
#define RCC_CRSEN (1 << 24)
/* Set and cleared by software.
   0: CRS clock disabled
   1: CRS clock enabled */

/* Bit 23 - I2C3 clock enable */
#define RCC_I2C3EN (1 << 23)
/* Set and cleared by software.
   0: I2C3 clock disabled
   1: I2C3 clock enabled Bit 22 I2C2EN(2): I2C2 clock enable Set and cleared
   by software.
   0: I2C2 clock disabled
   1: I2C2 clock enabled */

/* Bit 21 - I2C1 clock enable */
#define RCC_I2C1EN (1 << 21)
/* Set and cleared by software.
   0: I2C1 clock disabled
   1: I2C1 clock enabled */

/* Bit 20 - Reserved, must be kept at reset value. */

/* Bit 17 - USART2 clock enable */
#define RCC_USART2EN (1 << 17)
/* Set and cleared by software.
   0: USART2 clock disabled
   1: USART2 clock enabled */

/* Bit 16 - Reserved, must be kept at reset value. */

/* Bit 15 - SPI3 clock enable */
#define RCC_SPI3EN (1 << 15)
/* Set and cleared by software.
   0: SPI3 clock disabled
   1: SPI3 clock enabled Bit 14 SPI2EN(2): SPI2 clock enable Set and cleared
   by software.
   0: SPI2 clock disabled
   1: SPI2 clock enabled */

/* Bits [12..13] - Reserved, must be kept at reset value. */

/* Bit 11 - Window watchdog clock enable */
#define RCC_WWDGEN (1 << 11)
/* Set by software to enable the window watchdog clock. Reset by hardware
   system reset.
   This bit can also be set by hardware if the WWDG_SW option bit is reset.
   0: Window watchdog clock disabled
   1: Window watchdog clock enabled */

/* Bit 10 - RTC APB clock enable */
#define RCC_RTCAPBEN (1 << 10)
/* Set and cleared by software
   0: RTC APB clock disabled
   1: RTC APB clock enabled Bit 9 LCDEN(4): LCD clock enable Set and cleared
   by software.
   0: LCD clock disabled
   1: LCD clock enabled */

/* Bits [6..8] - Reserved, must be kept at reset value. */

/* Bit 4 - TIM6 timer clock enable */
#define RCC_TIM6EN (1 << 4)
/* Set and cleared by software.
   0: TIM6 clock disabled
   1: TIM6 clock enabled */

/* Bits [2..3] - Reserved, must be kept at reset value. */

/* Bit 0 - TIM2 timer clock enable */
#define RCC_TIM2EN (1 << 0)
/* Set and cleared by software.
   0: TIM2 clock disabled
   1: TIM2 clock enabled 1. Not available for STM32L431xx devices.
   2. Not available on STM32L432 and STM32L442 devices.
   3. Available on STM32L45xxx and STM32L46xxx devices only.
   4. Available on STM32L4x3xx devices only.
   5. Available on STM32L43xxx and STM32L44xxx devices only. */

/* ------------------------------------------------------------------------- */
/* APB1 peripheral clock enable register 2 - APB1ENR2 */
#define STM32_RCC_APB1ENR2 0x005c

/* Bits [6..31] - Reserved, must be kept at reset value. */

/* Bits [3..4] - Reserved, must be kept at reset value. */

/* Bit 1 - Reserved, must be kept at reset value. */

/* Bit 0 - Low power UART 1 clock enable */
#define RCC_LPUART1EN (1 << 0)
/* Set and cleared by software.
   0: LPUART1 clock disable
   1: LPUART1 clock enable 1. Available on STM32L43xxx and STM32L44xxx devices
   only.
   2. Available on STM32L45xxx and STM32L46xxx devices only. */

/* ------------------------------------------------------------------------- */
/* APB2 peripheral clock enable register - APB2ENR */
#define STM32_RCC_APB2ENR 0x0060

/* Bits [25..31] - Reserved, must be kept at reset value. */

/* Bit 21 - SAI1 clock enable */
#define RCC_SAI1EN (1 << 21)
/* Set and cleared by software.
   0: SAI1 clock disabled
   1: SAI1 clock enabled */

/* Bits [18..20] - Reserved, must be kept at reset value. */

/* Bit 17 - TIM16 timer clock enable */
#define RCC_TIM16EN (1 << 17)
/* Set and cleared by software.
   0: TIM16 timer clock disabled
   1: TIM16 timer clock enabled */

/* Bit 16 - TIM15 timer clock enable */
#define RCC_TIM15EN (1 << 16)
/* Set and cleared by software.
   0: TIM15 timer clock disabled
   1: TIM15 timer clock enabled */

/* Bit 15 - Reserved, must be kept at reset value. */

/* Bit 14 - USART1clock enable */
#define RCC_USART1EN (1 << 14)
/* Set and cleared by software.
   0: USART1clock disabled
   1: USART1clock enabled */

/* Bit 13 - Reserved, must be kept at reset value. */

/* Bit 12 - SPI1 clock enable */
#define RCC_SPI1EN (1 << 12)
/* Set and cleared by software.
   0: SPI1 clock disabled
   1: SPI1 clock enabled */

/* Bit 11 - TIM1 timer clock enable */
#define RCC_TIM1EN (1 << 11)
/* Set and cleared by software.
   0: TIM1 timer clock disabled
   1: TIM1P timer clock enabled Bit 10 SDMMC1EN(2): SDMMC clock enable Set and
   cleared by software.
   0: SDMMC clock disabled
   1: SDMMC clock enabled */

/* Bits [8..9] - Reserved, must be kept at reset value. */

/* Bit 7 - Firewall clock enable */
#define RCC_FWEN (1 << 7)
/* Set by software, reset by hardware. Software can only write 1. A write at 0
   has no effect.
   0: Firewall clock disabled
   1: Firewall clock enabled */

/* Bits [1..6] - Reserved, must be kept at reset value. */

/* Bit 0 - SYSCFG + COMP + VREFBUF clock enable */
#define RCC_SYSCFGEN (1 << 0)
/* Set and cleared by software.
   0: SYSCFG + COMP + VREFBUF clock disabled
   1: SYSCFG + COMP + VREFBUF clock enabled 1. Available on STM32L45xxx and
   STM32L46xxx devices only.
   2. Not available on STM32L432 and STM32L442 devices. */

/* ------------------------------------------------------------------------- */
/* AHB1 peripheral clocks enable in Sleep and Stop modes register - AHB1SMENR */
#define STM32_RCC_AHB1SMENR 0x0068

/* Bits [17..31] - Reserved, must be kept at reset value. */

/* Bit 16 - Touch Sensing Controller clocks enable during Sleep and Stop modes */
#define RCC_TSCSMEN (1 << 16)
/* Set and cleared by software.
   0: TSC clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: TSC clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bits [13..15] - Reserved, must be kept at reset value. */

/* Bit 12 - CRC clocks enable during Sleep and Stop modes */
#define RCC_CRCSMEN (1 << 12)
/* Set and cleared by software.
   0: CRC clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: CRC clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bits [10..11] - Reserved, must be kept at reset value. */

/* Bit 9 - SRAM1 interface clocks enable during Sleep and Stop modes */
#define RCC_SRAM1SMEN (1 << 9)
/* Set and cleared by software.
   0: SRAM1 interface clocks disabled by the clock gating(1) during Sleep and
   Stop modes
   1: SRAM1 interface clocks enabled by the clock gating(1) during Sleep and
   Stop modes */

/* Bit 8 - Flash memory interface clocks enable during Sleep and Stop modes */
#define RCC_FLASHSMEN (1 << 8)
/* Set and cleared by software.
   0: Flash memory interface clocks disabled by the clock gating(1) during
   Sleep and Stop modes
   1: Flash memory interface clocks enabled by the clock gating(1) during
   Sleep and Stop modes */

/* Bits [2..7] - Reserved, must be kept at reset value. */

/* Bit 1 - DMA2 clocks enable during Sleep and Stop modes */
#define RCC_DMA2SMEN (1 << 1)
/* Set and cleared by software during Sleep mode.
   0: DMA2 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: DMA2 clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bit 0 - DMA1 clocks enable during Sleep and Stop modes */
#define RCC_DMA1SMEN (1 << 0)
/* Set and cleared by software.
   0: DMA1 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: DMA1 clocks enabled by the clock gating(1) during Sleep and Stop modes
   1. This register only configures the clock gating, not the clock source
   itself. Most of the peripherals are clocked by a single clock (AHB or APB
   clock), which is always disabled in Stop mode. In this case setting the bit
   has no effect in Stop mode. */

/* ------------------------------------------------------------------------- */
/* AHB2 peripheral clocks enable in Sleep and Stop modes register - AHB2SMENR */
#define STM32_RCC_AHB2SMENR 0x006c

/* Bits [19..31] - Reserved, must be kept at reset value. */

/* Bit 18 - Random Number Generator clocks enable during Sleep and Stop modes */
#define RCC_RNGSMEN (1 << 18)
/* Set and cleared by software.
   0: Random Number Generator clocks disabled by the clock gating during Sleep
   and Stop modes
   1: Random Number Generator clocks enabled by the clock gating during Sleep
   and Stop modes */

/* Bit 17 - Reserved, must be kept at reset value. */

/* Bits [14..15] - Reserved, must be kept at reset value. */

/* Bit 13 - ADC clocks enable during Sleep and Stop modes */
#define RCC_ADCSMEN (1 << 13)
/* Set and cleared by software.
   0: ADC clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: ADC clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bits [10..12] - Reserved, must be kept at reset value. */

/* Bit 9 - SRAM2 interface clocks enable during Sleep and Stop modes */
#define RCC_SRAM2SMEN (1 << 9)
/* Set and cleared by software.
   0: SRAM2 interface clocks disabled by the clock gating(1) during Sleep and
   Stop modes
   1: SRAM2 interface clocks enabled by the clock gating(1) during Sleep and
   Stop modes */

/* Bit 8 - Reserved, must be kept at reset value. */

/* Bit 7 - IO port H clocks enable during Sleep and Stop modes */
#define RCC_GPIOHSMEN (1 << 7)
/* Set and cleared by software.
   0: IO port H clocks disabled by the clock gating(1) during Sleep and Stop
   modes
   1: IO port H clocks enabled by the clock gating(1) during Sleep and Stop
   modes */

/* Bits [5..6] - Reserved, must be kept at reset value. */

/* Bit 2 - IO port C clocks enable during Sleep and Stop modes */
#define RCC_GPIOCSMEN (1 << 2)
/* Set and cleared by software.
   0: IO port C clocks disabled by the clock gating(1) during Sleep and Stop
   modes
   1: IO port C clocks enabled by the clock gating(1) during Sleep and Stop
   modes */

/* Bit 1 - IO port B clocks enable during Sleep and Stop modes */
#define RCC_GPIOBSMEN (1 << 1)
/* Set and cleared by software.
   0: IO port B clocks disabled by the clock gating(1) during Sleep and Stop
   modes
   1: IO port B clocks enabled by the clock gating(1) during Sleep and Stop
   modes */

/* Bit 0 - IO port A clocks enable during Sleep and Stop modes */
#define RCC_GPIOASMEN (1 << 0)
/* Set and cleared by software.
   0: IO port A clocks disabled by the clock gating(1) during Sleep and Stop
   modes
   1: IO port A clocks enabled by the clock gating(1) during Sleep and Stop
   modes 1. Available on STM32L44xxx and STM32L46xxx devices only.
   2. Not available on STM32L432xx and STM32L442xx devices. */

/* ------------------------------------------------------------------------- */
/* AHB3 peripheral clocks enable in Sleep and Stop modes register - AHB3SMENR */
#define STM32_RCC_AHB3SMENR 0x0070

/* Bits [9..31] - Reserved, must be kept at reset value. */

/* Bits [0..7] - Reserved, must be kept at reset value. */

/* ------------------------------------------------------------------------- */
/* APB1 peripheral clocks enable in Sleep and Stop modes register 1 - APB1SMENR1 */
#define STM32_RCC_APB1SMENR1 0x0078

/* Bit 31 - Low power timer 1 clocks enable during Sleep and Stop modes */
#define RCC_LPTIM1SMEN (1 << 31)
/* Set and cleared by software.
   0: LPTIM1 clocks disabled by the clock gating(1) during Sleep and Stop
   modes
   1: LPTIM1 clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bit 30 - OPAMP interface clocks enable during Sleep and Stop modes */
#define RCC_OPAMPSMEN (1 << 30)
/* Set and cleared by software.
   0: OPAMP interface clocks disabled by the clock gating(1) during Sleep and
   Stop modes
   1: OPAMP interface clocks enabled by the clock gating(1) during Sleep and
   Stop modes */

/* Bit 29 - DAC1 interface clocks enable during Sleep and Stop modes */
#define RCC_DAC1SMEN (1 << 29)
/* Set and cleared by software.
   0: DAC1 interface clocks disabled by the clock gating(1) during Sleep and
   Stop modes
   1: DAC1 interface clocks enabled by the clock gating(1) during Sleep and
   Stop modes */

/* Bit 28 - Power interface clocks enable during Sleep and Stop modes */
#define RCC_PWRSMEN (1 << 28)
/* Set and cleared by software.
   0: Power interface clocks disabled by the clock gating(1) during Sleep and
   Stop modes
   1: Power interface clocks enabled by the clock gating(1) during Sleep and
   Stop modes */

/* Bit 27 - Reserved, must be kept at reset value. */

/* Bit 25 - CAN1 clocks enable during Sleep and Stop modes */
#define RCC_CAN1SMEN (1 << 25)
/* Set and cleared by software.
   0: CAN1 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: CAN1 clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bit 24 - CRS clock enable during Sleep and Stop modes */
#define RCC_CRSSMEN (1 << 24)
/* Set and cleared by software.
   0: CRS clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: CRS clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bit 23 - I2C3 clocks enable during Sleep and Stop modes */
#define RCC_I2C3SMEN (1 << 23)
/* Set and cleared by software.
   0: I2C3 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: I2C3 clocks enabled by the clock gating(1) during Sleep and Stop modes
   Bit 22 I2C2SMEN(3): I2C2 clocks enable during Sleep and Stop modes Set and
   cleared by software.
   0: I2C2 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: I2C2 clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bit 21 - I2C1 clocks enable during Sleep and Stop modes */
#define RCC_I2C1SMEN (1 << 21)
/* Set and cleared by software.
   0: I2C1 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: I2C1 clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bit 20 - Reserved, must be kept at reset value. */

/* Bit 17 - USART2 clocks enable during Sleep and Stop modes */
#define RCC_USART2SMEN (1 << 17)
/* Set and cleared by software.
   0: USART2 clocks disabled by the clock gating(1) during Sleep and Stop
   modes
   1: USART2 clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bit 16 - Reserved, must be kept at reset value. */

/* Bit 15 - SPI3 clocks enable during Sleep and Stop modes */
#define RCC_SPI3SMEN (1 << 15)
/* Set and cleared by software.
   0: SPI3 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: SPI3 clocks enabled by the clock gating(1) during Sleep and Stop modes
   Bit 14 SPI2SMEN(3): SPI2 clocks enable during Sleep and Stop modes Set and
   cleared by software.
   0: SPI2 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: SPI2 clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bits [12..13] - Reserved, must be kept at reset value. */

/* Bit 11 - Window watchdog clocks enable during Sleep and Stop modes */
#define RCC_WWDGSMEN (1 << 11)
/* Set and cleared by software. This bit is forced to ‘1’ by hardware when the
   hardware WWDG option is activated.
   0: Window watchdog clocks disabled by the clock gating(1) during Sleep and
   Stop modes
   1: Window watchdog clocks enabled by the clock gating(1) during Sleep and
   Stop modes */

/* Bit 10 - RTC APB clock enable during Sleep and Stop modes */
#define RCC_RTCAPBSMEN (1 << 10)
/* Set and cleared by software
   0: RTC APB clock disabled by the clock gating(1) during Sleep and Stop
   modes
   1: RTC APB clock enabled by the clock gating(1) during Sleep and Stop modes
   Bit 9 LCDSMEN(5): LCD clocks enable during Sleep and Stop modes Set and
   cleared by software.
   0: LCD clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: LCD clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bits [6..8] - Reserved, must be kept at reset value. */

/* Bit 4 - TIM6 timer clocks enable during Sleep and Stop modes */
#define RCC_TIM6SMEN (1 << 4)
/* Set and cleared by software.
   0: TIM6 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: TIM6 clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bits [2..3] - Reserved, must be kept at reset value. */

/* Bit 0 - TIM2 timer clocks enable during Sleep and Stop modes */
#define RCC_TIM2SMEN (1 << 0)
/* Set and cleared by software.
   0: TIM2 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: TIM2 clocks enabled by the clock gating(1) during Sleep and Stop modes
   1. This register only configures the clock gating, not the clock source
   itself. Most of the peripherals are clocked by a single clock (AHB or APB
   clock), which is always disabled in Stop mode. In this case setting the bit
   has no effect in Stop mode.
   2. Available on STM32L4x2xx and STM32L4x3xx devices only.
   3. Not available on STM32L432xx and STM32L442xx devices.
   4. Available on STM32L45xxx and STM32L46xxx devices only.
   5. Available on STM32L4x3xx devices only.
   6. Available on STM32L43xxx and STM32L44xxx devices only. */

/* ------------------------------------------------------------------------- */
/* APB1 peripheral clocks enable in Sleep and Stop modes register 2 - APB1SMENR2 */
#define STM32_RCC_APB1SMENR2 0x007c

/* Bits [6..31] - Reserved, must be kept at reset value. */

/* Bits [3..4] - Reserved, must be kept at reset value. */

/* Bit 1 - Reserved, must be kept at reset value. */

/* Bit 0 - Low power UART 1 clocks enable during Sleep and Stop modes */
#define RCC_LPUART1SMEN (1 << 0)
/* Set and cleared by software.
   0: LPUART1 clocks disabled by the clock gating(1) during Sleep and Stop
   modes
   1: LPUART1 clocks enabled by the clock gating(1) during Sleep and Stop
   modes 1. This register only configures the clock gating, not the clock
   source itself. Most of the peripherals are clocked by a single clock (AHB
   or APB clock), which is always disabled in Stop mode. In this case setting
   the bit has no effect in Stop mode.
   2. Available on STM32L43xxx and STM32L44xxx devices only. */

/* ------------------------------------------------------------------------- */
/* APB2 peripheral clocks enable in Sleep and Stop modes register - APB2SMENR */
#define STM32_RCC_APB2SMENR 0x0080

/* Bits [22..31] - Reserved, must be kept at reset value. */

/* Bit 21 - SAI1 clocks enable during Sleep and Stop modes */
#define RCC_SAI1SMEN (1 << 21)
/* Set and cleared by software.
   0: SAI1 clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: SAI1 clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bits [18..20] - Reserved, must be kept at reset value. */

/* Bit 17 - TIM16 timer clocks enable during Sleep and Stop modes */
#define RCC_TIM16SMEN (1 << 17)
/* Set and cleared by software.
   0: TIM16 timer clocks disabled by the clock gating(1) during Sleep and Stop
   modes
   1: TIM16 timer clocks enabled by the clock gating(1) during Sleep and Stop
   modes */

/* Bit 16 - TIM15 timer clocks enable during Sleep and Stop modes */
#define RCC_TIM15SMEN (1 << 16)
/* Set and cleared by software.
   0: TIM15 timer clocks disabled by the clock gating(1) during Sleep and Stop
   modes
   1: TIM15 timer clocks enabled by the clock gating(1) during Sleep and Stop
   modes */

/* Bit 15 - Reserved, must be kept at reset value. */

/* Bit 14 - USART1clocks enable during Sleep and Stop modes */
#define RCC_USART1SMEN (1 << 14)
/* Set and cleared by software.
   0: USART1clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: USART1clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bit 13 - Reserved, must be kept at reset value. */

/* Bit 12 - SPI1 clocks enable during Sleep and Stop modes */
#define RCC_SPI1SMEN (1 << 12)
/* Set and cleared by software.
   0: SPI1 clocks disabled by the clock gating during(1) Sleep and Stop modes
   1: SPI1 clocks enabled by the clock gating during(1) Sleep and Stop modes */

/* Bit 11 - TIM1 timer clocks enable during Sleep and Stop modes */
#define RCC_TIM1SMEN (1 << 11)
/* Set and cleared by software.
   0: TIM1 timer clocks disabled by the clock gating(1) during Sleep and Stop
   modes
   1: TIM1P timer clocks enabled by the clock gating(1) during Sleep and Stop
   modes */

/* Bit 10 - SDMMC clocks enable during Sleep and Stop modes */
#define RCC_SDMMC1SMEN (1 << 10)
/* Set and cleared by software.
   0: SDMMC clocks disabled by the clock gating(1) during Sleep and Stop modes
   1: SDMMC clocks enabled by the clock gating(1) during Sleep and Stop modes */

/* Bits [1..9] - Reserved, must be kept at reset value. */

/* Bit 0 - SYSCFG + COMP + VREFBUF clocks enable during Sleep and Stop modes */
#define RCC_SYSCFGSMEN (1 << 0)
/* Set and cleared by software.
   0: SYSCFG + COMP + VREFBUF clocks disabled by the clock gating(1) during
   Sleep and Stop modes
   1: SYSCFG + COMP + VREFBUF clocks enabled by the clock gating(1) during
   Sleep and Stop modes 1. This register only configures the clock gating, not
   the clock source itself. Most of the peripherals are clocked by a single
   clock (AHB or APB clock), which is always disabled in Stop mode. In this
   case setting the bit has no effect in Stop mode. */

/* ------------------------------------------------------------------------- */
/* Peripherals independent clock configuration register - CCIPR */
#define STM32_RCC_CCIPR 0x0088

/* Bit 31 - Reserved, must be kept at reset value. */

/* Bit 30 SWPMI1SEL(1): SWPMI1 clock source selection */
/* This bit is set and cleared by software to select the SWPMI1 clock source.
   0: APB1 (PCLK1) selected as SWPMI1 clock
   1: HSI16 clock selected as SWPMI1 clock */

/* Bits [28..29] - ADCs clock source selection */
#define RCC_ADCSEL_MSK (0x3 << 28)
#define RCC_ADCSEL_SET(VAL) (((VAL) << 28) & RCC_ADCSEL_MSK)
#define RCC_ADCSEL_GET(REG) (((REG) & RCC_ADCSEL_MSK) >> 28)
/* These bits are set and cleared by software to select the clock source used
   by the ADC interface.
   00: No clock selected
   01: PLLSAI1 “R” clock (PLLADC1CLK) selected as ADCs clock
   10: Reserved
   11: System clock selected as ADCs clock */
#define RCC_ADCSEL_NONE      (0 << 28)
#define RCC_ADCSEL_PLLSAI1   (1 << 28)
#define RCC_ADCSEL_SYSCLK    (3 << 28)

/* Bits [26..27] - 48 MHz clock source selection */
#define RCC_CLK48SEL_MSK (0x3 << 26)
#define RCC_CLK48SEL_SET(VAL) (((VAL) << 26) & RCC_CLK48SEL_MSK)
#define RCC_CLK48SEL_GET(REG) (((REG) & RCC_CLK48SEL_MSK) >> 26)
/* These bits are set and cleared by software to select the 48 MHz clock
   source used by USB FS, RNG and SDMMC.
   00: HSI48 clock selected as 48 MHz clock
   01: PLLSAI1 “Q” clock (PLL48M2CLK) selected as 48 MHz clock
   10: PLL “Q” clock (PLL48M1CLK) selected as 48 MHz clock
   11: MSI clock selected as 48 MHz clock */
#define RCC_CLK48SEL_MASK    (0x3 << 26)
#define RCC_CLK48SEL_HSI48   (0x0 << 26)
#define RCC_CLK48SEL_PLLSAI1 (0x1 << 26)
#define RCC_CLK48SEL_PLL     (0x2 << 26)
#define RCC_CLK48SEL_MSI     (0x3 << 26)

/* Bits [24..25] - Reserved, must be kept at reset value. */

/* Bits [22..23] - SAI1 clock source selection */
#define RCC_SAI1SEL_MSK (0x3 << 22)
#define RCC_SAI1SEL_SET(VAL) (((VAL) << 22) & RCC_SAI1SEL_MSK)
#define RCC_SAI1SEL_GET(REG) (((REG) & RCC_SAI1SEL_MSK) >> 22)
/* These bits are set and cleared by software to select the SAI1 clock source.
   00: PLLSAI1 “P” clock (PLLSAI1CLK) selected as SAI1 clock
   01: Reserved
   Note: When there is no PLL enabled, the HSI16 clock source is connected
   automatically to the SAI1 to allow audio detection without the need to turn
   on the PLL source.
   10: PLL “P” clock (PLLSAI3CLK) selected as SAI1 clock
   11: External input SAI1_EXTCLK selected as SAI1 clock
   Caution: If the selected clock is the external clock, it is not possible to
   switch to another clock if the external clock is not present. */
#define RCC_SAI1SEL_PLLSAI1   (0 << 22)
#define RCC_SAI1SEL_PLL       (2 << 22)
#define RCC_SAI1SEL_EXTCLK    (3 << 22)

/* Bits [20..21] - Low power timer 2 clock source selection */
#define RCC_LPTIM2SEL_MSK (0x3 << 20)
#define RCC_LPTIM2SEL_SET(VAL) (((VAL) << 20) & RCC_LPTIM2SEL_MSK)
#define RCC_LPTIM2SEL_GET(REG) (((REG) & RCC_LPTIM2SEL_MSK) >> 20)
/* These bits are set and cleared by software to select the LPTIM2 clock
   source.
   00: PCLK selected as LPTIM2 clock
   01: LSI clock selected as LPTIM2 clock
   10: HSI16 clock selected as LPTIM2 clock
   11: LSE clock selected as LPTIM2 clock */

/* Bits [18..19] - Low power timer 1 clock source selection */
#define RCC_LPTIM1SEL_MSK (0x3 << 18)
#define RCC_LPTIM1SEL_SET(VAL) (((VAL) << 18) & RCC_LPTIM1SEL_MSK)
#define RCC_LPTIM1SEL_GET(REG) (((REG) & RCC_LPTIM1SEL_MSK) >> 18)
/* These bits are set and cleared by software to select the LPTIM1 clock
   source.
   00: PCLK selected as LPTIM1 clock
   01: LSI clock selected as LPTIM1 clock
   10: HSI16 clock selected as LPTIM1 clock
   11: LSE clock selected as LPTIM1 clock */

/* Bits [16..17] - I2C3 clock source selection */
#define RCC_I2C3SEL_MSK (0x3 << 16)
#define RCC_I2C3SEL_SET(VAL) (((VAL) << 16) & RCC_I2C3SEL_MSK)
#define RCC_I2C3SEL_GET(REG) (((REG) & RCC_I2C3SEL_MSK) >> 16)
/* These bits are set and cleared by software to select the I2C3 clock source.
   00: PCLK selected as I2C3 clock
   01: System clock (SYSCLK) selected as I2C3 clock
   10: HSI16 clock selected as I2C3 clock
   11: Reserved Bits 15:14 I2C2SEL[1:0](2): I2C2 clock source selection These
   bits are set and cleared by software to select the I2C2 clock source.
   00: PCLK selected as I2C2 clock
   01: System clock (SYSCLK) selected as I2C2 clock
   10: HSI16 clock selected as I2C2 clock
   11: Reserved */

/* Bits [12..13] - I2C1 clock source selection */
#define RCC_I2C1SEL_MSK (0x3 << 12)
#define RCC_I2C1SEL_SET(VAL) (((VAL) << 12) & RCC_I2C1SEL_MSK)
#define RCC_I2C1SEL_GET(REG) (((REG) & RCC_I2C1SEL_MSK) >> 12)
/* These bits are set and cleared by software to select the I2C1 clock source.
   00: PCLK selected as I2C1 clock
   01: System clock (SYSCLK) selected as I2C1 clock
   10: HSI16 clock selected as I2C1 clock
   11: Reserved */

/* Bits [10..11] - LPUART1 clock source selection */
#define RCC_LPUART1SEL_MSK (0x3 << 10)
#define RCC_LPUART1SEL_SET(VAL) (((VAL) << 10) & RCC_LPUART1SEL_MSK)
#define RCC_LPUART1SEL_GET(REG) (((REG) & RCC_LPUART1SEL_MSK) >> 10)
/* These bits are set and cleared by software to select the LPUART1 clock
   source.
   00: PCLK selected as LPUART1 clock
   01: System clock (SYSCLK) selected as LPUART1 clock
   10: HSI16 clock selected as LPUART1 clock
   11: LSE clock selected as LPUART1 clock */

/* Bits [8..9] - Reserved, must be kept at reset value. */

/* Bits [4..5] - USART3 clock source selection */
#define RCC_USART3SEL_MSK (0x3 << 4)
#define RCC_USART3SEL_SET(VAL) (((VAL) << 4) & RCC_USART3SEL_MSK)
#define RCC_USART3SEL_GET(REG) (((REG) & RCC_USART3SEL_MSK) >> 4)
/* This bit is set and cleared by software to select the USART3 clock source.
   00: PCLK selected as USART3 clock
   01: System clock (SYSCLK) selected as USART3 clock
   10: HSI16 clock selected as USART3 clock
   11: LSE clock selected as USART3 clock */

/* Bits [2..3] - USART2 clock source selection */
#define RCC_USART2SEL_MSK (0x3 << 2)
#define RCC_USART2SEL_SET(VAL) (((VAL) << 2) & RCC_USART2SEL_MSK)
#define RCC_USART2SEL_GET(REG) (((REG) & RCC_USART2SEL_MSK) >> 2)
/* This bit is set and cleared by software to select the USART2 clock source.
   00: PCLK selected as USART2 clock
   01: System clock (SYSCLK) selected as USART2 clock
   10: HSI16 clock selected as USART2 clock
   11: LSE clock selected as USART2 clock */

/* Bits [0..1] - USART1 clock source selection */
#define RCC_USART1SEL_MSK (0x3 << 0)
#define RCC_USART1SEL_SET(VAL) (((VAL) << 0) & RCC_USART1SEL_MSK)
#define RCC_USART1SEL_GET(REG) (((REG) & RCC_USART1SEL_MSK) >> 0)
/* This bit is set and cleared by software to select the USART1 clock source.
   00: PCLK selected as USART1 clock
   01: System clock (SYSCLK) selected as USART1 clock
   10: HSI16 clock selected as USART1 clock
   11: LSE clock selected as USART1 clock 1. Available on STM32L43xxx and
   STM32L44xxx devices only.
   2. Not available on STM32L432xx and STM32L442xx devices.
   3. Available on STM32L45xxx and STM32L46xxx devices only. */

/* ------------------------------------------------------------------------- */
/* Backup domain control register - BDCR */
#define STM32_RCC_BDCR 0x0090

/* Bits [26..31] - Reserved, must be kept at reset value. */

/* Bit 25 - Low speed clock output selection */
#define RCC_LSCOSEL (1 << 25)
/* Set and cleared by software.
   0: LSI clock selected
   1: LSE clock selected */

/* Bit 24 - Low speed clock output enable */
#define RCC_LSCOEN (1 << 24)
/* Set and cleared by software.
   0: Low speed clock output (LSCO) disable
   1: Low speed clock output (LSCO) enable */

/* Bits [17..23] - Reserved, must be kept at reset value. */

/* Bit 16 - Backup domain software reset */
#define RCC_BDRST (1 << 16)
/* Set and cleared by software.
   0: Reset not activated
   1: Reset the entire Backup domain */

/* Bit 15 - RTC clock enable */
#define RCC_RTCEN (1 << 15)
/* Set and cleared by software.
   0: RTC clock disabled
   1: RTC clock enabled */

/* Bits [10..14] - Reserved, must be kept at reset value. */

/* Bits [8..9] - RTC clock source selection */
#define RCC_RTCSEL_MSK (0x3 << 8)
#define RCC_RTCSEL_SET(VAL) (((VAL) << 8) & RCC_RTCSEL_MSK)
#define RCC_RTCSEL_GET(REG) (((REG) & RCC_RTCSEL_MSK) >> 8)
/* Set by software to select the clock source for the RTC. Once the RTC clock
   source has been selected, it cannot be changed anymore unless the Backup
   domain is reset, or unless a failure is detected on LSE (LSECSSD is set).
   The BDRST bit can be used to reset them.
   00: No clock
   01: LSE oscillator clock used as RTC clock
   10: LSI oscillator clock used as RTC clock
   11: HSE oscillator clock divided by 32 used as RTC clock */

/* Bit 7 - Reserved, must be kept at reset value. */

/* Bit 2 - LSE oscillator bypass */
#define RCC_LSEBYP (1 << 2)
/* Set and cleared by software to bypass oscillator in debug mode. This bit
   can be written only when the external 32 kHz oscillator is disabled
   (LSEON=0 and LSERDY=0).
   0: LSE oscillator not bypassed
   1: LSE oscillator bypassed */

/* Bit 1 - LSE oscillator ready */
#define RCC_LSERDY (1 << 1)
/* Set and cleared by hardware to indicate when the external 32 kHz oscillator
   is stable. After the LSEON bit is cleared, LSERDY goes low after 6 external
   low-speed oscillator clock cycles.
   0: LSE oscillator not ready
   1: LSE oscillator ready */

/* Bit 0 - LSE oscillator enable */
#define RCC_LSEON (1 << 0)
/* Set and cleared by software.
   0: LSE oscillator OFF
   1: LSE oscillator ON */

/* ------------------------------------------------------------------------- */
/* Control/status register - CSR */
#define STM32_RCC_CSR 0x0094

/* Bit 31 - Low-power reset flag */
#define RCC_LPWRRSTF (1 << 31)
/* Set by hardware when a reset occurs due to illegal Stop, Standby or
   Shutdown mode entry.
   Cleared by writing to the RMVF bit.
   0: No illegal mode reset occurred
   1: Illegal mode reset occurred */

/* Bit 30 - Window watchdog reset flag */
#define RCC_WWDGRSTF (1 << 30)
/* Set by hardware when a window watchdog reset occurs.
   Cleared by writing to the RMVF bit.
   0: No window watchdog reset occurred
   1: Window watchdog reset occurred */

/* Bit 29 - Independent window watchdog reset flag */
#define RCC_IWDGRSTF (1 << 29)
/* Set by hardware when an independent watchdog reset domain occurs.
   Cleared by writing to the RMVF bit.
   0: No independent watchdog reset occurred
   1: Independent watchdog reset occurred */

/* Bit 28 - Software reset flag */
#define RCC_SFTRSTF (1 << 28)
/* Set by hardware when a software reset occurs.
   Cleared by writing to the RMVF bit.
   0: No software reset occurred
   1: Software reset occurred */

/* Bit 27 - BOR flag */
#define RCC_BORRSTF (1 << 27)
/* Set by hardware when a BOR occurs.
   Cleared by writing to the RMVF bit.
   0: No BOR occurred
   1: BOR occurred */

/* Bit 26 - Pin reset flag */
#define RCC_PINRSTF (1 << 26)
/* Set by hardware when a reset from the NRST pin occurs.
   Cleared by writing to the RMVF bit.
   0: No reset from NRST pin occurred
   1: Reset from NRST pin occurred */

/* Bit 25 - Option byte loader reset flag */
#define RCC_OBLRSTF (1 << 25)
/* Set by hardware when a reset from the Option Byte loading occurs.
   Cleared by writing to the RMVF bit.
   0: No reset from Option Byte loading occurred
   1: Reset from Option Byte loading occurred */

/* Bit 24 - Firewall reset flag */
#define RCC_FWRSTF (1 << 24)
/* Set by hardware when a reset from the firewall occurs.
   Cleared by writing to the RMVF bit.
   0: No reset from the firewall occurred
   1: Reset from the firewall occurred */

/* Bit 23 - Remove reset flag */
#define RCC_RMVF (1 << 23)
/* Set by software to clear the reset flags.
   0: No effect
   1: Clear the reset flags */

/* Bits [12..22] - Reserved, must be kept at reset value. */

/* Bits [2..7] - Reserved, must be kept at reset value. */

/* Bit 1 - LSI oscillator ready */
#define RCC_LSIRDY (1 << 1)
/* Set and cleared by hardware to indicate when the LSI oscillator is stable.
   After the LSION bit is cleared, LSIRDY goes low after 3 LSI oscillator
   clock cycles. This bit can be set even if LSION = 0 if the LSI is requested
   by the Clock Security System on LSE, by the Independent Watchdog or by the
   RTC.
   0: LSI oscillator not ready
   1: LSI oscillator ready */

/* Bit 0 - LSI oscillator enable */
#define RCC_LSION (1 << 0)
/* Set and cleared by software.
   0: LSI oscillator OFF
   1: LSI oscillator ON */

/* ------------------------------------------------------------------------- */
/* Clock recovery RC register - CRRCR */
#define STM32_RCC_CRRCR 0x0098

/* Bits [16..31] - Reserved, must be kept at reset value */

/* Bits [7..15] - HSI48 clock calibration */
#define RCC_HSI48CAL_MSK (0x1ff << 7)
#define RCC_HSI48CAL_SET(VAL) (((VAL) << 7) & RCC_HSI48CAL_MSK)
#define RCC_HSI48CAL_GET(REG) (((REG) & RCC_HSI48CAL_MSK) >> 7)
/* These bits are initialized at startup with the factory-programmed HSI48
   calibration trim value.
   They are ready only. */

/* Bits [2..6] - Reserved, must be kept at reset value */

/* Bit 1 - HSI48 clock ready flag */
#define RCC_HSI48RDY (1 << 1)
/* Set by hardware to indicate that HSI48 oscillator is stable. This bit is
   set only when HSI48 is enabled by software by setting HSI48ON.
   0: HSI48 oscillator not ready
   1: HSI48 oscillator ready */

/* Bit 0 - HSI48 clock enable */
#define RCC_HSI48ON (1 << 0)
/* Set and cleared by software.
   Cleared by hardware to stop the HSI48 when entering in Stop, Standby or
   Shutdown modes.
   0: HSI48 oscillator OFF
   1: HSI48 oscillator ON */

/* ------------------------------------------------------------------------- */
/* Peripherals independent clock configuration register - CCIPR2 */
#define STM32_RCC_CCIPR2 0x009c

/* Bits [2..31] - Reserved, must be kept at reset value. */

/* Bits [0..1] - I2C4 clock source selection */
#define RCC_I2C4SEL_MSK (0x3 << 0)
#define RCC_I2C4SEL_SET(VAL) (((VAL) << 0) & RCC_I2C4SEL_MSK)
#define RCC_I2C4SEL_GET(REG) (((REG) & RCC_I2C4SEL_MSK) >> 0)
/* These bits are set and cleared by software to select the I2C4 clock source.
   00: PCLK selected as I2C4 clock
   01: System clock (SYSCLK) selected as I2C4 clock
   10: HSI16 clock selected as I2C4 clock
   11: reserved 6.4.32 RCC register map The following table gives the RCC
   register map and the reset values.
   0x20 0x1C 0x18 RCC_ICSCR set PLLSAI1 0x00 CFGR RCC_CFGR 0x08 */


/* ------------------------------------------------------------------------- */
#define STM32_AHB  0
#define STM32_AHB1 0
#define STM32_APB1 1
#define STM32_APB2 2
#define STM32_AHB2 3
#define STM32_AHB3 4
#define STM32_APB1B 5

/* AHB1 preipheral bit mapping */
#define RCC_TSC    16
#define RCC_CRC    12
#define RCC_FLASH  8
#define RCC_DMA2   1
#define RCC_DMA1   0

/* AHB2 preipheral bit mapping */
#define RCC_RNG     18
#define RCC_AES     16
#define RCC_ADC     13
#define RCC_GPIOH   7
#define RCC_GPIOE   4
#define RCC_GPIOD   3
#define RCC_GPIOC   2
#define RCC_GPIOB   1
#define RCC_GPIOA   0

/* AHB3 preipheral bit mapping */
#define RCC_QSPI   8

/* APB1 preipheral bit mapping */
#define RCC_LPTIM1   31
#define RCC_OPAMP    30
#define RCC_DAC1     29
#define RCC_PWR      28
#define RCC_USBFS    26
#define RCC_CAN1     25
#define RCC_CRS      24
#define RCC_I2C3     23
#define RCC_I2C2     22
#define RCC_I2C1     21
#define RCC_UART4    19
#define RCC_USART3   18
#define RCC_USART2   17
#define RCC_SPI3     15
#define RCC_SPI2     14
#define RCC_WWDG     11
#define RCC_RTC      10
#define RCC_LCD       9
#define RCC_TIM7      5
#define RCC_TIM6      4
#define RCC_TIM3      1
#define RCC_TIM2      0

/* APB1 preipheral bit mapping 2 */
#define RCC_LPTIM2  5
#define RCC_SWPMI1  2
#define RCC_I2C4    1
#define RCC_LPUART1 0

/* APB2 preipheral bit mapping */
#define RCC_DFSDM1  24
#define RCC_SAI1    21
#define RCC_TIM16   17
#define RCC_TIM15   16
#define RCC_USART1  14
#define RCC_SPI1    12
#define RCC_TIM1    11
#define RCC_SDMMC1  10
#define RCC_SYSCFG  0

#define STM32_CLK_TSC       STM32_AHB1, RCC_TSC
#define STM32_CLK_CRC       STM32_AHB1, RCC_CRC
#define STM32_CLK_FLASH     STM32_AHB1, RCC_FLASH
#define STM32_CLK_DMA2      STM32_AHB1, RCC_DMA2
#define STM32_CLK_DMA1      STM32_AHB1, RCC_DMA1

#define STM32_CLK_RNG       STM32_AHB2, RCC_RNG
#define STM32_CLK_AES       STM32_AHB2, RCC_AES
#define STM32_CLK_ADC       STM32_AHB2, RCC_ADC

#define STM32_CLK_GPIOH     STM32_AHB2, RCC_GPIOH
#define STM32_CLK_GPIOE     STM32_AHB2, RCC_GPIOE
#define STM32_CLK_GPIOD     STM32_AHB2, RCC_GPIOD
#define STM32_CLK_GPIOC     STM32_AHB2, RCC_GPIOC
#define STM32_CLK_GPIOB     STM32_AHB2, RCC_GPIOB
#define STM32_CLK_GPIOA     STM32_AHB2, RCC_GPIOA

#define STM32_CLK_QSPI      STM32_AHB3, RCC_QSPI

#define STM32_CLK_LPTIM1    STM32_APB1, RCC_LPTIM1
#define STM32_CLK_OPAMP     STM32_APB1, RCC_OPAMP
#define STM32_CLK_DAC1      STM32_APB1, RCC_DAC1
#define STM32_CLK_PWR       STM32_APB1, RCC_PWR
#define STM32_CLK_USB       STM32_APB1, RCC_USBFS
#define STM32_CLK_CAN1      STM32_APB1, RCC_CAN1
#define STM32_CLK_CRS       STM32_APB1, RCC_CRS
#define STM32_CLK_I2C3      STM32_APB1, RCC_I2C3
#define STM32_CLK_I2C2      STM32_APB1, RCC_I2C2
#define STM32_CLK_I2C1      STM32_APB1, RCC_I2C1
#define STM32_CLK_UART4     STM32_APB1, RCC_UART4
#define STM32_CLK_USART3    STM32_APB1, RCC_USART3
#define STM32_CLK_USART2    STM32_APB1, RCC_USART2
#define STM32_CLK_SPI3      STM32_APB1, RCC_SPI3
#define STM32_CLK_SPI2      STM32_APB1, RCC_SPI2
#define STM32_CLK_LCD       STM32_APB1, RCC_LCD
#define STM32_CLK_TIM7      STM32_APB1, RCC_TIM7
#define STM32_CLK_TIM6      STM32_APB1, RCC_TIM6
#define STM32_CLK_TIM3      STM32_APB1, RCC_TIM3
#define STM32_CLK_TIM2      STM32_APB1, RCC_TIM2

#define STM32_CLK_LPTIM2    STM32_APB1B, RCC_LPTIM2
#define STM32_CLK_SWPMI1    STM32_APB1B, RCC_SWPMI1
#define STM32_CLK_I2C4      STM32_APB1B, RCC_I2C4
#define STM32_CLK_LPUART1   STM32_APB1B, RCC_LPUART1

#define STM32_CLK_DFSDM1    STM32_APB2, RCC_DFSDM1
#define STM32_CLK_SAI1      STM32_APB2, RCC_SAI1
#define STM32_CLK_TIM16     STM32_APB2, RCC_TIM16
#define STM32_CLK_TIM15     STM32_APB2, RCC_TIM15
#define STM32_CLK_USART1    STM32_APB2, RCC_USART1
#define STM32_CLK_SPI1      STM32_APB2, RCC_SPI1
#define STM32_CLK_TIM1      STM32_APB2, RCC_TIM1
#define STM32_CLK_SDMMC1    STM32_APB2, RCC_SDMMC1
#define STM32_CLK_SYSCFG    STM32_APB2, RCC_SYSCFG

#ifndef __ASSEMBLER__

#include <stdint.h>

struct stm32_rcc {
	volatile uint32_t cr; /* 0x00 */
	volatile uint32_t icscr; /* 0x04 */
	volatile uint32_t cfgr; /* 0x08 */
	volatile uint32_t pllcfgr; /* 0x0c */
	volatile uint32_t pllsai1cfgr; /* 0x10 */
	uint32_t res0[1];
	volatile uint32_t cier; /* 0x18 */
	volatile uint32_t cifr; /* 0x1c */
	volatile uint32_t cicr; /* 0x20 */
	uint32_t res1[1];
	volatile uint32_t ahb1rstr; /* 0x28 */
	volatile uint32_t ahb2rstr; /* 0x2c */
	volatile uint32_t ahb3rstr; /* 0x30 */
	uint32_t res2[1];
	volatile uint32_t apb1rstr1; /* 0x38 */
	volatile uint32_t apb1rstr2; /* 0x3c */
	volatile uint32_t apb2rstr; /* 0x40 */
	uint32_t res3[1];
	volatile uint32_t ahb1enr; /* 0x48 */
	volatile uint32_t ahb2enr; /* 0x4c */
	volatile uint32_t ahb3enr; /* 0x50 */
	uint32_t res4[1];
	volatile uint32_t apb1enr1; /* 0x58 */
	volatile uint32_t apb1enr2; /* 0x5c */
	volatile uint32_t apb2enr; /* 0x60 */
	uint32_t res5[1];
	volatile uint32_t ahb1smenr; /* 0x68 */
	volatile uint32_t ahb2smenr; /* 0x6c */
	volatile uint32_t ahb3smenr; /* 0x70 */
	uint32_t res6[1];
	volatile uint32_t apb1smenr1; /* 0x78 */
	volatile uint32_t apb1smenr2; /* 0x7c */
	volatile uint32_t apb2smenr; /* 0x80 */
	uint32_t res7[1];
	volatile uint32_t ccipr; /* 0x88 */
	uint32_t res8[1];
	volatile uint32_t bdcr; /* 0x90 */
	volatile uint32_t csr; /* 0x94 */
	volatile uint32_t crrcr; /* 0x98 */
	volatile uint32_t ccipr2; /* 0x9c */
};

struct stm32_clk {
	uint8_t bus:3;
	uint8_t bit:5;
} __attribute__ ((packed)) __;

#if 0
enum {
	STM32_CLK_AHB = 0,
	STM32_CLK_APB1 = 1,
	STM32_CLK_APB2 = 2,
	STM32_CLK_TIM1 = 3,
	STM32_CLK_TIM2 = 4,
	STM32_CLK_HSI = 5,
	STM32_CLK_HSE = 6,
	STM32_CLK_LSI = 7,
	STM32_CLK_LSE = 8,
	STM32_CLK_MSI = 9,
	STM32_CLK_MCO = 10,
	STM32_CLK_SAI = 11,
	STM32_CLK_I2S = 12
};

extern const uint32_t stm32_clk_hz[];
#endif

extern const uint32_t stm32f_apb1_hz;
extern const uint32_t stm32f_apb2_hz;
extern const uint32_t stm32f_ahb_hz;
extern const uint32_t stm32f_mco_hz;
extern const uint32_t stm32f_tim1_hz;
extern const uint32_t stm32f_tim2_hz;
extern const uint32_t stm32f_sai_hz;

#ifdef __cplusplus
extern "C" {
#endif

	static inline void stm32_clk_enable(struct stm32_rcc *rcc,
					    int bus, int bit) {
		uint32_t volatile *enr;

		if (bus == STM32_APB2)
			enr = &rcc->apb2enr;
		else if (bus == STM32_APB1)
			enr = &rcc->apb1enr1;
		else if (bus == STM32_APB1B)
			enr = &rcc->apb1enr2;
		else if (bus == STM32_AHB2)
			enr = &rcc->ahb2enr;
		else if (bus == STM32_AHB3)
			enr = &rcc->ahb3enr;
		else
			 enr = &rcc->ahb1enr;

		*enr |= 1 << bit;
		asm volatile ("dsb":);
	} static inline void stm32_clk_disable(struct stm32_rcc *rcc,
					       int bus, int bit) {
		uint32_t volatile *enr;

		if (bus == STM32_APB2)
			enr = &rcc->apb2enr;
		else if (bus == STM32_APB1)
			enr = &rcc->apb1enr1;
		else if (bus == STM32_APB1B)
			enr = &rcc->apb1enr2;
		else if (bus == STM32_AHB2)
			enr = &rcc->ahb2enr;
		else if (bus == STM32_AHB3)
			enr = &rcc->ahb3enr;
		else
			enr = &rcc->ahb1enr;

		*enr &= ~(1 << bit);
		asm volatile ("dsb":);
	}

	static inline void stm32_reset(struct stm32_rcc *rcc, int bus, int bit) {
		uint32_t volatile *rstr;

		if (bus == STM32_APB2)
			rstr = &rcc->apb2rstr;
		else if (bus == STM32_APB1)
			rstr = &rcc->apb1rstr1;
		else if (bus == STM32_APB1B)
			rstr = &rcc->apb1rstr2;
		else if (bus == STM32_AHB2)
			rstr = &rcc->ahb2rstr;
		else if (bus == STM32_AHB3)
			rstr = &rcc->ahb3rstr;
		else
			rstr = &rcc->ahb1rstr;

		*rstr |= 1 << bit;
		asm volatile ("dsb":);
		*rstr &= ~(1 << bit);
		asm volatile ("dsb":);
	}

	static inline uint32_t stm32_clk_hz(int bus, int bit) {
		if (bus == STM32_APB2)
			return stm32f_apb2_hz;
		else if ((bus == STM32_APB1) || (bus == STM32_APB1B))
			return stm32f_apb2_hz;
		return stm32f_ahb_hz;
	}

#ifdef __cplusplus
}
#endif

#endif				/* __ASSEMBLER__ */

#endif				/* __STM32L4_RCC_H__ */
