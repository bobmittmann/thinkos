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
/* FIXME: platform memory map should move from DBGMON and bootloader 
   to kernel... */
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>

#define __THINKOS_FLASH__
#include <thinkos/flash.h>

#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Os\")")
#endif
#include <thinkos.h>

#include <sys/flash-dev.h>
#include <sys/param.h>
#include <sys/dcclog.h>
#include <stdbool.h>


#if (THINKOS_ENABLE_FLASH_MEM)

extern const struct flash_dev * board_flash_dev;

struct flash_op_req {
	volatile uint32_t seq;
	volatile uint32_t ack;
	uint16_t req;
	uint16_t mem;
	uint16_t off;
	uint16_t len;
	void * buf;
	uint16_t sector;
	uint16_t block;
	int ret;
};

/*
*/

struct thinkos_flash_drv {
	struct thinkos_flash_dev * dev;
#if (THINKOS_ENABLE_MONITOR)
	struct flash_op_req krn_req;
#endif
};

struct thinkos_flash_drv thinkos_flash_drv_rt;

/*
   Process FLASH memory events.
   These events can be a request either from the kernel or from user
   space / notification of completion in case interrups are being used.

	FIXME: add support for interrupts

   */
void thinkos_flash_mem_hook(void)
{
	struct thinkos_flash_drv * drv = &thinkos_flash_drv_rt;
	const struct flash_dev * dev = board_flash_dev;
//	const struct thinkos_flash_memory  * mem;
	const struct flash_dev_ops * op = dev->op;
	void * ptr = dev->priv;
	unsigned int wq = THINKOS_WQ_FLASH_MEM;
	unsigned int req;
	int mem_id;
	int th;
	void * buf;
	off_t off;
	size_t len;
	int ret;

	(void)drv;

	th = __thinkos_wq_head(wq);

#if (THINKOS_ENABLE_MONITOR)
	/* Check for pending request from the kernel */
	if (drv->krn_req.seq != drv->krn_req.ack) {
			req = drv->krn_req.req;
			mem_id = drv->krn_req.mem;
			buf =  drv->krn_req.buf;
			off = drv->krn_req.off;
			len = drv->krn_req.len;
	} else 
#endif
	if (th != THINKOS_THREAD_NULL) {
			uint32_t * arg;
			arg = &thinkos_rt.ctx[th]->r0;
			req = arg[0];
			mem_id = arg[1];
			buf = (void *)arg[2];
			off = (off_t)arg[2];
			len = (size_t)arg[3];
	} else {
		DCC_LOG(LOG_WARNING, "flash_drv: no waiting threads!");
		return;
	}

//	mem = &board_flash_mem[mem_id];
	(void)mem_id;

	switch (req) {
	case THINKOS_FLASH_MEM_OPEN:
		DCC_LOG1(LOG_TRACE, "<%d> open", th + 1);
		ret = THINKOS_EINVAL;
		break;

	case THINKOS_FLASH_MEM_CLOSE:
		DCC_LOG1(LOG_TRACE, "<%d> close", th + 1);
		ret = THINKOS_EINVAL;
		break;

	case THINKOS_FLASH_MEM_READ:
		DCC_LOG1(LOG_TRACE, "<%d> read", th + 1);
		ret = THINKOS_EINVAL;
		break;

	case THINKOS_FLASH_MEM_WRITE:
		DCC_LOG1(LOG_TRACE, "<%d> write", th + 1);
		ret = op->write(ptr, off, buf, len);
		break;

	case THINKOS_FLASH_MEM_ERASE:
		DCC_LOG1(LOG_TRACE, "<%d> erase", th + 1);
		ret = op->erase(ptr, off, len);
		break;

	case THINKOS_FLASH_MEM_LOCK:
		DCC_LOG1(LOG_TRACE, "<%d> lock", th + 1);
		ret = op->lock(ptr, off, len);
		break;

	case THINKOS_FLASH_MEM_UNLOCK:
		DCC_LOG1(LOG_TRACE, "<%d> unlock", th + 1);
		ret = op->unlock(ptr, off, len);
		break;

	default:
		ret = THINKOS_EINVAL;
	}

#if (THINKOS_ENABLE_MONITOR)
	if (drv->krn_req.seq != drv->krn_req.ack) {
			drv->krn_req.ack++;
			drv->krn_req.ret = ret;
			/* Notify the debug/monitor */
			dbgmon_signal(DBGMON_FLASH_DRV); 
	} else 
#endif
	if (th != THINKOS_THREAD_NULL) {
		uint32_t * arg;

		arg = &thinkos_rt.ctx[th]->r0;
		arg[0] = ret;
		/* wakeup from the mutex wait queue */
		__thinkos_wakeup(wq, th);
		/* signal the scheduler ... */
		__thinkos_defer_sched();
	}
}

void thinkos_flash_mem_svc(int32_t * arg, int self)
{
	unsigned int wq = THINKOS_WQ_FLASH_MEM;
	
#if THINKOS_ENABLE_ARG_CHECK
#endif

	/* insert into the flash wait queue */
	__thinkos_wq_insert(wq, self);
	DCC_LOG2(LOG_TRACE , "<%d> waiting on flash mem %d...", self, wq);

	/* (1) suspend the thread by removing it from the
	   ready wait queue. The __thinkos_suspend() call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access 
	   itself, in case we have enabled the time sharing option. */
	__thinkos_suspend(self);
	/* (2) Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
	thinkos_rt.ctx[self] = (struct thinkos_context *)&arg[-CTX_R0];

	/* signal the  ... */
	__idle_hook_req(IDLE_HOOK_FLASH_MEM);
	/* signal the scheduler ... */
	__thinkos_defer_sched();
}

const char thinkos_flash_mem_nm[] = "FMM";

#endif /* THINKOS_ENABLE_FLASH_MEM */

