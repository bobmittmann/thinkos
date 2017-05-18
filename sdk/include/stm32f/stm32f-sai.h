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
 * @file stm32f-sai.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __STM32F_SAI_H__
#define __STM32F_SAI_H__

/*-------------------------------------------------------------------------
  Serial audio interface (SAI)
  -------------------------------------------------------------------------*/

/* SAI control register 1 (not used in I2S mode)*/
#define SAI_CR1 0x00


#ifndef __ASSEMBLER__

#include <stdint.h>

struct stm32f_sai {
	volatile uint32_t cr1;
	volatile uint32_t cr2;
	volatile uint32_t sr;
	volatile uint32_t dr;

	volatile uint32_t crcpr;
	volatile uint32_t rxcrcr;
	volatile uint32_t txcrcr;
	volatile uint32_t i2scfgr;

	volatile uint32_t i2spr;
};

#endif /* __ASSEMBLER__ */

#endif /* __STM32F_SAI_H__ */

