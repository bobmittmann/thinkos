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
 * @file stm32f0xx.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

/* 
   ARM Cortex-M0
*/

#ifndef __STM32F0XX_H__
#define __STM32F0XX_H__

#define STM32F
#define STM32F0
#define STM32F0X
#define STM32F0XX

#include <stm32f/stm32l4-pwr.h>
#include <stm32f/stm32l4-syscfg.h>
#include <stm32f/stm32f0-rcc.h>
#include <stm32f/stm32f-usart.h>
#include <stm32f/stm32f-gpio.h>
#include <stm32f/stm32l4-flash.h>
#include <stm32f/stm32f-dac.h>
#include <stm32f/stm32l4-adc.h>
#include <stm32f/stm32f-rtc.h>
#include <stm32f/stm32f-spi.h>
#include <stm32f/stm32f-i2c.h>
#include <stm32f/stm32f-usb.h>
#include <stm32f/stm32f-tim.h>
#include <stm32f/stm32f0-dma.h>
#include <stm32f/stm32f-exti.h>

#define STM32_BASE_UID          0x1ffff7ac

#define STM32_BASE_RNG          0x50060800

#define STM32_BASE_AES          0x50060000

#define STM32_BASE_ADCC         0x50040300
#define STM32_BASE_ADC1         0x50040000

#define STM32_BASE_GPIOH        0x48001c00
#define STM32_BASE_GPIOE        0x48001000
#define STM32_BASE_GPIOD        0x48000c00
#define STM32_BASE_GPIOC        0x48000800
#define STM32_BASE_GPIOB        0x48000400
#define STM32_BASE_GPIOA        0x48000000

#define STM32_BASE_TSC          0x40024000

#define STM32_BASE_CRC          0x40023000

#define STM32_BASE_FLASH        0x40022000

#define STM32_BASE_RCC          0x40021000

#define STM32_BASE_DMA2         0x40020400
#define STM32_BASE_DMA1         0x40020000

#define STM32_BASE_DFSDM1       0x40016000

#define STM32_BASE_SAI1         0x40015400

#define STM32_BASE_TIM16        0x40014400
#define STM32_BASE_TIM15        0x40014000

#define STM32_BASE_USART1       0x40013800

#define STM32_BASE_SPI1         0x40013000
#define STM32_BASE_TIM1         0x40012c00
#define STM32_BASE_SDMCC        0x40012800

#define STM32_BASE_FIREWALL     0x40011c00

#define STM32_BASE_EXTI         0x40010400
#define STM32_BASE_COMP         0x4001001c
#define STM32_BASE_VREFBUF      0x40010030
#define STM32_BASE_SYSCFG       0x40010000

#define STM32_BASE_LPTIM2       0x40009400

#define STM32_BASE_SWPMI1       0x40008800
#define STM32_BASE_I2C4         0x40008400
#define STM32_BASE_LPUART1      0x40008000
#define STM32_BASE_LPTIM1       0x40007c00
#define STM32_BASE_OPAMP        0x40007800
#define STM32_BASE_DAC1         0x40007400
#define STM32_BASE_PWR          0x40007000
#define STM32_BASE_USB_SRAM     0x40006c00
#define STM32_BASE_USB          0x40006800

#define STM32_BASE_CAN1         0x40006400
#define STM32_BASE_CRS          0x40006000
#define STM32_BASE_I2C3         0x40005c00
#define STM32_BASE_I2C2         0x40005800
#define STM32_BASE_I2C1         0x40005400

#define STM32_BASE_UART4        0x40004c00
#define STM32_BASE_USART3       0x40004800
#define STM32_BASE_USART2       0x40004400

#define STM32_BASE_SPI3         0x40003c00
#define STM32_BASE_SPI2         0x40003800

#define STM32_BASE_IWDG         0x40003000
#define STM32_BASE_WWDG         0x40002c00
#define STM32_BASE_RTC          0x40002800
#define STM32_BASE_LCD          0x40002400

#define STM32_BASE_TIM7         0x40001400
#define STM32_BASE_TIM6         0x40001000
#define STM32_BASE_TIM3         0x40000400
#define STM32_BASE_TIM2   	    0x40000000

#define STM32_IRQ_WWDG          0 /* Window WatchDog Interrupt */
#define STM32_IRQ_PVD           1 /* PVD through EXTI Line detection Interrupt */
#define STM32_IRQ_VDDIO2        1 /* PVD through EXTI Line detection Interrupt */
#define STM32_IRQ_RTC           2 /* Tamper interrupts through the EXTI line */
#define STM32_IRQ_FLASH         3 /* FLASH global Interrupt */
#define STM32_IRQ_RCC           4 /* RCC global Interrupt */
#define STM32_IRQ_EXTI0         5 /* EXTI Line0 Interrupt */
#define STM32_IRQ_EXTI1         5 /* EXTI Line1 Interrupt */

#define STM32_IRQ_EXTI2         6 /* EXTI Line2 Interrupt */
#define STM32_IRQ_EXTI3         6 /* EXTI Line3 Interrupt */
#define STM32_IRQ_EXTI4         7 /* EXTI Line4 Interrupt */
#define STM32_IRQ_EXTI5         7 /* EXTI Line4 Interrupt */
#define STM32_IRQ_EXTI6         7 /* EXTI Line4 Interrupt */
#define STM32_IRQ_EXTI7         7 /* EXTI Line4 Interrupt */
#define STM32_IRQ_EXTI8         7 /* EXTI Line4 Interrupt */
#define STM32_IRQ_EXTI9         7 /* EXTI Line4 Interrupt */
#define STM32_IRQ_EXTI10        7 /* EXTI Line4 Interrupt */
/* ...
*/
#define STM32_IRQ_TSC           8 /* TSC global Interrupt */

#define STM32_IRQ_DMA1_CH1      9 /* DMA1 Channel 1 Interrupt */
#define STM32_IRQ_DMA1_CH2      10 /* DMA1 Channel 2 Interrupt */
#define STM32_IRQ_DMA1_CH3      10 /* DMA1 Channel 3 Interrupt */

#define STM32_IRQ_DMA1_CH4      11 /* DMA1 Channel 4 Interrupt */
#define STM32_IRQ_DMA1_CH5      11 /* DMA1 Channel 5 Interrupt */
#define STM32_IRQ_DMA1_CH6      11 /* DMA1 Channel 6 Interrupt */
#define STM32_IRQ_DMA1_CH7      11 /* DMA1 Channel 7 Interrupt */

#define STM32_IRQ_ADC_COMP      12 /* ADC1 global Interrupt */

#define STM32_IRQ_TIM1_BRK      13 /* TIM1 Break interrupt  */
#define STM32_IRQ_TIM1_CC       13 /* TIM1 Capture Compare Interrupt */
#define STM32_IRQ_TIM1_TRG      13 /* TIM1 Trigger Interrupt */
#define STM32_IRQ_TIM1_COM      13 /* TIM1 Commutation Interrupt */
#define STM32_IRQ_TIM1_UP       13 /* TIM1 Update Interrupt */

#define STM32_IRQ_TIM2          15 /* TIM2 global Interrupt */
#define STM32_IRQ_TIM3          16 /* TIM3 global Interrupt */
#define STM32_IRQ_DACUNDER      17 /* DAC1&2 underrun error Interrupt */
#define STM32_IRQ_TIM6          17 /* TIM6 global Interrupt */

#define STM32_IRQ_TIM7          18 /* TIM7 global interrupt */

#define STM32_IRQ_TIM14         19 /* TIM14 global interrupt */
#define STM32_IRQ_TIM15         20 /* TIM15 global interrupt */
#define STM32_IRQ_TIM16         21 /* TIM16 global interrupt */
#define STM32_IRQ_TIM17         22 /* TIM17 global interrupt */
#define STM32_IRQ_I2C1          23 /* I2C1 global interrupt */
#define STM32_IRQ_I2C2          24 /* I2C2 global interrupt */

#define STM32_IRQ_SPI1          25 /* SPI1 global Interrupt */
#define STM32_IRQ_SPI2          26 /* SPI2 global Interrupt */

#define STM32_IRQ_USART1        27 /* USART1 global Interrupt */
#define STM32_IRQ_USART2        28 /* USART2 global Interrupt */

#define STM32_IRQ_USART3        29 /* USART3 global Interrupt */
#define STM32_IRQ_USART4        29 /* USART4 global Interrupt */
#define STM32_IRQ_USART5        29 /* USART5 global Interrupt */
#define STM32_IRQ_USART6        29 /* USART6 global Interrupt */
#define STM32_IRQ_USART7        29 /* USART7 global Interrupt */
#define STM32_IRQ_USART8        29 /* USART8 global Interrupt */

#define STM32_IRQ_CEC           30 /* CEC global interrupt */
#define STM32_IRQ_CAN           30 /* CAN global interrupt */

#define STM32_IRQ_USB_FS        31 /* USB global interrupt */
#define STM32_IRQ_EXTI18        31 /* EXTI Line 18 interrupt */

#ifndef __ASSEMBLER__

#define STM32_GPIO(N) ((struct stm32f_gpio *)(STM32_BASE_GPIOA + (N)*0x400))
#define STM32_AFIO ((struct stm32f_afio *)STM32_BASE_AFIO)
#define STM32_UID ((uint32_t *)STM32_BASE_UID)

enum {
	STM32_GPIOA_ID = 0,
	STM32_GPIOB_ID,
	STM32_GPIOC_ID,
	STM32_GPIOD_ID,
	STM32_GPIOE_ID,
	STM32_GPIOF_ID,
	STM32_GPIOG_ID,
	STM32_GPIOH_ID
};

enum {
	STM32_UART1_ID = 0,
	STM32_UART2_ID, 
	STM32_UART3_ID,
	STM32_UART4_ID
};

#define STM32F_USB_PKTBUF_ADDR  STM32_BASE_USB_SRAM
#define STM32F_USB_PKTBUF_SIZE  1024

#define STM32_FSMC_NE1 ((uint32_t *)0x60000000)
#define STM32_FLASH_MEM ((uint8_t *)0x08000000)

#endif /* __ASSEMBLER__ */

#endif /* __STM32F0XX_H__ */

