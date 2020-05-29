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
 * @file stm32f1x-flash.c
 * @brief STM32F flash access API
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#include <sys/stm32f.h>
#include <arch/cortex-m3.h>
#include <sys/flash-dev.h>
#include <sys/dcclog.h>

#if defined(STM32F1X) || defined(STM32F3X)

#if defined(STM32F1X)
#define FLASH_BLOCK_SIZE 1024
#elif defined(STM32F3X)
#define FLASH_BLOCK_SIZE 2048
#endif

#define FLASH_ERR (FLASH_WRPRTERR | FLASH_PGPERR)



int stm32f1x_flash_unlock(struct stm32_flash * flash, off_t offs, size_t len)
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

int stm32f1x_flash_lock(struct stm32_flash * flash, off_t offs, size_t len)
{
	return 0;
}

uint32_t __attribute__((section (".data#"), noinline)) 
	stm32f10x_flash_blk_erase(struct stm32_flash * flash, uint32_t addr)
{
	uint32_t sr;

	flash->cr = FLASH_SER;
	flash->ar = addr;
	flash->cr = FLASH_STRT | FLASH_SER;

#if defined(STM32F1X) 
/* STM32F10xx4 STM32F10xx6 Errata sheet
  2.11 Flash memory BSY bit delay versus STRT bit setting
  Description
  When the STRT bit in the Flash memory control register is set (to launch 
  an erase operation), the BSY bit in the Flash memory status register 
  goes high one cycle later.
  Therefore, if the FLASH_SR register is read immediately after the FLASH_CR 
  register is written (STRT bit set), the BSY bit is read as 0.
  Workaround
  Read the BSY bit at least one cycle after setting the STRT bit.
*/
	__nop();
	__nop();
#endif

	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	return sr;
}


int stm32f1x_flash_erase(struct stm32_flash * flash, off_t offs, size_t len)
{
	unsigned int size = 2048;
	uint32_t addr;
	uint32_t pri;
	uint32_t sr;

	pri = cm3_primask_get();

	if ((offs & ~(FLASH_BLOCK_SIZE - 1)) != offs) {
		DCC_LOG(LOG_ERROR, "offset must be a aligned to a page boundary.");
		return -2;
	};

	addr = (uint32_t)STM32_FLASH_MEM + offs;

	/* Clear errors */
	flash->sr = FLASH_ERR;

	cm3_primask_set(1);
	sr = stm32f10x_flash_blk_erase(flash, addr);
	cm3_primask_set(pri);

	if (sr & FLASH_ERR) {
		DCC_LOG1(LOG_WARNING, "stm32f10x_flash_blk_erase() failed"
				 " sr=%08x!", sr);
		return -1;
	}

	return size;
}

uint32_t __attribute__((section (".data#"), noinline)) 
stm32f10x_flash_wr16(struct stm32_flash * flash, 
                     uint16_t volatile addr[], uint32_t data)
{
	uint32_t sr;

	flash->cr = FLASH_PG;
	addr[0] = data;

	do {
		sr = flash->sr;
	} while (sr & FLASH_BSY);

	return sr;
}

int stm32f1x_flash_write(struct stm32_flash * flash, 
						 off_t offs, const void * buf, size_t len)
{
	uint32_t data;
	uint16_t * dst;
	uint8_t * src;
	uint32_t sr;
	uint32_t pri;

	if (offs & 0x00000001) {
		DCC_LOG(LOG_ERROR, "offset must be 16bits aligned!");
		return -1;
	}

	src = (uint8_t *)buf;
	dst = (uint16_t *)((uint32_t)STM32_FLASH_MEM + offs);

	DCC_LOG2(LOG_INFO, "0x%08x len=%d", addr, len);

	pri = cm3_primask_get();
	data = src[0] | (src[1] << 8);
	DCC_LOG3(LOG_TRACE, "0x%08x data=0x%04x 0x%04x", addr, data0, data1);
	cm3_primask_set(1);
	sr = stm32f10x_flash_wr16(flash, dst, data);
	cm3_primask_set(pri);
	if (sr & FLASH_ERR) {
		DCC_LOG(LOG_WARNING, "stm32f10x_flash_wr16() failed!");
		return -1;
	}
	
	return 2;
}

int stm32f1x_flash_read(struct stm32_flash * flash, 
						off_t offs, const void * buf, size_t len)
{
	uint32_t data;
	uint32_t * src;
	uint8_t * dst;

	src = (uint32_t *)(((uint32_t)STM32_FLASH_MEM + offs) & 0xfffffffc);
	dst = (uint8_t *)buf;

	data = src[0];
	dst[0] = data;
	dst[1] = data >> 8;
	dst[2] = data >> 16;
	dst[3] = data >> 24;

	return 4;
}

const struct flash_dev_ops stm32f1x_flash_dev_ops = {
	.write = (int (*)(void *, off_t, const void *, size_t))stm32f1x_flash_write,
	.read = (int (*)(void *, off_t, void *, size_t))stm32f1x_flash_read,
	.erase = (int (*)(void *, off_t, size_t count))stm32f1x_flash_erase,
	.lock = (int (*)(void *, off_t, size_t count))stm32f1x_flash_lock,
	.unlock = (int (*)(void *, off_t, size_t count))stm32f1x_flash_unlock
};

/* FLASH memory device object */
const struct flash_dev stm32f1x_flash_dev = {
	.priv = (void *)STM32_FLASH,
	.op = &stm32f1x_flash_dev_ops
};

#endif


