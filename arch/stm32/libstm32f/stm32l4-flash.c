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

#include <sys/stm32f.h>
#include <arch/cortex-m3.h>
#include <sys/flash-dev.h>
#include <sys/dcclog.h>

#if defined(STM32L4X)

#define FLASH_ERR (FLASH_OPTVERR | FLASH_RDERR | FLASH_FASTERR | \
				   FLASH_MISERR | FLASH_PGSERR | FLASH_PGSIZERR | \
				   FLASH_PGAERR | FLASH_WRPERR | FLASH_PROGERR | FLASH_OPERR)

#if 0
int stm32l4xx_flash_bsy_wait(uint32_t sr)
{
	if (sr & FERR) {
		/* clear errors */
		WARN("FLASH error:%s%s%s%s%s%s%s%s%s%s", 
					 sr & FLASH_OPTVERR ? " OPTVERR" : "",
					 sr & FLASH_RDERR ? " RDERR" : "",
					 sr & FLASH_FASTERR ? " FASTERR" : "",
					 sr & FLASH_MISERR ? " MISERR" : "",
					 sr & FLASH_PGSERR ? " PGSERR" : "",
					 sr & FLASH_PGSIZERR ? " PGSIZERR" : "",
					 sr & FLASH_PGAERR ? " PGAERR" : "",
					 sr & FLASH_WRPERR ? " WRPER" : "",
					 sr & FLASH_PROGERR ? " PROGERR" : "",
					 sr & FLASH_OPERR ? " OPERR" : "");
		return sr & FERR;
	}

	return -1;
}
#endif


int stm32l4x_flash_unlock(struct stm32_flash * flash, off_t offs, size_t len)
{
	uint32_t cr;

	cr = flash->cr;
	if (cr & FLASH_LOCK) {
		DCC_LOG(LOG_TRACE, "unlocking flash...");
		/* unlock flash write */
		flash->keyr = FLASH_KEY1;
		flash->keyr = FLASH_KEY2;
	}

	/* Clear errors */
	flash->sr = FLASH_ERR;

	return 0;
}

int stm32l4x_flash_lock(struct stm32_flash * flash, off_t offs, size_t len)
{
	return 0;
}

uint32_t __attribute__((section (".data#"), noinline)) 
	stm32l4x_flash_page_erase(struct stm32_flash * flash, uint32_t cr)
{
	uint32_t sr;

	flash->cr = cr;

	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	return sr;
}

int stm32l4x_flash_erase(struct stm32_flash * flash, off_t offs, size_t len)
{
	unsigned int size = 2048;
	unsigned int page;
	uint32_t pri;
	uint32_t cr;
	uint32_t sr;

	pri = cm3_primask_get();

	page = offs >> 11;
	if ((page << 11) != (offs)) {
		DCC_LOG(LOG_ERROR, "offset must be a aligned to a page boundary.");
		return -2;
	};

	DCC_LOG2(LOG_TRACE, "page=%d size=%d", page, size);

	/* Clear errors */
	flash->sr = FLASH_ERR;

	cr = FLASH_STRT | FLASH_PER | FLASH_PNB(page);
	cm3_primask_set(1);
	sr = stm32l4x_flash_page_erase(flash, cr);
	cm3_primask_set(pri);

	if (sr & FLASH_ERR) {
		DCC_LOG1(LOG_WARNING, "stm32f2x_flash_sect_erase() failed"
				 " sr=%08x!", sr);
		return -1;
	}

	return size;
}

uint32_t __attribute__((section (".data#"), noinline)) 
	stm32l4x_flash_wr64(struct stm32_flash * flash, 
						uint32_t volatile addr[], 
						uint32_t data0, uint32_t data1)
{
	uint32_t sr;

	flash->cr = FLASH_PG;
	addr[0] = data0;
	addr[1] = data1;
	
	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	return sr;
}

int stm32l4x_flash_write(struct stm32_flash * flash, 
						 off_t offs, const void * buf, size_t len)
{
	uint32_t data0;
	uint32_t data1;
	uint32_t * addr;
	uint8_t * ptr;
	uint32_t sr;
	uint32_t pri;

	ptr = (uint8_t *)buf;
	addr = (uint32_t *)((uint32_t)STM32_FLASH_MEM + offs);

	DCC_LOG2(LOG_INFO, "0x%08x len=%d", addr, len);

	pri = cm3_primask_get();
	data0 = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
	data1 = ptr[4] | (ptr[5] << 8) | (ptr[6] << 16) | (ptr[7] << 24);
	DCC_LOG3(LOG_TRACE, "0x%08x data=0x%04x 0x%04x", addr, data0, data1);
	cm3_primask_set(1);
	sr = stm32l4x_flash_wr64(flash, addr, data0, data1);
	cm3_primask_set(pri);
	if (sr & FLASH_ERR) {
		DCC_LOG(LOG_WARNING, "stm32f2x_flash_wr32() failed!");
		return -1;
	}
	
	return 8;
}

void __thinkos_memcpy(void * __dst, const void * __src,  unsigned int __n);

int stm32l4x_flash_read(struct stm32_flash * flash, 
						off_t offs, const void * buf, size_t len)
{
	uint8_t * src;
	uint8_t * dst;

	dst = (uint8_t *)buf;
	src = (uint8_t *)((uint32_t)STM32_FLASH_MEM + offs);
	__thinkos_memcpy(dst, src,  len);

	return len;
}


const struct flash_dev_ops stm32l4x_flash_dev_ops = {
	.write = (int (*)(void *, off_t, const void *, size_t))stm32l4x_flash_write,
	.read = (int (*)(void *, off_t, void *, size_t))stm32l4x_flash_read,
	.erase = (int (*)(void *, off_t, size_t count))stm32l4x_flash_erase,
	.lock = (int (*)(void *, off_t, size_t count))stm32l4x_flash_lock,
	.unlock = (int (*)(void *, off_t, size_t count))stm32l4x_flash_unlock
};

/* FLASH memory device object */
const struct flash_dev stm32l4x_flash_dev = {
	.priv = (void *)STM32_FLASH,
	.op = &stm32l4x_flash_dev_ops
};

#endif

