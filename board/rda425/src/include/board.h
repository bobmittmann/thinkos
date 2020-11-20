/* board.h - hardware configuration
 * -------
 *
 *   ************************************************************************
 *   **            Company Confidential - For Internal Use Only            **
 *   **          Mircom Technologies Ltd. & Affiliates ("Mircom")          **
 *   **                                                                    **
 *   **   This information is confidential and the exclusive property of   **
 *   ** Mircom.  It is intended for internal use and only for the purposes **
 *   **   provided,  and may not be disclosed to any third party without   **
 *   **                prior written permission from Mircom.               **
 *   **                                                                    **
 *   **                        Copyright 2017-2018                         **
 *   ************************************************************************
 *
 */

/** 
 * @file board.h
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 * @brief Hardware configuration options.
 * 
 * This file is common to the application as well as the bootloader.
 * It holds configuration options related to the microconroler and 
 * hardware in general.
 * 
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>

/* -------------------------------------------------------------------------
 * USB 
 * ------------------------------------------------------------------------- */

#define OTG_FS_DP   STM32_GPIOA, 12
#define OTG_FS_DM   STM32_GPIOA, 11
#define OTG_FS_VBUS STM32_GPIOA, 9

/* -------------------------------------------------------------------------
 * LEDs 
 * ------------------------------------------------------------------------- */
#define IO_LED1A           STM32_GPIOE, 9
#define IO_LED1B           STM32_GPIOE, 10
#define IO_LED1C           STM32_GPIOE, 11
#define IO_LED1D           STM32_GPIOE, 12

#define IO_LED2A           STM32_GPIOE, 13
#define IO_LED2B           STM32_GPIOE, 14
#define IO_LED2C           STM32_GPIOE, 15
#define IO_LED2D           STM32_GPIOD, 9

#define IO_LED3A           STM32_GPIOD, 10
#define IO_LED3B           STM32_GPIOD, 11
#define IO_LED3C           STM32_GPIOD, 12
#define IO_LED3D           STM32_GPIOD, 13

#define IO_LED4A           STM32_GPIOD, 14
#define IO_LED4B           STM32_GPIOD, 15
#define IO_LED4C           STM32_GPIOC, 8
#define IO_LED4D           STM32_GPIOC, 9

#define IO_LEDTB1          STM32_GPIOE, 2
#define IO_LEDTB2          STM32_GPIOE, 3

/* -------------------------------------------------------------------------
 * Serial Flash  connections
 * ------------------------------------------------------------------------- */

/* IO Pins */
#define IO_SFLASH_SCK     STM32_GPIOE, 12
#define IO_SFLASH_MISO    STM32_GPIOE, 13
#define IO_SFLASH_MOSI    STM32_GPIOE, 14
#define IO_SFLASH_CS      STM32_GPIOE, 11
#define AF_SPI_SFLASH     GPIO_AF5
/* SPI */
#define SPI_SFLASH        STM32F_SPI4
#define CLK_SPI_SFLASH    STM32_CLK_SPI4
#define IRQ_SPI_SFLASH    STM32F_IRQ_SPI4
/* DMA */
#define DMA_SPI_SFLASH              STM32F_DMA2
#define DMA_SPI_RX_CHAN_SFLASH      5
#define DMA_SPI_RX_STREAM_SFLASH    3
#define DMA_SPI_TX_CHAN_SFLASH      5
#define DMA_SPI_TX_STREAM_SFLASH    4

#if 0
#define DMA_SPI_RX_CHAN_SFLASH      4
#define DMA_SPI_RX_STREAM_SFLASH    0
#define DMA_SPI_TX_CHAN_SFLASH      4
#define DMA_SPI_TX_STREAM_SFLASH    2
#endif

/* -------------------------------------------------------------------------
 * ADCs
 * ------------------------------------------------------------------------- */

/* General analog inputs */
#define IO_LSNS           STM32_GPIOC, 0	/* Link sense */
#define IO_BOARD_ID       STM32_GPIOC, 1	/* Board Id */
#define IO_PWVS           STM32_GPIOA, 7	/* Power booster voltage sense */

#define ADC_LSNS          STM32F_ADC1
#define CLK_ADC_LSNS      STM32_CLK_ADC1
#define ADC_CHAN_LSNS     10

#define ADC_ID            STM32F_ADC1
#define CLK_ADC_ID        STM32_CLK_ADC1
#define ADC_CHAN_BOARD_ID 11

#define ADC_PWVS          STM32F_ADC1
#define CLK_ADC_PWVS      STM32_CLK_ADC1
#define ADC_CHAN_PWVS     7

/* current meter - Line supervision (per amplifier circuit) */
#define IO_IMET1          STM32_GPIOA, 0
#define IO_IMET2          STM32_GPIOA, 1
#define IO_IMET3          STM32_GPIOA, 2
#define IO_IMET4          STM32_GPIOA, 3

#define ADC_CHAN_IMET1    0
#define ADC_CHAN_IMET2    1
#define ADC_CHAN_IMET3    2
#define ADC_CHAN_IMET4    3

#define ADC_IMET          STM32F_ADC3

/* voltage meter - Audio supervision (per amplifier circuit) */
#define IO_VMET1          STM32_GPIOB, 0
#define IO_VMET2          STM32_GPIOB, 1
#define IO_VMET3          STM32_GPIOC, 4
#define IO_VMET4          STM32_GPIOC, 5

#define ADC_CHAN_VMET1    8
#define ADC_CHAN_VMET2    9
#define ADC_CHAN_VMET3    14
#define ADC_CHAN_VMET4    15

#define ADC_VMET          STM32F_ADC2

#define ADC_CHAN_VREFINT  17
#define ADC_CHAN_VBAT     18
#define ADC_CHAN_TEMP     18

/* -------------------------------------------------------------------------
 * Relays
 * ------------------------------------------------------------------------- */

#define IO_LINA1          STM32_GPIOA, 5
#define IO_LINA2          STM32_GPIOA, 6
#define IO_LINA3          STM32_GPIOC, 13
#define IO_LINA4          STM32_GPIOA, 8

#define IO_LINB1          STM32_GPIOC, 2
#define IO_LINB2          STM32_GPIOC, 3
#define IO_LINB3          STM32_GPIOC, 14
#define IO_LINB4          STM32_GPIOC, 15

/* -------------------------------------------------------------------------
 * CODEC 
 * ------------------------------------------------------------------------- */
#define IO_CODEC_DEM0     STM32_GPIOD, 10
#define IO_CODEC_DEM1     STM32_GPIOD, 11
#define IO_CODEC_RST      STM32_GPIOD, 12

#define IO_I2S2_SD        STM32_GPIOB, 15
#define IO_I2S2_SCLK      STM32_GPIOB, 13
#define IO_I2S2_FS        STM32_GPIOB, 12
#define IO_I2S2_MCLK      STM32_GPIOC, 6

#define IO_I2S3_SD        STM32_GPIOC, 12
#define IO_I2S3_SCLK      STM32_GPIOC, 10
#define IO_I2S3_FS        STM32_GPIOA, 4
#define IO_I2S3_MCLK      STM32_GPIOC, 7

/* -------------------------------------------------------------------------
 * Amplifier 
 * ------------------------------------------------------------------------- */
/*#define IO_AMP_POL        STM32_GPIOD, 13
#define IO_AMP_LSPV       STM32_GPIOA, 8
#define IO_AMP_ASPV       STM32_GPIOA, 6
*/

#define IO_PWR1           STM32_GPIOB, 7
#define IO_PWR2           STM32_GPIOB, 8
#define IO_PWR3           STM32_GPIOB, 9
#define IO_PWR4           STM32_GPIOB, 10

#define IO_TRIG           STM32_GPIOB, 14

#define IO_FAULT1         STM32_GPIOB, 2
//#define IO_FAULT3         STM32_GPIOB, 4
#define IO_FAULT2         STM32_GPIOD, 7
#define IO_FAULT3         STM32_GPIOB, 5
#define IO_FAULT4         STM32_GPIOB, 6

#define IO_JTMS           STM32_GPIOA, 13
#define IO_JTDI           STM32_GPIOA, 15
#define IO_JTDO           STM32_GPIOB, 3
#define IO_JTCK           STM32_GPIOA, 14
#define IO_NJTRST         STM32_GPIOB, 4

/* -------------------------------------------------------------------------
 * SAI (TDM)
 * ------------------------------------------------------------------------- */
#define IO_SAI_SD         STM32_GPIOE, 6
#define IO_SAI_SCK        STM32_GPIOE, 5
#define IO_SAI_FS         STM32_GPIOE, 4

/* -------------------------------------------------------------------------
 * COMM (Remote Operation)
 * ------------------------------------------------------------------------- */
#if 0
#define IO_USART5_TXRX    STM32_GPIOE, 8
#define IO_USART5_TXEN    STM32_GPIOE, 7
#define IO_COMM_EN        STM32_GPIOD, 8
#endif

#define IO_COMM_TX        STM32_GPIOD, 5
#define IO_COMM_RX        STM32_GPIOD, 6

#define IO_COMM_NTXEN     STM32_GPIOD, 3
#define IRQ_COMM_NTXEN    STM32F_IRQ_EXTI3
#define EXTI_COMM_NTXEN   (1 << 3)

#define IO_COMM_NSEL      STM32_GPIOD, 8
#define IRQ_COMM_NSEL     STM32F_IRQ_EXTI9_5
#define EXTI_COMM_NSEL    (1 << 8)

/* -------------------------------------------------------------------------
 * Power Booster
 * ------------------------------------------------------------------------- */
#define IO_PWON           STM32_GPIOA, 10

/* -------------------------------------------------------------------------
 * Calibration
 * ------------------------------------------------------------------------- */
#define IO_CAL            STM32_GPIOD, 4

 /* ------------------------------------------------------------------------
  * DEBUG
  * ------------------------------------------------------------------------ */

#define IO_CTL            STM32_GPIOD, 5
#define IO_CTDIR          STM32_GPIOD, 6

#define IO_TP41     STM32_GPIOC, 11
#define IO_TP46     STM32_GPIOD, 0
#define IO_TP42     STM32_GPIOD, 1
#define IO_TP47     STM32_GPIOD, 2
#define IO_TP51     STM32_GPIOE, 0
#define IO_TP48     STM32_GPIOE, 1
#define IO_TP50     STM32_GPIOE, 7
#define IO_TP44     STM32_GPIOE, 8

#define IO_DBG_1    IO_TP41
#define IO_DBG_2    IO_TP42

enum amp_jmp_cobminations {
	AMP_4X25,
	AMP_1X50_2X25,
	AMP_1X25_1X50_1X25,
	AMP_1X75_1X25,
	AMP_2X25_1X50,
	AMP_2X50,
	AMP_1X25_1X75,
	AMP_1X100,
	AMP_COMB_MAX
};

/* high level led on/off functions */
static inline void __led_on(struct stm32_gpio *__gpio, int __pin)
{
	stm32_gpio_set(__gpio, __pin);
}

static inline void __led_off(struct stm32_gpio *__gpio, int __pin)
{
	stm32_gpio_clr(__gpio, __pin);
}

/* high level test point functions */
static inline void __tp_hi(struct stm32_gpio *__gpio, int __pin)
{
	stm32_gpio_set(__gpio, __pin);
}

/* high level test point functions */
static inline void __tp_toggle(struct stm32_gpio *__gpio, int __pin)
{
	if (stm32_gpio_stat(__gpio, __pin))
		stm32_gpio_clr(__gpio, __pin);
	else
		stm32_gpio_set(__gpio, __pin);
}

static inline void __tp_lo(struct stm32_gpio *__gpio, int __pin)
{
	stm32_gpio_clr(__gpio, __pin);
}

/* Microsecond resolution free running 32bits hardware timer. 
   XXX: This is the same timer used in the trace library and it's initialized 
   there... */
#define HWTIMER_FREQ_HZ 1000000000

static inline uint32_t __hwtimer_cnt(void)
{
	struct stm32f_tim *tim = STM32F_TIM2;
	return tim->cnt;
}

/* -------------------------------------------------------------------------
 * ThinkOS application address
 * ------------------------------------------------------------------------- */

#define FLASH_BLK_THINKOS_OFFS      0x00000000
#define FLASH_BLK_THINKOS_SIZE      (48 * 1024)

#define FLASH_BLK_CONF_OFFS         0x00010000
#define FLASH_BLK_CONF_SIZE         (64 * 1024)

#define FLASH_BLK_FIRMWARE_OFFS     0x00020000
#define FLASH_BLK_FIRMWARE_SIZE     (384 * 1024)

#ifdef __cplusplus
extern "C" {
#endif

void board_gpio_init(void);

int board_jmp_init(void);

uint32_t board_jmp_supervise(uint32_t clk);

int32_t link_sense(void);

int board_amp_groups(uint8_t sets[], int code);

/* Return the representative ckt of a configuration with at least one jumper */
int board_group_rep(uint8_t grbmp);

/* Return the number of disjoint amplifier sets */
uint8_t board_group_size(int code);

/* Return the number of amplifier ckts that are part of this group bitmap */
unsigned int board_count_elements(uint8_t grbmp);

int32_t board_temperature(void);

int32_t board_vcc(void);

int board_init(void);

#ifdef __cplusplus
}
#endif
#endif				/* __BOARD_H__ */
