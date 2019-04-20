/* 
 * stm32f446xe.ld
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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


/* Devices:
   STM32L433CC 
   STM32L433RC 
   STM32L433VC 
 */

OUTPUT_FORMAT ("elf32-littlearm")
ENTRY(_reset)
SEARCH_DIR(.)

MEMORY
{
	vectors (r) : ORIGIN = 0x00000000, LENGTH = 1K
	flash (rx) : ORIGIN = 0x08000000, LENGTH = 256K
	sram1  (rw) : ORIGIN = 0x20000000, LENGTH = 48K
	sram2  (rw) : ORIGIN = 0x10000000, LENGTH = 16K
	rtcbkr (rw) : ORIGIN = 0x40002850, LENGTH = 80
	dcclog  (r) : ORIGIN = 0x4ff00000, LENGTH = 1024K
}

REGION_ALIAS("CODE", flash);
REGION_ALIAS("DATA", sram1);
REGION_ALIAS("STACK", sram2);

INCLUDE stm32l4xx.ld

