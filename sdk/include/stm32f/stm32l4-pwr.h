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
 * @file stm32l4-pwr.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __STM32L4_PWR_H__
#define __STM32L4_PWR_H__

/*-------------------------------------------------------------------------
  Power Control (PWR)
  -------------------------------------------------------------------------*/

/* PWR power control register 1 */
#define PWR_CR1 0x00 

/* Low power run */
#define PWR_LPR (1 << 14)
/*  */

/* Voltage scaling range selection */
#define PWR_VOS      (3 << 9)
#define PWR_VOS_1_8V (1 << 9)
#define PWR_VOS_1_5V (2 << 9)
#define PWR_VOS_1_2V (3 << 9)
/* These bits are used to select the internal regulator voltage range.
   Before resetting the power interface by resetting the PWRRST bit in 
   the RCC_APB1RSTR register, these bits have to be set to "10" and the 
   frequency of the system has to be configured accordingly.
   00: forbidden (bits are unchanged and keep the previous value, no 
   voltage scaling range changed)
   01: 1.8 V (range 1)
   10: 1.5 V (range 2)
   11: 1.2 V (range 3) */

#define PWR_DBP (1 << 8)

/* . */
#define PWR_LPMS (0x7 << 0)

/* PWR power control register 2 */
#define STM32_PWR_CR2 0x04 

/* Bit 10 USV(1): VDDUSB USB supply valid */
#define PWR_USV (1 << 10)

/* This bit is used to validate the VDDUSB supply for electrical and 
   logical isolation purpose.
   Setting this bit is mandatory to use the USB FS peripheral. If VDDUSB is 
   not always present in the application, the PVM can be used to determine 
   whether this supply is ready or not.
   0: VDDUSB is not present. Logical and electrical isolation is applied 
   to ignore this supply.
   1: VDDUSB is valid */

/* Bits [3..1]  - Power voltage detector level selection. */
#define PWR_PLS_MSK (0x3 << 1)
#define PWR_PLS_SET(VAL) (((VAL) << 1) & PWR_PLS_MSK)
#define PWR_PLS_GET(REG) (((REG) & PWR_PLS_MSK) >> 1)
#define PWR_PLS (0x3 << 1)
#define PWR_PLS_2_0V (0x0 << 1)
#define PWR_PLS_2_2V (0x1 << 1)
#define PWR_PLS_2_4V (0x2 << 1)
#define PWR_PLS_2_5V (0x3 << 1)
#define PWR_PLS_2_6V (0x4 << 1)
#define PWR_PLS_2_8V (0x5 << 1)
#define PWR_PLS_2_9V (0x6 << 1)
/* These bits select the voltage threshold detected by the power 
   voltage detector:
   000: VPVD0 around 2.0 V
   001: VPVD1 around 2.2 V
   010: VPVD2 around 2.4 V
   011: VPVD3 around 2.5 V
   100: VPVD4 around 2.6 V
   101: VPVD5 around 2.8 V
   110: VPVD6 around 2.9 V
   111: External input analog voltage PVD_IN (compared internally to VREFINT)
   Note: These bits are write-protected when the bit PVDL (PVD Lock) is 
   set in the SYSCFG_CBR register.

   These bits are reset only by a system reset.
*/


/*  Bit 0 PVDE: Power voltage detector enable */
#define PWR_USV (1 << 0)
/* 0: Power voltage detector disable.
   1: Power voltage detector enable.
   Note: This bit is write-protected when the bit PVDL (PVD Lock) is set in 
   the SYSCFG_CBR register.
   This bit is reset only by a system reset. */


#ifndef __ASSEMBLER__

#include <stdint.h>

struct stm32_pwr {
	volatile uint32_t cr1; /* Control Register 1 */
	volatile uint32_t cr2; /* Control Register 2 */
	volatile uint32_t cr3; /* Control Register 3 */
	volatile uint32_t cr4; /* Control Register 4*/

	volatile uint32_t sr1; /* Status register 1 */
	volatile uint32_t sr2; /* Status register 2 */
	volatile uint32_t scr; /* */
	uint32_t res1;

	volatile uint32_t pucra ; /* */
	volatile uint32_t pdcra ; /* */
	volatile uint32_t pucrb ; /* */
	volatile uint32_t pdcrb ; /* */

	volatile uint32_t pucrc ; /* */
	volatile uint32_t pdcrc ; /* */
	volatile uint32_t pucrd ; /* */
	volatile uint32_t pdcrd ; /* */

	volatile uint32_t pucre ; /* */
	volatile uint32_t pdcre ; /* */
	uint32_t res2[2];

	uint32_t res3[2];
	volatile uint32_t pucrh ; /* */
	volatile uint32_t pdcrh ; /* */
};

#endif /* __ASSEMBLER__ */

#endif /* __STM32L4_PWR_H__ */

