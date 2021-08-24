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
 * @file stm32f4x-flash.c
 * @brief STM32F4x flash operations 
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <sys/stm32f.h>
#include <arch/cortex-m3.h>
#include <sys/flash-dev.h>
#include <sys/dcclog.h>


#if defined(STM32F4X)

#define FLASH_ERR (FLASH_PGSERR | FLASH_PGPERR | FLASH_PGAERR | FLASH_WRPERR | \
				   FLASH_OPERR)

int stm32f4x_flash_unlock(struct stm32_flash * flash, off_t offs, size_t len)
{
	uint32_t cr;

	if (len == 0)
		return 0;

	cr = flash->cr;
	if (cr & FLASH_LOCK) {
		DCC_LOG(LOG_TRACE, "unlocking flash...");
		/* unlock flash write */
		flash->keyr = FLASH_KEY1;
		flash->keyr = FLASH_KEY2;
	}

	return 0;
}

int stm32f4x_flash_lock(struct stm32_flash * flash, off_t offs, size_t len)
{
	if (len == 0)
		return 0;

	return 0;
}

uint32_t __attribute__((section (".data#"), noinline)) 
stm32f4x_flash_wr32(struct stm32_flash * flash, uint32_t cr,
					uint32_t volatile * addr, uint32_t data)
{
	uint32_t sr;

	flash->cr = cr;
	*addr = data;

	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	return sr;
}

int stm32f4x_flash_write(struct stm32_flash * flash, 
						 off_t offs, const void * buf, size_t len)
{
	uint32_t data;
	uint32_t * dst;
	uint8_t * src;
	uint32_t cr;
	uint32_t sr;
	uint32_t pri;

	if (len == 0)
		return 0;

	src = (uint8_t *)buf;
	dst = (uint32_t *)(((uint32_t)STM32_FLASH_MEM + offs) & 0xfffffffc);

	data = src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);

	/* Clear errors */
	flash->sr = FLASH_ERR;
	cr = FLASH_PG | FLASH_PSIZE_32;

	pri = cm3_primask_get();
	cm3_primask_set(1);
	sr = stm32f4x_flash_wr32(flash, cr, dst, data);
	cm3_primask_set(pri);

	if (sr & FLASH_ERR) {
		DCC_LOG1(LOG_WARNING, "stm32f4x_flash_wr32() failed" " sr=%08x!", sr);
		return -1;
	}

	return (len >= 4 ? 4 : len);
}

int stm32f4x_flash_read(struct stm32_flash * flash, 
						off_t offs, void * buf, size_t len)
{
	uint32_t data;
	uint32_t * src;
	uint8_t * dst;

	if (len == 0)
		return 0;

	src = (uint32_t *)(((uint32_t)STM32_FLASH_MEM + offs) & 0xfffffffc);
	dst = (uint8_t *)buf;

	data = src[0];
	dst[0] = data;
	dst[1] = data >> 8;
	dst[2] = data >> 16;
	dst[3] = data >> 24;

	return (len >= 4 ? 4 : len);
}

struct stm32f4x_seq_state {
	uint32_t addr;
	uint32_t data;
	uint32_t cr;
};

uint32_t __attribute__((section (".data#"), noinline)) 
stm32f4x_flash_start(uint32_t cr, struct stm32_flash * flash)
{
	flash->cr = cr;

	return flash->sr & FLASH_BSY;
}

uint32_t __attribute__((section (".data#"), noinline)) 
stm32f4x_flash_status(uint32_t cr, struct stm32_flash * flash)
{
	return flash->sr & FLASH_BSY;
}

uint32_t __attribute__((section (".data#"), noinline)) 
stm32f4x_flash_stop(uint32_t cr, struct stm32_flash * flash)
{
	return flash->sr & FLASH_BSY;
}

uint32_t __attribute__((section (".data#"), noinline)) 
stm32f4x_flash_finish(uint32_t cr, struct stm32_flash * flash)
{
	return flash->sr & FLASH_BSY;
}

#if 0
intptr_t stm32f4x_flash_erase_init(struct stm32_flash * flash, 
						 unsigned int offs, unsigned int len)
{
	unsigned int page;
	unsigned int sect;
	unsigned int size;
	uint32_t cr;

	page = offs >> 14;
	if ((page << 14) != (offs)) {
		DCC_LOG(LOG_ERROR, "offset must be aligned to a page boundary.");
		return -2;
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
		DCC_LOG(LOG_ERROR, "offset must be aligned to a "
				"sector boundary.");
		return -3;
	}

	/* Clear errors */
	flash->sr = FLASH_ERR;

	DCC_LOG1(LOG_TRACE, "sector %d", sect);
	cr = FLASH_STRT | FLASH_SER | FLASH_SNB(sect);

	(void)size;

	return cr;
}

#else

int stm32f4x_flash_erase(struct stm32_flash * flash, 
						 unsigned int offs, unsigned int len)
{
	unsigned int page;
	unsigned int sect;
	unsigned int size;
	uint32_t dummy;
	uint32_t pri;
	uint32_t cr;
	uint32_t sr;

	if (len == 0)
		return 0;

	page = offs >> 14;
	if ((page << 14) != (offs)) {
		DCC_LOG(LOG_ERROR, "offset must be aligned to a page boundary.");
		return -2;
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
		DCC_LOG(LOG_ERROR, "offset must be aligned to a "
				"sector boundary.");
		return -3;
	}

	/* Clear errors */
	flash->sr = FLASH_ERR;

	DCC_LOG1(LOG_TRACE, "sector %d", sect);
	cr = FLASH_STRT | FLASH_SER | FLASH_SNB(sect);

	pri = cm3_primask_get();
	cm3_primask_set(1);
	sr = stm32f4x_flash_wr32(flash, cr, &dummy, 0);
	cm3_primask_set(pri);

	if (sr & FLASH_ERR) {
		DCC_LOG1(LOG_WARNING, "stm32f4x_flash_wr32() failed" " sr=%08x!", sr);
		return -1;
	}

	return size;
}
#endif

const struct flash_dev_seq stm32f4x_flash_dev_erase_seq  = {
	.start = (intptr_t (*)(intptr_t, void *))stm32f4x_flash_start,
	.status = (intptr_t (*)(intptr_t, void *))stm32f4x_flash_status,
	.stop = (intptr_t (*)(intptr_t, void *))stm32f4x_flash_stop,
	.finish = (intptr_t (*)(intptr_t, void *))stm32f4x_flash_finish
};

const struct flash_dev_ops stm32f4x_flash_dev_ops = {
	.write = (int (*)(void *, off_t, const void *, size_t))stm32f4x_flash_write,
	.read = (int (*)(void *, off_t, void *, size_t))stm32f4x_flash_read,
	.erase = (int (*)(void *, off_t, size_t ))stm32f4x_flash_erase,
	.lock = (int (*)(void *, off_t, size_t))stm32f4x_flash_lock,
	.unlock = (int (*)(void *, off_t, size_t))stm32f4x_flash_unlock,
#if 0
	.erase = {
		.init = (intptr_t (*)(void *, off_t, size_t)) stm32f4x_flash_erase_init,
		.seq = &stm32f4x_flash_dev_erase_seq
	}
#endif
};

/* FLASH memory device object */
const struct flash_dev stm32f4x_flash_dev = {
	.priv = (void *)STM32_FLASH,
	.op = &stm32f4x_flash_dev_ops
};

#endif

