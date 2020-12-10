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
 * @file thinkos_memory.c
 * @brief ThinkOS
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include "thinkos_krn-i.h"
#include <sys/dcclog.h>

extern void * __krn_data_start;
extern void * __krn_data_end;
extern void * __krn_code_start;
extern void * __krn_code_end;

extern void * __usr_data_start;
extern void * __usr_data_end;
extern void * __usr_code_start;
extern void * __usr_code_end;

/* -------------------------------------------------------------------------
 * Memory auxiliary functions 
 * ------------------------------------------------------------------------- */

#if (THINKOS_ENABLE_MEMORY_MAP)
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
#endif

#if (THINKOS_ENABLE_SANITY_CHECK)
bool __thinkos_mem_usr_rx_chk(uint32_t addr, int32_t size)
{
#if 0
	uint32_t code_base = thinkos_rt.mem.krn_code.base;
	uint32_t code_top = thinkos_rt.mem.krn_code.top;
	uint32_t data_base = thinkos_rt.mem.krn_data.base;
	uint32_t data_top = thinkos_rt.mem.krn_data.top;
#else
	uint32_t code_base = (uintptr_t)&__krn_code_start;
	uint32_t code_top = (uintptr_t)&__krn_code_end;
	uint32_t data_base = (uintptr_t)&__krn_data_start;
	uint32_t data_top = (uintptr_t)&__krn_data_end;
#endif
	uint32_t usr_base = addr;
	uint32_t usr_top;

	if (size < 0)
		return false;

	usr_top = addr + size;

	DCC_LOG3(LOG_MSG, "code: %08x-%08x (%d)",
			 code_base, code_top, code_top - code_base);

	DCC_LOG3(LOG_MSG, "data: %08x-%08x (%d)",
			 data_base, data_top, data_top - data_base);

	DCC_LOG3(LOG_MSG, "user: %08x-%08x (%d)",
			 usr_base, usr_top, usr_top - usr_base);
	
	/* FIXME: this is a minimum implementation just to avoid
	   invalid accesses to the kernel memory */

	if ((usr_base >= data_base) && (usr_base < data_top)) {
		return false;
	}
	if ((usr_top >= data_base) && (usr_top < data_top)) {
		return false;
	}
	if ((usr_base >= code_base) && (usr_base < code_top)) {
		return false;
	}
	if ((usr_top >= code_base) && (usr_top < code_top)) {
		return false;
	}
	return true;
}

bool __thinkos_mem_usr_rd_chk(uint32_t addr, int32_t size) 
	__attribute__((alias("__thinkos_mem_usr_rx_chk")));

bool __thinkos_mem_usr_rw_chk(uint32_t addr, int32_t size) 
	__attribute__((alias("__thinkos_mem_usr_rx_chk")));

#endif /* (THINKOS_ENABLE_SANITY_CHECK) */


#if DEBUG
void __dmap(const struct thinkos_mem_map * map)
{
	int j;

	for (j = 0; j < map->cnt; ++j) {
		const struct thinkos_mem_desc * mem = map->desc[j];
		int i;

		DCC_LOG2(LOG_TRACE, "[%-8s]    %08x", mem->tag, mem->base);
		for (i = 0; i < mem->cnt; ++i) {
			const struct thinkos_mem_blk * blk = &mem->blk[i];
			uint32_t begin;
			uint32_t size;
			uint32_t end;
			begin = mem->base + blk->off;
			size = (1 << blk->siz) * blk->cnt;
			end = begin + size - 1;
			DCC_LOG4(LOG_TRACE, "  +- %-8s %08x - %08x (%8d)", blk->tag, begin, end, size);
		}
	}
}
#endif
 
int __thinkos_krn_mem_init(struct thinkos_rt * krn, 
							const struct thinkos_mem_map * map)
{

	DCC_LOG2(LOG_TRACE, "kernel code: %08x-%08x", (uintptr_t)&__krn_code_start, 
			 (uintptr_t)&__krn_code_end);
	DCC_LOG2(LOG_TRACE, "kernel data: %08x-%08x", (uintptr_t)&__krn_data_start, 
			 (uintptr_t)&__krn_data_end);
#if (THINKOS_ENABLE_MEMORY_MAP)
	if (map == NULL) {
		DCC_LOG(LOG_ERROR, "mull pointer!");
		/* FIXME: return code */
		return -1;
	}
	krn->mem_map = map;

#if DEBUG
	__dmap(map);
#endif

#endif

#if (THINKOS_ENABLE_MPU)
	thinkos_krn_mpu_init((uintptr_t)&__krn_code_start, 
						 (uintptr_t)&__krn_code_end,
						 (uintptr_t)&__krn_data_start, 
						 (uintptr_t)&__krn_data_end);
#endif

#if 0
	krn->mem.krn_code.base = (uintptr_t)&__krn_code_start;
	krn->mem.krn_code.top = (uintptr_t)&__krn_code_end;
	krn->mem.krn_data.base = (uintptr_t)&__krn_data_start;
	krn->mem.krn_data.top = (uintptr_t)&__krn_data_end;
#endif

	return 0;
}

static uint64_t tag2hash(const char * tag)
{
	char * cp = (char *)tag;
	uint64_t hash = 0;
	int i = 0;
	int c;

	for (i = 0; i < 8; ++i) {
		if ((c = cp[i]) == '\0')
			break;
		hash |= c << (i * 8); 
	}

	return hash;
}

const struct thinkos_mem_blk * __mem_blk_lookup(const struct thinkos_mem_desc * mem, 
													const char * tag)
{
	uint64_t hash = tag2hash(tag);
	int i;

	DCC_LOG2(LOG_YAP, "hash=%08x%08x", hash >> 32, hash);

	for (i = 0; i < mem->cnt; ++i) {
		const struct thinkos_mem_blk * blk = &mem->blk[i];
		uint64_t cmp = tag2hash(blk->tag);

		DCC_LOG2(LOG_YAP, "cmp=%08x%08x", cmp >> 32, cmp);
		if (hash == cmp) {
			return blk;
		}
	}

	return NULL;
}
 
const struct thinkos_mem_desc * __mem_desc_lookup(const struct thinkos_mem_map * map, 
													  const char * tag)
{
	uint64_t hash = tag2hash(tag);
	int i;

	DCC_LOG2(LOG_YAP, "hash=%08x%08x", hash >> 32, hash);

	for (i = 0; i < map->cnt; ++i) {
		const struct thinkos_mem_desc * mem = map->desc[i];
		uint64_t cmp = tag2hash(mem->tag);

		DCC_LOG2(LOG_TRACE, "cmp=%08x%08x", cmp >> 32, cmp);
		if (hash == cmp) {
			return mem;
		}
	}

	return NULL;
}

bool __krn_mem_part_lookup(struct thinkos_rt * krn, const char * desc, const char * part,
					 struct thinkos_mem_part * mp)
{
#if (THINKOS_ENABLE_MEMORY_MAP)
	const struct thinkos_mem_map * map = krn->mem_map;
	const struct thinkos_mem_desc * mem;
	const struct thinkos_mem_blk * blk;

	DCC_LOG3(LOG_YAP, "map=%08x '%s'/'%s'", map, desc, part);

	if ((mem = __mem_desc_lookup(map, desc)) == NULL) {
		DCC_LOG(LOG_WARNING, "__mem_desc_lookup() failed!");
		return false;
	}

	if ((blk = __mem_blk_lookup(mem, part)) == NULL) {
		DCC_LOG(LOG_WARNING, "__mem_blk_lookup() failed!");
		return false;
	}

	if (mp != NULL) {
		mp->begin = mem->base + blk->off;
		mp->end = mem->base + blk->off + (1 << blk->siz) * blk->cnt;
		mp->opt = blk->opt;
		mp->dev = mem->dev;
		mp->type = mem->typ;
		mp->perm = mem->opt;
	}

	return true;
#else
	return false;
#endif
}

bool __mem_part_get(const struct thinkos_mem_map * map, unsigned int j, 
					unsigned int i, struct thinkos_mem_part * mp)
{
	if (j < map->cnt) {
		const struct thinkos_mem_desc * mem = map->desc[j];
		if (i < mem->cnt) {
			const struct thinkos_mem_blk * blk = &mem->blk[i];

			if (mp != NULL) {
				mp->begin = mem->base + blk->off;
				mp->end = mem->base + blk->off + (1 << blk->siz) * blk->cnt;
				mp->opt = blk->opt;
				mp->dev = mem->dev;
				mp->type = mem->typ;
				mp->perm = mem->opt;
			}

			return true;
		}
	}
	return false;
}

