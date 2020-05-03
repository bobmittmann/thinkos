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
/* FIXME: platform memory map should move from DBGMON and bootloader 
   to kernel... */
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

#define __THINKOS_FLASH__
#include <thinkos/flash.h>

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Os\")")
#endif
#include <thinkos.h>

#include <sys/param.h>
#include <sys/dcclog.h>
#include <stdbool.h>

#if (THINKOS_ENABLE_FLASH_MEM)

int32_t genkey_lcg24(int32_t key)
{
	key = (key * 1140671485 + 12820163) & 0x00ffffff;

    return key;
}

uint16_t genkey_lcg16(uint16_t key)
{
	key = key * 25385 + 25385;
    return key;
}

uint64_t tag2hash(const char * tag)
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

const struct blk_desc * mem_blk_lookup(const struct mem_desc * mem, const char * tag)
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

void write_fault(void)
{
	volatile uint32_t * ptr = (uint32_t *)(0x0);
	uint32_t x = 0;
	int i;

	for (i = 0; i < (16 << 4); ++i) {
		*ptr = x;
		ptr += 0x10000000 / (2 << 4);
	}
}


/*
   Process FLASH memory requests.
   These can be a request either from the kernel or from user
   space / notification of completion in case interrups are being used.

	FIXME: add support for interrupts

   */

static int thinkos_flash_drv_req(struct thinkos_flash_drv * drv, 
								 const struct thinkos_flash_desc * desc,
								 struct flash_op_req * req)
{
	const struct mem_desc * mem = desc->mem;
	const struct flash_dev * dev = desc->dev;
	const struct flash_dev_ops * op = dev->op;
	const struct blk_desc * blk;
	void * ptr = dev->priv;
	int ret;

	switch (req->opc) {
	case THINKOS_FLASH_MEM_OPEN:
		if ((blk = mem_blk_lookup(mem, req->tag)) != NULL) {
			DCC_LOG1(LOG_WARNING, "<%d> memory block not found", th + 1);
			ret = THINKOS_EINVAL;
			break;
		}
		if ((drv->ropen) || (drv->wopen)) {
			DCC_LOG1(LOG_WARNING, "<%d> already open", th + 1);
			ret = THINKOS_EINVAL;
			break;
		}
		ret = genkey_lcg16(drv->key);
		drv->key = ret;
		drv->off = blk->off;
		drv->size = (1 << blk->siz) * blk->cnt;
		drv->pos = 0;
		drv->wopen = (blk->opt & M_RO) ? 0 : 1;
		drv->ropen = 1;
		//		asm("movs r3, #1\n"
		//			"1: cmp r3, #0\n"
		//			"bne 1b\n" : : : "r3");
		DCC_LOG3(LOG_TRACE, "<%d> open(tag=\"%s\") --> key=%d", th + 1, req->tag, ret);
		break;

	case THINKOS_FLASH_MEM_CLOSE:
		DCC_LOG2(LOG_TRACE, "<%d> close(key=%d)", th + 1, req->key);
		if ((req->key == drv->key) && (drv->ropen || drv->wopen)) {
			drv->ropen = 0;
			drv->wopen = 0;
			ret = 0;
		} else {
			ret = THINKOS_EINVAL;
		}
		break;

	case THINKOS_FLASH_MEM_SEEK:
		DCC_LOG3(LOG_TRACE, "<%d> seek(key=%d offset=%d)", th + 1, req->key, req->size);
		if ((req->key == drv->key) && (drv->ropen || drv->wopen)) {
			size_t size = req->size;

			if (size > drv->size)
				size = drv->size;
			drv->pos = size;
			ret = size;
		} else {
			ret = THINKOS_EINVAL;
		}
		break;

	case THINKOS_FLASH_MEM_READ:
		DCC_LOG3(LOG_TRACE, "<%d> read(key=%d len=%d)", th + 1, req->key, req->size);
		if ((req->key == drv->key) && drv->ropen) {
			size_t size = req->size;
			off_t off;

			off = drv->off + drv->pos;
			if (drv->pos + size > drv->size)
				size = drv->size - drv->pos;
			ret = op->read(ptr, off, req->buf, size);
			if (ret > 0)
				drv->pos += ret;
		} else {
			DCC_LOG1(LOG_WARNING, "<%d> memory block invalid", th + 1);
			ret = THINKOS_EINVAL;
		}
		break;

	case THINKOS_FLASH_MEM_WRITE:
		DCC_LOG3(LOG_TRACE, "<%d> write(key=%d len=%d)", th + 1, req->key, req->size);
		if ((req->key == drv->key) && drv->wopen) {
			size_t size = req->size;
			off_t off;

			off = drv->off + drv->pos;
			if (drv->pos + size > drv->size)
				size = drv->size - drv->pos;
			ret = op->write(ptr, off, req->buf, size);
			if (ret > 0)
				drv->pos += ret;
		} else {
			DCC_LOG1(LOG_WARNING, "<%d> memory block invalid", th + 1);
			ret = THINKOS_EINVAL;
		}
		break;

	case THINKOS_FLASH_MEM_ERASE:
		DCC_LOG2(LOG_TRACE, "<%d> erase %d", th + 1, req->key);
		if ((req->key == drv->key) && drv->wopen) {
			size_t size = req->size;
			off_t off;

			off = drv->off + drv->pos;
			if (drv->pos + size > drv->size)
				size = drv->size - drv->pos;
			ret = op->erase(ptr, off, size);
			if (ret > 0)
				drv->pos += ret;
		} else {
			DCC_LOG1(LOG_WARNING, "<%d> memory block invalid", th + 1);
			ret = THINKOS_EINVAL;
		}
		break;

	case THINKOS_FLASH_MEM_LOCK:
		DCC_LOG2(LOG_TRACE, "<%d> lock", th + 1, req->key);
		if ((req->key == drv->key) && drv->wopen) {
			size_t size = req->size;
			off_t off;

			off = drv->off + drv->pos;
			if (drv->pos + size > drv->size)
				size = drv->size - drv->pos;
			ret = op->lock(ptr, off, size);
			break;
		} else {
			DCC_LOG1(LOG_WARNING, "<%d> memory block invalid", th + 1);
			ret = THINKOS_EINVAL;
		}
		break;

	case THINKOS_FLASH_MEM_UNLOCK:
		DCC_LOG2(LOG_TRACE, "<%d> unlock", th + 1, req->key);
		if ((req->key == drv->key) && drv->wopen) {
			size_t size = req->size;
			off_t off;

			off = drv->off + drv->pos;
			if (drv->pos + size > drv->size)
				size = drv->size - drv->pos;
			ret = op->unlock(ptr, off, size);
		} else {
			DCC_LOG1(LOG_WARNING, "<%d> memory block invalid", th + 1);
			ret = THINKOS_EINVAL;
		}
		break;

	default:
		ret = THINKOS_EINVAL;
	}

	return ret;
}

void thinkos_flash_drv_tasklet(struct thinkos_flash_drv * drv, 
							   const struct thinkos_flash_desc * desc)
{
	unsigned int wq = THINKOS_WQ_FLASH_MEM;
	struct flash_op_req * req;
	int ret;
	int th;

	th = __thinkos_wq_head(wq);

#if (THINKOS_ENABLE_MONITOR)
	/* Check for pending request from the kernel */
	if (drv->krn_req.opc != THINKOS_FLASH_MEM_NOP) {
			req = &drv->krn_req;
			req->ret = thinkos_flash_drv_req(drv, desc, req);
			req->opc = THINKOS_FLASH_MEM_NOP;
			/* Notify the debug/monitor */
			dbgmon_signal(DBGMON_FLASH_DRV); 
	} else 
#endif
	if (th != THINKOS_THREAD_NULL) {
			req = (struct flash_op_req *)&thinkos_rt.ctx[th]->r0;
			ret = thinkos_flash_drv_req(drv, desc, req);
			/* wakeup from the mutex wait queue */
			__thinkos_wakeup_return(wq, th, ret);
			/* signal the scheduler ... */
			__thinkos_defer_sched();
	} else {
		DCC_LOG(LOG_YAP, "flash_drv: no waiting threads!");
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

//	asm("movs r3, #1\n"
//		"1: cmp r3, #0\n"
//		"bne 1b\n" : : : "r3");
}

const char thinkos_flash_mem_nm[] = "FMM";

#endif /* THINKOS_ENABLE_FLASH_MEM */

