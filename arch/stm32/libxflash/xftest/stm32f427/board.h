/* 
 * File:	 led.h
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

#include "config.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <thinkos.h>
#define __THINKOS_SYS__
#include <thinkos_sys.h>
#include <sys/stm32f.h>
#include <arch/cortex-m3.h>


#ifdef __cplusplus
extern "C" {
#endif

void io_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BOARD_H__ */


