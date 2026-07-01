/* 
 * stm32l1xx.ld
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

REGION_ALIAS("CODE", flash);
REGION_ALIAS("DATA", sram);
REGION_ALIAS("HEAP", sram);
REGION_ALIAS("STACK", sram);
REGION_ALIAS("LOG", dcclog);
REGION_ALIAS("BKPCTL", sram);
REGION_ALIAS("BKPDAT", sram);

