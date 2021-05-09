/* 
 * stm32f446x-nvic.S
 *
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

#include <sys/stm32f.h>

	.syntax unified
	.cpu cortex-m3

	/* ---------------------------------------------------------------------
	 * Generate the vectors table 
	 * --------------------------------------------------------------------- */

	.section .nvic_vectors, "ax", %progbits
__nvic_vectors:
	.global __nvic_vectors
	.type __nvic_vectors, %object

	.word	stm32f_wwdg_isr         /*  0 Window WatchDog */
	.word	stm32f_pvd_isr          /*  1 PVD through EXTI Line detection */
	.word	stm32f_tamp_stamp_isr   /*  2 Tamper and TimeStamps through 
									        the EXTI line */
	.word	stm32f_rtc_wkup_isr     /*  3 RTC Wakeup through the EXTI line */
	.word	stm32f_flash_isr        /*  4 FLASH */
	.word	stm32f_rcc_isr          /*  5 RCC */
	.word	stm32f_exti0_isr        /*  6 EXTI Line0 */
	.word	stm32f_exti1_isr        /*  7 EXTI Line1 */

	.word	stm32f_exti2_isr        /*  8 EXTI Line2 */
	.word	stm32f_exti3_isr        /*  9 EXTI Line3 */
	.word	stm32f_exti4_isr        /* 10 EXTI Line4 */
	.word	stm32f_dma1_stream0_isr /* 11 DMA1 Stream 0 */
	.word	stm32f_dma1_stream1_isr /* 12 DMA1 Stream 1 */
	.word	stm32f_dma1_stream2_isr /* 13 DMA1 Stream 2 */
	.word	stm32f_dma1_stream3_isr /* 14 DMA1 Stream 3 */
	.word	stm32f_dma1_stream4_isr /* 15 DMA1 Stream 4 */

	.word	stm32f_dma1_stream5_isr /* 16 DMA1 Stream 5 */
	.word	stm32f_dma1_stream6_isr /* 17 DMA1 Stream 6 */
	.word	stm32f_adc_isr          /* 18 ADC1, ADC2 and ADC3s */
	.word	stm32f_can1_tx_isr         /* 19 CAN1 TX */
	.word	stm32f_can1_rx0_isr        /* 20 CAN1 RX0 */
	.word	stm32f_can1_rx1_isr        /* 21 CAN1 RX1 */
	.word	stm32f_can1_sce_isr        /* 22 CAN1 SCE */
	.word	stm32f_exti9_5_isr         /* 23 External Lines [9:5] */

	.word	stm32f_tim1_brk_tim9_isr      /* 24 TIM1 Break and TIM9 */
	.word	stm32f_tim1_up_tim10_isr      /* 25 TIM1 Update and TIM10 */
	.word	stm32f_tim1_trg_com_tim11_isr /* 26 TIM1 Trigger and Commutation 
	                                            and TIM11 */
	.word	stm32f_tim1_cc_isr        /* 27 TIM1 Capture Compare */
	.word	stm32f_tim2_isr           /* 28 TIM2 */
	.word	stm32f_tim3_isr           /* 29 TIM3 */
	.word	stm32f_tim4_isr           /* 30 TIM4 */
	.word	stm32f_i2c1_ev_isr        /* 31 I2C1 Event */

	.word	stm32f_i2c1_er_isr        /* 32 I2C1 Error */
	.word	stm32f_i2c2_ev_isr        /* 33 I2C2 Event */
	.word	stm32f_i2c2_er_isr        /* 34 I2C2 Error */
	.word	stm32f_spi1_isr           /* 35 SPI1 */
	.word	stm32f_spi2_isr           /* 36 SPI2 */
	.word	stm32f_usart1_isr         /* 37 USART1 */
	.word	stm32f_usart2_isr         /* 38 USART2 */
	.word	stm32f_usart3_isr         /* 39 USART3 */

	.word	stm32f_exti15_10_isr      /* 40 External Line[15:10]s */
	.word	stm32f_rtc_alarm_isr      /* 41 RTC Alarm (A and B) 
										 through EXTI Line */
	.word	stm32f_otg_fs_wkup_isr    /* 42 USB OTG FS Wakeup 
	                                     through EXTI line */
	.word	stm32f_tim8_brk_tim12_isr     /* 43 TIM8 Break and TIM12 */
	.word	stm32f_tim8_up_tim13_isr      /* 44 TIM8 Update and TIM13 */
	.word	stm32f_tim8_trg_com_tim14_isr /* 45 TIM8 Trigger and Commutation 
	                                            and TIM14 */
	.word	stm32f_tim8_cc_isr        /* 46 TIM8 Capture Compare */
	.word	stm32f_dma1_stream7_isr   /* 47 DMA1 Stream7 */

	.word	stm32f_fsmc_isr           /* 48 FSMC */
	.word	stm32f_sdio_isr           /* 49 SDIO */
	.word	stm32f_tim5_isr           /* 50 TIM5 */
	.word	stm32f_spi3_isr           /* 51 SPI3 */
	.word	stm32f_uart4_isr          /* 52 UART4 */
	.word	stm32f_uart5_isr          /* 53 UART5 */
	.word	stm32f_tim6_dac_isr       /* 54 TIM6 and DAC1&2 underrun errors */
	.word	stm32f_tim7_isr           /* 55 TIM7 */

	.word	stm32f_dma2_stream0_isr   /* 56 DMA2 Stream 0 */
	.word	stm32f_dma2_stream1_isr   /* 57 DMA2 Stream 1 */
	.word	stm32f_dma2_stream2_isr   /* 58 DMA2 Stream 2 */
	.word	stm32f_dma2_stream3_isr   /* 59 DMA2 Stream 3 */
	.word	stm32f_dma2_stream4_isr   /* 60 DMA2 Stream 4 */
	.word	stm32f_eth_isr            /* 61 Ethernet */
	.word	stm32f_eth_wkup_isr       /* 62 Ethernet Wakeup through EXTI line */
	.word	stm32f_can2_tx_isr        /* 63 CAN2 TX */

	.word	stm32f_can2_rx0_isr       /* 64 CAN2 RX0 */
	.word	stm32f_can2_rx1_isr       /* 65 CAN2 RX1 */
	.word	stm32f_can2_sce_isr       /* 66 CAN2 SCE */
	.word	stm32f_otg_fs_isr         /* 67 USB OTG FS */
	.word	stm32f_dma2_stream5_isr   /* 68 DMA2 Stream 5 */
	.word	stm32f_dma2_stream6_isr   /* 69 DMA2 Stream 6 */
	.word	stm32f_dma2_stream7_isr   /* 70 DMA2 Stream 7 */
	.word	stm32f_usart6_isr         /* 71 USART6 */

	.word	stm32f_i2c3_ev_isr        /* 72 I2C3 event */
	.word	stm32f_i2c3_er_isr        /* 73 I2C3 error */
	.word	stm32f_otg_hs_ep1_out_isr /* 74 USB OTG HS End Point 1 Out */
	.word	stm32f_otg_hs_ep1_in_isr  /* 75 USB OTG HS End Point 1 In */
	.word	stm32f_otg_hs_wkup_isr    /* 76 USB OTG HS Wakeup through EXTI */
	.word	stm32f_otg_hs_isr         /* 77 USB OTG HS */
	.word	stm32f_dcmi_isr           /* 78 DCMI */
	.word	stm32f_cryp_isr           /* 79 CRYP crypto */

	.word	stm32f_hash_rng_isr       /* 80 Hash and Rng */
	.word   stm32f_fpu_isr            /* 81 FPU */
	.word   stm32f_uart7_isr          /* 82 UART7 */
	.word   stm32f_uart8_isr          /* 83 UART8 */
	.word   stm32f_spi4_isr           /* 84 SPI4 */
	.word   stm32f_spi5_isr           /* 85 SPI5 */
	.word   stm32f_spi6_isr           /* 86 SPI6 */
	.word   stm32f_sai1_isr           /* 87 SAI1 */

	.word   stm32f_lcd_isr            /* 88 LCD-TFT */
	.word   stm32f_lcd_err_isr        /* 89 LCD-TFT error */
	.word   stm32f_dma2d_isr          /* 90 DMA2D */
	.word   stm32f_sai2_isr           /* 91 SAI2 */
	.word   stm32f_qspi_isr           /* 92 QuadSPI */
	.word   stm32f_cec_isr            /* 93 HDMI-CEC */
	.word   stm32f_spdif_rx_isr       /* 94 SPDIF-Rx */
	.word   stm32f_fmpi2c1_isr        /* 95 FMPI2C1 event */
	.word   stm32f_fmpi2c1_err_isr    /* 96 FMPI2C1 error */

__nvic_vectors_end:
	.global __nvic_vectors_end
	.size   __nvic_vectors, . - __nvic_vectors

	.text
	.align  2
	.global _isr
	.thumb
	.thumb_func
	.type   _isr, %function
_isr:
	mrs     r0, IPSR
	subs    r0, #16
	b       cm3_default_isr
	.size   _isr, . - _isr

	/***********************************************************************
	 * Provide weak aliases for each Exception handler to _isr().
	 * Any function with the same name will override these definitions.
	 */

	.weak       stm32f_wwdg_isr
	.thumb_set  stm32f_wwdg_isr,_isr

	.weak       stm32f_pvd_isr
	.thumb_set  stm32f_pvd_isr,_isr

	.weak       stm32f_tamp_stamp_isr
	.thumb_set  stm32f_tamp_stamp_isr,_isr

	.weak       stm32f_rtc_wkup_isr
	.thumb_set  stm32f_rtc_wkup_isr,_isr

	.weak       stm32f_flash_isr
	.thumb_set  stm32f_flash_isr,_isr

	.weak       stm32f_rcc_isr
	.thumb_set  stm32f_rcc_isr,_isr

	.weak       stm32f_exti0_isr
	.thumb_set  stm32f_exti0_isr,_isr

	.weak       stm32f_exti1_isr
	.thumb_set  stm32f_exti1_isr,_isr

	.weak       stm32f_exti2_isr
	.thumb_set  stm32f_exti2_isr,_isr

	.weak       stm32f_exti2_tsc_isr
	.thumb_set  stm32f_exti2_tsc_isr,_isr

	.weak       stm32f_exti3_isr
	.thumb_set  stm32f_exti3_isr,_isr

	.weak       stm32f_exti4_isr
	.thumb_set  stm32f_exti4_isr,_isr

	.weak       stm32f_dma1_stream0_isr
	.thumb_set  stm32f_dma1_stream0_isr,_isr

	.weak       stm32f_dma1_stream1_isr
	.thumb_set  stm32f_dma1_stream1_isr,_isr

	.weak       stm32f_dma1_stream2_isr
	.thumb_set  stm32f_dma1_stream2_isr,_isr

	.weak       stm32f_dma1_stream3_isr
	.thumb_set  stm32f_dma1_stream3_isr,_isr

	.weak       stm32f_dma1_stream4_isr
	.thumb_set  stm32f_dma1_stream4_isr,_isr

	.weak       stm32f_dma1_stream5_isr
	.thumb_set  stm32f_dma1_stream5_isr,_isr

	.weak       stm32f_dma1_stream6_isr
	.thumb_set  stm32f_dma1_stream6_isr,_isr

	.weak       stm32f_adc_isr
	.thumb_set  stm32f_adc_isr,_isr

	.weak       stm32f_can1_tx_isr
	.thumb_set  stm32f_can1_tx_isr,_isr

	.weak       stm32f_can1_rx0_isr
	.thumb_set  stm32f_can1_rx0_isr,_isr

	.weak       stm32f_can1_tx_usb_hp_isr
	.thumb_set  stm32f_can1_tx_usb_hp_isr,_isr

	.weak       stm32f_can1_rx0_usb_lp_isr
	.thumb_set  stm32f_can1_rx0_usb_lp_isr,_isr

	.weak       stm32f_can1_rx1_isr
	.thumb_set  stm32f_can1_rx1_isr,_isr

	.weak       stm32f_can1_sce_isr
	.thumb_set  stm32f_can1_sce_isr,_isr

	.weak       stm32f_exti9_5_isr
	.thumb_set  stm32f_exti9_5_isr,_isr

	.weak       stm32f_tim1_brk_tim9_isr
	.thumb_set  stm32f_tim1_brk_tim9_isr,_isr

	.weak       stm32f_tim1_up_tim10_isr
	.thumb_set  stm32f_tim1_up_tim10_isr,_isr

	.weak       stm32f_tim1_trg_com_tim11_isr
	.thumb_set  stm32f_tim1_trg_com_tim11_isr,_isr

	.weak       stm32f_tim1_brk_isr
	.thumb_set  stm32f_tim1_brk_isr,_isr

	.weak       stm32f_tim1_up_isr
	.thumb_set  stm32f_tim1_up_isr,_isr

	.weak       stm32f_tim1_trg_com_isr
	.thumb_set  stm32f_tim1_trg_com_isr,_isr

	.weak       stm32f_tim1_brk_tim15_isr
	.thumb_set  stm32f_tim1_brk_tim15_isr,_isr

	.weak       stm32f_tim1_up_tim16_isr
	.thumb_set  stm32f_tim1_up_tim16_isr,_isr

	.weak       stm32f_tim1_trg_com_tim17_isr
	.thumb_set  stm32f_tim1_trg_com_tim17_isr,_isr

	.weak       stm32f_tim1_cc_isr
	.thumb_set  stm32f_tim1_cc_isr,_isr

	.weak       stm32f_tim2_isr
	.thumb_set  stm32f_tim2_isr,_isr

	.weak       stm32f_tim3_isr
	.thumb_set  stm32f_tim3_isr,_isr

	.weak       stm32f_tim4_isr
	.thumb_set  stm32f_tim4_isr,_isr

	.weak       stm32f_i2c1_ev_isr
	.thumb_set  stm32f_i2c1_ev_isr,_isr

	.weak       stm32f_i2c1_ev_exti23_isr
	.thumb_set  stm32f_i2c1_ev_exti23_isr,_isr

	.weak       stm32f_i2c1_er_isr
	.thumb_set  stm32f_i2c1_er_isr,_isr

	.weak       stm32f_i2c2_ev_isr
	.thumb_set  stm32f_i2c2_ev_isr,_isr

	.weak       stm32f_i2c2_ev_exti24_isr
	.thumb_set  stm32f_i2c2_ev_exti24_isr,_isr

	.weak       stm32f_i2c2_er_isr
	.thumb_set  stm32f_i2c2_er_isr,_isr

	.weak       stm32f_spi1_isr
	.thumb_set  stm32f_spi1_isr,_isr

	.weak       stm32f_spi2_isr
	.thumb_set  stm32f_spi2_isr,_isr

	.weak       stm32f_usart1_isr
	.thumb_set  stm32f_usart1_isr,_isr

	.weak       stm32f_usart1_exti25_isr
	.thumb_set  stm32f_usart1_exti25_isr,_isr

	.weak       stm32f_usart2_isr
	.thumb_set  stm32f_usart2_isr,_isr

	.weak       stm32f_usart2_exti26_isr
	.thumb_set  stm32f_usart2_exti26_isr,_isr

	.weak       stm32f_usart3_isr
	.thumb_set  stm32f_usart3_isr,_isr

	.weak       stm32f_usart3_exti28_isr
	.thumb_set  stm32f_usart3_exti28_isr,_isr

	.weak       stm32f_exti15_10_isr
	.thumb_set  stm32f_exti15_10_isr,_isr

	.weak       stm32f_rtc_alarm_isr
	.thumb_set  stm32f_rtc_alarm_isr,_isr

	.weak       stm32f_otg_fs_wkup_isr
	.thumb_set  stm32f_otg_fs_wkup_isr,_isr

	.weak       stm32f_usb_wkup_isr
	.thumb_set  stm32f_usb_wkup_isr,_isr

	.weak       stm32f_cec_isr
	.thumb_set  stm32f_cec_isr,_isr

	.weak       stm32f_tim12_isr
	.thumb_set  stm32f_tim12_isr,_isr

	.weak       stm32f_tim13_isr
	.thumb_set  stm32f_tim13_isr,_isr

	.weak       stm32f_tim14_isr
	.thumb_set  stm32f_tim14_isr,_isr

	.weak       stm32f_tim8_brk_tim12_isr
	.thumb_set  stm32f_tim8_brk_tim12_isr,_isr

	.weak       stm32f_tim8_up_tim13_isr
	.thumb_set  stm32f_tim8_up_tim13_isr,_isr

	.weak       stm32f_tim8_trg_com_tim14_isr
	.thumb_set  stm32f_tim8_trg_com_tim14_isr,_isr

	.weak       stm32f_tim8_cc_isr
	.thumb_set  stm32f_tim8_cc_isr,_isr

	.weak       stm32f_dma1_stream7_isr
	.thumb_set  stm32f_dma1_stream7_isr,_isr

	.weak       stm32f_fsmc_isr
	.thumb_set  stm32f_fsmc_isr,_isr

	.weak       stm32f_sdio_isr
	.thumb_set  stm32f_sdio_isr,_isr

	.weak       stm32f_tim5_isr
	.thumb_set  stm32f_tim5_isr,_isr

	.weak       stm32f_spi3_isr
	.thumb_set  stm32f_spi3_isr,_isr

	.weak       stm32f_uart4_isr
	.thumb_set  stm32f_uart4_isr,_isr

	.weak       stm32f_uart5_isr
	.thumb_set  stm32f_uart5_isr,_isr

	.weak       stm32f_tim6_dac_isr
	.thumb_set  stm32f_tim6_dac_isr,_isr

	.weak       stm32f_tim7_isr
	.thumb_set  stm32f_tim7_isr,_isr

	.weak       stm32f_dma2_stream0_isr
	.thumb_set  stm32f_dma2_stream0_isr,_isr

	.weak       stm32f_dma2_stream1_isr
	.thumb_set  stm32f_dma2_stream1_isr,_isr

	.weak       stm32f_dma2_stream2_isr
	.thumb_set  stm32f_dma2_stream2_isr,_isr

	.weak       stm32f_dma2_stream3_isr
	.thumb_set  stm32f_dma2_stream3_isr,_isr

	.weak       stm32f_dma2_stream4_isr
	.thumb_set  stm32f_dma2_stream4_isr,_isr

	.weak       stm32f_eth_isr
	.thumb_set  stm32f_eth_isr,_isr

	.weak       stm32f_eth_wkup_isr
	.thumb_set  stm32f_eth_wkup_isr,_isr

	.weak       stm32f_can2_tx_isr
	.thumb_set  stm32f_can2_tx_isr,_isr

	.weak       stm32f_can2_rx0_isr
	.thumb_set  stm32f_can2_rx0_isr,_isr

	.weak       stm32f_can2_rx1_isr
	.thumb_set  stm32f_can2_rx1_isr,_isr

	.weak       stm32f_can2_sce_isr
	.thumb_set  stm32f_can2_sce_isr,_isr

	.weak       stm32f_otg_fs_isr
	.thumb_set  stm32f_otg_fs_isr,_isr

	.weak       stm32f_dma2_stream5_isr
	.thumb_set  stm32f_dma2_stream5_isr,_isr

	.weak       stm32f_dma2_stream6_isr
	.thumb_set  stm32f_dma2_stream6_isr,_isr

	.weak       stm32f_dma2_stream7_isr
	.thumb_set  stm32f_dma2_stream7_isr,_isr

	.weak       stm32f_usart6_isr
	.thumb_set  stm32f_usart6_isr,_isr

	.weak       stm32f_i2c3_ev_isr
	.thumb_set  stm32f_i2c3_ev_isr,_isr

	.weak       stm32f_i2c3_er_isr
	.thumb_set  stm32f_i2c3_er_isr,_isr

	.weak       stm32f_otg_hs_ep1_out_isr
	.thumb_set  stm32f_otg_hs_ep1_out_isr,_isr

	.weak       stm32f_otg_hs_ep1_in_isr
	.thumb_set  stm32f_otg_hs_ep1_in_isr,_isr

	.weak       stm32f_otg_hs_wkup_isr
	.thumb_set  stm32f_otg_hs_wkup_isr,_isr

	.weak       stm32f_otg_hs_isr
	.thumb_set  stm32f_otg_hs_isr,_isr

	.weak       stm32f_dcmi_isr
	.thumb_set  stm32f_dcmi_isr,_isr

	.weak       stm32f_cryp_isr
	.thumb_set  stm32f_cryp_isr,_isr

	.weak       stm32f_hash_rng_isr
	.thumb_set  stm32f_hash_rng_isr,_isr

	.weak       stm32f_fpu_isr
	.thumb_set  stm32f_fpu_isr,_isr

	.weak       stm32f_uart7_isr
	.thumb_set  stm32f_uart7_isr,_isr

	.weak       stm32f_uart8_isr
	.thumb_set  stm32f_uart8_isr,_isr

	.weak       stm32f_spi4_isr
	.thumb_set  stm32f_spi4_isr,_isr

	.weak       stm32f_spi5_isr
	.thumb_set  stm32f_spi5_isr,_isr

	.weak       stm32f_spi6_isr
	.thumb_set  stm32f_spi6_isr,_isr

	.weak       stm32f_sai1_isr
	.thumb_set  stm32f_sai1_isr,_isr

	.weak       stm32f_lcd_isr
	.thumb_set  stm32f_lcd_isr,_isr

	.weak       stm32f_lcd_err_isr
	.thumb_set  stm32f_lcd_err_isr,_isr

	.weak       stm32f_dma2d_isr
	.thumb_set  stm32f_dma2d_isr,_isr

	.weak       stm32f_sai2_isr
	.thumb_set  stm32f_sai2_isr,_isr

	.weak       stm32f_qspi_isr
	.thumb_set  stm32f_qspi_isr,_isr

	.weak       stm32f_cec_isr
	.thumb_set  stm32f_cec_isr,_isr

	.weak       stm32f_spdif_rx_isr
	.thumb_set  stm32f_spdif_rx_isr,_isr

	.weak       stm32f_fmpi2c1_isr
	.thumb_set  stm32f_fmpi2c1_isr,_isr

	.weak       stm32f_fmpi2c1_err_isr
	.thumb_set  stm32f_fmpi2c1_err_isr,_isr

	.end


