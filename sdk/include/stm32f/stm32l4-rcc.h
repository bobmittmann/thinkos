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

#ifndef __STM32L4_RCC_H__
#define __STM32L4_RCC_H__

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

/*-------------------------------------------------------------------------
  Reset and clock control (RCC)
  ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* RCC clock control register */ 
#define STM32_RCC_CR 0x00

#define RCC_PLLSAI1RDY (1 << 27)
/* PLLSAI1 clock ready flag
   Set by hardware to indicate that the PLLSAI1 is locked.
0: PLLSAI1 unlocked
1: PLLSAI1 locked*/

#define RCC_PLLSAI1ON (1 << 26)
/* PLLSAI1 enable
   Set and cleared by software to enable PLLSAI1.
   Cleared by hardware when entering Stop or Standby mode.
0: PLLSAI1 OFF
1: PLLSAI1 ON*/

#define RCC_PLLRDY (1 << 25)
/* Main PLL (PLL) clock ready flag
   Set by hardware to indicate that PLL is locked.
0: PLL unlocked
1: PLL locked*/

#define RCC_PLLON (1 << 24)
/* Main PLL (PLL) enable
   Set and cleared by software to enable PLL.
   Cleared by hardware when entering Stop or Standby mode. This bit cannot be reset if PLL
   clock is used as the system clock.
0: PLL OFF
1: PLL ON
 */

#define RCC_CSSON (1 << 19)
/* Clock security system enable
   Set and cleared by software to enable the clock security system. When CSSON is set, the
   clock detector is enabled by hardware when the HSE oscillator is ready, and disabled by
   hardware if an oscillator failure is detected.
0: Clock security system OFF (Clock detector OFF)
1: Clock security system ON (Clock detector ON if HSE oscillator is stable, OFF if not)*/

#define RCC_HSEBYP (1 << 18)
/* HSE clock bypass
   Set and cleared by software to bypass the oscillator with an external clock. The external
   clock must be enabled with the HSEON bit, to be used by the device.
   The HSEBYP bit can be written only if the HSE oscillator is disabled.
0: HSE oscillator not bypassed
1: HSE oscillator bypassed with an external clock*/

#define RCC_HSERDY (1 << 17)
/* HSE clock ready flag
   Set by hardware to indicate that the HSE oscillator is stable. After the HSION bit is cleared,
   HSERDY goes low after 6 HSE oscillator clock cycles.
0: HSE oscillator not ready
1: HSE oscillator ready*/

#define RCC_HSEON (1 << 16)
/* HSE clock enable
   Set and cleared by software.
   Cleared by hardware to stop the HSE oscillator when entering Stop or Standby mode. This
   bit cannot be reset if the HSE oscillator is used directly or indirectly as the system clock.
0: HSE oscillator OFF
1: HSE oscillator ON*/



#define RCC_HSIASFS (1 << 11)
/*
HSIASFS: HSI16 automatic start from Stop
Set and cleared by software. When the system wakeup clock is MSI, this bit is used to
wakeup the HSI16 is parallel of the system wakeup.
0: HSI16 oscillator is not enabled by hardware when exiting Stop mode with MSI as wakeup
clock.
1: HSI16 oscillator is enabled by hardware when exiting Stop mode with MSI as wakeup
clock.
*/

#define RCC_HSIRDY (1 << 10)
/* Internal high-speed clock ready flag
   Set by hardware to indicate that the HSI oscillator is stable. After the HSION bit is cleared,
   HSIRDY goes low after 6 HSI clock cycles.
0: HSI oscillator not ready
1: HSI oscillator ready*/

#define RCC_HSIKERON (1 << 9)
/*Bit 10 HSIRDY: HSI16 clock ready flag
Set by hardware to indicate that HSI16 oscillator is stable. This bit is set only when HSI16 is
enabled by software by setting HSION.
0: HSI16 oscillator not ready
1: HSI16 oscillator ready
Note: Once the HSION bit is cleared, HSIRDY goes low after 6 HSI16 clock cycles.
Bit 9 HSIKERON: HSI16 always enable for peripheral kernels.
Set and cleared by software to force HSI16 ON even in Stop modes. The HSI16 can only
feed USARTs and I2Cs peripherals configured with HSI16 as kernel clock. Keeping the
HSI16 ON in Stop mode allows to avoid slowing down the communication speed because of
the HSI16 startup time. This bit has no effect on HSION value.
0: No effect on HSI16 oscillator.
1: HSI16 oscillator is forced ON even in Stop mode.
Bit 8 HSION: HSI16 clock enable
Set and cleared by software.
Cleared by hardware to stop the HSI16 oscillator when entering Stop, Standby or Shutdown
mode.
Set by hardware to force the HSI16 oscillator ON when STOPWUCK=1 or HSIASFS = 1
when leaving Stop modes, or in case of failure of the HSE crystal oscillator.
This bit is set by hardware if the HSI16 is used directly or indirectly as system clock.
0: HSI16 oscillator OFF
1: HSI16 oscillator ON
*/

#define RCC_HSION (1 << 8)
/* Internal high-speed clock enable
   Set and cleared by software.
   Set by hardware to force the HSI oscillator ON when leaving the Stop or Standby mode or in
   case of a failure of the HSE oscillator used directly or indirectly as the system clock. This bit
   cannot be cleared if the HSI is used directly or indirectly as the system clock.
0: HSI oscillator OFF
1: HSI oscillator ON
 */

/*
MSIRANGE[3:0]: MSI clock ranges
These bits are configured by software to choose the frequency range of MSI when
MSIRGSEL is set.12 frequency ranges are available:
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
Note: Warning: MSIRANGE can be modified when MSI is OFF (MSION=0) or when MSI is
ready (MSIRDY=1). MSIRANGE must NOT be modified when MSI is ON and NOT
ready (MSION=1 and MSIRDY=0)
	Bit 3 MSIRGSEL: MSI clock range selection
	Set by software to select the MSI clock range with MSIRANGE[3:0]. Write 0 has no effect.
	After a standby or a reset MSIRGSEL is at 0 and the MSI range value is provided by
	MSISRANGE in CSR register.
	0: MSI Range is provided by MSISRANGE[3:0] in RCC_CSR register
	1: MSI Range is provided by MSIRANGE[3:0] in the RCC_CR register
	Bit 2 MSIPLLEN: MSI clock PLL enable
	Set and cleared by software to enable/ disable the PLL part of the MSI clock source.
	MSIPLLEN must be enabled after LSE is enabled (LSEON enabled) and ready (LSERDY set
																			 by hardware).There is a hardware protection to avoid enabling MSIPLLEN if LSE is not
	ready.
	This bit is cleared by hardware when LSE is disabled (LSEON = 0) or when the Clock
	Security System on LSE detects a LSE failure (refer to RCC_CSR register).
	0: MSI PLL OFF
	1: MSI PLL ON
	Bit 1 MSIRDY: MSI clock ready flag
	This bit is set by hardware to indicate that the MSI oscillator is stable.
	0: MSI oscillator not ready
	1: MSI oscillator ready
	Note: Once the MSION bit is cleared, MSIRDY goes low after 6 MSI clock cycles
	*/

#define RCC_MSION (1 << 0)
/* MSI clock enable
   This bit is set and cleared by software.
   Set by hardware to force the MSI oscillator ON when exiting from Stop 
   or Standby mode, or in case of a failure of the HSE oscillator used 
   directly or indirectly as system clock. This bit cannot be cleared if the 
   MSI is used as system clock.
 */

/* ------------------------------------------------------------------------- */
/* RCC clock configuration register */

/* STM32F10x RCC clock configuration register */
#define STM32_RCC_CFGR 0x08

#define RCC_MCOPRE(N) (ILOG2(N) << 28)
#define RCC_MCOPRE_1  (0x0 << 28)
#define RCC_MCOPRE_2  (0x1 << 28)
#define RCC_MCOPRE_4  (0x2 << 28)
#define RCC_MCOPRE_8  (0x3 << 28)
#define RCC_MCOPRE_16 (0x4 << 28)
/* Bits 30:28 MCOPRE[2:0]: Microcontroller clock output prescaler
These bits are set and cleared by software.
It is highly recommended to change this prescaler before MCO output is enabled.
000: MCO is divided by 1
001: MCO is divided by 2
010: MCO is divided by 4
011: MCO is divided by 8
100: MCO is divided by 16
Others: not allowed */

#define RCC_MCO_SYSCLK (0x1 << 24)
#define RCC_MCO_MSI    (0x2 << 24)
#define RCC_MCO_HSI    (0x3 << 24)
#define RCC_MCO_HSE    (0x4 << 24)
#define RCC_MCO_PLL    (0x5 << 24)
#define RCC_MCO_LSI    (0x6 << 24)
#define RCC_MCO_LSE    (0x7 << 24)
#define RCC_MCO_HSI48  (0x8 << 24)
/* Bits 27:24 MCOSEL[3:0]: Microcontroller clock output
Set and cleared by software.
0000: MCO output disabled, no clock on MCO
0001: SYSCLK system clock selected
0010: MSI clock selected.
0011: HSI16 clock selected.
0100: HSE clock selected
0101: Main PLL clock selected
0110: LSI clock selected
0111: LSE clock selected
1000: Internal HSI48 clock selected */

#define RCC_STOPWUCK (1 << 15)
/* Bit 15 STOPWUCK: Wakeup from Stop and CSS backup clock selection
	Set and cleared by software to select the system clock used when 
	exiting Stop mode.
	The selected clock is also used as emergency clock for the Clock 
	Security System on HSE.
	Warning: STOPWUCK must not be modified when the Clock Security 
	System is enabled by HSECSSON in RCC_CR register and the system 
	clock is HSE (SWS=”10”) or a switch on
	HSE is requested (SW=”10”).
	0: MSI oscillator selected as wakeup from stop clock and CSS backup clock.
	1: HSI16 oscillator selected as wakeup from stop clock and CSS backup clock
*/

/* APB high-speed prescaler (APB2) */
#define RCC_PPRE2(N) ((ILOG2(N) + 3) << 11)
#define RCC_PPRE2_1  (0x0 << 11)
#define RCC_PPRE2_2  (0x4 << 11)
#define RCC_PPRE2_4  (0x5 << 11)
#define RCC_PPRE2_8  (0x6 << 11)
#define RCC_PPRE2_16 (0x7 << 11)
/* These bits are set and cleared by software to control the division 
   factor of the APB high- speed clock (PCLK2).
0xx: HCLK not divided
100: HCLK divided by 2
101: HCLK divided by 4
110: HCLK divided by 8
111: HCLK divided by 16 */

/* APB low-speed prescaler (APB1) */
#define RCC_PPRE1(N) (ILOG2(N) << 8)
#define RCC_PPRE1_1  (0x0 << 8)
#define RCC_PPRE1_2  (0x4 << 8)
#define RCC_PPRE1_4  (0x5 << 8)
#define RCC_PPRE1_8  (0x6 << 8)
#define RCC_PPRE1_16 (0x7 << 8)

/* AHB prescaler */
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
/* Set and cleared by software to control AHB clock division factor.
Caution: The clocks are divided with the new prescaler factor from 
   1 to 16 AHB cycles after HPRE write.
Caution: The AHB clock frequency must be at least 25 MHz when the Ethernet 
   is used.
0xxx: system clock not divided
1000: system clock divided by 2
1001: system clock divided by 4
1010: system clock divided by 8
1011: system clock divided by 16
1100: system clock divided by 64
1101: system clock divided by 128
1110: system clock divided by 256
1111: system clock divided by 512 */

#define RCC_SWS_MASK (0x3 << 2)
#define RCC_SWS_MSI (0x0 << 2)
#define RCC_SWS_HSI (0x1 << 2)
#define RCC_SWS_HSE (0x2 << 2)
#define RCC_SWS_PLL (0x3 << 2)
/* System clock switch status. Set and cleared by hardware to indicate which 
   clock source is used as the system clock.
00: HSI oscillator used as the system clock
01: HSE oscillator used as the system clock
10: PLL used as the system clock
11: not applicable */

#define RCC_SW (0x3 << 0)
#define RCC_SW_MSI (0x0 << 0)
#define RCC_SW_HSI (0x1 << 0)
#define RCC_SW_HSE (0x2 << 0)
#define RCC_SW_PLL (0x3 << 0)

/* System clock switch
   Set and cleared by software to select the system clock source.
   Set by hardware to force the HSI selection when leaving the Stop or Standby mode or in case of failure of the HSE oscillator used directly or indirectly as the system clock.
00: HSI oscillator selected as system clock
01: HSE oscillator selected as system clock
10: PLL selected as system clock
11: not allowed
 */

/* ------------------------------------------------------------------------- */
/* RCC PLL configuration register */
#define STM32_RCC_PLLCFGR 0x0C

/* Main PLL division factor for PLLSAI2CLK */
#define RCC_PLLPDIV_MASK (0x1f << 27)
#define RCC_PLLPDIV(Q) (((Q) & 0x1f) << 27)
/* Set and cleared by software to control the SAI1 clock frequency. 
   PLLSAI2CLK output clock
   frequency = VCO frequency / PLLPDIV.
   00000: PLLSAI2CLK is controlled by the bit PLLP
   00001: Reserved.
   00010: PLLSAI2CLK = VCO / 2
   ....
   11111: PLLSAI2CLK = VCO / 31
   */

#define RCC_PLLR_MASK (0x3 << 25)
#define RCC_PLLR(R) (((((R) >> 1) - 1) & 0x3) << 25)
/* Main PLL division factor for PLLCLK (system clock)
   Set and cleared by software to control the frequency of the main 
   PLL output clock PLLCLK. This output can be selected as system clock. 
   These bits can be written only if PLL is disabled.
   PLLCLK output clock frequency = VCO frequency / PLLR 
   with PLLR = 2, 4, 6, or 8
   00: PLLR = 2
   01: PLLR = 4
   10: PLLR = 6
   11: PLLR = 8
   */

#define RCC_PLLREN (1 << 24)
/* Main PLL PLLCLK output enable
   Set and reset by software to enable the PLLCLK output of the main 
   PLL (used as system clock).
   This bit cannot be written when PLLCLK output of the PLL is used as 
   System Clock.
   In order to save power, when the PLLCLK output of the PLL is not 
   used, the value of
   PLLREN should be 0.
   0: PLLCLK output disable
   1: PLLCLK output enable
*/

#define RCC_PLLQ_MASK (0x3 << 21)
#define RCC_PLLQ(R) (((((R) >> 1) - 1) & 0x3) << 21)
/* Main PLL division factor for PLL48M1CLK (48 MHz clock).
   Set and cleared by software to control the frequency of the main PLL output 
   clock PLL48M1CLK. This output can be selected for USB, RNG, 
   SDMMC (48 MHz clock). These bits can be written only if PLL is disabled.
   PLL48M1CLK output clock frequency = VCO frequency / PLLQ with 
   PLLQ = 2, 4, 6, or 8
   00: PLLQ = 2
   01: PLLQ = 4
   10: PLLQ = 6
   11: PLLQ = 8
*/

#define RCC_PLLQEN (1 << 20)
/* Main PLL PLL48M1CLK output enable
   Set and reset by software to enable the PLL48M1CLK output of the main PLL.
   In order to save power, when the PLL48M1CLK output of the PLL is not 
   used, the value of
   PLLQEN should be 0.
   0: PLL48M1CLK output disable
   1: PLL48M1CLK output enable
*/

#define RCC_PLLP   (1 << 17)
/* Main PLL division factor for PLLSAI2CLK (SAI1 clock).
   Set and cleared by software to control the frequency of the main PLL 
   output clock PLLSAI2CLK. This output can be selected for SAI1. These bits 
   can be written only if PLL is disabled.
   When the PLLPDIV[4:0] is set to “00000”, PLLSAI2CLK output clock 
   frequency = VCO frequency / PLLP with PLLP =7, or 17
   0: PLLP = 7
   1: PLLP = 17
*/

#define RCC_PLLPEN  (1 << 16)
/* Main PLL PLLSAI1CLK output enable
   Set and reset by software to enable the PLLSAI2CLK output of the main PLL.
   In order to save power, when the PLLSAI2CLK output of the PLL is not 
   used, the value of PLLPEN should be 0.
   0: PLLSAI2CLK output disable
   1: PLLSAI2CLK output enable
*/

#define RCC_PLLN_MASK (0x7f << 8)
#define RCC_PLLN(N) (((N) & 0x7f) << 8)
/* Main PLL multiplication factor for VCO
   Set and cleared by software to control the multiplication factor 
   of the VCO. These bits can be written only when the PLL is disabled.
   VCO output frequency = VCO input frequency x PLLN with 8 =< PLLN =< 86
   0000000: PLLN = 0 wrong configuration
   0000001: PLLN = 1 wrong configuration
   ...
   0000111: PLLN = 7 wrong configuration
   0001000: PLLN = 8
   0001001: PLLN = 9
   ...
   1010101: PLLN = 85
   1010110: PLLN = 86
   1010111: PLLN = 87 wrong configuration
   ...
   1111111: PLLN = 127 wrong configuration
   Caution: The software has to set correctly these bits to assure that the VCO
   output frequency is between 64 and 344 MHz
*/

#define RCC_PLLM_MASK (0x7 << 4)
#define RCC_PLLM(M) ((((M) - 1) & 0x7) << 4)
/* Division factor for the main PLL and audio PLL (PLLSAI1) input clock
   Set and cleared by software to divide the PLL and PLLSAI1 input 
   clock before the VCO.
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
   Caution: The software has to set these bits correctly to ensure 
   that the VCO input frequency ranges from 4 to 16 MHz.
*/

#define RCC_PLLMSRC_MASK (0x3 << 0)
#define RCC_PLLSRC_NONE (0 << 0)
#define RCC_PLLSRC_MSI  (1 << 0)
#define RCC_PLLSRC_HSI  (2 << 0)
#define RCC_PLLSRC_HSE  (3 << 0)
/* Main PLL and PLLSAI1 entry clock source
   Set and cleared by software to select PLL and PLLSAI1 clock source. These 
   bits can be written only when PLL and PLLSAI1 are disabled.
   In order to save power, when no PLL is used, the value of PLLSRC 
   should be 00.
   00: No clock sent to PLL and PLLSAI1
   01: MSI clock selected as PLL and PLLSAI1 clock entry
   10: HSI16 clock selected as PLL and PLLSAI1 clock entry
   11: HSE clock selected as PLL and PLLSAI1 clock entry
   */

/* ------------------------------------------------------------------------- */
/* RCC PLLSAI1 configuration register */
#define STM32_RCC_PLLSAI1CFGR 0x10
/* This register is used to configure the PLLSAI1 clock outputs according 
   to the formulas:
   • f(VCOSAI1 clock) = f(PLL clock input) × (PLLSAI1N / PLLM)
   • f(PLLSAI1_P) = f(VCOSAI1 clock) / PLLSAI1P
   • f(PLLSAI1_Q) = f(VCOSAI1 clock) / PLLSAI1Q
   • f(PLLSAI1_R) = f(VCOSAI1 clock) / PLLSAI1R
   */

/*
Bits 31:27 PLLSAI1PDIV[4:0]: PLLSAI1 division factor for PLLSAI1CLK
Set and cleared by software to control the SAI1 clock frequency. PLLSAI1CLK output clock
frequency = VCOSAI1 frequency / PLLPDIV.
00000: PLLSAI1CLK is controlled by the bit PLLP
00001: Reserved.
00010: PLLSAI1CLK = VCOSAI1 / 2
....
11111: PLLSAI1CLK = VCOSAI1 / 31
Bits 26:25 PLLSAI1R[1:0]: PLLSAI1 division factor for PLLADC1CLK (ADC clock)
Set and cleared by software to control the frequency of the PLLSAI1 output clock
PLLADC1CLK. This output can be selected as ADC clock. These bits can be written only if
PLLSAI1 is disabled.
PLLADC1CLK output clock frequency = VCOSAI1 frequency / PLLSAI1R with PLLSAI1R =
2, 4, 6, or 8
00: PLLSAI1R = 2
01: PLLSAI1R = 4
10: PLLSAI1R = 6
11: PLLSAI1R = 8
Bit 24 PLLSAI1REN: PLLSAI1 PLLADC1CLK output enable
Set and reset by software to enable the PLLADC1CLK output of the PLLSAI1 (used as clock
for ADC).
In order to save power, when the PLLADC1CLK output of the PLLSAI1 is not used, the value
of PLLSAI1REN should be 0.
0: PLLADC1CLK output disable
1: PLLADC1CLK output enable
Bit 23 Reserved, must be kept at reset value.

Bits 22:21 PLLSAI1Q[1:0]: PLLSAI1 division factor for PLL48M2CLK (48 MHz clock)
Set and cleared by software to control the frequency of the PLLSAI1 output clock
PLL48M2CLK. This output can be selected for USB(1), RNG, SDMMC (48 MHz clock).
These bits can be written only if PLLSAI1 is disabled.
PLL48M2CLK output clock frequency = VCOSAI1 frequency / PLLQ with PLLQ = 2, 4, 6, or
8
00: PLLQ = 2
01: PLLQ = 4
10: PLLQ = 6
11: PLLQ = 8
Caution: The software has to set these bits correctly not to exceed 80 MHz on
this domain.
Bit 20 PLLSAI1QEN: PLLSAI1 PLL48M2CLK output enable
Set and reset by software to enable the PLL48M2CLK output of the PLLSAI1.
In order to save power, when the PLL48M2CLK output of the PLLSAI1 is not used, the value
of PLLSAI1QEN should be 0.
0: PLL48M2CLK output disable
1: PLL48M2CLK output enable
Bits 19:18 Reserved, must be kept at reset value.
Bit 17 PLLSAI1P: PLLSAI1 division factor for PLLSAI1CLK (SAI1 clock).
Set and cleared by software to control the frequency of the PLLSAI1 output clock
PLLSAI1CLK. This output can be selected for SAI1. These bits can be written only if
PLLSAI1 is disabled.
When the PLLSAI1PDIV[4:0] is set to “00000”, PLLSAI1CLK output clock frequency =
VCOSAI1 frequency / PLLSAI1P with PLLSAI1P =7, or 17
0: PLLSAI1P = 7
1: PLLSAI1P = 17
Bit 16 PLLSAI1PEN: PLLSAI1 PLLSAI1CLK output enable
Set and reset by software to enable the PLLSAI1CLK output of the PLLSAI1.
In order to save power, when the PLLSAI1CLK output of the PLLSAI1 is not used, the value
of PLLSAI1PEN should be 0.
0: PLLSAI1CLK output disable
1: PLLSAI1CLK output enable


Bit 15 Reserved, must be kept at reset value.
Bits 14:8 PLLSAI1N[6:0]: PLLSAI1 multiplication factor for VCO
Set and cleared by software to control the multiplication factor of the VCO. These bits can be
written only when the PLLSAI1 is disabled.
VCOSAI1 output frequency = VCOSAI1 input frequency x PLLSAI1N
with 8 =< PLLSAI1N =< 86
0000000: PLLSAI1N = 0 wrong configuration
0000001: PLLSAI1N = 1 wrong configuration
...
0000111: PLLSAI1N = 7 wrong configuration
0001000: PLLSAI1N = 8
0001001: PLLSAI1N = 9
...
1010101: PLLSAI1N = 85
1010110: PLLSAI1N = 86
1010111: PLLSAI1N = 87 wrong configuration
...
1111111: PLLSAI1N = 127 wrong configuration
Caution: The software has to set correctly these bits to ensure that the VCO
output frequency is between 64 and 344 MHz.
Bits 7:0 Reserved, must be kept at reset value.

   */

/* ------------------------------------------------------------------------- */
/* AHB1 peripheral reset register  */
#define STM32_RCC_AHB1RSTR 0x28
/* AHB1 peripheral clock enable register */
#define STM32_RCC_AHB1ENR 0x48
/* AHB1 peripheral clock enable in low power mode register */
#define STM32_RCC_AHBSMENR 0x68
/* AHB1 preipheral bit mapping */

#define RCC_TSC    16
#define RCC_CRC    12
#define RCC_FLASH  8
#define RCC_DMA2   1
#define RCC_DMA1   0

/* ------------------------------------------------------------------------- */
/* AHB1 peripheral reset register  */
#define STM32_RCC_AHB2RSTR 0x2c
/* AHB1 peripheral clock enable register */
#define STM32_RCC_AHB2ENR 0x4c
/* AHB1 peripheral clock enable in low power mode register */
#define STM32_RCC_AHB2SMENR 0x6c
/* AHB1 preipheral bit mapping */

#define RCC_RNG     18
#define RCC_AES     16
#define RCC_ADC     13

#define RCC_GPIOH   7
#define RCC_GPIOE   4
#define RCC_GPIOD   3
#define RCC_GPIOC   2
#define RCC_GPIOB   1
#define RCC_GPIOA   0

/* ------------------------------------------------------------------------- */
/* AHB3 peripheral reset register  */
#define STMF3_RCC_AHB3RSTR 0x30
/* AHB3 peripheral clock enable register */
#define STM32_RCC_AHB3ENR 0x50
/* AHB3 peripheral clock enable in low power mode register */
#define STM32_RCC_AHB3SMENR 0x70
/* AHB3 preipheral bit mapping */

#define RCC_QSPI   8

/* ------------------------------------------------------------------------- */
/* APB1 peripheral reset register */
#define STM32_RCC_APB1RSTR1 0x38
/* APB1 peripheral clock enable register */
#define STM32_RCC_APB1ENR1 0x58
/* APB1 peripheral clock enable in low power mode register */
#define STM32_RCC_APB1LPENR1 0x78
/* APB1 preipheral bit mapping */
#define RCC_LPTIM1  31
#define RCC_OPAMP   30
#define RCC_DAC1    29 
#define RCC_PWR     28 
#define RCC_USBFS   26
#define RCC_CAN1    25 
#define RCC_CRS     24 
#define RCC_I2C3    23 
#define RCC_I2C2    22 
#define RCC_I2C1    21 
#define RCC_UART4   19 
#define RCC_USART3  18  
#define RCC_USART2  17   
#define RCC_SPI3    15
#define RCC_SPI2    14
#define RCC_LCD     9
#define RCC_TIM7    5
#define RCC_TIM6    4
#define RCC_TIM3    1
#define RCC_TIM2    0

/* ------------------------------------------------------------------------- */
/* APB1 peripheral reset register 2 */
#define STM32_RCC_APB1RSTR2 0x3c
/* APB1 peripheral clock enable register 2 */
#define STM32_RCC_APB1ENR2 0x5c
/* APB1 peripheral clock enable in low power mode register 2 */
#define STM32_RCC_APB1LPENR2 0x7c
/* APB1 preipheral bit mapping 2 */
#define RCC_LPTIM2  5
#define RCC_SWPMI1  2
#define RCC_I2C4    1 
#define RCC_LPUART1 0

/* ------------------------------------------------------------------------- */
/* APB2 peripheral reset register */
#define STM32_RCC_APB2RSTR 0x14
/* APB2 peripheral clock enable register */
#define STM32_RCC_APB2ENR 0x20
/* APB2 peripheral clock enable in low power mode register */
#define STM32_RCC_APB2LPENR 0x2c
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

/* ------------------------------------------------------------------------- */
/* RCC CCIPR Peripherals independent clock configuration register  */
#define STM32_RCC_CCIPR 0x88

/* Bit 31 Reserved, must be kept at reset value. */

/*   Bit 30 SWPMI1SEL(1): SWPMI1 clock source selection */
 /*  This bit is set and cleared by software to select the SWPMI1 clock source.
   0: APB1 (PCLK1) selected as SWPMI1 clock
   1: HSI16 clock selected as SWPMI1 clock */

/*   Bits 29:28 ADCSEL[1:0]: ADCs clock source selection */
/*   These bits are set and cleared by software to select the clock source used by the ADC
   interface.
   00: No clock selected
   01: PLLSAI1 “R” clock (PLLADC1CLK) selected as ADCs clock */

/* 10: Reserved */

/* 11: System clock selected as ADCs clock */

/* Bits 27:26 CLK48SEL[1:0]: 48 MHz clock source selection */
#define RCC_CLK48SEL_MASK    (0x3 << 26)
#define RCC_CLK48SEL_HSI48   (0x0 << 26)
#define RCC_CLK48SEL_PLLSAI1 (0x1 << 26)
#define RCC_CLK48SEL_PLL     (0x2 << 26)
#define RCC_CLK48SEL_MSI     (0x3 << 26)
/* These bits are set and cleared by software to select the 48 MHz clock 
   source used by USB FS, RNG and SDMMC.
   00: HSI48 clock selected as 48 MHz clock
   01: PLLSAI1 “Q” clock (PLL48M2CLK) selected as 48 MHz clock
   10: PLL “Q” clock (PLL48M1CLK) selected as 48 MHz clock
   11: MSI clock selected as 48 MHz clock */

/* Bits 25:24 Reserved, must be kept at reset value. */

/* Bits 23:22 SAI1SEL[1:0]: SAI1 clock source selection */
#define RCC_SAI1SEL (0x3 << 22)
/* These bits are set and cleared by software to select the SAI1 clock source.
   00: PLLSAI1 “P” clock (PLLSAI1CLK) selected as SAI1 clock
   01: Reserved
   Note: When there is no PLL enabled, the HSI16 clock source is connected 
   automatically to the SAI1 to allow audio detection without the need to 
   turn on the PLL source.
   10: PLL “P” clock (PLLSAI3CLK) selected as SAI1 clock
   11: External input SAI1_EXTCLK selected as SAI1 clock
   Caution: If the selected clock is the external clock, it is not possible 
   to switch to another clock if the external clock is not present. */

/* Bits 21:20 LPTIM2SEL[1:0]: Low power timer 2 clock source selection
   These bits are set and cleared by software to select the LPTIM2 clock source.
   00: PCLK selected as LPTIM2 clock
   01: LSI clock selected as LPTIM2 clock
   10: HSI16 clock selected as LPTIM2 clock
   11: LSE clock selected as LPTIM2 clock
   Bits 19:18 LPTIM1SEL[1:0]: Low power timer 1 clock source selection
   These bits are set and cleared by software to select the LPTIM1 clock source.
   00: PCLK selected as LPTIM1 clock
   01: LSI clock selected as LPTIM1 clock
   10: HSI16 clock selected as LPTIM1 clock
   11: LSE clock selected as LPTIM1 clock
   Bits 17:16 I2C3SEL[1:0]: I2C3 clock source selection
   These bits are set and cleared by software to select the I2C3 clock source.
   00: PCLK selected as I2C3 clock
   01: System clock (SYSCLK) selected as I2C3 clock
   10: HSI16 clock selected as I2C3 clock
   11: Reserved
   Bits 15:14 I2C2SEL[1:0](2): I2C2 clock source selection
   These bits are set and cleared by software to select the I2C2 clock source.
   00: PCLK selected as I2C2 clock
   01: System clock (SYSCLK) selected as I2C2 clock
   10: HSI16 clock selected as I2C2 clock
   11: Reserved
   Bits 13:12 I2C1SEL[1:0]: I2C1 clock source selection
   These bits are set and cleared by software to select the I2C1 clock source.
   00: PCLK selected as I2C1 clock
   01: System clock (SYSCLK) selected as I2C1 clock
   10: HSI16 clock selected as I2C1 clock
   11: Reserved


   Bits 11:10 LPUART1SEL[1:0]: LPUART1 clock source selection
   These bits are set and cleared by software to select the LPUART1 clock source.
   00: PCLK selected as LPUART1 clock
   01: System clock (SYSCLK) selected as LPUART1 clock
   10: HSI16 clock selected as LPUART1 clock
   11: LSE clock selected as LPUART1 clock
   */
   /* Bits 9:8 Reserved, must be kept at reset value. */

   /* Bits 7:6 UART4SEL[1:0](3): UART4 clock source selection */

   /*
   This bit is set and cleared by software to select the UART4 clock source.
   00: PCLK selected as UART4 clock
   01: System clock (SYSCLK) selected as UART4 clock
   10: HSI16 clock selected as UART4 clock
   11: LSE clock selected as UART4 clock
   Bits 5:4 USART3SEL[1:0](2): USART3 clock source selection
   This bit is set and cleared by software to select the USART3 clock source.
   00: PCLK selected as USART3 clock
   01: System clock (SYSCLK) selected as USART3 clock
   10: HSI16 clock selected as USART3 clock
   11: LSE clock selected as USART3 clock
   Bits 3:2 USART2SEL[1:0]: USART2 clock source selection
   This bit is set and cleared by software to select the USART2 clock source.
   00: PCLK selected as USART2 clock
   01: System clock (SYSCLK) selected as USART2 clock
   10: HSI16 clock selected as USART2 clock
   11: LSE clock selected as USART2 clock
   Bits 1:0 USART1SEL[1:0]: USART1 clock source selection
   This bit is set and cleared by software to select the USART1 clock source.
   00: PCLK selected as USART1 clock
   01: System clock (SYSCLK) selected as USART1 clock
   10: HSI16 clock selected as USART1 clock
   11: LSE clock selected as USART1 clock
	*/
/* ------------------------------------------------------------------------- */
/* Backup domain control register */
#define STM32_RCC_BDCR 0x90

/* ------------------------------------------------------------------------- */
/* Control/status register */
#define STM32_RCC_CSR 0x94

/* ------------------------------------------------------------------------- */
/* Clock recovery RC register (RCC_CRRCR) */
#define STM32_RCC_CRRCR 0x98
/* Bits 15:7 HSI48CAL[8:0]: HSI48 clock calibration
   These bits are initialized at startup with the factory-programmed 
   HSI48 calibration trim value.
   They are ready only. */

/* Bit 1 HSI48RDY: HSI48 clock ready flag */
#define RCC_HSI48RDY (1 << 1)
/* Set by hardware to indicate that HSI48 oscillator is stable. This bit 
   is set only when HSI48 is enabled by software by setting HSI48ON.
   0: HSI48 oscillator not ready
   1: HSI48 oscillator ready */

/*  Bit 0 HSI48ON: HSI48 clock enable */
#define RCC_HSI48ON (1 << 0)
/* Set and cleared by software. 
   Cleared by hardware to stop the HSI48 when entering in Stop, Standby or 
   Shutdown modes.
   0: HSI48 oscillator OFF
   1: HSI48 oscillator ON
*/

/* ------------------------------------------------------------------------- */
/* RCC CCIPR Peripherals independent clock configuration register  */
#define STM32_RCC_CCIPR2 0x9c

/* Bits 1:0 I2C4SEL[1:0]: I2C4 clock source selection */
#define RCC_I2C4SEL (3 << 0)
/* These bits are set and cleared by software to select the I2C4 clock source.
   00: PCLK selected as I2C4 clock
   01: System clock (SYSCLK) selected as I2C4 clock
   10: HSI16 clock selected as I2C4 clock
   11: reserved
*/

/* ------------------------------------------------------------------------- */
#define STM32_AHB  0
#define STM32_AHB1 0
#define STM32_APB1 1
#define STM32_APB2 2
#define STM32_AHB2 3
#define STM32_AHB3 4
#define STM32_APB1B 5

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

#if defined(STM32L4X)
struct stm32_rcc {
	volatile uint32_t cr; /* Control Register */
	volatile uint32_t icscr; 
	volatile uint32_t cfgr; 
	volatile uint32_t pllcfgr; 
	/* 0x10 */
	volatile uint32_t pllsaicfgr;
	uint32_t res1; 
	volatile uint32_t cier; 
	volatile uint32_t cifr; 
	/* 0x20 */
	volatile uint32_t cicr; 
	uint32_t res2; 
	volatile uint32_t ahb1rstr;
	volatile uint32_t ahb2rstr;
	/* 0x30 */
	volatile uint32_t ahb3rstr;
	uint32_t res3; 
	volatile uint32_t apb1rstr1;
	volatile uint32_t apb1rstr2;
	/* 0x40 */
	volatile uint32_t apb2rstr;
	uint32_t res4; 
	volatile uint32_t ahb1enr;
	volatile uint32_t ahb2enr;
	/* 0x50 */
	volatile uint32_t ahb3enr;
	uint32_t res5; 
	volatile uint32_t apb1enr1;
	volatile uint32_t apb1enr2;
	/* 0x60 */
	volatile uint32_t apb2enr;
	uint32_t res6; 
	volatile uint32_t ahb1smenr;
	volatile uint32_t ahb2smenr; 
	/* 0x70 */
	volatile uint32_t ahb3smenr;
	uint32_t res7; 
	volatile uint32_t apb1smenr1;
	volatile uint32_t apb1smenr2;
	/* 0x80 */
	volatile uint32_t apb2smenr2;
	uint32_t res8; 
	volatile uint32_t ccipr;
	uint32_t res9; 
	/* 0x90 */
	volatile uint32_t bdcr;
	volatile uint32_t csr;
	volatile uint32_t crrcr;
	volatile uint32_t ccipr2;

};
#endif /* STM32L4X */

struct stm32_clk {
	uint8_t bus:3;
	uint8_t bit:5;
} __attribute__((packed))__;

#if 0
enum {
	STM32_CLK_AHB  = 0,
	STM32_CLK_APB1 = 1,
	STM32_CLK_APB2 = 2,
	STM32_CLK_TIM1 = 3,
	STM32_CLK_TIM2 = 4,
	STM32_CLK_HSI  = 5,
	STM32_CLK_HSE  = 6,
	STM32_CLK_LSI  = 7,
	STM32_CLK_LSE  = 8,
	STM32_CLK_MSI  = 9,
	STM32_CLK_MCO  = 10,
	STM32_CLK_SAI  = 11,
	STM32_CLK_I2S  = 12
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

static inline void stm32_clk_enable(struct stm32_rcc * rcc, 
									int bus, int bit) {
	uint32_t volatile * enr;

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
	asm volatile ("dsb" : );
}

static inline void stm32_clk_disable(struct stm32_rcc * rcc, 
									 int bus, int bit) {
	uint32_t volatile * enr;

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
	asm volatile ("dsb" : );
}

static inline void stm32_reset(struct stm32_rcc * rcc, 
									int bus, int bit) {
	uint32_t volatile * rstr;

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
	asm volatile ("dsb" : );
	*rstr &= ~(1 << bit);
	asm volatile ("dsb" : );
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

#endif /* __ASSEMBLER__ */

#endif /* __STM32L4_RCC_H__ */

