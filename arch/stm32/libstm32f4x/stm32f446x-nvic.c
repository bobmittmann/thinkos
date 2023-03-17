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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#include <arch/cortex-m3.h>
#include <sys/stm32f.h>

typedef void (* __isr_t)(void);

void cm3_default_isr(int irq);

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

#if (THINKOS_IRQ_MAX) > 0
void __attribute__ ((aligned(16))) __default_isr(void)
{
	uint32_t ipsr = cm3_ipsr_get();
	uint32_t irq;

	irq = ipsr - 16;

	/* disable this interrupt source */
	cm3_irq_disable(irq);
	struct thinkos_rt * krn = &thinkos_rt;
#if (THINKOS_ENABLE_IRQ_CYCCNT)
	/* set the thread's return value to cyle count */
	uint32_t cyccnt = CM3_DWT->cyccnt;
#endif
	unsigned int th;

	/* disable this interrupt source */
	cm3_irq_disable(irq);

	th = krn->irq_th[irq];

#if (IRQ_DEBUG)
#if (THINKOS_ENABLE_SANITY_CHECK)
	krn->irq_th[irq] = THINKOS_THREAD_VOID;
#endif 
	if (th >= THINKOS_THREAD_IDLE) {
		DCC_LOG2(LOG_ERROR, "<%2d> IRQ %d invalid thread!", th, irq);
		return;
	} else if (th == THINKOS_THREAD_VOID) {
		DCC_LOG1(LOG_WARNING, "VOID IRQ %d !!!", irq);
	} else {
		DCC_LOG2(LOG_MSG, "<%2d> IRQ %d..", th, irq);
	}
#endif

#if (THINKOS_ENABLE_IRQ_CYCCNT)
	/* set the thread's cyccnt value */
	__thread_r1_set(krn, th, cyccnt);
#endif

#if (THINKOS_ENABLE_WQ_IRQ)
	__krn_wq_thread_del(krn, THINKOS_WQ_IRQ, th);  
#endif 

	/* insert the thread into ready queue */
	__krn_wq_ready_thread_ins(krn, th);

	/* signal the scheduler ... */
	__krn_preempt(krn);
}
#endif

void stm32f_wwdg_isr(void);
void stm32f_pvd_isr(void);
void stm32f_tamp_stamp_isr(void);
void stm32f_rtc_wkup_isr(void);
void stm32f_flash_isr(void);
void stm32f_rcc_isr(void);
void stm32f_exti0_isr(void);
void stm32f_exti1_isr(void);
void stm32f_exti2_isr(void);
void stm32f_exti2_tsc_isr(void);
void stm32f_exti3_isr(void);
void stm32f_exti4_isr(void);
void stm32f_dma1_stream0_isr(void);
void stm32f_dma1_stream1_isr(void);
void stm32f_dma1_stream2_isr(void);
void stm32f_dma1_stream3_isr(void);
void stm32f_dma1_stream4_isr(void);
void stm32f_dma1_stream5_isr(void);
void stm32f_dma1_stream6_isr(void);
void stm32f_adc_isr(void);
void stm32f_can1_tx_isr(void);
void stm32f_can1_rx0_isr(void);
void stm32f_can1_tx_usb_hp_isr(void);
void stm32f_can1_rx0_usb_lp_isr(void);
void stm32f_can1_rx1_isr(void);
void stm32f_can1_sce_isr(void);
void stm32f_exti9_5_isr(void);
void stm32f_tim1_brk_tim9_isr(void);
void stm32f_tim1_up_tim10_isr(void);
void stm32f_tim1_trg_com_tim11_isr(void);
void stm32f_tim1_brk_isr(void);
void stm32f_tim1_up_isr(void);
void stm32f_tim1_trg_com_isr(void);
void stm32f_tim1_brk_tim15_isr(void);
void stm32f_tim1_up_tim16_isr(void);
void stm32f_tim1_trg_com_tim17_isr(void);
void stm32f_tim1_cc_isr(void);
void stm32f_tim2_isr(void);
void stm32f_tim3_isr(void);
void stm32f_tim4_isr(void);
void stm32f_i2c1_ev_isr(void);
void stm32f_i2c1_ev_exti23_isr(void);
void stm32f_i2c1_er_isr(void);
void stm32f_i2c2_ev_isr(void);
void stm32f_i2c2_ev_exti24_isr(void);
void stm32f_i2c2_er_isr(void);
void stm32f_spi1_isr(void);
void stm32f_spi2_isr(void);
void stm32f_usart1_isr(void);
void stm32f_usart1_exti25_isr(void);
void stm32f_usart2_isr(void);
void stm32f_usart2_exti26_isr(void);
void stm32f_usart3_isr(void);
void stm32f_usart3_exti28_isr(void);
void stm32f_exti15_10_isr(void);
void stm32f_rtc_alarm_isr(void);
void stm32f_otg_fs_wkup_isr(void);
void stm32f_usb_wkup_isr(void);
void stm32f_tim12_isr(void);
void stm32f_tim13_isr(void);
void stm32f_tim14_isr(void);
void stm32f_tim8_brk_tim12_isr(void);
void stm32f_tim8_up_tim13_isr(void);
void stm32f_tim8_trg_com_tim14_isr(void);
void stm32f_tim8_cc_isr(void);
void stm32f_dma1_stream7_isr(void);
void stm32f_fsmc_isr(void);
void stm32f_sdio_isr(void);
void stm32f_tim5_isr(void);
void stm32f_spi3_isr(void);
void stm32f_uart4_isr(void);
void stm32f_uart5_isr(void);
void stm32f_tim6_dac_isr(void);
void stm32f_tim7_isr(void);
void stm32f_dma2_stream0_isr(void);
void stm32f_dma2_stream1_isr(void);
void stm32f_dma2_stream2_isr(void);
void stm32f_dma2_stream3_isr(void);
void stm32f_dma2_stream4_isr(void);
void stm32f_eth_isr(void);
void stm32f_eth_wkup_isr(void);
void stm32f_can2_tx_isr(void);
void stm32f_can2_rx0_isr(void);
void stm32f_can2_rx1_isr(void);
void stm32f_can2_sce_isr(void);
void stm32f_otg_fs_isr(void);
void stm32f_dma2_stream5_isr(void);
void stm32f_dma2_stream6_isr(void);
void stm32f_dma2_stream7_isr(void);
void stm32f_usart6_isr(void);
void stm32f_i2c3_ev_isr(void);
void stm32f_i2c3_er_isr(void);
void stm32f_otg_hs_ep1_out_isr(void);
void stm32f_otg_hs_ep1_in_isr(void);
void stm32f_otg_hs_wkup_isr(void);
void stm32f_otg_hs_isr(void);
void stm32f_dcmi_isr(void);
void stm32f_cryp_isr(void);
void stm32f_hash_rng_isr(void);
void stm32f_fpu_isr(void);
void stm32f_uart7_isr(void);
void stm32f_uart8_isr(void);
void stm32f_spi4_isr(void);
void stm32f_spi5_isr(void);
void stm32f_spi6_isr(void);
void stm32f_sai1_isr(void);
void stm32f_lcd_isr(void);
void stm32f_lcd_err_isr(void);
void stm32f_dma2d_isr(void);
void stm32f_sai2_isr(void);
void stm32f_qspi_isr(void);
void stm32f_cec_isr(void);
void stm32f_spdif_rx_isr(void);
void stm32f_fmpi2c1_isr(void);
void stm32f_fmpi2c1_err_isr(void);

const __isr_t __attribute__ ((section(".nvic_vectors"))) __nvic_vectors[] = {
	stm32f_wwdg_isr,         /*  0 Window WatchDog */
	stm32f_pvd_isr,          /*  1 PVD through EXTI Line detection */
	stm32f_tamp_stamp_isr,   /*  2 Tamper and TimeStamps through 
									        the EXTI line */
	stm32f_rtc_wkup_isr,     /*  3 RTC Wakeup through the EXTI line */
	stm32f_flash_isr,        /*  4 FLASH */
	stm32f_rcc_isr,          /*  5 RCC */
	stm32f_exti0_isr,        /*  6 EXTI Line0 */
	stm32f_exti1_isr,        /*  7 EXTI Line1 */

	stm32f_exti2_isr,        /*  8 EXTI Line2 */
	stm32f_exti3_isr,        /*  9 EXTI Line3 */
	stm32f_exti4_isr,        /* 10 EXTI Line4 */
	stm32f_dma1_stream0_isr, /* 11 DMA1 Stream 0 */
	stm32f_dma1_stream1_isr, /* 12 DMA1 Stream 1 */
	stm32f_dma1_stream2_isr, /* 13 DMA1 Stream 2 */
	stm32f_dma1_stream3_isr, /* 14 DMA1 Stream 3 */
	stm32f_dma1_stream4_isr, /* 15 DMA1 Stream 4 */

	stm32f_dma1_stream5_isr, /* 16 DMA1 Stream 5 */
	stm32f_dma1_stream6_isr, /* 17 DMA1 Stream 6 */
	stm32f_adc_isr,          /* 18 ADC1, ADC2 and ADC3s */
	stm32f_can1_tx_isr,         /* 19 CAN1 TX */
	stm32f_can1_rx0_isr,        /* 20 CAN1 RX0 */
	stm32f_can1_rx1_isr,        /* 21 CAN1 RX1 */
	stm32f_can1_sce_isr,        /* 22 CAN1 SCE */
	stm32f_exti9_5_isr,         /* 23 External Lines [9:5] */

	stm32f_tim1_brk_tim9_isr,      /* 24 TIM1 Break and TIM9 */
	stm32f_tim1_up_tim10_isr,      /* 25 TIM1 Update and TIM10 */
	stm32f_tim1_trg_com_tim11_isr, /* 26 TIM1 Trigger and Commutation 
	                                            and TIM11 */
	stm32f_tim1_cc_isr,        /* 27 TIM1 Capture Compare */
	stm32f_tim2_isr,           /* 28 TIM2 */
	stm32f_tim3_isr,           /* 29 TIM3 */
	stm32f_tim4_isr,           /* 30 TIM4 */
	stm32f_i2c1_ev_isr,        /* 31 I2C1 Event */

	stm32f_i2c1_er_isr,        /* 32 I2C1 Error */
	stm32f_i2c2_ev_isr,        /* 33 I2C2 Event */
	stm32f_i2c2_er_isr,        /* 34 I2C2 Error */
	stm32f_spi1_isr,           /* 35 SPI1 */
	stm32f_spi2_isr,           /* 36 SPI2 */
	stm32f_usart1_isr,         /* 37 USART1 */
	stm32f_usart2_isr,         /* 38 USART2 */
	stm32f_usart3_isr,         /* 39 USART3 */

	stm32f_exti15_10_isr,      /* 40 External Line[15:10]s */
	stm32f_rtc_alarm_isr,      /* 41 RTC Alarm (A and B) 
										 through EXTI Line */
	stm32f_otg_fs_wkup_isr,    /* 42 USB OTG FS Wakeup 
	                                     through EXTI line */
	stm32f_tim8_brk_tim12_isr,     /* 43 TIM8 Break and TIM12 */
	stm32f_tim8_up_tim13_isr,      /* 44 TIM8 Update and TIM13 */
	stm32f_tim8_trg_com_tim14_isr, /* 45 TIM8 Trigger and Commutation 
	                                            and TIM14 */
	stm32f_tim8_cc_isr,        /* 46 TIM8 Capture Compare */
	stm32f_dma1_stream7_isr,   /* 47 DMA1 Stream7 */

	stm32f_fsmc_isr,           /* 48 FSMC */
	stm32f_sdio_isr,           /* 49 SDIO */
	stm32f_tim5_isr,           /* 50 TIM5 */
	stm32f_spi3_isr,           /* 51 SPI3 */
	stm32f_uart4_isr,          /* 52 UART4 */
	stm32f_uart5_isr,          /* 53 UART5 */
	stm32f_tim6_dac_isr,       /* 54 TIM6 and DAC1&2 underrun errors */
	stm32f_tim7_isr,           /* 55 TIM7 */

	stm32f_dma2_stream0_isr,   /* 56 DMA2 Stream 0 */
	stm32f_dma2_stream1_isr,   /* 57 DMA2 Stream 1 */
	stm32f_dma2_stream2_isr,   /* 58 DMA2 Stream 2 */
	stm32f_dma2_stream3_isr,   /* 59 DMA2 Stream 3 */
	stm32f_dma2_stream4_isr,   /* 60 DMA2 Stream 4 */
	stm32f_eth_isr,            /* 61 Ethernet */
	stm32f_eth_wkup_isr,       /* 62 Ethernet Wakeup through EXTI line */
	stm32f_can2_tx_isr,        /* 63 CAN2 TX */

	stm32f_can2_rx0_isr,       /* 64 CAN2 RX0 */
	stm32f_can2_rx1_isr,       /* 65 CAN2 RX1 */
	stm32f_can2_sce_isr,       /* 66 CAN2 SCE */
	stm32f_otg_fs_isr,         /* 67 USB OTG FS */
	stm32f_dma2_stream5_isr,   /* 68 DMA2 Stream 5 */
	stm32f_dma2_stream6_isr,   /* 69 DMA2 Stream 6 */
	stm32f_dma2_stream7_isr,   /* 70 DMA2 Stream 7 */
	stm32f_usart6_isr,         /* 71 USART6 */

	stm32f_i2c3_ev_isr,        /* 72 I2C3 event */
	stm32f_i2c3_er_isr,        /* 73 I2C3 error */
	stm32f_otg_hs_ep1_out_isr, /* 74 USB OTG HS End Point 1 Out */
	stm32f_otg_hs_ep1_in_isr,  /* 75 USB OTG HS End Point 1 In */
	stm32f_otg_hs_wkup_isr,    /* 76 USB OTG HS Wakeup through EXTI */
	stm32f_otg_hs_isr,         /* 77 USB OTG HS */
	stm32f_dcmi_isr,           /* 78 DCMI */
	stm32f_cryp_isr,           /* 79 CRYP crypto */

	stm32f_hash_rng_isr,       /* 80 Hash and Rng */

	stm32f_fpu_isr,            /* 81 FPU */
	stm32f_uart7_isr,          /* 82 UART7 */
	stm32f_uart8_isr,          /* 83 UART8 */
	stm32f_spi4_isr,           /* 84 SPI4 */
	stm32f_spi5_isr,           /* 85 SPI5 */
	stm32f_spi6_isr,           /* 86 SPI6 */
	stm32f_sai1_isr,           /* 87 SAI1 */

	stm32f_lcd_isr,            /* 88 LCD-TFT */
	stm32f_lcd_err_isr,        /* 89 LCD-TFT error */
	stm32f_dma2d_isr,          /* 90 DMA2D */
	stm32f_sai2_isr,           /* 91 SAI2 */
	stm32f_qspi_isr,           /* 92 QuadSPI */
	stm32f_cec_isr,            /* 93 HDMI-CEC */
	stm32f_spdif_rx_isr,       /* 94 SPDIF-Rx */
	stm32f_fmpi2c1_isr,        /* 95 FMPI2C1 event */
	stm32f_fmpi2c1_err_isr    /* 96 FMPI2C1 error */
};

void stm32f_wwdg_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_pvd_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tamp_stamp_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_rtc_wkup_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_flash_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_rcc_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_exti0_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_exti1_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_exti2_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_exti2_tsc_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_exti3_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_exti4_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma1_stream0_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma1_stream1_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma1_stream2_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma1_stream3_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma1_stream4_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma1_stream5_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma1_stream6_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_adc_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_can1_tx_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_can1_rx0_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_can1_tx_usb_hp_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_can1_rx0_usb_lp_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_can1_rx1_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_can1_sce_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_exti9_5_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim1_brk_tim9_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim1_up_tim10_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim1_trg_com_tim11_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim1_brk_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim1_up_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim1_trg_com_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim1_brk_tim15_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim1_up_tim16_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim1_trg_com_tim17_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim1_cc_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim2_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim3_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim4_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_i2c1_ev_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_i2c1_ev_exti23_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_i2c1_er_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_i2c2_ev_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_i2c2_ev_exti24_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_i2c2_er_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_spi1_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_spi2_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_usart1_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_usart1_exti25_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_usart2_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_usart2_exti26_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_usart3_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_usart3_exti28_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_exti15_10_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_rtc_alarm_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_otg_fs_wkup_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_usb_wkup_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim12_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim13_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim14_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim8_brk_tim12_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim8_up_tim13_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim8_trg_com_tim14_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim8_cc_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma1_stream7_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_fsmc_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_sdio_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim5_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_spi3_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_uart4_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_uart5_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim6_dac_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_tim7_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma2_stream0_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma2_stream1_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma2_stream2_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma2_stream3_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma2_stream4_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_eth_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_eth_wkup_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_can2_tx_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_can2_rx0_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_can2_rx1_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_can2_sce_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_otg_fs_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma2_stream5_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma2_stream6_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma2_stream7_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_usart6_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_i2c3_ev_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_i2c3_er_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_otg_hs_ep1_out_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_otg_hs_ep1_in_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_otg_hs_wkup_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_otg_hs_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dcmi_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_cryp_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_hash_rng_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_fpu_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_uart7_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_uart8_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_spi4_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_spi5_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_spi6_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_sai1_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_lcd_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_lcd_err_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_dma2d_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_sai2_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_qspi_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_cec_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_spdif_rx_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_fmpi2c1_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

void stm32f_fmpi2c1_err_isr(void)
	__attribute__ ((weak, alias("__default_isr")));

