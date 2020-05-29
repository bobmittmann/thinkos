/* 
 * stm32f103.ld
 *
 * Copyright(C) 2013 Robinson Mittmann. All Rights Reserved.
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

EXTERN(stm32f_wwdg_isr)
EXTERN(stm32f_pvd_isr)
EXTERN(stm32f_tamp_stamp_isr)
EXTERN(stm32f_rtc_wkup_isr)
EXTERN(stm32f_flash_isr)
EXTERN(stm32f_rcc_isr)
EXTERN(stm32f_exti0_isr)
EXTERN(stm32f_exti1_isr)
EXTERN(stm32f_exti2_isr)
EXTERN(stm32f_exti3_isr)
EXTERN(stm32f_exti4_isr)
EXTERN(stm32f_dma1_stream0_isr)
EXTERN(stm32f_dma1_stream1_isr)
EXTERN(stm32f_dma1_stream2_isr)
EXTERN(stm32f_dma1_stream3_isr)
EXTERN(stm32f_dma1_stream4_isr)
EXTERN(stm32f_dma1_stream5_isr)
EXTERN(stm32f_dma1_stream6_isr)
EXTERN(stm32f_adc_isr)
EXTERN(stm32f_can1_tx_usb_hp_isr)
EXTERN(stm32f_can1_rx0_usb_lp_isr)
EXTERN(stm32f_can1_rx1_isr)
EXTERN(stm32f_can1_sce_isr)
EXTERN(stm32f_exti9_5_isr)
EXTERN(stm32f_tim1_brk_isr)
EXTERN(stm32f_tim1_up_isr)
EXTERN(stm32f_tim1_trg_com_isr)
EXTERN(stm32f_tim1_cc_isr)
EXTERN(stm32f_tim2_isr)
EXTERN(stm32f_tim3_isr)
EXTERN(stm32f_tim4_isr)
EXTERN(stm32f_i2c1_ev_isr)
EXTERN(stm32f_i2c1_er_isr)
EXTERN(stm32f_i2c2_ev_isr)
EXTERN(stm32f_i2c2_er_isr)
EXTERN(stm32f_spi1_isr)
EXTERN(stm32f_spi2_isr)
EXTERN(stm32f_usart1_isr)
EXTERN(stm32f_usart2_isr)
EXTERN(stm32f_usart3_isr)
EXTERN(stm32f_exti15_10_isr)
EXTERN(stm32f_rtc_alarm_isr)
EXTERN(stm32f_otg_fs_wkup_isr)
EXTERN(stm32f_tim5_isr)
EXTERN(stm32f_spi3_isr)
EXTERN(stm32f_uart4_isr)
EXTERN(stm32f_uart5_isr)
EXTERN(stm32f_tim6_isr)
EXTERN(stm32f_tim7_isr)
EXTERN(stm32f_dma2_stream0_isr)
EXTERN(stm32f_dma2_stream1_isr)
EXTERN(stm32f_dma2_stream2_isr)
EXTERN(stm32f_dma2_stream3_isr)

