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

#include <thinkos.h>
#include <stdbool.h>
#include <sys/param.h>
#include <sys/delay.h>

#include <sys/dcclog.h>

#if (THINKOS_FLASH_MEM_MAX > 0)
#if 0
int32_t genkey_lcg24(int32_t key)
{
	key = (key * 1140671485 + 12820163) & 0x00ffffff;

    return key;
}
#endif

#if 0
static uint16_t genkey_lcg16(uint16_t key)
{
	key = key * 25385 + 25385;
    return key;
}
#endif

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
	return blk->hash;
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

#if 0
int32_t __attribute__((section (".data#"), noinline)) 
thinkos_flash_seq(intptr_t status, void * dev, struct flash_dev_seq * seq)
{
	struct cm3_systick * systick = CM3_SYSTICK;
	uint32_t ticks;

	ticks = thinkos_rt.ticks; 
	status = seq->start(status, dev);
	while (status > 0) {
		if (systick->csr & SYSTICK_CSR_COUNTFLAG)
			ticks++;
		status = seq->status(status, dev);
	}
	thinkos_rt.ticks = ticks; 

	status = seq->finish(status, dev);
	return status;
}

int thinkos_flash_drv_erase(const struct flash_dev * dev, off_t 
							off, size_t count)
{
	struct flash_dev_seq seq;
	intptr_t stat;
	uint32_t pri;
	uint32_t start_ticks;
	int32_t dt;

	seq = *dev->op->erase.seq;
	stat = flash_dev_erase_init(dev, off, count);

	if (stat < 0)
		return stat;

	start_ticks = thinkos_rt.ticks; 

	pri = cm3_primask_get();
	cm3_primask_set(1);
	stat = thinkos_flash_seq(stat, dev->priv, &seq);
	cm3_primask_set(pri);

	dt = (int32_t)(thinkos_rt.ticks - start_ticks);
	(void)dt;
	DCC_LOG1(LOG_TRACE, "dt = %dms", dt);

	/* FIXME: experimental */
	return count;
}
#endif

/*
   Process FLASH memory requests.
   These can be a request either from the kernel or from user
   space / notification of completion in case interrups are being used.

	FIXME: add support for interrupts

   */

int thinkos_flash_drv_req(struct thinkos_flash_drv * drv, 
								 struct flash_op_req * req)
{
	const struct flash_dev * dev = drv->dev;
	uint32_t off;
	uint32_t size;
	uint8_t * ptr;
	int32_t rem;
	int opc;

	opc = req->opc;
	DCC_LOG1(LOG_TRACE, "opc=%d!", opc);
#if DEBUG
	udelay(0x8000);
#endif

	off = req->offset;
	size = req->size;

	if (off > drv->partition.size) {
		DCC_LOG2(LOG_ERROR, "read off=%d > part.size=%d", 
				 off, drv->partition.size);
		return -THINKOS_EINVAL;
	}

	if (off + size > drv->partition.size)
		size = drv->partition.size - off;

	off += drv->partition.offset;
	rem = size; 
	ptr = (uint8_t *)req->buf;

	while (rem > 0) {
		int ret = 0;

		switch (opc) {
		case THINKOS_FLASH_MEM_READ:
			DCC_LOG2(LOG_INFO, "read off=0x%08x rem=%d", off, rem);
			ret = flash_dev_read(dev, off, ptr, rem);
			break;
		case THINKOS_FLASH_MEM_WRITE:
			DCC_LOG2(LOG_TRACE, "write off=0x%08x rem=%d", off, rem);
			ret = flash_dev_write(dev, off, ptr, rem);
			break;
		case THINKOS_FLASH_MEM_ERASE:
			DCC_LOG2(LOG_TRACE, "erase off=0x%08x rem=%d", off, rem);
			ret = flash_dev_erase(dev, off, rem);
#if 0
			ret = thinkos_flash_drv_erase(dev, off, rem);
#endif
			break;
		case THINKOS_FLASH_MEM_LOCK:
			DCC_LOG2(LOG_INFO, "lock off=0x%08x rem=%d", off, rem);
			ret = flash_dev_lock(dev, off, rem);
			break;
		case THINKOS_FLASH_MEM_UNLOCK:
			DCC_LOG2(LOG_INFO, "unlock off=0x%08x rem=%d", off, rem);
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

int thinkos_flash_drv_init(unsigned int idx, 
						   const struct thinkos_flash_desc * desc)
{
	struct thinkos_flash_drv * drv;

#if THINKOS_ENABLE_ARG_CHECK
	if ((idx > THINKOS_FLASH_MEM_MAX) || (desc == NULL))
		return -THINKOS_EINVAL;
#endif

	drv = &thinkos_rt.flash_drv[idx];

	DCC_LOG2(LOG_TRACE, "flash_drv_init(drv=%08x desc=%08x)...", drv, desc);

	/* Bind device driver and the memory descriptor */
	drv->dev = desc->dev;
	drv->mem = desc->mem;
	/* Initialize internal state... */
	drv->wopen = 0;
	drv->ropen = 0;

	/* TODO: Call the device driver initialization ... */
	return 0;	
}

void thinkos_flash_drv_tasklet(unsigned int idx, struct thinkos_flash_drv * drv)
{
	unsigned int wq = THINKOS_FLASH_MEM_DESC(idx);
	struct flash_op_req * req;
	int th;
	int ret;

	th = __thinkos_wq_head(wq);
	if (th != THINKOS_THREAD_NULL) {
		DCC_LOG2(LOG_YAP, "<%d> flash_drv tasklet r0=0x%08x", th + 1, 
				 __thinkos_thread_r0_get(th));

		req = (struct flash_op_req *)__thinkos_thread_frame_get(th);

		ret = thinkos_flash_drv_req(drv, req);

		/* wakeup from the flash wait queue */
		__thinkos_wakeup_return(wq, th, ret);
		/* signal the scheduler ... */
		__thinkos_defer_sched();
	} else {
		DCC_LOG(LOG_TRACE, "flash_drv: no waiting threads!");
	}
}

void thinkos_flash_mem_svc(int32_t arg[], int self)
{
	struct thinkos_flash_drv * drv;
	struct flash_op_req * req;
	unsigned int opc;
	unsigned int idx;
	unsigned int wq;
	int ret = THINKOS_EINVAL;
	
	req = (struct flash_op_req *)arg;
	opc = req->opc;
	
	DCC_LOG2(LOG_TRACE, "<%d> opc=%d", self + 1, opc);

	if (opc == THINKOS_FLASH_MEM_OPEN) {

		for (idx = 0; idx < THINKOS_FLASH_MEM_MAX; ++idx) {
			const struct blk_desc * blk;

			drv = &thinkos_rt.flash_drv[idx];

			if ((blk = mem_blk_lookup(drv->mem, req->tag)) != NULL) {
				if (drv->ropen || drv->wopen) {
					ret = THINKOS_EBUSY;
					break;
				}
				wq = THINKOS_FLASH_MEM_DESC(idx);
				drv->partition.offset = blk->off;
				drv->partition.size = (1 << blk->siz) * blk->cnt;
				drv->wopen = (blk->opt & M_RO) ? 0 : 1;
				drv->ropen = 1;
#if 0
				key = genkey_lcg16(drv->key);
				drv->key = key;
#endif
				DCC_LOG2(LOG_TRACE, "open(tag=\"%s\") --> ret=%d", 
						 req->tag, ret);
				ret = wq;
				break;
			}
		}
		arg[0] = ret;
		return;
	} 

	wq = req->wq;
	idx = wq - THINKOS_FLASH_MEM_BASE;

#if THINKOS_ENABLE_ARG_CHECK
	if (idx >= THINKOS_FLASH_MEM_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid flash %d!", wq);
		__THINKOS_ERROR(THINKOS_ERR_FLASH_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	drv = &thinkos_rt.flash_drv[idx];

	if (opc == THINKOS_FLASH_MEM_CLOSE) {
		if (!(drv->ropen || drv->wopen)) {
			DCC_LOG(LOG_WARNING, "not open");
			arg[0] = THINKOS_EINVAL;
			return;
		}

		DCC_LOG2(LOG_TRACE, "closing: wq=%d idx=%d", wq, idx);
		/* 
         * TODO: validate operation...
		 */
		drv->wopen = 0;
		drv->ropen = 0;
		arg[0] = THINKOS_OK;
		return;
	} 

	if ((opc == THINKOS_FLASH_MEM_READ) && (!drv->ropen)) {
		DCC_LOG(LOG_WARNING, "not open for read");
		arg[0] = THINKOS_EPERM;
		return;
	} 

	if ((opc == THINKOS_FLASH_MEM_WRITE) && (!drv->wopen)) {
		DCC_LOG(LOG_WARNING, "not open for write");
		arg[0] = THINKOS_EPERM;
		return;
	} 

	DCC_LOG2(LOG_TRACE, "flash_drv: r0=%08x r1=%08x", arg[0], arg[1]);
	DCC_LOG2(LOG_TRACE, "flash_drv: wq=%d idx=%d", wq, idx);

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
	__thinkos_thread_ctx_flush(arg, self);

	/* signal the scheduler ... */
	__thinkos_defer_sched();
}

const char thinkos_flash_mem_nm[] = "FMM";

#endif /* (THINKOS_FLASH_MEM_MAX > 0) */

