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

#define __STM32_I2S_I__
#include "stm32-i2s-i.h"

/*
           | DMA | Channel | Stream | 
   SPI3_RX |   1 |       0 |      0 |
   SPI3_RX |   1 |       0 |      2 |
   SPI3_TX |   1 |       0 |      5 |
   SPI3_TX |   1 |       0 |      7 |
 */


struct stm32_spi_i2s_drv stm32_spi3_i2s_drv = {
	.spi = STM32F_SPI3,
};

void stm32f_spi3_isr(void)
{
	stm32_spi_i2s_isr(&stm32_spi3_i2s_drv);
}

void stm32f_dma1_stream2_isr(void)
{
	stm32_i2s_dma_rx_isr(&stm32_spi3_i2s_drv);
}

void stm32f_dma1_stream7_isr(void)
{
	stm32_i2s_dma_tx_isr(&stm32_spi3_i2s_drv);
}


const struct i2s_dev spi3_i2s_dev = {
	.drv = &stm32_spi3_i2s_drv,
	.op = &stm32_spi_i2s_op
};


struct i2s_dev * stm32_spi3_i2s_init(unsigned int samplerate, 
									 unsigned int flags)
{
	struct stm32_spi_i2s_drv * drv = &stm32_spi3_i2s_drv;

	DCC_LOG(LOG_TRACE, "...");

	stm32_dmactl_init(&drv->rx.dmactl, STM32F_DMA1, 2); 
	stm32_dmactl_init(&drv->tx.dmactl, STM32F_DMA1, 7); 


	stm32_reset(STM32_RCC, STM32_RST_SPI3);
	stm32_clk_enable(STM32_RCC, STM32_CLK_SPI3);

	stm32_spi_i2s_init(drv, samplerate, flags, 0);

	/* configure and Enable interrupts */
#if 0
#ifdef THINKAPP
	thinkos_irq_register(STM32F_IRQ_SPI3, I2S_IRQ_PRIORITY, 
						 stm32f_spi3_isr);

	thinkos_irq_register(STM32F_IRQ_DMA1_STREAM2, I2S_IRQ_PRIORITY, 
						 stm32f_dma1_stream2_isr);

	thinkos_irq_register(STM32F_IRQ_DMA1_STREAM7, I2S_IRQ_PRIORITY, 
						 stm32f_dma1_stream7_isr);
#else
	cm3_irq_pri_set(STM32F_IRQ_SPI3, I2S_IRQ_PRIORITY);
	cm3_irq_enable(STM32F_IRQ_SPI3);

	cm3_irq_pri_set(STM32F_IRQ_DMA1_STREAM2, I2S_IRQ_PRIORITY);
	cm3_irq_enable(STM32F_IRQ_DMA1_STREAM2);

	cm3_irq_pri_set(STM32F_IRQ_DMA1_STREAM7, I2S_IRQ_PRIORITY);
	cm3_irq_enable(STM32F_IRQ_DMA1_STREAM7);
#endif
#endif
	DCC_LOG(LOG_TRACE, "done!");

	return (struct i2s_dev *)&spi3_i2s_dev;
}

