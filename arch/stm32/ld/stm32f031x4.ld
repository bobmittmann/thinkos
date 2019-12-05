/* 
 * stm32f031x4.ld
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
   STM32F031C4 
   STM32F031F4 
   STM32F031G4 
   STM32F031K4 
 */

OUTPUT_FORMAT ("elf32-littlearm")
ENTRY(_reset)
SEARCH_DIR(.)

MEMORY
{
	vectors  (r) : ORIGIN = 0x00000000, LENGTH = 1K
	flash   (rx) : ORIGIN = 0x08000000, LENGTH = 16K
	sram    (rw) : ORIGIN = 0x20000000, LENGTH = 4K
	dcclog   (r) : ORIGIN = 0x4f800000, LENGTH = 4096K
	alloc    (r) : ORIGIN = 0x4fc00000, LENGTH = 4096K
}

REGION_ALIAS("CODE", flash);
REGION_ALIAS("DATA", sram);
REGION_ALIAS("HEAP", sram);
REGION_ALIAS("STACK", sram);
REGION_ALIAS("LOG", dcclog);

/* Device family vectors */
INCLUDE stm32f0xx.ld

