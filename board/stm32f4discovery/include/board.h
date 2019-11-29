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

/* ---------------------------------------------------------------------------
 * STM32F4Discovery Board
 *
 * STM32F407VGT6 microcontroller: FPU core, * 1-Mbyte Flash memory, 
 * 192-Kbyte RAM, LQFP100 package
 *
 * LIS302DL or LIS3DSH ST MEMS 3-axis accelerometer
 * 
 * MP45DT02 ST-MEMS audio sensor omni-directional digital microphone
 *
 * CS43L22 audio DAC with integrated class D speaker driver
 *
 * ---------------------------------------------------------------------------
 */

/* ---------------------------------------------------------------------------
 * LEDs
 * ---------------------------------------------------------------------------
 */
#define IO_LED3	            STM32_GPIOD, 13 /* LED3 */
#define IO_LED4             STM32_GPIOD, 12 /* LED4 */
#define IO_LED5	            STM32_GPIOD, 14 /* LED5 */
#define IO_LED6	            STM32_GPIOD, 15 /* LED6 */

/* low level led on/off functions */
static inline void __led_on(struct stm32_gpio *__gpio, int __pin) {
	stm32_gpio_set(__gpio, __pin);
}

static inline void __led_off(struct stm32_gpio *__gpio, int __pin) {
	stm32_gpio_clr(__gpio, __pin);
}

static inline void __led_toggle(struct stm32_gpio *__gpio, int __pin) {
	if (stm32_gpio_stat(__gpio, __pin))
		stm32_gpio_clr(__gpio, __pin);
	else
		stm32_gpio_set(__gpio, __pin);
}

/* ---------------------------------------------------------------------------
 * USB
 * ---------------------------------------------------------------------------
 */

#define IO_OTG_FS_DP        STM32_GPIOA, 12
#define IO_OTG_FS_DM        STM32_GPIOA, 11
#define IO_OTG_FS_ID        STM32_GPIOA, 10
#define IO_OTG_FS_VBUS      STM32_GPIOA, 9
#define IO_OTG_PWR_SW       STM32_GPIOC, 0 /* Power switch */
#define IO_OTG_PWR_OVR      STM32_GPIOD, 5 /* Overcurrent */

/* ---------------------------------------------------------------------------
 * Push button B1
 * ---------------------------------------------------------------------------
 */
#define IO_PUSH_BTN         STM32_GPIOA, 0  

/* ---------------------------------------------------------------------------
 * MEMS
 * ---------------------------------------------------------------------------
 */
#define IO_LIS302_MISO      STM32_GPIOA, 6
#define IO_LIS302_MOSI      STM32_GPIOA, 7
#define IO_LIS302_SCK       STM32_GPIOA, 5
#define IO_LIS302_CS        STM32_GPIOE, 3
#define IO_LIS302_INT2      STM32_GPIOE, 1
#define IO_LIS302_INT1      STM32_GPIOE, 0

/* ---------------------------------------------------------------------------
 * Microphone MP45DT02
 * ---------------------------------------------------------------------------
 */
#define IO_MP45DT02_I2C_DAT STM32_GPIOC, 3
#define IO_MP45DT02_I2C_CLK STM32_GPIOB, 10

/* ---------------------------------------------------------------------------
 * Audio CODEC CS43L22
 * ---------------------------------------------------------------------------
 */
#define IO_CS43L22_SDA      STM32_GPIOB, 9
#define IO_CS43L22_SLC      STM32_GPIOB, 6

#define IO_CS43L22_I2S_MCK  STM32_GPIOC, 7
#define IO_CS43L22_I2S_SCK  STM32_GPIOC, 10
#define IO_CS43L22_I2S_SD   STM32_GPIOC, 12
#define IO_CS43L22_I2S_WD   STM32_GPIOA, 4

#define IO_CS43L22_DAC      STM32_GPIOA, 4
#define IO_CS43L22_PDM      STM32_GPIOC, 3
#define IO_CS43L22_PWM      STM32_GPIOC, 4
#define IO_CS43L22_RST      STM32_GPIOD, 4

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */

