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
#define USB_FS_DP         STM32_GPIOA, 12
#define USB_FS_DM         STM32_GPIOA, 11
#define USB_FS_VBUS       STM32_GPIOA, 9

/* -------------------------------------------------------------------------
 * JTAG 
 * ------------------------------------------------------------------------- */
#define IO_JTMS           STM32_GPIOA, 13
#define IO_JTDI           STM32_GPIOA, 15
#define IO_JTDO           STM32_GPIOB, 3
#define IO_JTCK           STM32_GPIOA, 14

/**************************************************************************
 * Lattice FPGA (iCE40LP384) connections
 **************************************************************************/

#define IO_ICE40_SPI_SCK  STM32_GPIOA, 1
#define IO_ICE40_SPI_SDO  STM32_GPIOB, 4
#define IO_ICE40_SPI_SDI  STM32_GPIOB, 5
#define IO_ICE40_SPI_SS   STM32_GPIOB, 8
#define ICE40_SPI         STM32F_SPI1
#define ICE40_CLK_SPI     STM32_CLK_SPI1
#define ICE4_SPI_AF       GPIO_AF5

#define IO_ICE40_CRESET   STM32_GPIOB, 9
#define IO_ICE40_CDONE    STM32_GPIOB, 7

/* -------------------------------------------------------------------------
 * FPGA
 * ------------------------------------------------------------------------- */

#define IO_MCO            STM32_GPIOA, 8
#define IO_MODE           STM32_GPIOA, 10

/* -------------------------------------------------------------------------
 * DAC 
 * ------------------------------------------------------------------------- */

#define IO_DAC1           STM32_GPIOA, 5

#define IO_SW0            STM32_GPIOA, 0
#define IO_SW1            STM32_GPIOC, 13
#define IO_SW2            STM32_GPIOC, 14
#define IO_SW3            STM32_GPIOC, 15

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

