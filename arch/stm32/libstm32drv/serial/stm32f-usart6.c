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
 * @file yard-ice.c
 * @brief YARD-ICE UART console
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#define __STM32_SERIAL_I__
#include "stm32-serial-i.h"

#ifdef STM32_USART6

struct stm32f_serial_drv uart6_serial_drv = {
	.uart = STM32_USART6,
#if SERIAL_ENABLE_DMA
	.dma = STM32F_DMA1,
	.rx = {
		.irq = STM32F_IRQ_DMA1_STREAM0,
		.s = &STM32F_DMA1->s[0]
	},
	.tx = {
		.irq = STM32F_IRQ_DMA1_STREAM7,
		.s = &STM32F_DMA1->s[7]
	}
#endif
};

void stm32f_usart6_isr(void)
{
	stm32f_serial_isr(&uart6_serial_drv);
}

const struct serial_dev uart6_serial_dev = {
	.drv = &uart6_serial_drv,
	.op = &stm32f_uart_serial_op
};

struct serial_dev * stm32f_uart6_serial_init(unsigned int baudrate, 
											 unsigned int flags)
{
	struct stm32f_serial_drv * drv = &uart6_serial_drv;

	DCC_LOG(LOG_TRACE, "IDLE!");

	stm32f_serial_init(drv, baudrate, flags);

#ifdef THINKAPP
	/* configure and Enable interrupt */
	thinkos_irq_register(STM32_IRQ_USART6, SERIAL_IRQ_PRIORITY,
						 stm32f_usart6_isr);
#else
	/* configure interrupts */
	cm3_irq_pri_set(STM32_IRQ_USART6, SERIAL_IRQ_PRIORITY);
	/* enable interrupts */
	cm3_irq_enable(STM32_IRQ_USART6);
#endif

	return (struct serial_dev *)&uart6_serial_dev;
}

#endif

