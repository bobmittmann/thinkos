/* 
 * File:	 stm32_uid.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
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

#include <sys/stm32f.h>
#include <stdint.h>

#ifdef STM32_UID
#define STM32F_UID STM32_UID
#endif

#ifdef STM32F_UID

void stm32_uid(uint32_t esn[])
{
	uint32_t * uid = STM32F_UID;

	esn[0] = uid[0];
	esn[1] = uid[1];
}

void thinkos_arch_esn_get(uint32_t esn[]) 
	__attribute__ ((weak, alias ("stm32_uid")));

#endif
