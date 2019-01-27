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

#define OTG_FS_DP       STM32_GPIOA, 12
#define OTG_FS_DM       STM32_GPIOA, 11
#define OTG_FS_VBUS     STM32_GPIOA, 9

/* -------------------------------------------------------------------------
 * LEDs 
 * ------------------------------------------------------------------------- */
#define IO_LED1         STM32_GPIOC, 8
#define IO_LED2         STM32_GPIOC, 7
#define IO_LED3         STM32_GPIOC, 14
#define IO_LED4         STM32_GPIOC, 1

/* -------------------------------------------------------------------------
 * Switch
 * ------------------------------------------------------------------------- */

#define IO_SW1          STM32_GPIOC, 9

/* -------------------------------------------------------------------------
 * USART1 (Debug connector)
 * ------------------------------------------------------------------------- */

#define IO_USART1_TX    STM32_GPIOB, 6
#define IO_USART1_RX    STM32_GPIOB, 7

/* -------------------------------------------------------------------------
 * I2S2
 * ------------------------------------------------------------------------- */

#define IO_I2S2_WS      STM32_GPIOB, 12
#define IO_I2S2_CK      STM32_GPIOB, 13
#define IO_I2S2EXT_SD   STM32_GPIOB, 14
#define IO_I2S2_SD      STM32_GPIOB, 15


/* high level led on/off functions */
static inline void __led_on(struct stm32_gpio *__gpio, int __pin)
{
	stm32_gpio_set(__gpio, __pin);
}

static inline void __led_off(struct stm32_gpio *__gpio, int __pin)
{
	stm32_gpio_clr(__gpio, __pin);
}
#define FLASH_BLK_BOOT_OFFS     0x00000000
#define FLASH_BLK_BOOT_SIZE     (16 * 1024)

#define FLASH_BLK_FS_OFFS       0x00004000
#define FLASH_BLK_FS_SIZE       (48 * 1024)

#define FLASH_BLK_RBF_OFFS      0x00010000
#define FLASH_BLK_RBF_SIZE      (64 * 1024)

#define FLASH_BLK_APP_OFFS      0x00020000
#define FLASH_BLK_APP_SIZE      (384 * 1024)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */

