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
 * @file gdb-rsp.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


#include <stdlib.h>
#include <stdbool.h>

#include <sys/dcclog.h>
#include <sys/stm32f.h>
#include <sys/param.h>

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#include <thinkos.h>

/* -------------------------------------------------------------------------
 * Memory auxiliary functions 
 * ------------------------------------------------------------------------- */

bool thinkos_mem_belong(const struct thinkos_mem_desc * mem, uint32_t addr)
{
	uint32_t size;
	int32_t off;
	int i;

	DCC_LOG2(LOG_MSG, "addr=0x%08x mem='%s'", addr, mem->tag);

	if ((off = (addr - mem->base)) < 0)
		return false;

	if ((mem->blk[0].cnt == 0) || addr < mem->blk[0].off)
		return false;

	for (i = 0; mem->blk[i + 1].cnt != 0; ++i);
	
	size = mem->blk[i].cnt << mem->blk[i].siz;

	if (addr >= (mem->blk[i].off + size))
		return false;

	DCC_LOG4(LOG_MSG, "addr=0x%08x match: '%s' 0x%08x - 0x%08x", 
			 addr, mem->tag, mem->blk[0].off, mem->blk[i].off + size - 1); 

	return true;
}

const struct thinkos_mem_desc * thinkos_mem_lookup(
	const struct thinkos_mem_desc * const lst[], 
	unsigned int cnt, uint32_t addr)
{
	unsigned int j;

	for (j = 0; j < cnt; ++j) {
		const struct thinkos_mem_desc * mem = lst[j];
		if (mem == NULL)
			continue;
		if (thinkos_mem_belong(mem, addr))
			return mem;
	}

	return NULL;
}


/* Safe read and write operations to avoid faults in the debugger */
bool thinkos_mem_wr32(const struct thinkos_mem_desc * mem, 
					  uint32_t addr, uint32_t val)
{
	uint32_t base;
	uint32_t size;
	int i;

	if (addr & 3)
		return false;

	for (i = 0; mem->blk[i].cnt != 0; ++i) {
		size = mem->blk[i].cnt << mem->blk[i].siz;
		base = mem->blk[i].off;
		if ((addr >= base) && (addr < (base + size - 4))) {
			uint32_t * dst = (uint32_t *)addr;
			if (mem->blk[i].opt == M_RO)
				return false;
			*dst = val;
			return true;
		}
	}
	return false;
}

bool thinkos_mem_rd32(const struct thinkos_mem_desc * mem, 
					 uint32_t addr, uint32_t * val)
{
	uint32_t base;
	uint32_t size;
	int i;

	if (addr & 3)
		return false;

	for (i = 0; mem->blk[i].cnt != 0; ++i) {
		size = mem->blk[i].cnt << mem->blk[i].siz;
		base = mem->blk[i].off;
		if ((addr >= base) && (addr <= (base + size - 4))) {
			uint32_t * src = (uint32_t *)addr;
			*val = *src;
			return true;
		}
	}
	return false;
}

bool thinkos_mem_wr64(const struct thinkos_mem_desc * mem, 
					 uint32_t addr, uint64_t val)
{
	uint32_t base;
	uint32_t size;
	int i;

	if (addr & 3)
		return false;

	for (i = 0; mem->blk[i].cnt != 0; ++i) {
		size = mem->blk[i].cnt << mem->blk[i].siz;
		base = mem->blk[i].off;
		if ((addr >= base) && (addr < (base + size - 8))) {
			uint64_t * dst = (uint64_t *)addr;
			if (mem->blk[i].opt == M_RO)
				return false;
			*dst = val;
			return true;
		}
	}
	return false;
}

bool thinkos_mem_rd64(const struct thinkos_mem_desc * mem, 
					 uint32_t addr, uint64_t * val)
{
	uint32_t base;
	uint32_t size;
	int i;

	if (addr & 3)
		return false;

	for (i = 0; mem->blk[i].cnt != 0; ++i) {
		size = mem->blk[i].cnt << mem->blk[i].siz;
		base = mem->blk[i].off;
		if ((addr >= base) && (addr <= (base + size - 8))) {
			uint64_t * src = (uint64_t *)addr;
			*val = *src;
			return true;
		}
	}
	return false;
}

/* 32bits access memory read */

int thinkos_mem_read(const struct thinkos_mem_desc * mem,
					uint32_t addr, void * ptr, unsigned int len)
{
	uint8_t * dst = (uint8_t *)ptr;
	uint32_t * src;
	unsigned int rem;
	uint32_t base;
	uint32_t size;
	int j = 0;

	if (mem == NULL)
		return 0;

	DCC_LOG2(LOG_TRACE, "addr=0x%08x mem='%s'", addr, mem->tag);

	rem = len;
	while (rem) {
		unsigned int n;
		uint32_t dat;
		uint32_t cnt;

		if (mem->blk[j].cnt == 0) {
			/* end of list */
			break;
		}

		if ((base = mem->blk[j].off) > addr) {
			/* address out of bounds */
			break;
		}

		size = mem->blk[j].cnt << mem->blk[j].siz;
		if (addr >= (base + size)) {
			j++;
			continue;
		}

		cnt = (base + size) - addr;
		cnt = MIN(cnt, rem);

		src = (uint32_t *)(addr & ~3);

		if ((n = (addr & 3)) > 0) {
			dat = *src++;
			dat >>= (4 - n) * 8;
			if (n > cnt)
				n = cnt;

			switch (n) {
			case 3:
				*dst++ = dat;
				dat >>= 8;
				/* FALLTHROUGH */
			case 2:
				*dst++ = dat;
				dat >>= 8;
				/* FALLTHROUGH */
			case 1:
				*dst++ = dat;
				dat >>= 8;
			}
		}

		rem -= cnt;
		addr += cnt;
		cnt -= n;
		n = cnt & ~3;
	
		for (n = 0; n < (cnt & ~3); n += 4) {
			dat = *src++;
			*dst++ = dat;
			dat >>= 8;
			*dst++ = dat;
			dat >>= 8;
			*dst++ = dat;
			dat >>= 8;
			*dst++ = dat;
		}

		cnt -= n;
		if ((n = cnt) > 0) {
			dat = *src;

			switch (n) {
			case 3:
				*dst++ = dat;
				dat >>= 8;
				/* FALLTHROUGH */
			case 2:
				*dst++ = dat;
				dat >>= 8;
				/* FALLTHROUGH */
			case 1:
				*dst++ = dat;
				dat >>= 8;
			}
		}
	}

	return len - rem;
}


