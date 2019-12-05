/* 
 * File:	 thinkos_flash_mem.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <sys/stm32f.h>

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#include <thinkos.h>

#include <sys/param.h>
#include <stdbool.h>

#if (THINKOS_ENABLE_FLASH_MEM)

int board_flash_mem_close(int mem);
int board_flash_mem_open(int mem);
int board_flash_mem_read(int mem, void * buf, size_t len);
int board_flash_mem_write(int mem, const void * buf, size_t len);
int board_flash_mem_seek(int mem, off_t offset);
int board_flash_mem_erase(int mem, off_t offset, size_t len);
int board_flash_mem_lock(int mem, off_t offset, size_t len);
int board_flash_mem_unlock(int mem, off_t offset, size_t len);

struct {
	int8_t thread;
	uint8_t open;
	int32_t * arg;
} thinkos_flash_rt;

#define FLASH_ERR (FLASH_PGSERR | FLASH_PGPERR | FLASH_PGAERR | FLASH_WRPERR | \
				   FLASH_OPERR)

void thinkos_flash_mem_cont(int32_t * arg, int self);

void thinkos_flash_mem_cont(int32_t * arg, int self)
{
	for(;;);
}

void __svc_continue(void * arg, int self)
{
	struct armv7m_basic_frame * frm = (struct armv7m_basic_frame *)arg;
	uint32_t pc;

	pc = frm->pc;
	frm->r2 = pc;
	pc = (uint32_t)thinkos_flash_mem_cont;
	pc |= 1;	
	frm->pc = pc;
}

uint32_t __attribute__((section (".data#"), noinline)) 
	__stm32f2x_flash_wr32(struct stm32_flash * flash, uint32_t cr,
						 uint32_t volatile * addr, uint32_t data)
{
	struct thinkos_rt * thinkos = &thinkos_rt;
	uint32_t ticks;
	uint32_t sr;

	flash->cr = cr;
	*addr = data;
	
	do {
		sr = flash->sr;
			ticks = thinkos->ticks; 
			ticks++;
			thinkos->ticks = ticks; 
	} while (sr & FLASH_BSY);

	return sr;
}

uint32_t __attribute__((section (".data#"), noinline)) 
	__stm32f2x_flash_sect_erase(struct stm32_flash * flash, uint32_t cr,
								struct cm3_systick * systick, 
								struct thinkos_rt * thinkos)
{
	uint32_t ticks;
	uint32_t sr;

	flash->cr = cr;

	do {
		if (systick->csr & SYSTICK_CSR_COUNTFLAG) {
			ticks = thinkos->ticks; 
			ticks++;
			thinkos->ticks = ticks; 
		}
		sr = flash->sr;
	} while (sr & FLASH_BSY);
	flash->cr = 0;

	return sr;
}


int __stm32_flash_write(uint32_t offs, const void * buf, unsigned int len)
{
	struct stm32_flash * flash = STM32_FLASH;
	uint32_t data;
	uint32_t * addr;
	uint8_t * ptr;
	uint32_t cr;
	uint32_t sr;
//	uint32_t pri;
	int n;
	int i;
	uint32_t clk;

	if (offs & 0x00000003) {
		DCC_LOG(LOG_ERROR, "offset must be 32bits aligned!");
		return -1;
	}

	n = (len + 3) / 4;

	ptr = (uint8_t *)buf;
	addr = (uint32_t *)((uint32_t)STM32_FLASH_MEM + offs);

	cr = flash->cr;
	if (cr & FLASH_LOCK) {
		DCC_LOG(LOG_TRACE, "unlocking flash...");
		/* unlock flash write */
		flash->keyr = FLASH_KEY1;
		flash->keyr = FLASH_KEY2;
	}

	DCC_LOG2(LOG_INFO, "0x%08x len=%d", addr, len);

	/* Clear errors */
	flash->sr = FLASH_ERR;

//	pri = cm3_primask_get();
	for (i = 0; i < n; i++) {
		data = ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24);
//		DCC_LOG2(LOG_TRACE, "0x%08x data=0x%08x", addr, data);
		cr = FLASH_PG | FLASH_PSIZE_32;
	//	cm3_primask_set(1);
		clk = thinkos_rt.ticks; 
		sr = __stm32f2x_flash_wr32(flash, cr, addr, data);
		DCC_LOG1(LOG_TRACE, "dt=%d", thinkos_rt.ticks - clk);

	//	cm3_primask_set(pri);
		if (sr & FLASH_ERR) {
			DCC_LOG(LOG_WARNING, "stm32f2x_flash_wr32() failed!");
			return -1;
		}
		ptr += 4;
		addr++;
	}
	
	return n * 4;
}

int __stm32_flash_erase(unsigned int offs, unsigned int len)
{
	struct stm32_flash * flash = STM32_FLASH;
	struct cm3_systick * systick = CM3_SYSTICK;
	struct cm3_scb * scb = CM3_SCB;
	uint32_t icsr;
	unsigned int cnt;
	uint32_t pri;
	uint32_t cr;
	uint32_t sr;

	pri = cm3_primask_get();

	cr = flash->cr;
	if (cr & FLASH_LOCK) {
		DCC_LOG(LOG_TRACE, "unlocking flash...");
		/* unlock flash write */
		flash->keyr = FLASH_KEY1;
		flash->keyr = FLASH_KEY2;
	}

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

		DCC_LOG2(LOG_TRACE, "sector=%d size=%d", sect, size);
		cr = FLASH_STRT | FLASH_SER | FLASH_SNB(sect);
		cm3_cpsid_i();
		sr = __stm32f2x_flash_sect_erase(flash, cr, systick, &thinkos_rt);
		icsr = scb->icsr;
		cm3_cpsie_i();
		if (icsr != 0) {
			DCC_LOG1(LOG_WARNING, "stm32f2x_flash_sect_erase() "
					 " icsr=%08x!", icsr);
		}

		cm3_primask_set(1);
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

#endif /* THINKOS_ENABLE_FLASH_MEM */

