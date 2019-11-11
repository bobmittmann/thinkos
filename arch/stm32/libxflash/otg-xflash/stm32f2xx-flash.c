/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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

/** 
 * @file stm32l-flash.c
 * @brief STM32L flash access API
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#define STM32F2X
#include <stm32f/stm32f-flash.h>
#include <arch/cortex-m3.h>
#include <sys/param.h>

#include <sys/dcclog.h>

#define STM32_BASE_FLASH 0x40023c00
#define STM32_FLASH ((struct stm32_flash *)STM32_BASE_FLASH)
#define STM32_FLASH_ADDR 0x08000000

#define FLASH_ERR (FLASH_PGSERR | FLASH_PGPERR | FLASH_PGAERR | FLASH_WRPERR | \
				   FLASH_OPERR)

void flash_unlock(void)
{
	struct stm32_flash * flash = STM32_FLASH;
	uint32_t cr;

	cr = flash->cr;
	if (cr & FLASH_LOCK) {
		/* unlock flash write */
		flash->keyr = FLASH_KEY1;
		flash->keyr = FLASH_KEY2;
	}

}


uint32_t stm32f2x_flash_sect_erase(struct stm32_flash * flash, uint32_t cr)
{
	uint32_t sr;

	flash->cr = cr;
	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	return sr;
}

int flash_erase(unsigned int offs, unsigned int len)
{
	struct stm32_flash * flash = STM32_FLASH;
	unsigned int cnt;
	uint32_t cr;
	uint32_t sr;

#if 0
	uint32_t pri;

	pri = cm3_primask_get();

	cr = flash->cr;
	if (cr & FLASH_LOCK) {
		DCC_LOG(LOG_TRACE, "unlocking flash...");
		/* unlock flash write */
		flash->keyr = FLASH_KEY1;
		flash->keyr = FLASH_KEY2;
	}
#endif

	cnt = 0;
	while (cnt < len) {
		unsigned int page;
		unsigned int sect;
		unsigned int size;

		page = offs >> 14;
		if ((page << 14) != (offs)) {
			DCC_LOG(LOG_WARNING, "offset must be a aligned to a page boundary.");
			return 0;
		};

		if (page < 4) {
			sect = page;
			size = 16384;
		} else if (page == 4) {
			sect = 4;
			size = 65536;
		} else if ((page % 8) == 0) {
			sect = ((page - 7) / 8) + 5;
			size = 131072;
		} else {
			DCC_LOG(LOG_WARNING, "offset must be a aligned to a "
					"sector boundary.");
			return 0;
		}

		/* clear error flags */
		flash->sr = FLASH_ERR;

		cr = FLASH_STRT | FLASH_SER | FLASH_PSIZE_32 | FLASH_SNB(sect);
//		cm3_primask_set(1);
		sr = stm32f2x_flash_sect_erase(flash, cr);
//		cm3_primask_set(pri);

		if (sr & FLASH_ERR) {
			DCC_LOG1(LOG_WARNING, "stm32f2x_flash_sect_erase() failed"
					 " sr=%08x!", sr);
			return -sr;
		}

		cnt += size;
		offs += size;
	}

	return cnt;
}


uint32_t stm32f2x_flash_wr32(struct stm32_flash * flash,
							 uint32_t volatile * addr, uint32_t data)
{
	uint32_t sr;

	flash->cr = FLASH_PG | FLASH_PSIZE_32;
	*addr = data;
	
	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	return sr;
}

int flash_write(uint32_t offs, const void * buf, unsigned int len)
{
	struct stm32_flash * flash = STM32_FLASH;
	uint32_t data;
	uint32_t * addr;
	uint8_t * ptr;
	uint32_t sr;
	int n;
	int i;
//	uint32_t cr;
//	uint32_t pri;

#if 0
	if (offs & 0x00000003) {
		DCC_LOG(LOG_ERROR, "offset must be 32bits aligned!");
		return -1;
	}
#endif
	n = (len + 3) / 4;

	ptr = (uint8_t *)buf;
	addr = (uint32_t *)((uint32_t)STM32_FLASH_ADDR + offs);
	DCC_LOG2(LOG_INFO, "0x%08x len=%d", addr, len);

#if 0
	cr = flash->cr;
	if (cr & FLASH_LOCK) {
		DCC_LOG(LOG_TRACE, "unlocking flash...");
		/* unlock flash write */
		flash->keyr = FLASH_KEY1;
		flash->keyr = FLASH_KEY2;
	}

	pri = cm3_primask_get();
#endif

	/* clear error flags */
	flash->sr = FLASH_ERR;

	for (i = 0; i < n; i++) {
		data = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
		DCC_LOG2(LOG_MSG, "0x%08x data=0x%04x", addr, data);
//		cm3_primask_set(1);
		sr = stm32f2x_flash_wr32(flash, addr, data);
//		cm3_primask_set(pri);
		if (sr & FLASH_ERR) {
			DCC_LOG(LOG_WARNING, "stm32f2x_flash_wr32() failed!");
			return -sr;
		}
		ptr += 4;
		addr++;
	}
	
	return n * 4;
}


