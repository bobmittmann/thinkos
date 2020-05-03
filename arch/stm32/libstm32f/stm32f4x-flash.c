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

	return 0;
}

uint32_t __attribute__((section (".data#"), noinline)) 
stm32f2x_flash_wr32(struct stm32_flash * flash, uint32_t cr,
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


int stm32f4x_flash_erase(struct stm32_flash * flash, 
						 unsigned int offs, unsigned int len)
{
	unsigned int cnt;
	uint32_t dummy;
	uint32_t pri;
	uint32_t cr;
	uint32_t sr;

	pri = cm3_primask_get();

	cnt = 0;
	while (cnt < len) {
		unsigned int page;
		unsigned int sect;
		unsigned int size;

		page = offs >> 14;
		if ((page << 14) != (offs)) {
			DCC_LOG(LOG_ERROR, "offset must be a aligned to a page boundary.");
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
			DCC_LOG(LOG_ERROR, "offset must be a aligned to a "
					"sector boundary.");
			return -3;
		}

		/* Clear errors */
		flash->sr = FLASH_ERR;

		cr = FLASH_STRT | FLASH_SER | FLASH_SNB(sect);
		cm3_primask_set(1);
		sr = stm32f2x_flash_wr32(flash, cr, &dummy, 0);
		cm3_primask_set(pri);

		if (sr & FLASH_ERR) {
			DCC_LOG1(LOG_WARNING, "stm32f2x_flash_sect_erase() failed"
					 " sr=%08x!", sr);
			return -1;
		}

		cnt += size;
		offs += size;
	}

	return cnt;
}

int stm32f4x_flash_write(struct stm32_flash * flash, 
						 off_t offs, const void * buf, size_t len)
{
	uint32_t data;
	uint32_t * addr;
	uint8_t * ptr;
	uint32_t cr;
	uint32_t sr;
	uint32_t pri;
	int n;
	int i;

	n = (len + 3) / 4;

	ptr = (uint8_t *)buf;
	addr = (uint32_t *)((uint32_t)STM32_FLASH_MEM + offs);

	DCC_LOG2(LOG_INFO, "0x%08x len=%d", addr, len);

	/* Clear errors */
	flash->sr = FLASH_ERR;

	pri = cm3_primask_get();
	for (i = 0; i < n; i++) {
		data = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
		DCC_LOG2(LOG_MSG, "0x%08x data=0x%04x", addr, data);
		cr = FLASH_PG | FLASH_PSIZE_32;
		cm3_primask_set(1);
		sr = stm32f2x_flash_wr32(flash, cr, addr, data);
		cm3_primask_set(pri);
		if (sr & FLASH_ERR) {
			DCC_LOG(LOG_WARNING, "stm32f2x_flash_wr32() failed!");
			return -1;
		}
		ptr += 4;
		addr++;
	}

	return n * 4;
}

void __thinkos_memcpy(void * __dst, const void * __src,  unsigned int __n);

int stm32f4x_flash_read(struct stm32_flash * flash, 
						off_t offs, const void * buf, size_t len)
{
	uint8_t * src;
	uint8_t * dst;

	dst = (uint8_t *)buf;
	src = (uint8_t *)((uint32_t)STM32_FLASH_MEM + offs);
	__thinkos_memcpy(dst, src,  len);

	return len;
}

const struct flash_dev_ops stm32f4x_flash_dev_ops = {
	.write = (int (*)(void *, off_t, const void *, size_t))stm32f4x_flash_write,
	.read = (int (*)(void *, off_t, void *, size_t))stm32f4x_flash_read,
	.erase = (int (*)(void *, off_t, size_t count))stm32f4x_flash_erase,
	.lock = (int (*)(void *, off_t, size_t count))stm32f4x_flash_lock,
	.unlock = (int (*)(void *, off_t, size_t count))stm32f4x_flash_unlock
};

/* FLASH memory device object */
const struct flash_dev stm32f4x_flash_dev = {
	.priv = (void *)STM32_FLASH,
	.op = &stm32f4x_flash_dev_ops
};

#if 0
/* FIXME: these should be obsoleted and all calls should be replaced by 
   the corresponding device operations methods ... 
 */

int stm32_flash_write(uint32_t offs, const void * buf, unsigned int len)
{
	struct stm32_flash * flash = STM32_FLASH;

	if (offs & 0x00000003) {
		DCC_LOG(LOG_ERROR, "offset must be 32bits aligned!");
		return -1;
	}

	stm32f4x_flash_unlock(flash, offs, len);

	return stm32f4x_flash_write(flash, offs, buf, len);
}

int stm32_flash_erase(unsigned int offs, unsigned int len)
{
	struct stm32_flash * flash = STM32_FLASH;

	if (offs & 0x00000003) {
		DCC_LOG(LOG_ERROR, "offset must be 32bits aligned!");
		return -1;
	}

	stm32f4x_flash_unlock(flash, offs, len);

	return stm32f4x_flash_erase(flash, offs, len);
}

void stm32_flash_unlock(void)
{
	struct stm32_flash * flash = STM32_FLASH;

	stm32f4x_flash_unlock(flash, 0, 0x80000000);
}
#endif

#endif

