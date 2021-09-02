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

#include "thinkos_krn-i.h"
#include <sys/dcclog.h>

#if ((THINKOS_FLASH_MEM_MAX) > 0)
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

#if 0
int32_t __attribute__((section (".data#"), noinline)) 
thinkos_flash_seq(intptr_t status, void * dev, struct flash_dev_seq * seq)
{
	struct cm3_systick * systick = CM3_SYSTICK;
	uint32_t ticks;

	ticks = krn->ticks; 
	status = seq->start(status, dev);
	while (status > 0) {
		if (systick->csr & SYSTICK_CSR_COUNTFLAG)
			ticks++;
		status = seq->status(status, dev);
	}
	krn->ticks = ticks; 

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

	start_ticks = krn->ticks; 

	pri = cm3_primask_get();
	cm3_primask_set(1);
	stat = thinkos_flash_seq(stat, dev->priv, &seq);
	cm3_primask_set(pri);

	dt = (int32_t)(krn->ticks - start_ticks);
	(void)dt;
	DCC_LOG1(LOG_INFO, "dt = %dms", dt);

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
	DCC_LOG1(LOG_INFO, "opc=%d!", opc);
#if DEBUG
	udelay(0x8000);
#endif

	off = req->offset;
	size = req->size;

	DCC_LOG2(LOG_INFO, "off=0x%08x size=%d", off, size);

	if (off > drv->partition.size) {
		DCC_LOG2(LOG_ERROR, "read off=%d > part.size=%d", 
				 off, drv->partition.size);
		return -THINKOS_EINVAL;
	}

	if ((off + size) > drv->partition.size)
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
			DCC_LOG2(LOG_INFO, "write off=0x%08x rem=%d", off, rem);
			ret = flash_dev_write(dev, off, ptr, rem);
			break;
		case THINKOS_FLASH_MEM_ERASE:
			DCC_LOG2(LOG_INFO, "erase off=0x%08x rem=%d", off, rem);
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

		if (ret > rem) {
			ret = rem; 
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
	struct thinkos_rt * krn = &thinkos_rt;
	struct thinkos_flash_drv * drv;

#if THINKOS_ENABLE_ARG_CHECK
	if ((idx > THINKOS_FLASH_MEM_MAX) || (desc == NULL))
		return -THINKOS_EINVAL;
#endif

	drv = &krn->flash_drv[idx];

	DCC_LOG2(LOG_INFO, "flash_drv_init(drv=%08x desc=%08x)...", drv, desc);

	/* Bind device driver and the memory descriptor */
	drv->dev = desc->dev;
	drv->mem = desc->mem;
	/* Initialize internal state... */
	drv->wopen = 0;
	drv->ropen = 0;

	/* TODO: Call the device driver initialization ... */
	return 0;	
}

#if (THINKOS_ENABLE_IDLE_HOOKS)
void thinkos_flash_drv_tasklet(struct thinkos_rt * krn,
							   unsigned int idx, struct thinkos_flash_drv * drv)
{
	unsigned int wq = THINKOS_FLASH_MEM_DESC(idx);
	struct flash_op_req * req;
	int th;
	int ret;

	th =__krn_wq_head(krn, wq);
	if (th != THINKOS_THREAD_NULL) {
		DCC_LOG2(LOG_YAP, "<%d> flash_drv tasklet r0=0x%08x", th + 1, 
				 __thread_r0_get(krn, th));

		req = (struct flash_op_req *)__thread_frame_get(krn, th);

		DCC_LOG1(LOG_INFO, "[req]=0x%08x", req);

		ret = thinkos_flash_drv_req(drv, req);

		/* wakeup from the flash wait queue */
		__wq_wakeup_return(krn, wq, th, ret);
		/* signal the scheduler ... */
		__krn_sched_defer(krn);
	} else {
		DCC_LOG(LOG_INFO, "flash_drv: no waiting threads!");
	}
}
#endif

void thinkos_flash_mem_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	struct thinkos_flash_drv * drv;
	struct flash_op_req * req;
	unsigned int opc;
	unsigned int idx;
	unsigned int wq;
	int ret = THINKOS_EINVAL;
	
	req = (struct flash_op_req *)arg;
	opc = req->opc;
	
	DCC_LOG2(LOG_INFO, "<%d> opc=%d", self, opc);

	if (opc == THINKOS_FLASH_MEM_OPEN) {

		for (idx = 0; idx < THINKOS_FLASH_MEM_MAX; ++idx) {
			const struct thinkos_mem_blk * blk;

			drv = &krn->flash_drv[idx];

			if ((blk = __mem_blk_lookup(drv->mem, req->tag)) != NULL) {
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
				ret = wq;

				DCC_LOG2(LOG_INFO, "open(tag=\"%s\") --> ret=%d", 
						 req->tag, ret);
				break;
			}
		}
		arg[0] = ret;
		return;
	} 
	
	if (opc == THINKOS_FLASH_MEM_STAT) {
		for (idx = 0; idx < THINKOS_FLASH_MEM_MAX; ++idx) {
			const struct thinkos_mem_blk * blk;
			const struct thinkos_mem_desc * mem;

			drv = &krn->flash_drv[idx];
			mem = drv->mem;

			if ((blk = __mem_blk_lookup(mem, req->tag)) != NULL) {
				struct thinkos_mem_part * mp;

				mp = req->mp;
				mp->begin = mem->base + blk->off;
				mp->end = mem->base + blk->off + (1 << blk->siz) * blk->cnt;
				mp->opt = blk->opt;
				mp->dev = mem->dev;
				mp->type = mem->typ;
				mp->perm = mem->opt;

				ret = THINKOS_OK;
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
		__THINKOS_ERROR(self, THINKOS_ERR_FLASH_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	drv = &krn->flash_drv[idx];

	if (opc == THINKOS_FLASH_MEM_CLOSE) {
		if (!(drv->ropen || drv->wopen)) {
			DCC_LOG(LOG_WARNING, "not open");
			arg[0] = THINKOS_EINVAL;
			return;
		}

		DCC_LOG2(LOG_INFO, "closing: wq=%d idx=%d", wq, idx);
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

	DCC_LOG2(LOG_INFO, "flash_drv: r0=%08x r1=%08x", arg[0], arg[1]);
	DCC_LOG2(LOG_INFO, "flash_drv: wq=%d idx=%d", wq, idx);

	DCC_LOG1(LOG_INFO, "flash_drv: [req]=0x%08x", req);
	DCC_LOG2(LOG_INFO, "flash_drv: off=0x%08x size=%d", 
			 req->offset, req->size);

#if (THINKOS_ENABLE_IDLE_HOOKS)
	/* schedule the IDLE hook ... */
	__idle_hook_req(IDLE_HOOK_FLASH_MEM0 + idx);
	/* insert into the flash wait queue */

	/* (2) Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
/* NEW: 2020-12-02 performed by the svc call entry stub 
	__thread_ctx_flush(krn, arg, self);
*/

	DCC_LOG1(LOG_INFO, "<%2d> sleeping...", self);

	/* wait for event ... */
	__krn_thread_wait(krn, self, wq);
#else
	ret = thinkos_flash_drv_req(drv, req);
	arg[0] = ret;
#endif
}

#endif /* (THINKOS_FLASH_MEM_MAX > 0) */


