/* 
 * stm32-crt0.S
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

#include <sys/stm32.h>

	.syntax unified
	.cpu cortex-m3

	.extern _reset

	/* ---------------------------------------------------------------------
	 * Generate the vectors table 
	 * --------------------------------------------------------------------- */
.macro cortex_m_vectors
	.word	_stack
	.word   _reset
	.word   cm3_nmi_isr
	.word   cm3_hard_fault_isr
	.word   cm3_mem_manage_isr
	.word   cm3_bus_fault_isr
	.word   cm3_usage_fault_isr
	.word   cm3_except7_isr
	.word   ice_comm_blk
	.word   cm3_except9_isr
	.word   cm3_except10_isr
	.word   cm3_svc_isr
	.word   cm3_debug_mon_isr
	.word   cm3_except13_isr
	.word   cm3_pendsv_isr
	.word   cm3_systick_isr
.endm

.macro stm32_irq_vectors
	.word	stm32_wwdg_isr         /*  0 Window WatchDog */
	.word	stm32_pvd_isr          /*  1 PVD through EXTI Line detection */
	.word	stm32_tamp_stamp_isr   /*  2 Tamper and TimeStamps through 
									        the EXTI line */
	.word	stm32_rtc_wkup_isr     /*  3 RTC Wakeup through the EXTI line */
	.word	stm32_flash_isr        /*  4 FLASH */
	.word	stm32_rcc_isr          /*  5 RCC */
	.word	stm32_exti0_isr        /*  6 EXTI Line0 */
	.word	stm32_exti1_isr        /*  7 EXTI Line1 */
	.word	stm32_exti2_isr        /*  8 EXTI Line2 */
	.word	stm32_exti3_isr        /*  9 EXTI Line3 */
	.word	stm32_exti4_isr        /* 10 EXTI Line4 */

	.word	stm32_dma1_channel1_isr   /* 11 DMA1 Channel 1 */
	.word	stm32_dma1_channel2_isr   /* 12 DMA1 Channel 2 */
	.word	stm32_dma1_channel3_isr   /* 13 DMA1 Channel 3 */
	.word	stm32_dma1_channel4_isr   /* 14 DMA1 Channel 4 */
	.word	stm32_dma1_channel5_isr   /* 15 DMA1 Channel 5 */

	.word	stm32_dma1_channel6_isr   /* 16 DMA1 Channel 6 */
	.word	stm32_dma1_channel7_isr   /* 17 DMA1 Channel 7 */

	.word	stm32_adc_isr             /* 18 ADC1, ADC2 and ADC3s */

	.word	stm32_can1_tx_isr         /* 19 CAN1 TX */
	.word	stm32_can1_rx0_isr        /* 20 CAN1 RX0 */
	.word	stm32_can1_rx1_isr        /* 21 CAN1 RX1 */
	.word	stm32_can1_sce_isr        /* 22 CAN1 SCE */
	.word	stm32_exti9_5_isr         /* 23 External Lines [9:5] */

	.word	stm32_tim1_brk_tim15_isr  /* 24 TIM1 Break and TIM15 */
	.word	stm32_tim1_up_tim16_isr   /* 25 TIM1 Update and TIM16 */
	.word	stm32_tim1_trg_com_isr    /* 26 TIM1 Trigger and Commutation */

	.word	stm32_tim1_cc_isr        /* 27 TIM1 Capture Compare */
	.word	stm32_tim2_isr           /* 28 TIM2 */
	.word	stm32_tim3_isr           /* 29 TIM3 */

	.word	0

	.word	stm32_i2c1_ev_isr        /* 31 I2C1 Event */
	.word	stm32_i2c1_er_isr        /* 32 I2C1 Error */
	.word	stm32_i2c2_ev_isr        /* 33 I2C2 Event */
	.word	stm32_i2c2_er_isr        /* 34 I2C2 Error */

	.word	stm32_spi1_isr           /* 35 SPI1 */
	.word	stm32_spi2_isr           /* 36 SPI2 */

	.word	stm32_usart1_isr         /* 37 USART1 */
	.word	stm32_usart2_isr         /* 38 USART2 */
	.word	stm32_usart3_isr         /* 39 USART3 */

	.word	stm32_exti15_10_isr      /* 40 External Line[15:10]s */
	.word	stm32_rtc_alarm_isr      /* 41 RTC Alarm (A and B) 
										 through EXTI Line */

	.word	_fault                   /* 42 Reserved */
	.word	_fault                   /* 43 Reserved */
	.word	_fault                   /* 44 Reserved */
	.word	_fault                   /* 45 Reserved */
	.word	_fault                   /* 46 Reserved */
	.word	_fault                   /* 47 Reserved */
	.word	_fault                   /* 48 Reserved */

	.word	stm32_sdmmc1_isr         /* 49 SDMMC */
	.word	_fault                   /* 50 Reserved */

	.word	stm32_spi3_isr           /* 51 SPI3 */
	.word	stm32_uart4_isr          /* 52 UART4 */
	.word	_fault                   /* 53 Reserved */

	.word	stm32_tim6_dac_isr       /* 54 TIM6 and DAC1&2 underrun errors */
	.word	stm32_tim7_isr           /* 55 TIM7 */

	.word	stm32_dma2_channel1_isr   /* 56 DMA2 Channel 1 */
	.word	stm32_dma2_channel2_isr   /* 57 DMA2 Channel 2 */
	.word	stm32_dma2_channel3_isr   /* 59 DMA2 Channel 3 */
	.word	stm32_dma2_channel4_isr   /* 59 DMA2 Channel 4 */
	.word	stm32_dma2_channel5_isr   /* 60 DMA2 Channel 5 */

	.word	stm32_dfsdm1_flt0        /* 61 DFSDM1_FLT0 global interrupt */
	.word	stm32_dfsdm1_flt1        /* 62 DFSDM1_FLT1 global interrupt */
	.word	_fault                   /* 63 Reserved */
	.word	stm32_comp_isr           /* 64 Comparator */
	.word	stm32_lptim1_isr         /* 65 LPTIM1 */
	.word	stm32_lptim2_isr         /* 66 LPTIM2 */

	.word	stm32_usb_fs_isr         /* 67 USB FS */
	.word	stm32_dma2_channel6_isr  /* 68 DMA2 Channel 6 */
	.word	stm32_dma2_channel7_isr  /* 69 DMA2 Channel 7 */
	.word   stm32_lpuart1_isr        /* 70 LPUART1 */
	.word   stm32_qspi_isr           /* 71 QuadSPI */
	.word	stm32_i2c3_ev_isr        /* 72 I2C3 event */
	.word	stm32_i2c3_er_isr        /* 73 I2C3 error */
	.word   stm32_sai1_isr           /* 74 SAI1 */
	.word	_fault                   /* 75 Reserved */

	.word	stm32_swpmi_isr          /* 76 SWPMI */
	.word	stm32_tsc_isr            /* 77 TSC */
	.word	stm32_lcd_isr            /* 78 LCD */
	.word	stm32_aes_isr            /* 79 AES crypto */
	.word	stm32_hash_rng_isr       /* 80 Hash and Rng */

	.word   stm32_fpu_isr            /* 81 FPU */
	.word   stm32_cec_isr            /* 82 */
	.word	stm32_i2c4_ev_isr        /* 83 I2C4 event */
	.word	stm32_i2c4_er_isr        /* 84 I2C4 error */

.endm


	.section .vectors
__vectors:
	.global __vectors

	/* ---------------------------------------------------------------------
	 * vectors residing in FLASH 
	 * --------------------------------------------------------------------- */
	.section .rom_vectors, "ax", %progbits
__rom_vectors:
	.global __rom_vectors
	.type __rom_vectors, %object
	cortex_m_vectors
	stm32_irq_vectors
__rom_vectors_end:
	.global __rom_vectors_end
	.size   __rom_vectors, . - __rom_vectors

	.text
	.align  2
	.global __sizeof_rom_vectors
__sizeof_rom_vectors:
	.type   __sizeof_rom_vectors, %object
	.word	__rom_vectors_end - __rom_vectors
	.size   __sizeof_rom_vectors, . - __sizeof_rom_vectors

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

	.text
	.align  2
	.global _fault
	.thumb
	.thumb_func
	.type   _fault, %function
_fault:
	b		.
	.size   _fault, . - _fault

	/***********************************************************************
	 * Provide weak aliases for each Exception handler to _fault().
	 * Any function with the same name will override these definitions.
	 */
	.weak       cm3_default_isr
	.thumb_set  cm3_default_isr,_fault

	.weak       cm3_nmi_isr
	.thumb_set  cm3_nmi_isr,_fault

	.weak       cm3_hard_fault_isr
	.thumb_set  cm3_hard_fault_isr,_fault

	.weak       cm3_mem_manage_isr
	.thumb_set  cm3_mem_manage_isr,_fault

	.weak       cm3_bus_fault_isr
	.thumb_set  cm3_bus_fault_isr,_fault

	.weak       cm3_usage_fault_isr
	.thumb_set  cm3_usage_fault_isr,_fault

	.weak       cm3_except7_isr
	.thumb_set  cm3_except7_isr,_fault

	.weak       ice_comm_blk
	.set		ice_comm_blk, __vectors

	.weak       cm3_except9_isr
	.thumb_set  cm3_except9_isr,_fault

	.weak       cm3_except10_isr
	.thumb_set  cm3_except10_isr,_fault

	.weak       cm3_svc_isr
	.thumb_set  cm3_svc_isr,_fault

	.weak       cm3_debug_mon_isr
	.thumb_set  cm3_debug_mon_isr,_fault

	.weak       cm3_except13_isr
	.thumb_set  cm3_except13_isr,_fault

	.weak     	cm3_pendsv_isr
	.thumb_set  cm3_pendsv_isr,_fault

	.weak       cm3_systick_isr
	.thumb_set  cm3_systick_isr,_fault

	.weak       stm32_wwdg_isr
	.thumb_set  stm32_wwdg_isr,_fault

	.weak       stm32_pvd_isr
	.thumb_set  stm32_pvd_isr,_isr

	.weak       stm32_tamp_stamp_isr
	.thumb_set  stm32_tamp_stamp_isr,_isr

	.weak       stm32_rtc_wkup_isr
	.thumb_set  stm32_rtc_wkup_isr,_isr

	.weak       stm32_flash_isr
	.thumb_set  stm32_flash_isr,_isr

	.weak       stm32_rcc_isr
	.thumb_set  stm32_rcc_isr,_isr

	.weak       stm32_exti0_isr
	.thumb_set  stm32_exti0_isr,_isr

	.weak       stm32_exti1_isr
	.thumb_set  stm32_exti1_isr,_isr

	.weak       stm32_exti2_isr
	.thumb_set  stm32_exti2_isr,_isr

	.weak       stm32_exti3_isr
	.thumb_set  stm32_exti3_isr,_isr

	.weak       stm32_exti4_isr
	.thumb_set  stm32_exti4_isr,_isr

	.weak       stm32_dma1_channel1_isr
	.thumb_set  stm32_dma1_channel1_isr,_isr

	.weak       stm32_dma1_channel2_isr
	.thumb_set  stm32_dma1_channel2_isr,_isr

	.weak       stm32_dma1_channel3_isr
	.thumb_set  stm32_dma1_channel3_isr,_isr

	.weak       stm32_dma1_channel4_isr
	.thumb_set  stm32_dma1_channel4_isr,_isr

	.weak       stm32_dma1_channel5_isr
	.thumb_set  stm32_dma1_channel5_isr,_isr

	.weak       stm32_dma1_channel6_isr
	.thumb_set  stm32_dma1_channel6_isr,_isr

	.weak       stm32_dma1_channel7_isr
	.thumb_set  stm32_dma1_channel7_isr,_isr

	.weak       stm32_adc_isr
	.thumb_set  stm32_adc_isr,_isr

	.weak       stm32_can1_tx_isr
	.thumb_set  stm32_can1_tx_isr,_isr

	.weak       stm32_can1_rx0_isr
	.thumb_set  stm32_can1_rx0_isr,_isr

	.weak       stm32_can1_rx1_isr
	.thumb_set  stm32_can1_rx1_isr,_isr

	.weak       stm32_can1_sce_isr
	.thumb_set  stm32_can1_sce_isr,_isr

	.weak       stm32_exti9_5_isr
	.thumb_set  stm32_exti9_5_isr,_isr

	.weak       stm32_tim1_brk_tim15_isr
	.thumb_set  stm32_tim1_brk_tim15_isr,_isr

	.weak       stm32_tim1_up_tim16_isr
	.thumb_set  stm32_tim1_up_tim16_isr,_isr

	.weak       stm32_tim1_trg_com_isr
	.thumb_set  stm32_tim1_trg_com_isr,_isr

	.weak       stm32_tim1_cc_isr
	.thumb_set  stm32_tim1_cc_isr,_isr

	.weak       stm32_tim2_isr
	.thumb_set  stm32_tim2_isr,_isr

	.weak       stm32_tim3_isr
	.thumb_set  stm32_tim3_isr,_isr

	.weak       stm32_i2c1_ev_isr
	.thumb_set  stm32_i2c1_ev_isr,_isr

	.weak       stm32_i2c1_er_isr
	.thumb_set  stm32_i2c1_er_isr,_isr

	.weak       stm32_i2c2_ev_isr
	.thumb_set  stm32_i2c2_ev_isr,_isr

	.weak       stm32_i2c2_er_isr
	.thumb_set  stm32_i2c2_er_isr,_isr

	.weak       stm32_spi1_isr
	.thumb_set  stm32_spi1_isr,_isr

	.weak       stm32_spi2_isr
	.thumb_set  stm32_spi2_isr,_isr

	.weak       stm32_usart1_isr
	.thumb_set  stm32_usart1_isr,_isr

	.weak       stm32_usart2_isr
	.thumb_set  stm32_usart2_isr,_isr

	.weak       stm32_usart3_isr
	.thumb_set  stm32_usart3_isr,_isr

	.weak       stm32_exti15_10_isr
	.thumb_set  stm32_exti15_10_isr,_isr

	.weak       stm32_rtc_alarm_isr
	.thumb_set  stm32_rtc_alarm_isr,_isr

	.weak       stm32_sdmmc1_isr
	.thumb_set  stm32_sdmmc1_isr,_isr

	.weak       stm32_spi3_isr
	.thumb_set  stm32_spi3_isr,_isr

	.weak       stm32_uart4_isr
	.thumb_set  stm32_uart4_isr,_isr

	.weak       stm32_tim6_dac_isr
	.thumb_set  stm32_tim6_dac_isr,_isr

	.weak       stm32_tim7_isr
	.thumb_set  stm32_tim7_isr,_isr

	.weak       stm32_dma2_channel1_isr
	.thumb_set  stm32_dma2_channel1_isr,_isr

	.weak       stm32_dma2_channel2_isr
	.thumb_set  stm32_dma2_channel2_isr,_isr

	.weak       stm32_dma2_channel3_isr
	.thumb_set  stm32_dma2_channel3_isr,_isr

	.weak       stm32_dma2_channel4_isr
	.thumb_set  stm32_dma2_channel4_isr,_isr

	.weak       stm32_dma2_channel5_isr
	.thumb_set  stm32_dma2_channel5_isr,_isr

	.weak       stm32_dfsdm1_flt0
	.thumb_set  stm32_dfsdm1_flt0,_isr

	.weak       stm32_dfsdm1_flt1
	.thumb_set  stm32_dfsdm1_flt1,_isr

	.weak       stm32_comp_isr
	.thumb_set  stm32_comp_isr,_isr

	.weak       stm32_lptim1_isr
	.thumb_set  stm32_lptim1_isr,_isr

	.weak       stm32_lptim2_isr
	.thumb_set  stm32_lptim2_isr,_isr

	.weak       stm32_usb_fs_isr
	.thumb_set  stm32_usb_fs_isr,_isr

	.weak       stm32_dma2_channel6_isr
	.thumb_set  stm32_dma2_channel6_isr,_isr

	.weak       stm32_dma2_channel7_isr
	.thumb_set  stm32_dma2_channel7_isr,_isr

	.weak       stm32_lpuart1_isr
	.thumb_set  stm32_lpuart1_isr,_isr

	.weak       stm32_qspi_isr
	.thumb_set  stm32_qspi_isr,_isr

	.weak       stm32_i2c3_ev_isr
	.thumb_set  stm32_i2c3_ev_isr,_isr

	.weak       stm32_i2c3_er_isr
	.thumb_set  stm32_i2c3_er_isr,_isr

	.weak       stm32_sai1_isr
	.thumb_set  stm32_sai1_isr,_isr

	.weak       stm32_swpmi_isr
	.thumb_set  stm32_swpmi_isr,_isr

	.weak       stm32_tsc_isr
	.thumb_set  stm32_tsc_isr,_isr

	.weak       stm32_lcd_isr
	.thumb_set  stm32_lcd_isr,_isr

	.weak       stm32_aes_isr
	.thumb_set  stm32_aes_isr,_isr

	.weak       stm32_hash_rng_isr
	.thumb_set  stm32_hash_rng_isr,_isr
	
	.weak       stm32_fpu_isr
	.thumb_set  stm32_fpu_isr,_isr

	.weak       stm32_cec_isr
	.thumb_set  stm32_cec_isr,_isr

	.weak       stm32_i2c4_ev_isr
	.thumb_set  stm32_i2c4_ev_isr,_isr

	.weak       stm32_i2c4_er_isr
	.thumb_set  stm32_i2c4_er_isr,_isr

	.end
