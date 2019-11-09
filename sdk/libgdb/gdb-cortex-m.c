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
 * @file gdb-cortex-m.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


#include "gdb-i.h"

#define __THINKOS_IDLE__
#include <thinkos/idle.h>
#include <sys/stm32f.h>


/* -------------------------------------------------------------------------
 * Memory auxiliarly functions
 * ------------------------------------------------------------------------- */

/* Expanded Memory Block */
struct mem_blk {
	uint32_t addr;
	uint32_t size;
	uint32_t ro;
};


static bool addr2block(const struct mem_desc * mem, 
					   uint32_t addr, struct mem_blk * blk) 
{
	uint32_t base;
	uint32_t size;
	int i;

	for (i = 0; mem->blk[i].cnt != 0; ++i) {
		size = mem->blk[i].cnt << mem->blk[i].siz;
		base = mem->blk[i].ref;
		if ((addr >= base) && (addr < (base + size))) {
			if (blk != NULL) {
				int pos;
				pos = (addr - base) >> mem->blk[i].siz;
				blk->addr = base + (pos << mem->blk[i].siz);
				blk->size = 1 << mem->blk[i].siz;
				blk->ro = (mem->blk[i].opt == M_RO) ? 1 : 0;
			}
			return true;
		}
	}

	return false;
}

/* -------------------------------------------------------------------------
 * Threads auxiliarly functions
 * ------------------------------------------------------------------------- */

int thread_getnext(int thread_id)
{
	int id;

	if (thread_id == THREAD_ID_ANY) {
		id = THINKOS_THREAD_IDLE;
		DCC_LOG1(LOG_MSG, "thread %d (IDLE)", id);
		return THREAD_ID_IDLE;
	} 
	
	if (thread_id == THREAD_ID_IRQ) {
		DCC_LOG(LOG_MSG, "no more threads.");
		return -1;
	}

	if (thread_id == THREAD_ID_IDLE)
		id = __thinkos_thread_getnext(-1);
	else
		id = __thinkos_thread_getnext(thread_id - THREAD_ID_OFFS);

	if (id < 0) {
		if (thinkos_rt.xcpt_ipsr != 0) {
			DCC_LOG1(LOG_WARNING, "Exception at IRQ:%d!", 
					 thinkos_rt.xcpt_ipsr - 16);
			return THREAD_ID_IRQ;
		}
		DCC_LOG(LOG_MSG, "no more threads.");
		return -1;
	}

	DCC_LOG1(LOG_MSG, "thread %d", id);
	id += THREAD_ID_OFFS;

	return id;
}

int thread_active(void)
{
	if (thinkos_rt.active == THINKOS_THREAD_IDLE) {
		DCC_LOG(LOG_TRACE, "IDLE");
	}
	return thinkos_rt.active + THREAD_ID_OFFS;
}

int thread_step_id(void)
{
	if (thinkos_rt.step_id == -1) {
		DCC_LOG(LOG_WARNING, "invalid step thread!");
		return thinkos_rt.active + THREAD_ID_OFFS;
	}

	return thinkos_rt.step_id + THREAD_ID_OFFS;
}

int thread_break_id(void)
{
	int break_id;
	
	if ((break_id = dbgmon_thread_break_get()) == -1) {
		DCC_LOG(LOG_WARNING, "invalid break thread!");
		if (thinkos_rt.xcpt_ipsr == 0) {
			DCC_LOG(LOG_WARNING, "No exception at IRQ!");
			return thinkos_rt.active + THREAD_ID_OFFS;
		}
		DCC_LOG1(LOG_WARNING, "Exception at IRQ:%d!", 
				 thinkos_rt.xcpt_ipsr - 16);
		return THREAD_ID_IRQ;
	}

	DCC_LOG1(LOG_INFO, "break_id=%d", break_id);

	return break_id + THREAD_ID_OFFS;
}


int thread_any(void)
{
	int thread_id;

	if ((unsigned int)thinkos_rt.active < THINKOS_THREADS_MAX) {
		DCC_LOG1(LOG_TRACE, "active=%d", thinkos_rt.active);
		return thinkos_rt.active + THREAD_ID_OFFS;
	}

	if (thinkos_rt.active == THINKOS_THREAD_IDLE)
		DCC_LOG(LOG_MSG, "IDLE thread!");
	else if (thinkos_rt.active == THINKOS_THREAD_VOID)
		DCC_LOG(LOG_MSG, "VOID thread!");
	else {
		DCC_LOG1(LOG_MSG, "active=%d is invalid!", thinkos_rt.active);
	}

	/* Active thread is IDLE or invalid, try to get the first 
	   initialized thread. */
	thread_id = __thinkos_thread_getnext(-1);
	
	if (thread_id < 0) {
		DCC_LOG(LOG_WARNING, "No threads!");
		thread_id = THINKOS_THREAD_IDLE;
	}

	return thread_id + THREAD_ID_OFFS;
}

bool thread_isalive(int gdb_thread_id)
{
	unsigned int thread_id = gdb_thread_id - THREAD_ID_OFFS;

	return __thinkos_thread_isalive(thread_id);
}

const int16_t __ctx_offs[] = {
	[0] = offsetof(struct thinkos_context, r0),
	[1] = offsetof(struct thinkos_context, r1),
	[2] = offsetof(struct thinkos_context, r2),
	[3] = offsetof(struct thinkos_context, r3),
	[4] = offsetof(struct thinkos_context, r4),
	[5] = offsetof(struct thinkos_context, r5),
	[6] = offsetof(struct thinkos_context, r6),
	[7] = offsetof(struct thinkos_context, r7),
	[8] = offsetof(struct thinkos_context, r8),
	[9] = offsetof(struct thinkos_context, r9),
	[10] = offsetof(struct thinkos_context, r10),
	[11] = offsetof(struct thinkos_context, r11),
	[12] = offsetof(struct thinkos_context, r12),
	[13] = INT16_MIN, /*sp */
	[14] = offsetof(struct thinkos_context, lr),
	[15] = offsetof(struct thinkos_context, pc),
	[16] = INT16_MIN,
	[17] = INT16_MIN,
	[18] = INT16_MIN,
	[19] = INT16_MIN,
	[20] = INT16_MIN,
	[21] = INT16_MIN,
	[22] = INT16_MIN,
	[23] = INT16_MIN,
	[24] = INT16_MIN,
	[25] = offsetof(struct thinkos_context, xpsr),
#if THINKOS_ENABLE_FPU
	[26] = (18 *4),
	[27] = (20 *4),
	[28] = (22 *4),
	[29] = (24 *4),
	[30] = (26 *4),
	[31] = (28 *4),
	[32] = (30 *4),
	[33] = (32 *4),
	[34] = -(16 * 4),
	[35] = -(12 * 4),
	[36] = -(12 * 4),
	[37] = -(10 * 4),
	[38] = -(8 * 4),
	[39] = -(6 * 4),
	[40] = -(4 * 4),
	[41] = -(34 * 4), /* fpscr */
	[42] = -(35 * 4)
#endif
};

/*
uint32_t __reg_addr(struct thinkos_context * ctx, unsigned int reg)
{
	uint32_t addr;
	int offs;

	if ((reg <= 42) && (__ctx_offs[reg] >= 0)) {
		addr = (uintptr_t)ctx + __ctx_offs[reg];
	}
}
*/

int thread_register_get(int gdb_thread_id, int reg, uint64_t * val)
{
	unsigned int thread_id = gdb_thread_id - THREAD_ID_OFFS;
	struct dbgmon_thread_inf inf;
	struct thinkos_context * ctx;
	struct mem_blk blk;
	uint32_t addr;

	if (dbgmon_thread_inf_get(thread_id, &inf) < 0)
		return -1;

	ctx = inf.ctx;

	if (!addr2block(this_board.memory.ram, (uintptr_t)ctx, &blk)) {
		DCC_LOG(LOG_ERROR, "invalid context address");
		return -1;
	}

#if THINKOS_ENABLE_FPU
	if (reg > 42) {
		DCC_LOG1(LOG_ERROR, "Invalid register %d", reg);
		return -1;
	}
#else
	if (reg > 25) {
		DCC_LOG1(LOG_ERROR, "Invalid register %d", reg);
		return -1;
	}
#endif

	if (reg == 13) { /*sp */
		*val = inf.sp;
		return 0;
	} 

	if (__ctx_offs[reg] == INT16_MIN) {
		DCC_LOG1(LOG_ERROR, "Invalid register %d", reg);
		return -1;
	}

	addr = (uintptr_t)ctx + __ctx_offs[reg];
#if THINKOS_ENABLE_FPU
	if (reg > 25) {
		uint32_t dat;
		if (!dbgmon_mem_rd32(this_board.memory.ram, addr, &dat)) {
			DCC_LOG(LOG_ERROR, "invalid context address");
			return -1;
		}
		*val = dat;
	} else {
#endif
		if (!dbgmon_mem_rd64(this_board.memory.ram, addr, val)) {
			DCC_LOG(LOG_ERROR, "invalid context address");
			return -1;
		}
#if THINKOS_ENABLE_FPU
	}
#endif

	return 0;
}

int thread_register_set(unsigned int gdb_thread_id, int reg, uint64_t val)
{
	unsigned int thread_id = gdb_thread_id - THREAD_ID_OFFS;
	struct dbgmon_thread_inf inf;
	struct thinkos_context * ctx;
	struct mem_blk blk;
	uint32_t addr;

	if (dbgmon_thread_inf_get(thread_id, &inf) < 0)
		return -1;

	ctx = inf.ctx;
	if (thread_id > THINKOS_THREADS_MAX) {
		DCC_LOG(LOG_ERROR, "Invalid thread!");
		return -1;
	}

	if (addr2block(this_board.memory.ram, (uintptr_t)ctx, &blk)) {
		if (blk.ro) {
			DCC_LOG2(LOG_ERROR, "read only block addr=0x%08x size=%d", 
					 blk.addr, blk.size);
			return -1;
		}
	} else {
		DCC_LOG(LOG_ERROR, "invalid context address");
		return -1;
	}

#if THINKOS_ENABLE_FPU
	if (reg > 42) {
		DCC_LOG(LOG_ERROR, "Invalid register");
		return -1;
	}
#else
	if (reg > 25) {
		DCC_LOG(LOG_ERROR, "Invalid register");
		return -1;
	}
#endif

	if (reg == 13) { /*sp */
		uint32_t sp = val;
#if (THINKOS_ENABLE_IDLE_MSP) || (THINKOS_ENABLE_FPU)
		sp -= (ctx->ret & CM3_EXC_RET_nFPCA) ? (8*4) : (26*4);
		DCC_LOG3(LOG_MSG, _ATTR_PUSH_ _FG_GREEN_ 
				 "<%d> SP=%08x! RET=[%s]!" _ATTR_POP_, 
				 thread_id + 1, sp, __retstr(ctx->ret));				
		ctx->sp = sp;
#else
		sp -= (uint32_t)ctx + sizeof(struct thinkos_context);
		__thinkos_thread_ctx_set(thread_id, (struct thinkos_context *)sp);
#endif
	} 

	if (__ctx_offs[reg] == INT16_MIN) {
		DCC_LOG(LOG_ERROR, "Invalid register");
		return -1;
	}

	addr = (uintptr_t)ctx + __ctx_offs[reg];
#if THINKOS_ENABLE_FPU
	if (reg > 25) {
		if (!dbgmon_mem_wr32(this_board.memory.ram, addr, val)) {
			DCC_LOG(LOG_ERROR, "invalid context address");
			return -1;
		}
	} else {
#endif
		if (!dbgmon_mem_wr64(this_board.memory.ram, addr, val)) {
			DCC_LOG(LOG_ERROR, "invalid context address");
			return -1;
		}
#if THINKOS_ENABLE_FPU
	}
#endif

	return 0;
}

int thread_goto(unsigned int gdb_thread_id, uint32_t addr)
{
	unsigned int thread_id = gdb_thread_id - THREAD_ID_OFFS;
	struct dbgmon_thread_inf inf;

	if (dbgmon_thread_inf_get(thread_id, &inf) < 0)
		return -1;

	inf.ctx->pc = addr;

	return 0;
}

int thread_step_req(unsigned int gdb_thread_id)
{
	unsigned int thread_id = gdb_thread_id - THREAD_ID_OFFS;
	struct dbgmon_thread_inf inf;

	if (thread_id == THINKOS_THREAD_IDLE) {
		/* thread is the IDLE try to get the first initialized 
		   thread instead. */
		thread_id = __thinkos_thread_getnext(-1);
	}

	if (dbgmon_thread_inf_get(thread_id, &inf) < 0)
		return -1;

	return dbgmon_thread_step(thread_id, false);
}

int thread_continue(unsigned int gdb_thread_id)
{
	unsigned int thread_id = gdb_thread_id - THREAD_ID_OFFS;

	if (thread_id >= THINKOS_THREADS_MAX)
		return -1;

	return __thinkos_thread_resume(thread_id);
}

int thread_info(unsigned int gdb_thread_id, char * buf)
{
	unsigned int thread_id = gdb_thread_id - THREAD_ID_OFFS;
	struct thinkos_context * ctx;
	struct dbgmon_thread_inf inf;
	char * cp = buf;
	int oid;
	int type;
	bool tmw;
	int n;

	if (thread_id > THINKOS_THREAD_VOID) {
		DCC_LOG(LOG_ERROR, "Invalid thread!");
		return -1;
	}

	if (dbgmon_thread_inf_get(thread_id, &inf) < 0)
		return -1;

	ctx = inf.ctx;

	if (thread_id == THINKOS_THREAD_IDLE) {
		cp += str2hex(cp, "IDLE");
	} else if (thread_id == THINKOS_THREAD_VOID) {
		int ipsr;
		ipsr = (ctx->xpsr & 0x1ff);
		cp += str2hex(cp, "IRQ");
		cp += int2str2hex(cp, ipsr - 16);
	} else {
#if THINKOS_ENABLE_THREAD_INFO
		if (thinkos_rt.th_inf[thread_id] != NULL)
			n = str2hex(cp, thinkos_rt.th_inf[thread_id]->tag);
		else
			n = int2str2hex(cp, thread_id + THREAD_ID_OFFS);
#else
		n = int2str2hex(cp, thread_id + THREAD_ID_OFFS);
#endif
		cp += n;
	}
	cp += char2hex(cp, ':');
	cp += char2hex(cp, ' ');

	if (thread_id == THINKOS_THREAD_IDLE) {
		oid = THINKOS_WQ_READY;
		tmw = false;
	} else if (thread_id == THINKOS_THREAD_VOID) {
		oid = THINKOS_WQ_READY;
		tmw = false;
	} else if (__thinkos_thread_isfaulty(thread_id)) {
		oid = THINKOS_WQ_FAULT;
		tmw = false;
	} else {
#if THINKOS_ENABLE_THREAD_STAT
		oid = thinkos_rt.th_stat[thread_id] >> 1;
		tmw = thinkos_rt.th_stat[thread_id] & 1;
#else
		oid = THINKOS_WQ_READY; /* FIXME */
		tmw = (thinkos_rt.wq_clock & (1 << thread_id)) ? true : false;
#endif
	}
	if (tmw) {
		if (oid == THINKOS_WQ_READY) {
			cp += str2hex(cp, "time wait");
		} else {
			cp += str2hex(cp, "timedwait on ");
		}
	} else {
		if (oid == THINKOS_WQ_FAULT) {
			struct thinkos_except * xcpt = __thinkos_except_buf();
			uint32_t bfsr;
			uint32_t ufsr;
			uint32_t mmfsr;

			switch (inf.errno) {
			case THINKOS_ERR_HARD_FAULT:
				cp += str2hex(cp, "Hard Fault ");
				break;
			case THINKOS_ERR_MEM_MANAGE:
				cp += str2hex(cp, "Memory Fault ");
				mmfsr = SCB_CFSR_MMFSR_GET(xcpt->cfsr);
				cp += str2hex(cp, "MMFSR=[");
				if (mmfsr & MMFSR_MMARVALID)
					cp += str2hex(cp, " MMARVALID");
				if (mmfsr & MMFSR_MLSPERR)
					cp += str2hex(cp, " MLSPERR");
				if (mmfsr & MMFSR_MSTKERR)
					cp += str2hex(cp, " MSTKERR");
				if (mmfsr & MMFSR_MUNSTKERR)
					cp += str2hex(cp, " MUNSTKERR");
				if (mmfsr & MMFSR_DACCVIOL)  
					cp += str2hex(cp, " DACCVIOL");
				if (mmfsr & MMFSR_IACCVIOL)  
					cp += str2hex(cp, " IACCVIOL");
				cp += str2hex(cp, " ] ");
				if (mmfsr & MMFSR_MMARVALID) {
					cp += str2hex(cp, " MMFAR=");
					cp += uint2hex2hex(cp, xcpt->mmfar);
				}
				break;
			case THINKOS_ERR_BUS_FAULT:
				cp += str2hex(cp, " Bus Fault ");
				bfsr = SCB_CFSR_BFSR_GET(xcpt->cfsr);
				cp += str2hex(cp, " BFSR=[");
				if (bfsr & BFSR_BFARVALID)  
					cp += str2hex(cp, " BFARVALID");
				if (bfsr & BFSR_LSPERR)
					cp += str2hex(cp, " LSPERR");
				if (bfsr & BFSR_STKERR)  
					cp += str2hex(cp, " STKERR");
				if (bfsr & BFSR_UNSTKERR)  
					cp += str2hex(cp, " UNSTKERR");
				if (bfsr & BFSR_IMPRECISERR)  
					cp += str2hex(cp, " IMPRECISERR");
				if (bfsr & BFSR_PRECISERR)
					cp += str2hex(cp, " PRECISERR");
				if (bfsr & BFSR_IBUSERR)  
					cp += str2hex(cp, " IBUSERR");
				cp += str2hex(cp, " ]");
				if (bfsr & BFSR_BFARVALID) {
					cp += str2hex(cp, " BFAR=");
					cp += uint2hex2hex(cp, xcpt->bfar);
				}
				break;
			case THINKOS_ERR_USAGE_FAULT: 
				cp += str2hex(cp, " Usage Fault ");
				ufsr = SCB_CFSR_UFSR_GET(xcpt->cfsr);
				cp += str2hex(cp, " UFSR=[");
				if (ufsr & UFSR_DIVBYZERO)  
					cp += str2hex(cp, " DIVBYZERO");
				if (ufsr & UFSR_UNALIGNED)  
					cp += str2hex(cp, " UNALIGNED");
				if (ufsr & UFSR_NOCP)  
					cp += str2hex(cp, " NOCP");
				if (ufsr & UFSR_INVPC)  
					cp += str2hex(cp, " INVPC");
				if (ufsr & UFSR_INVSTATE)  
					cp += str2hex(cp, " INVSTATE");
				if (ufsr & UFSR_UNDEFINSTR)  
					cp += str2hex(cp, " UNDEFINSTR");
				cp += str2hex(cp, " ]");
				break;
			default:
				cp += str2hex(cp, thinkos_err_name_lut[inf.errno]);
				cp += str2hex(cp, " [");
				cp += int2str2hex(cp, inf.errno);
				cp += str2hex(cp, " ]");
			}
		} else if (oid == THINKOS_WQ_READY) {
#if THINKOS_IRQ_MAX > 0
			if (thread_id != THINKOS_THREAD_IDLE) {
				int irq;
				for (irq = 0; irq < THINKOS_IRQ_MAX; ++irq) {
					if (thinkos_rt.irq_th[irq] == (int)thread_id) {
						break;
					}
				}
				if (irq < THINKOS_IRQ_MAX) {
					cp += str2hex(cp, "wait on IRQ[");
					cp += int2str2hex(cp, irq);
					cp += char2hex(cp, ']');
				} else
					cp += str2hex(cp, "ready");
			} else
#endif
			cp += str2hex(cp, "ready");
		} else {
			cp += str2hex(cp, "wait on ");
		}
	}
	if ((oid != THINKOS_WQ_READY) && (oid != THINKOS_WQ_FAULT)) {
		type = thinkos_obj_type_get(oid);
		if (type == THINKOS_OBJ_PAUSED) {
			DCC_LOG1(LOG_ERROR, "thread %d is paused!!!", thread_id);
		}
		cp += str2hex(cp, thinkos_type_name_lut[type]);
		cp += char2hex(cp, '[');
		cp += int2str2hex(cp, oid);
		cp += char2hex(cp, ']');
	}
	
	return cp - buf;
}

#define FLASH_BASE ((uint32_t)STM32_FLASH_MEM)

int target_mem_write(uint32_t addr, 
					 const void * ptr, unsigned int len)
{
	uint8_t * src = (uint8_t *)ptr;
	struct mem_blk blk;
	unsigned int cnt;
	unsigned int rem;

	rem = len;

	while (rem) {
		if (addr2block(this_board.memory.ram, addr, &blk)) {
			if (blk.ro) {
				DCC_LOG2(LOG_ERROR, "read only block addr=0x%08x size=%d", 
						 blk.addr, blk.size);
				return -1;
			}

			DCC_LOG2(LOG_TRACE, "RAM block addr=0x%08x size=%d", 
					 blk.addr, blk.size);

			cnt = blk.size - (addr - blk.addr);
			if (cnt > rem)
				cnt = rem;

			__thinkos_memcpy((void *)addr, src, cnt);
		} else if (addr2block(this_board.memory.flash, addr, &blk)) {
			uint32_t offs;

			if (blk.ro) {
				DCC_LOG2(LOG_ERROR, "read only block addr=0x%08x size=%d", 
						 blk.addr, blk.size);
				return -1;
			}

			DCC_LOG2(LOG_INFO, "FLASH block addr=0x%08x size=%d", 
					 blk.addr, blk.size);

			offs = addr - FLASH_BASE;
			if (blk.addr == addr) {
				DCC_LOG2(LOG_INFO, "block erase addr=0x%08x size=%d", 
						 blk.addr, blk.size);
				stm32_flash_erase(offs, blk.size);
			};

			cnt = blk.size - (addr - blk.addr);
			if (cnt > rem)
				cnt = rem;

			if (stm32_flash_write(offs, src, cnt) < 0) {
				DCC_LOG1(LOG_ERROR, "stm32_flash_write(0x%08x) failed!", addr);
				return -1;
			}

		} else if (addr2block(this_board.memory.periph, addr, &blk)) {
			/* flash */
			DCC_LOG2(LOG_TRACE, "PERIPH block addr=0x%08x size=%d", 
					 blk.addr, blk.size);

			cnt = blk.size - (addr - blk.addr);
			if (cnt > rem)
				cnt = rem;

			/* FIXME: 32bits access only */
			__thinkos_memcpy((void *)addr, src, cnt);
		} else {
			DCC_LOG1(LOG_ERROR, "invalid address 0x%08x", addr);
			return -1;
		}

		addr += cnt;
		src += cnt;
		rem -= cnt;
	}

	return len - rem;
}

int target_mem_erase(uint32_t addr, unsigned int len)
{
	struct mem_blk blk;

	if (addr2block(this_board.memory.flash, addr, &blk)) {
		uint32_t offs;

		if (blk.ro) {
			DCC_LOG2(LOG_ERROR, "read only block addr=0x%08x size=%d", 
					 blk.addr, blk.size);
			return -1;
		}

		DCC_LOG2(LOG_INFO, "FLASH block addr=0x%08x size=%d", 
				 blk.addr, blk.size);

		offs = addr - FLASH_BASE;
		if (blk.addr == addr) {
			DCC_LOG2(LOG_INFO, "block erase addr=0x%08x size=%d", 
					 blk.addr, blk.size);
		};
		stm32_flash_erase(offs, len);
	}

	return len;
}

int target_mem_read(uint32_t addr, void * ptr, unsigned int len)
{
	const struct mem_desc * mem;

	mem = dbgmon_mem_lookup(this_board.memory.lst, 3, addr);

	return dbgmon_mem_read(mem, addr, ptr, len);
}

/* -------------------------------------------------------------------------
 * Core auxiliarly functions
 * ------------------------------------------------------------------------- */

void target_halt(void)
{
	dbgmon_app_suspend();
}

bool target_continue(void)
{
	return dbgmon_app_continue();
}

int target_goto(uint32_t addr, int opt)
{
	return 0;
}

/* -------------------------------------------------------------------------
 * Core files
 * ------------------------------------------------------------------------- */

const char target_xml[] = 
"<target>\n"
"<architecture>arm</architecture>\n"
"<feature name=\"org.gnu.gdb.arm.m-profile\">\n"
"<reg name=\"r0\" bitsize=\"32\"/>\n"
"<reg name=\"r1\" bitsize=\"32\"/>\n"
"<reg name=\"r2\" bitsize=\"32\"/>\n"
"<reg name=\"r3\" bitsize=\"32\"/>\n"
"<reg name=\"r4\" bitsize=\"32\"/>\n"
"<reg name=\"r5\" bitsize=\"32\"/>\n"
"<reg name=\"r6\" bitsize=\"32\"/>\n"
"<reg name=\"r7\" bitsize=\"32\"/>\n"
"<reg name=\"r8\" bitsize=\"32\"/>\n"
"<reg name=\"r9\" bitsize=\"32\"/>\n"
"<reg name=\"r10\" bitsize=\"32\"/>\n"
"<reg name=\"r11\" bitsize=\"32\"/>\n"
"<reg name=\"r12\" bitsize=\"32\"/>\n"
"<reg name=\"sp\" bitsize=\"32\" type=\"data_ptr\"/>\n"
"<reg name=\"lr\" bitsize=\"32\"/>\n"
"<reg name=\"pc\" bitsize=\"32\" type=\"code_ptr\"/>\n"
"<reg name=\"xpsr\" bitsize=\"32\" regnum=\"25\"/>\n"
"</feature>\n"
#if THINKOS_ENABLE_FPU
"<feature name=\"org.gnu.gdb.arm.vfp\">\n"
"<reg name=\"d0\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d1\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d2\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d3\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d4\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d5\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d6\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d7\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d8\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d9\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d10\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d11\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d12\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d13\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d14\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"d15\" bitsize=\"64\" type=\"ieee_double\"/>\n"
"<reg name=\"fpscr\" bitsize=\"32\" type=\"int\" group=\"float\"/>\n"
"</feature>\n"
#endif
"</target>";


#if (GDB_ENABLE_QXFER_MEMORY_MAP) 
const char memory_map_xml[] = 
"<memory-map>"
"<memory type=\"flash\" start=\"0x8000000\" length=\"0x100000\">"
"<property name=\"blocksize\">2048</property>"
"</memory>"
"<memory type=\"flash\" start=\"0x8100000\" length=\"0x100000\">"
"<property name=\"blocksize\">2048</property>"
"</memory>"
"<memory type=\"ram\" start=\"0x20000000\" length=\"0x80000\"/>"
"<memory type=\"ram\" start=\"0x10000000\" length=\"0x10000\"/>"
"</memory-map>";
#endif

#if (GDB_ENABLE_QXFER_THREADS) 
const char threads_xml[] = 
"<?xml version=\"1.0\"?>"
"<threads>"
    "<thread id=\"%d\" core=\"0\" name=\"%s\">"
    "</thread>"
"</threads>"
#endif

int target_file_read(const char * name, char * dst, 
					  unsigned int offs, unsigned int size)
{
	char * src;
	unsigned int len;
	unsigned int cnt;
	unsigned int i;

	if (prefix(name, "target.xml")) {
		src = (char *)target_xml;
		len = sizeof(target_xml) - 1;
		DCC_LOG(LOG_TRACE, "target.xml");
#if (GDB_ENABLE_QXFER_MEMORY_MAP) 
	} else if (prefix(name, "memmap.xml")) {
		src = (char *)memory_map_xml;
		len = sizeof(memory_map_xml) - 1;
#endif
	} else
		return -1;

	if (offs >= len)
		return 0;

	DCC_LOG3(LOG_TRACE, "offs=%d len=%d size=%d", offs, len, size);

	cnt = len - offs;
	if (cnt > size)
		cnt = size;

	src += offs;

	for (i = 0; i < cnt; ++i)
		dst[i] = src[i];

	return cnt;
}

