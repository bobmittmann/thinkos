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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_IDLE__
#include <thinkos/idle.h>
/* FIXME: platform memory map should move from MONITOR and bootloader 
   to kernel... */
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>

#define __THINKOS_FLASH__
#include <thinkos/flash.h>

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Os\")")
#endif
#include <thinkos.h>

#include <sys/param.h>
#include <sys/dcclog.h>
#include <stdbool.h>

#if 0
int32_t genkey_lcg24(int32_t key)
{
	key = (key * 1140671485 + 12820163) & 0x00ffffff;

    return key;
}
#endif

static uint16_t genkey_lcg16(uint16_t key)
{
	key = key * 25385 + 25385;
    return key;
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

static inline uint64_t mem_blk_hash(const struct blk_desc * blk)
{
	return *(uint64_t *)blk->tag;
}

static const struct blk_desc * mem_blk_lookup(const struct mem_desc * mem, 
											  const char * tag)
{
	const struct blk_desc * blk = mem->blk;
	uint64_t hash = tag2hash(tag);
	int i;

	DCC_LOG2(LOG_YAP, "hash=%08x%08x", hash >> 32, hash);

	for (i = 0; i < mem->cnt; ++i) {
		uint64_t cmp = mem_blk_hash(&blk[i]);
		DCC_LOG2(LOG_YAP, "cmp=%08x%08x", cmp >> 32, cmp);
		if (hash == cmp) {
			return &blk[i];
		}
	}

	return NULL;
}


/*
   Process FLASH memory requests.
   These can be a request either from the kernel or from user
   space / notification of completion in case interrups are being used.

	FIXME: add support for interrupts

   */

int thinkos_flash_drv_req(struct thinkos_flash_drv * drv, 
						  const struct thinkos_flash_desc * desc,
						  struct flash_op_req * req)
{
	const struct flash_dev * dev = desc->dev;
	const struct mem_desc * mem = desc->mem;
	const struct blk_desc * blk;
	uint32_t off;
	uint32_t size;
	uint8_t * ptr;
	int32_t rem;
	int opc;

	opc = req->opc;
	DCC_LOG1(LOG_TRACE, "opc=%d!", opc);

	if (opc == THINKOS_FLASH_MEM_OPEN) {
		int ret;

		if ((blk = mem_blk_lookup(mem, req->tag)) == NULL) {
			DCC_LOG(LOG_WARNING, "memory block not found");
			return THINKOS_EINVAL;
		}
		ret = genkey_lcg16(drv->key);
		drv->key = ret;
		drv->partition.offset = blk->off;
		drv->partition.size = (1 << blk->siz) * blk->cnt;
		drv->wopen = (blk->opt & M_RO) ? 0 : 1;
		drv->ropen = 1;
		//		asm("movs r3, #1\n"
		//			"1: cmp r3, #0\n"
		//			"bne 1b\n" : : : "r3");
		DCC_LOG2(LOG_TRACE, "open(tag=\"%s\") --> key=%d", req->tag, ret);
		return ret;
	} 

	if (req->key != drv->key) {
		return THINKOS_EINVAL;
	}

	if (opc == THINKOS_FLASH_MEM_CLOSE) {
		drv->ropen = 0;
		drv->wopen = 0;
		return THINKOS_OK;
	}

	off = req->offset;
	size = req->size;

	if (off > drv->partition.size)
		return THINKOS_EINVAL;

	if (off + size > drv->partition.size)
		size = drv->partition.size - off;

	off += drv->partition.offset;
	rem = size; 
	ptr = (uint8_t *)req->buf;

	while (rem > 0) {
		int ret = 0;

		switch (opc) {
		case THINKOS_FLASH_MEM_READ:
			DCC_LOG2(LOG_YAP, "read off=0x%08x rem=%d", off, rem);
			ret = flash_dev_read(dev, off, ptr, rem);
			break;
		case THINKOS_FLASH_MEM_WRITE:
			DCC_LOG2(LOG_YAP, "write off=0x%08x rem=%d", off, rem);
			ret = flash_dev_write(dev, off, ptr, rem);
			break;
		case THINKOS_FLASH_MEM_ERASE:
			DCC_LOG2(LOG_YAP, "erase off=0x%08x rem=%d", off, rem);
			ret = flash_dev_erase(dev, off, rem);
			break;
		case THINKOS_FLASH_MEM_LOCK:
			DCC_LOG2(LOG_YAP, "lock off=0x%08x rem=%d", off, rem);
			ret = flash_dev_lock(dev, off, rem);
			break;
		case THINKOS_FLASH_MEM_UNLOCK:
			DCC_LOG2(LOG_YAP, "unlock off=0x%08x rem=%d", off, rem);
			ret = flash_dev_unlock(dev, off, rem);
			break;
		}

		if (ret < 0) {
			break;
		}

		off += ret;
		ptr += ret;
		rem -= ret;
	}

	return size - rem;
}

void thinkos_flash_drv_init(struct thinkos_flash_drv * drv, 
							const struct thinkos_flash_desc * desc)
{
	DCC_LOG(LOG_TRACE, "flash_drv init...");

	drv->wopen = 0;
	drv->ropen = 0;
	drv->krn_req.opc = THINKOS_FLASH_MEM_NOP;
}

#if (THINKOS_ENABLE_FLASH_MEM)
void thinkos_flash_drv_tasklet(struct thinkos_flash_drv * drv, 
							   const struct thinkos_flash_desc * desc)
{
	struct flash_op_req * req;

#if (THINKOS_ENABLE_MONITOR)
	/* Check for pending request from the kernel */
	if (drv->krn_req.opc != THINKOS_FLASH_MEM_NOP) {
			req = &drv->krn_req;

			DCC_LOG1(LOG_TRACE, "flash_drv tasklet kernel req=%08x .", req);

			req->ret = thinkos_flash_drv_req(drv, desc, req);
			req->opc = THINKOS_FLASH_MEM_NOP;
			/* Notify the debug/monitor */
			monitor_signal(MONITOR_FLASH_DRV); 
			return;
	}
#endif 
	{
		unsigned int wq = THINKOS_WQ_FLASH_MEM;
		int th = __thinkos_wq_head(wq);
		int ret;

		if (th != THINKOS_THREAD_NULL) {
			int opc;

			req = (struct flash_op_req *)&thinkos_rt.ctx[th]->r0;
			opc = req->opc;
			DCC_LOG2(LOG_TRACE, "<%d> flash_drv tasklet r0=0x%08x", th + 1, 
					 thinkos_rt.ctx[th]->r0);

			if ((opc == THINKOS_FLASH_MEM_OPEN) && (drv->ropen || drv->wopen)) {
				DCC_LOG(LOG_WARNING, "already open");
				ret = THINKOS_EINVAL;
			} else if ((opc == THINKOS_FLASH_MEM_CLOSE) && 
					   !(drv->ropen || drv->wopen)) {
				DCC_LOG(LOG_WARNING, "not open");
				ret = THINKOS_EINVAL;
			} else if ((opc == THINKOS_FLASH_MEM_READ) && (!drv->ropen)) {
				DCC_LOG(LOG_WARNING, "not open for read");
				ret = THINKOS_EPERM;
			} else if (!drv->wopen) {
				DCC_LOG(LOG_WARNING, "not open for write");
				ret = THINKOS_EPERM;
			} else {
				ret = thinkos_flash_drv_req(drv, desc, req);
			}

			/* wakeup from the mutex wait queue */
			__thinkos_wakeup_return(wq, th, ret);
			/* signal the scheduler ... */
			__thinkos_defer_sched();
		} else {
			DCC_LOG(LOG_YAP, "flash_drv: no waiting threads!");
		}
	}
}

void thinkos_flash_mem_svc(int32_t * arg, int self)
{
	unsigned int wq = THINKOS_WQ_FLASH_MEM;
	
#if THINKOS_ENABLE_ARG_CHECK
#endif
	/* schedule the IDLE hook ... */
	__idle_hook_req(IDLE_HOOK_FLASH_MEM);
	/* insert into the flash wait queue */
	__thinkos_wq_insert(wq, self);
	/* (1) suspend the thread by removing it from the
	   ready wait queue. The __thinkos_suspend() call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access 
	   itself, in case we have enabled the time sharing option. */
	__thinkos_suspend(self);
	/* (2) Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
	thinkos_rt.ctx[self] = (struct thinkos_context *)&arg[-CTX_R0];
	/* signal the scheduler ... */
	__thinkos_defer_sched();

	asm("movs r3, #1\n"
		"1: cmp r3, #0\n"
		"bne 1b\n" : : : "r3");
}

const char thinkos_flash_mem_nm[] = "FMM";

#endif /* THINKOS_ENABLE_FLASH_MEM */


