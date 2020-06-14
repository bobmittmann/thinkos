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
#include <sys/delay.h>
#include <arch/cortex-m3.h>
#include <sys/flash-dev.h>
#include <sys/dcclog.h>
#include <sys/param.h>
#include <fixpt.h>

#define FLASH_ERR (FLASH_RDERR | FLASH_OPTVERRUSR | FLASH_OPTVERR | \
				   FLASH_SIZERR | FLASH_PGAERR | FLASH_WRPERR)


#if defined(STM32L1X)

int stm32l1x_flash_unlock(struct stm32_flash * flash, off_t offs, size_t len)
{
	uint32_t pecr;

	pecr = flash->pecr;
	DCC_LOG1(LOG_INFO, "PECR=0x%08x", pecr);
	if (pecr & FLASH_PRGLOCK) {
		DCC_LOG(LOG_INFO, "unlocking flash...");
		if (pecr & FLASH_PELOCK) {
			flash->pekeyr = FLASH_PEKEY1;
			flash->pekeyr = FLASH_PEKEY2;
		}
		flash->prgkeyr= FLASH_PRGKEYR1;
		flash->prgkeyr= FLASH_PRGKEYR2;
	}

	return 0;
}

int stm32l1x_flash_lock(struct stm32_flash * flash, off_t offs, size_t len)
{
	return 0;
}

uint32_t __attribute__((section (".data#"), noinline)) 
	stm32l1x_flash_blk_erase(struct stm32_flash * flash, 
							 uint32_t volatile * addr)
{
	uint32_t sr = flash->sr;

	flash->pecr = FLASH_ERASE | FLASH_PROG;
	*addr = 0x00000000;

	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	flash->pecr = 0;

	return sr;
}

#define FLASH_PAGE_SIZE 256


int stm32l1x_flash_erase(struct stm32_flash * flash, off_t offs, size_t len)
{
	unsigned int size = FLASH_PAGE_SIZE;
	unsigned int page;
	uint32_t pri;
	uint32_t sr;
	uint32_t addr;

	pri = cm3_primask_get();

	page = offs >> ILOG2(FLASH_PAGE_SIZE);
	if ((page << ILOG2(FLASH_PAGE_SIZE)) != (offs)) {
		DCC_LOG(LOG_ERROR, "offset must be a aligned to a page boundary.");
		return -2;
	};

	addr = (uint32_t)STM32_MEM_FLASH + offs;

	DCC_LOG2(LOG_TRACE, "page=%d size=%d", page, size);

	/* Clear errors */
	flash->sr = FLASH_ERR;

	cm3_primask_set(1);
	sr = stm32l1x_flash_blk_erase(flash, (uint32_t *)addr);
	cm3_primask_set(pri);

	if (sr & FLASH_ERR) {
		DCC_LOG1(LOG_WARNING, "stm32f2x_flash_sect_erase() failed"
				 " sr=%08x!", sr);
		return -1;
	}

	return size;
}

/* 
   write half flash page .
   offs must be half page aligned.
 */

uint32_t __attribute__((section (".data#"), noinline)) 
	stm32l1x_flash_pg_wr(struct stm32_flash * flash,
						   uint32_t * dst, uint32_t * src)
{
	uint32_t sr;
	uint32_t i;

	/* start half page write */
	flash->pecr = FLASH_FPRG | FLASH_PROG;

	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	if (sr & FLASH_ERR)
		return sr;

	for (i = 0; i < ((FLASH_PAGE_SIZE / 2) / 4); ++i) {
		*dst++ = *src++;
	}

	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	flash->pecr = 0;

	return sr;
}


int stm32l1x_flash_write(struct stm32_flash * flash, 
						 off_t offs, const void * buf, size_t len)
{
	uint32_t data[FLASH_PAGE_SIZE / 2 / 4];
	uint32_t * dst;
	uint8_t * src;
	uint32_t sr;
	uint32_t pri;
	unsigned i;

	src = (uint8_t *)buf;
	dst = (uint32_t *)((uint32_t)STM32_MEM_FLASH + offs);

	DCC_LOG2(LOG_INFO, "0x%08x len=%d", addr, len);

	for (i = 0; i < (FLASH_PAGE_SIZE / 2 / 4); ++i) {
		data[i] = src[0] | (src[1] << 8) | (src[2] << 16)| (src[3] << 24);
		src += 4;
	}

	pri = cm3_primask_get();
	DCC_LOG3(LOG_TRACE, "0x%08x data=0x%04x 0x%04x", addr, data0, data1);
	cm3_primask_set(1);
	sr = stm32l1x_flash_pg_wr(flash, dst, data);
	cm3_primask_set(pri);
	if (sr & FLASH_ERR) {
		DCC_LOG(LOG_WARNING, "stm32f2x_flash_wr32() failed!");
		return -1;
	}
	
	return 8;
}

int stm32l1x_flash_read(struct stm32_flash * flash, 
						off_t offs, const void * buf, size_t len)
{
	uint32_t data;
	uint32_t * src;
	uint8_t * dst;

	src = (uint32_t *)(((uint32_t)STM32_MEM_FLASH + offs) & 0xfffffffc);
	dst = (uint8_t *)buf;

	data = src[0];
	dst[0] = data;
	dst[1] = data >> 8;
	dst[2] = data >> 16;
	dst[3] = data >> 24;

	return 4;
}

const struct flash_dev_ops stm32l1x_flash_dev_ops = {
	.write = (int (*)(void *, off_t, const void *, size_t))stm32l1x_flash_write,
	.read = (int (*)(void *, off_t, void *, size_t))stm32l1x_flash_read,
	.erase = (int (*)(void *, off_t, size_t count))stm32l1x_flash_erase,
	.lock = (int (*)(void *, off_t, size_t count))stm32l1x_flash_lock,
	.unlock = (int (*)(void *, off_t, size_t count))stm32l1x_flash_unlock
};

/* FLASH memory device object */
const struct flash_dev stm32l1x_flash_dev = {
	.priv = (void *)STM32_FLASH,
	.op = &stm32l1x_flash_dev_ops
};

#endif

#if 0
uint32_t __attribute__((section (".data#"))) 
	stm32l_flash_blk_erase(struct stm32_flash * flash, uint32_t volatile * addr)
{
	uint32_t sr = flash->sr;

	flash->pecr = FLASH_ERASE | FLASH_PROG;
	*addr = 0x00000000;

	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	flash->pecr = 0;

	return sr;
}

#define FLASH_PAGE_SIZE 256

int stm32_flash_erase(unsigned int offs, unsigned int len)
{
	struct stm32_flash * flash = STM32_FLASH;
	uint32_t addr;
	uint32_t pecr;
	int rem = len;
	int cnt;

	offs &= ~(FLASH_PAGE_SIZE - 1);

	addr = (uint32_t)STM32_MEM_FLASH + offs;

	DCC_LOG2(LOG_INFO, "addr=0x%08x len=%d", addr, len);

	pecr = flash->pecr;
	DCC_LOG1(LOG_INFO, "PECR=0x%08x", pecr);
	if (pecr & FLASH_PRGLOCK) {
		DCC_LOG(LOG_INFO, "unlocking flash...");
		if (pecr & FLASH_PELOCK) {
			flash->pekeyr = FLASH_PEKEY1;
			flash->pekeyr = FLASH_PEKEY2;
		}
		flash->prgkeyr= FLASH_PRGKEYR1;
		flash->prgkeyr= FLASH_PRGKEYR2;
	}

	cnt = 0;
	rem = len;
	while (rem) {
		uint32_t pri;
		uint32_t sr;

		DCC_LOG1(LOG_INFO, "addr=0x%08x", addr);

		pri = cm3_primask_get();
		cm3_primask_set(1);
		sr = stm32l_flash_blk_erase(flash, (uint32_t *)addr);
		cm3_primask_set(pri);

		if (sr & FLASH_ERR) {
#if DEBUG
			DCC_LOG6(LOG_WARNING, "erase failed: %s%s%s%s%s%s", 
					 sr & FLASH_RDERR ? "RDERR" : "",
					 sr & FLASH_OPTVERRUSR ? "OPTVERRUSR" : "",
					 sr & FLASH_OPTVERR ? "OPTVERR " : "",
					 sr & FLASH_SIZERR ? "SIZERR " : "",
					 sr & FLASH_PGAERR ? "PGAERR" : "",
					 sr & FLASH_WRPERR ? "WRPERR" : "");
#endif
			cnt = -1;
			break;
		}
		addr += FLASH_PAGE_SIZE;
		rem -= FLASH_PAGE_SIZE;
		cnt += FLASH_PAGE_SIZE;

	}

	return cnt;
}

/* 
   write half flash page .
   offs must be half page aligned.
 */

uint32_t __attribute__((section (".data#"))) 
stm32l_flash_pg_wr(uint32_t offs, uint8_t * src,  uint32_t pos, uint32_t cnt)
{
	struct stm32_flash * flash = STM32_FLASH;
	uint32_t base = (uint32_t)STM32_MEM_FLASH;
	uint32_t * dst;
	uint32_t sr;
	uint32_t i;

	/* start half page write */
	flash->pecr = FLASH_FPRG | FLASH_PROG;

	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	if (sr & FLASH_ERR)
		return sr;

	dst = (uint32_t *)(base + offs);

	/* zero the initial words */
	for (i = 0; i < (pos / 4); ++i)
		*dst++ = 0;

	/* adjust counter to count words */
	for (; i < ((pos + cnt) / 4); ++i) {
		*dst++ = src[0] | (src[1] << 8) | (src[2] << 16)| (src[3] << 24);
		src += 4;
	}

	for (; i < ((FLASH_PAGE_SIZE / 2) / 4); ++i)
		*dst++ = 0;

	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	flash->pecr = 0;

	return sr;
}

int stm32_flash_write(uint32_t offs, const void * buf, unsigned int len)
{
	struct stm32_flash * flash = STM32_FLASH;
	uint8_t * ptr;
	uint32_t pecr;
	unsigned int rem;

	DCC_LOG2(LOG_INFO, "offs=0x%06x len=%d", offs, len);

	if (offs & 3) {
		DCC_LOG(LOG_ERROR, "offset must be word aligned!");
		return -1;
	}

	pecr = flash->pecr;
	DCC_LOG1(LOG_INFO, "PECR=0x%08x", pecr);
	if (pecr & FLASH_PRGLOCK) {
		DCC_LOG(LOG_TRACE, "unlocking flash...");
		if (pecr & FLASH_PELOCK) {
			flash->pekeyr = FLASH_PEKEY1;
			flash->pekeyr = FLASH_PEKEY2;
		}
		flash->prgkeyr= FLASH_PRGKEYR1;
		flash->prgkeyr= FLASH_PRGKEYR2;
	}

	ptr = (uint8_t *)buf;
	rem = len;
	while (rem > 3) {
		uint32_t pos;
		uint32_t cnt;
		uint32_t pri;
		uint32_t sr;

		/* adjust offset for half page alignement */
		pos = offs - (offs & ~((FLASH_PAGE_SIZE / 2) - 1));
		offs -= pos;

		/* make sure we don't exceed the page boundary */
		cnt = MIN((FLASH_PAGE_SIZE / 2) - pos, rem);

		DCC_LOG3(LOG_INFO, "1. offs=0x%06x pos=%d cnt=%d", offs, pos, cnt);

		/* adjust counter to count words */
		cnt &= ~3;

		pri = cm3_primask_get();
		cm3_primask_set(1);
		/* start half page write */
		sr = stm32l_flash_pg_wr(offs, ptr, pos, cnt);

		cm3_primask_set(pri);

		if (sr & FLASH_ERR) {
			DCC_LOG(LOG_WARNING, "Flash write failed!");
			return -1;
		}

		ptr += cnt;
		rem -= cnt;
		offs += pos + cnt;
	}

	if (rem) {
		uint32_t pos;
		uint32_t pri;
		uint8_t data[4];
		unsigned int ret;
		int i;

		/* FIXME: single word writing.... */

		/* adjust offset for half page alignement */
		pos = offs - (offs & ~((FLASH_PAGE_SIZE / 2) - 1));
		offs -= pos;

		/* copy data to temporary buffer */ 
		for (i = 0; i < rem; ++i)
			data[i] = ptr[i];
		/* fill the remaining buffer */
		for (; i < 4; ++i)
			data[i] = 0;

		DCC_LOG3(LOG_INFO, "2. offs=0x%06x pos=%d cnt=%d", offs, pos, 4);

		pri = cm3_primask_get();
		cm3_primask_set(1);
		/* start half page write */
		ret = stm32l_flash_pg_wr(offs, data, pos, 4);
		cm3_primask_set(pri);

		if (ret < 0) {
			DCC_LOG(LOG_WARNING, "Flash write failed!");
			return ret;
		}
	};

	return len;
}
#endif
