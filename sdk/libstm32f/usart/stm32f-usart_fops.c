/* 
 * File:	 stm32f-usart_fops.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2013 Bob Mittmann. All Rights Reserved.
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

#include "usart-priv.h"

const struct fileop stm32_usart_fops = {
	.write = (int (*)(void *, const void *, size_t))stm32_usart_canon_write,
	.read = (int (*)(void *, void *, size_t, unsigned int))stm32_usart_read,
	.flush = (int (*)(void *))stm32_usart_flush,
	.close = (int (*)(void *))stm32_usart_power_off
};

