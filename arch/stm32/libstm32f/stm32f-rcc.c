/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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
 * @file stm32f-rcc.c
 * @brief STM32F
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 


#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>

/* for 11025 freq @ 8 oversample rate */
//#define PLLI2SR 3
//#define PLLI2SN 254
/* for 11025 freq @ 16 oversample rate */
#define PLLI2SR 2
#define PLLI2SN 271

#define PLLI2SM 9
#define PLLI2SQ 8
#define PLLI2SP 2

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
#endif

#define __VCOI2S_HZ (((uint64_t)STM32_HSE_HZ * PLLI2SN) / PLLI2SM)
#define __I2S_HZ (__VCOI2S_HZ / PLLI2SR)

#if defined(STM32F446)
const uint32_t stm32f_i2s_hz = __I2S_HZ;

void stm32_rcc_i2s_pll_init(void)
{
	struct stm32_rcc * rcc = STM32_RCC;
	uint32_t cr;
	int again;

	cr = rcc->cr;
	/* disable I2SPLL */
	cr &= ~RCC_PLLI2SON;

	/* configure IS2 PLL */
	rcc->plli2scfgr =  RCC_PLLI2SR(PLLI2SR) | RCC_PLLI2SQ(PLLI2SQ) | 
		RCC_PLLI2SP(PLLI2SP) | RCC_PLLI2SN(PLLI2SN) | RCC_PLLI2SM(PLLI2SM);

	/* enable I2SPLL */
	cr |= RCC_PLLI2SON;
	rcc->cr = cr;;

	for (again = 8192; ; again--) {
		cr = rcc->cr;
		if (cr & RCC_PLLI2SRDY)
			break;
		if (again == 0) {
			/* PLL lock fail */
			return;
		}
	}

	rcc->dckcfgr = I2S2SRC_PLLI2S_R | I2S1SRC_PLLI2S_R;
}
#endif

