/* 
 * File:	 board.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(c) 2003-2006 BORESTE (www.boreste.com). All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __BOARD_H__
#define __BOARD_H__

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

#define IO_LED2A           STM32_GPIOE, 13
#define IO_LED2B           STM32_GPIOE, 14
#define IO_LED2C           STM32_GPIOE, 15
#define IO_LED2D           STM32_GPIOD, 9

#define IO_LED1A           STM32_GPIOE, 9
#define IO_LED1B           STM32_GPIOE, 10
#define IO_LED1C           STM32_GPIOE, 11
#define IO_LED1D           STM32_GPIOE, 12

#define IO_LED4A           STM32_GPIOD, 14
#define IO_LED4B           STM32_GPIOD, 15
#define IO_LED4C           STM32_GPIOC, 8
#define IO_LED4D           STM32_GPIOC, 9

#define IO_LED3A           STM32_GPIOD, 10
#define IO_LED3B           STM32_GPIOD, 11
#define IO_LED3C           STM32_GPIOD, 12
#define IO_LED3D           STM32_GPIOD, 13

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
 * DACs
 * ------------------------------------------------------------------------- */
#define IO_DAC1           STM32_GPIOA, 4
#define IO_DAC2           STM32_GPIOA, 5

/* -------------------------------------------------------------------------
 * ADCs
 * ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
 * Relays
 * ------------------------------------------------------------------------- */

#define IO_BKP1           STM32_GPIOA, 5
#define IO_BKP2           STM32_GPIOA, 6
#define IO_BKP3           STM32_GPIOA, 7
#define IO_BKP4           STM32_GPIOA, 8

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

#define IO_CODEC_I2S_DIN  STM32_GPIOC, 11
#define IO_CODEC_I2S_DOUT STM32_GPIOC, 12
#define IO_CODEC_I2S_SCLK STM32_GPIOC, 10
#define IO_CODEC_I2S_FS   STM32_GPIOA, 15
#define IO_CODEC_I2S_MCLK STM32_GPIOC, 7

/* -------------------------------------------------------------------------
 * Amplifier 
 * ------------------------------------------------------------------------- */
#define IO_AMP_POL        STM32_GPIOD, 13
#define IO_AMP_LSPV       STM32_GPIOA, 8
#define IO_AMP_ASPV       STM32_GPIOA, 6

#define IO_PWR1           STM32_GPIOB, 7
#define IO_PWR2           STM32_GPIOB, 8
#define IO_PWR3           STM32_GPIOB, 9
#define IO_PWR4           STM32_GPIOB, 10

#define IO_TRIG           STM32_GPIOB, 14

#define IO_FAULT1         STM32_GPIOB, 2
#define IO_FAULT2         STM32_GPIOB, 4
#define IO_FAULT3         STM32_GPIOB, 5
#define IO_FAULT4         STM32_GPIOB, 6


/* high level led on/off functions */
static inline void __led_on(struct stm32_gpio *__gpio, int __pin) {
	stm32_gpio_set(__gpio, __pin);
}

static inline void __led_off(struct stm32_gpio *__gpio, int __pin) {
	stm32_gpio_clr(__gpio, __pin);
}

/* -------------------------------------------------------------------------
 * ThinkOS application address
 * ------------------------------------------------------------------------- */
#define FLASH_BLK_THINKOS_OFFS      0x00000000
#define FLASH_BLK_THINKOS_SIZE      (32 * 1024)

#define THINKOS_APP_ADDR ((uintptr_t)STM32_MEM_FLASH + \
						  FLASH_BLK_THINKOS_OFFS)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */

