/* 
 * stm32l1xx.ld
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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

REGION_ALIAS("CODE", flash);
REGION_ALIAS("DATA", sram);
REGION_ALIAS("HEAP", sram);
REGION_ALIAS("STACK", sram);
REGION_ALIAS("LOG", dcclog);
REGION_ALIAS("BKPCTL", sram);
REGION_ALIAS("BKPDAT", sram);

EXTERN(stm32_wwdg_isr)
EXTERN(stm32_pvd_isr)
EXTERN(stm32_tamp_stamp_isr)
EXTERN(stm32_rtc_wkup_isr)
EXTERN(stm32_flash_isr)
EXTERN(stm32_rcc_isr)
EXTERN(stm32_exti0_isr)
EXTERN(stm32_exti1_isr)

EXTERN(stm32_exti2_isr)
EXTERN(stm32_exti3_isr)
EXTERN(stm32_exti4_isr)
EXTERN(stm32_dma1_channel1_isr)
EXTERN(stm32_dma1_channel2_isr)
EXTERN(stm32_dma1_channel3_isr)
EXTERN(stm32_dma1_channel4_isr)
EXTERN(stm32_dma1_channel5_isr)

EXTERN(stm32_dma1_channel6_isr)
EXTERN(stm32_dma1_channel7_isr)
EXTERN(stm32_adc_isr)
EXTERN(stm32_usb_hp_isr)
EXTERN(stm32_usb_lp_isr)
EXTERN(stm32_dac_isr)
EXTERN(stm32_comp_tsc_isr)
EXTERN(stm32_exti9_5_isr)

EXTERN(stm32_lcd)
EXTERN(stm32_tim9_isr)
EXTERN(stm32_tim10_isr)
EXTERN(stm32_tim11_isr)
EXTERN(stm32_tim2_isr)
EXTERN(stm32_tim3_isr)
EXTERN(stm32_tim4_isr)
EXTERN(stm32_i2c1_ev_isr)

EXTERN(stm32_i2c1_er_isr)
EXTERN(stm32_i2c2_ev_isr)
EXTERN(stm32_i2c2_er_isr)
EXTERN(stm32_spi1_isr)
EXTERN(stm32_spi2_isr)
EXTERN(stm32_usart1_isr)
EXTERN(stm32_usart2_isr)
EXTERN(stm32_usart3_isr)
 
EXTERN(stm32_exti15_10_isr)
EXTERN(stm32_rtc_alarm_isr)
EXTERN(stm32_tim6_isr)
EXTERN(stm32_tim7_isr)
	

