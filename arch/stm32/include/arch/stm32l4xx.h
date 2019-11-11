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
 * @file stm32f3xx.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

/* 
   ARM Cortex-M4F 32b MCU+FPU, up to 256KB Flash+48KB SRAM
   4 ADCs, 2 DAC ch., 7 comp, 4 PGA, timers, 2.0-3.6 V operation
*/

#ifndef __STM32L4XX_H__
#define __STM32L4XX_H__

#define STM32L
#define STM32L4X
#define STM32L4XX

#include <stm32f/stm32l4-pwr.h>
#include <stm32f/stm32l4-syscfg.h>
#include <stm32f/stm32l4-rcc.h>
#include <stm32f/stm32f-usart.h>
#include <stm32f/stm32f-gpio.h>
#include <stm32f/stm32l4-flash.h>
#include <stm32f/stm32f-dac.h>
#include <stm32f/stm32l4-adc.h>
#include <stm32f/stm32f-rtc.h>
#include <stm32f/stm32f-spi.h>
#include <stm32f/stm32f-i2c.h>
#include <stm32f/stm32f-eth.h>
#include <stm32f/stm32f-usb.h>
#include <stm32f/stm32f-tim.h>
#include <stm32f/stm32f-dma.h>
#include <stm32f/stm32f-exti.h>
#include <stm32f/stm32l4-sai.h>

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
#define STM32_IRQ_TAMP_STAMP    2 /* Tamper interrupts through the EXTI line */
#define STM32_IRQ_RTC_WKUP      3 /* RTC Wakeup interrupt through the EXTI line */
#define STM32_IRQ_FLASH         4 /* FLASH global Interrupt */
#define STM32_IRQ_RCC           5 /* RCC global Interrupt */
#define STM32_IRQ_EXTI0         6 /* EXTI Line0 Interrupt */
#define STM32_IRQ_EXTI1         7 /* EXTI Line1 Interrupt */

#define STM32_IRQ_EXTI2         8 /* EXTI Line2 Interrupt */
#define STM32_IRQ_EXTI3         9 /* EXTI Line3 Interrupt */
#define STM32_IRQ_EXTI4        10 /* EXTI Line4 Interrupt */

#define STM32_IRQ_DMA1_CH1      11 /* DMA1 Channel 1 Interrupt */
#define STM32_IRQ_DMA1_CH2      12 /* DMA1 Channel 2 Interrupt */
#define STM32_IRQ_DMA1_CH3      13 /* DMA1 Channel 3 Interrupt */
#define STM32_IRQ_DMA1_CH4      14 /* DMA1 Channel 4 Interrupt */
#define STM32_IRQ_DMA1_CH5      15 /* DMA1 Channel 5 Interrupt */

#define STM32_IRQ_DMA1_CH6      16 /* DMA1 Channel 6 Interrupt */
#define STM32_IRQ_DMA1_CH7      17 /* DMA1 Channel 7 Interrupt */
#define STM32_IRQ_ADC1         18 /* ADC1 global Interrupt */
#define STM32_IRQ_CAN1_TX      19 /* CAN1 TX Interrupt */
#define STM32_IRQ_CAN1_RX0     20 /* CAN1 RX0 Interrupt */
#define STM32_IRQ_CAN1_RX1     21 /* CAN1 RX1 Interrupt */
#define STM32_IRQ_CAN1_SCE     22 /* CAN1 SCE Interrupt */
#define STM32_IRQ_EXTI9_5      23 /* EXTI Line[9:5] Interrupts */

#define STM32_IRQ_TIM1_BRK     24 /* TIM1 Break interrupt  */
#define STM32_IRQ_TIM15        24 /* TIM15 global interrupt */
#define STM32_IRQ_TIM1_UP      25 /* TIM1 Update Interrupt */
#define STM32_IRQ_TIM16        25 /* TIM16 global interrupt */
#define STM32_IRQ_TIM1_TRG_COM 26 /* TIM1 Trigger and Commutation Interrupt */
#define STM32_IRQ_TIM1_CC      27 /* TIM1 Capture Compare Interrupt */
#define STM32_IRQ_TIM2         28 /* TIM2 global Interrupt */
#define STM32_IRQ_TIM3         29 /* TIM3 global Interrupt */
#define STM32_IRQ_I2C1_EV      31 /* I2C1 Event Interrupt */

#define STM32_IRQ_I2C1_ER      32 /* I2C1 Error Interrupt */
#define STM32_IRQ_I2C2_EV      33 /* I2C2 Event Interrupt */
#define STM32_IRQ_SPI1         35 /* SPI1 global Interrupt */
#define STM32_IRQ_SPI2         36 /* SPI2 global Interrupt */
#define STM32_IRQ_USART1        37 /* USART1 global Interrupt */
#define STM32_IRQ_USART2        38 /* USART2 global Interrupt */
#define STM32_IRQ_USART3        39 /* USART3 global Interrupt */

#define STM32_IRQ_EXTI15_10    40 /* EXTI Line[15:10] Interrupts */
#define STM32_IRQ_RTC_ALARM    41 /* RTC Alarm Interrupt */

#define STM32_IRQ_SDMMC1       48 /* SDMMC1 global Interrupt */

#define STM32_IRQ_SPI3         51 /* SPI3 global Interrupt */
#define STM32_IRQ_UART4         52 /* UART4 global Interrupt */
#define STM32_IRQ_TIM6         54 /* TIM6 global Interrupt */
#define STM32_IRQ_DACUNDER     54 /* DAC1&2 underrun error Interrupt */
#define STM32_IRQ_TIM7         55 /* TIM7 global interrupt */

#define STM32_IRQ_DMA2_CH1      56 /* DMA2 Channel 1 Interrupt */
#define STM32_IRQ_DMA2_CH2      57 /* DMA2 Channel 2 Interrupt */
#define STM32_IRQ_DMA2_CH3      58 /* DMA2 Channel 3 Interrupt */
#define STM32_IRQ_DMA2_CH4      59 /* DMA2 Channel 4 Interrupt */
#define STM32_IRQ_DMA2_CH5      60 /* DMA2 Channel 5 Interrupt */
#define STM32_IRQ_DFSDM1_FLT0   61 /* DFSDM1_FLT0 global Interrupt */
#define STM32_IRQ_DFSDM1_FLT1   62 /* DFSDM1_FLT1 global Interrupt */

#define STM32_IRQ_COMP         64 /* COMP1/COMP2 through EXTI lines 21/22
									interrupts */
#define STM32_IRQ_LPTIM1       65 /* LPTIM1 global Interrupt */
#define STM32_IRQ_LPTIM2       66 /* LPTIM1 global Interrupt */

#define STM32_IRQ_USB_FS       67 /* USB event interrupt through 
									  EXTI line 17 */
#define STM32_IRQ_DMA2_CH6      68 /* DMA2 Channel 6 Interrupt */
#define STM32_IRQ_DMA2_CH7      69 /* DMA2 Channel 7 Interrupt */

#define STM32_IRQ_LPUART1      70 /* LPUART1 global Interrupt */
#define STM32_IRQ_QSPI         71 /* QSPI global Interrupt */
#define STM32_IRQ_QUADSPI      71 /* QUADSPI global Interrupt */

#define STM32_IRQ_I2C3_EV      72 /* I2C3 event interrupt */
#define STM32_IRQ_I2C3_ER      73 /* I2C3 error interrupt */

#define STM32_IRQ_SAI1         74 /* SAI1 global Interrupt */

#define STM32_IRQ_SWPMI1       76 /* SWPMI1 global Interrupt */
#define STM32_IRQ_TSC          77 /* TSC global Interrupt */
#define STM32_IRQ_LCD          78 /* LCD global Interrupt */
#define STM32_IRQ_AES          79 /* AES global Interrupt */

#define STM32_IRQ_RNG          80 /* RNG global Interrupt */
#define STM32_IRQ_FPU          81 /* Floating Point Interrupt */

#define STM32_IRQ_I2C4_EV      83 /* I2C4 event interrupt */
#define STM32_IRQ_I2C4_ER      84 /* I2C4 error interrupt */

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

#endif /* __STM32L4XX_H__ */

