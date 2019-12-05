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
#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Os\")")
#endif
#include <thinkos.h>

#include <sys/param.h>
#include <stdbool.h>

#if (THINKOS_ENABLE_FLASH_MEM)


void thinkos_flash_mem_hook(void)
{
	unsigned int wq = THINKOS_WQ_FLASH_MEM;
	uint32_t * arg;
	int th;
	unsigned int req;
	int mem;
	void * buf;
	off_t off;
	size_t len;

	if ((th = __thinkos_wq_head(wq)) == THINKOS_THREAD_NULL) {
		return;
	}

#if THINKOS_ENABLE_ARG_CHECK
#endif
	arg = &thinkos_rt.ctx[th]->r0;
	req = arg[0];
	mem = arg[1];
	buf = (void *)arg[2];
	off= (off_t)arg[2];
	len = (size_t)arg[3];

//	clk = thinkos_clock();
//	__stm32_flash_erase(0x40000, 0x20000);
//	DCC_LOG2(LOG_TRACE, "<%d> dt=%d", th + 1, 
//			 thinkos_clock() - clk);

//	DCC_LOG2(LOG_TRACE, "<%d> flash mem req=%d", th + 1, req);
//	__stm32_flash_write(0x40000, "Test", 100);
//	DCC_LOG2(LOG_TRACE, "<%d> flash mem req=%d", th + 1, req);

	switch (req) {
	case THINKOS_FLASH_MEM_OPEN:
		DCC_LOG1(LOG_TRACE, "<%d> open", th + 1);
		arg[0] = board_flash_mem_open(mem);
		break;

	case THINKOS_FLASH_MEM_CLOSE:
		DCC_LOG1(LOG_TRACE, "<%d> close", th + 1);
		arg[0] = board_flash_mem_close(mem);
		break;

	case THINKOS_FLASH_MEM_READ:
		DCC_LOG1(LOG_TRACE, "<%d> read", th + 1);
		arg[0] = board_flash_mem_read(mem, buf, len);
		break;

	case THINKOS_FLASH_MEM_WRITE:
		DCC_LOG1(LOG_TRACE, "<%d> write", th + 1);
		arg[0] = board_flash_mem_write(mem, buf, len);
		break;

	case THINKOS_FLASH_MEM_SEEK:
		DCC_LOG1(LOG_TRACE, "<%d> seek", th + 1);
		arg[0] = board_flash_mem_seek(mem, off);
		break;

	case THINKOS_FLASH_MEM_ERASE:
		DCC_LOG1(LOG_TRACE, "<%d> erase", th + 1);
		arg[0] = board_flash_mem_erase(mem, off, len);
		break;

	case THINKOS_FLASH_MEM_LOCK:
		DCC_LOG1(LOG_TRACE, "<%d> lock", th + 1);
		arg[0] = board_flash_mem_lock(mem, off, len);
		break;

	case THINKOS_FLASH_MEM_UNLOCK:
		DCC_LOG1(LOG_TRACE, "<%d> unlock", th + 1);
		arg[0] = board_flash_mem_unlock(mem, off, len);
		break;

	default:
		arg[0] = THINKOS_EINVAL;
		break;
	}
#if  0
	uint32_t pc;

	pc = thinkos_rt.ctx[th]->pc;
	thinkos_rt.ctx[th]->r1 = pc;
	pc = (uint32_t)thinkos_flash_mem_cont;
	pc |= 1;	
	thinkos_rt.ctx[th]->pc = pc;
#endif

	/* wakeup from the mutex wait queue */
	__thinkos_wakeup(wq, th);
	/* signal the scheduler ... */
	__thinkos_defer_sched();
}


void thinkos_flash_mem_svc(int32_t * arg, int self)
{
//	struct armv7m_basic_frame * frm = (struct armv7m_basic_frame *)arg;
	unsigned int wq = THINKOS_WQ_FLASH_MEM;
	/*
	uint32_t pc;

	pc = frm->pc;
	frm->r2 = pc;
	pc = (uint32_t)thinkos_flash_mem_cont;
	pc |= 1;	
	frm->pc = pc;
*/
	
#if THINKOS_ENABLE_ARG_CHECK
#endif

	/* insert into the flash wait queue */
	__thinkos_wq_insert(wq, self);
	DCC_LOG2(LOG_TRACE , "<%d> waiting on flash mem %d...", self, wq);

	//arg[0] = 0;
	/* (1) suspend the thread by removing it from the
	   ready wait queue. The __thinkos_suspend() call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access 
	   itself, in case we have anabled the time sharing option. */
	__thinkos_suspend(self);
	/* (2) Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
	thinkos_rt.ctx[self] = (struct thinkos_context *)&arg[-CTX_R0];

	/* signal the  ... */
	__idle_hook_req(IDLE_HOOK_FLASH_MEM);
	/* signal the scheduler ... */
	__thinkos_defer_sched();
#if 0
	switch (req) {
	case THINKOS_FLASH_MEM_OPEN:
		if (thinkos_flash_rt.open) {
			arg[0] = THINKOS_EINVAL;
			break;
		}

		thinkos_flash_rt.open = 1;
		arg[0] = THINKOS_OK;
		break;

	case THINKOS_FLASH_MEM_CLOSE:
		if (!thinkos_flash_rt.open) {
			arg[0] = THINKOS_EINVAL;
			break;
		}
		thinkos_flash_rt.open = 0;
		arg[0] = THINKOS_OK;
		break;

	default:
		arg[0] = THINKOS_EINVAL;
		break;
	}
#endif
}

const char thinkos_flash_mem_nm[] = "FMM";

#endif /* THINKOS_ENABLE_FLASH_MEM */

