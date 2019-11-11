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
 * @file stm32-crc.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __STM32_RNG_H__
#define __STM32_RNG_H__

/*-------------------------------------------------------------------------
 * Random Number generator (RNG)
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
 * RNG control register (RNG_CR) */
#define STM32_RNG_CR 0x00

/* Bit 3 IE: Interrupt enable */
#define RNG_IE (1 << 3)
/*  0: RNG Interrupt is disabled
	1: RNG Interrupt is enabled. An interrupt is pending as 
	soon as DRDY=1 or SEIS=1 or CEIS=1 in the RNG_SR register. */

/* Bit 2 RNGEN: Random number generator enable */
#define RNG_RNGEN (1 << 2)
/*  0: Random number generator is disabled
	1: random Number Generator is enabled. */

/* RNG status register (RNG_SR) */
#define STM32_RNG_SR 0x04

/* Bit 6 SEIS: Seed error interrupt status */
#define RNG_SEIS (1 << 6)
/*	This bit is set at the same time as SECS, it is cleared by writing it to 0.
	0: No faulty sequence detected
	1: One of the following faulty sequences has been detected:
	– More than 64 consecutive bits at the same value (0 or 1)
	– More than 32 consecutive alternances of 0 and 1 (0101010101...01)
	An interrupt is pending if IE = 1 in the RNG_CR register. */

/* Bit 5 CEIS: Clock error interrupt status */
#define RNG_CEIS (1 << 5)
/*	This bit is set at the same time as CECS, it is cleared by writing it to 0.
	0: The RNG_CLK clock was correctly detected
	1: The RNG_CLK was not correctly detected (fRNG_CLK< fHCLK/16)
	An interrupt is pending if IE = 1 in the RNG_CR register. */


/* Bit 2 SECS: Seed error current status */
#define RNG_SECS (1 << 2)
/* 0: No faulty sequence has currently been detected. If the SEIS bit is set, 
   this means that a faulty sequence was detected and the situation has 
   been recovered.
   1: One of the following faulty sequences has been detected:
   – More than 64 consecutive bits at the same value (0 or 1)
   – More than 32 consecutive alternances of 0 and 1 (0101010101...01) */

/* Bit 1 CECS: Clock error current status */
#define RNG_CECS (1 << 1)
/*	0: The RNG_CLK clock has been correctly detected. If the CEIS bit is 
	set, this means that a clock error was detected and the situation has 
	been recovered
	1: The RNG_CLK was not correctly detected (fRNG_CLK< fHCLK/16). */

/* Bit 0 DRDY: Data ready */
#define RNG_DRDY (1 << 0)
/*	0: The RNG_DR register is not yet valid, no random data is available
	1: The RNG_DR register contains valid random data
	Note: An interrupt is pending if IE = 1 in the RNG_CR register.
	Once the RNG_DR register has been read, this bit returns to 0 until 
	a new valid value is computed. */

/* RNG data register (RNG_DR) */
#define STM32_RNG_DR 0x08

#ifndef __ASSEMBLER__

#include <stdint.h>

struct stm32_rng {
	volatile uint32_t cr;
	volatile uint32_t sr;
	volatile uint32_t dr;
};

#endif /* __ASSEMBLER__ */

#endif /* __STM32_RNG_H__ */

