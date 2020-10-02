/* 
 * stm32f4xx.ld
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

/* Memory regions */

REGION_ALIAS("CODE", flash);
REGION_ALIAS("DATA", sram1);
REGION_ALIAS("HEAP", sram1);
REGION_ALIAS("STACK", ccm);
REGION_ALIAS("LOG", dcclog);
REGION_ALIAS("BKPCTL", rtcbkr);
REGION_ALIAS("BKPDAT", bkpsram);


/* Interrupt vectors */

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
EXTERN(stm32f_can1_tx_isr)
EXTERN(stm32f_can1_rx0_isr)
EXTERN(stm32f_can1_rx1_isr)
EXTERN(stm32f_can1_sce_isr)
EXTERN(stm32f_exti9_5_isr)
EXTERN(stm32f_tim1_brk_tim9_isr)
EXTERN(stm32f_tim1_up_tim10_isr)
EXTERN(stm32f_tim1_trg_com_tim11_isr)
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
EXTERN(stm32f_tim8_brk_tim12_isr)
EXTERN(stm32f_tim8_up_tim13_isr)
EXTERN(stm32f_tim8_trg_com_tim14_isr)
EXTERN(stm32f_tim8_cc_isr)
EXTERN(stm32f_dma1_stream7_isr)
EXTERN(stm32f_fsmc_isr)
EXTERN(stm32f_sdio_isr)
EXTERN(stm32f_tim5_isr)
EXTERN(stm32f_spi3_isr)
EXTERN(stm32f_uart4_isr)
EXTERN(stm32f_uart5_isr)
EXTERN(stm32f_tim6_dac_isr)
EXTERN(stm32f_tim7_isr)
EXTERN(stm32f_dma2_stream0_isr)
EXTERN(stm32f_dma2_stream1_isr)
EXTERN(stm32f_dma2_stream2_isr)
EXTERN(stm32f_dma2_stream3_isr)
EXTERN(stm32f_dma2_stream4_isr)
EXTERN(stm32f_eth_isr)
EXTERN(stm32f_eth_wkup_isr)
EXTERN(stm32f_can2_tx_isr)
EXTERN(stm32f_can2_rx0_isr)
EXTERN(stm32f_can2_rx1_isr)
EXTERN(stm32f_can2_sce_isr)
EXTERN(stm32f_otg_fs_isr)
EXTERN(stm32f_dma2_stream5_isr)
EXTERN(stm32f_dma2_stream6_isr)
EXTERN(stm32f_dma2_stream7_isr)
EXTERN(stm32f_usart6_isr)
EXTERN(stm32f_i2c3_ev_isr)
EXTERN(stm32f_i2c3_er_isr)
EXTERN(stm32f_otg_hs_ep1_out_isr)
EXTERN(stm32f_otg_hs_ep1_in_isr)
EXTERN(stm32f_otg_hs_wkup_isr)
EXTERN(stm32f_otg_hs_isr)
EXTERN(stm32f_dcmi_isr)
EXTERN(stm32f_cryp_isr)
EXTERN(stm32f_hash_rng_isr)
EXTERN(stm32f_fpu_isr)
EXTERN(stm32f_uart7_isr)
EXTERN(stm32f_uart8_isr)
EXTERN(stm32f_spi4_isr)
EXTERN(stm32f_spi5_isr)
EXTERN(stm32f_spi6_isr)
EXTERN(stm32f_sai1_isr)
EXTERN(stm32f_lcd_isr)
EXTERN(stm32f_lcd_err_isr)
EXTERN(stm32f_dma2d_isr)
EXTERN(stm32f_sai2_isr)
EXTERN(stm32f_qspi_isr)
EXTERN(stm32f_cec_isr)
EXTERN(stm32f_spdif_rx_isr)
EXTERN(stm32f_fmpi2c1_isr)
EXTERN(stm32f_fmpi2c1_err_isr)

