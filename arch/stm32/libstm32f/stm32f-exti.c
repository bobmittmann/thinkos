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
 * @file stm32f-exti.c
 * @brief STM32F
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <sys/stm32f.h>

#include <sys/dcclog.h>

#if defined(STM32F2X) || defined(STM32F4X)

void stm32_exti_init(struct stm32_gpio * gpio, unsigned int pin, 
					  unsigned int opt)
{
	struct stm32_syscfg * syscfg = STM32_SYSCFG;
	struct stm32_exti * exti = STM32_EXTI;
	int port;

	/* System configuration controller clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_SYSCFG);

	port = stm32_gpio_id(gpio);

	/* Select exti */
	syscfg->exticr[pin >> 2] = port << ((pin & 0x3) * 4);

	if (opt & EXTI_EDGE_RISING)
		/* Select rising edge trigger */
		exti->rtsr |= (1 << pin);
	else
		/* Select falling edge trigger */
		exti->ftsr |= (1 << pin);

	/* Clear pending flag */
	exti->pr = (1 << pin);

	/* Unmask interrupt */
	exti->imr |= (1 << pin);

}

#endif

