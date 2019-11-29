/* 
 * stm32f429xi.ld
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

OUTPUT_FORMAT ("elf32-littlearm")
ENTRY(_reset)
SEARCH_DIR(.)

/* Devices:
   STM32F429VI 
   STM32F429ZI 
   STM32F429II 
   STM32F429BI 
   STM32F429NI 
   STM32F429AI 
 */

MEMORY
{
	vectors  (r) : ORIGIN = 0x00000000, LENGTH = 1K
	flash   (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
	sram1   (rw) : ORIGIN = 0x20000000, LENGTH = 112K
	sram2   (rw) : ORIGIN = 0x2001c000, LENGTH = 16K
	sram3   (rw) : ORIGIN = 0x20020000, LENGTH = 64K
	ccm     (rw) : ORIGIN = 0x10000000, LENGTH = 64K
	fmc1    (rw) : ORIGIN = 0x60000000, LENGTH = 256M
	fmc2    (rw) : ORIGIN = 0x70000000, LENGTH = 256M
	fmc3    (rw) : ORIGIN = 0x80000000, LENGTH = 256M
	fmc4    (rw) : ORIGIN = 0x90000000, LENGTH = 256M
	fmc5    (rw) : ORIGIN = 0xC0000000, LENGTH = 256M
	fmc6    (rw) : ORIGIN = 0xD0000000, LENGTH = 256M
	rtcbkr  (rw) : ORIGIN = 0x40002850, LENGTH = 80
	bkpsram (rw) : ORIGIN = 0x40024000, LENGTH = 4K
	dcclog   (r) : ORIGIN = 0x4f800000, LENGTH = 4096K
	alloc    (r) : ORIGIN = 0x4fc00000, LENGTH = 4096K
}

REGION_ALIAS("CODE", flash);
REGION_ALIAS("DATA", sram1);
REGION_ALIAS("HEAP", sram1);
REGION_ALIAS("STACK", ccm);
REGION_ALIAS("LOG", dcclog);

/* Device family vectors */
INCLUDE stm32f4xx.ld

