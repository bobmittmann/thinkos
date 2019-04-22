/* 
 * File:	 usb-cdc.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
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

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <sys/dcclog.h>

void __dmon_irq_pause_all(void);
void __dmon_irq_restore_all(void);
void __dmon_irq_force_enable(void);

static const char * const app_argv[] = {
	"thinkos_app"
};

static int __attribute__((naked, noreturn)) app_bootstrap(void * arg)
{
	int (* app_reset)(int argc, char ** argv);
	uintptr_t thumb_call = (uintptr_t)arg | 1;

	DCC_LOG1(LOG_TRACE, "sp=0x%08x", cm3_sp_get());

	app_reset = (int (*)(int argc, char ** argv))thumb_call;
	for (;;) {
		app_reset(1, (char **)app_argv);
	}
}

/* -------------------------------------------------------------------------
 * Application execution
 * ------------------------------------------------------------------------- */

static bool magic_match(const struct magic_blk * magic, void * ptr)
{
	uint32_t * mem = (uint32_t *)ptr;
	int k;
	int j;

	k = magic->hdr.pos;
	for (j = 0; j < magic->hdr.cnt; ++j) {
		if ((mem[k++] & magic->rec[j].mask) != magic->rec[j].comp)
			return false;
	}	

	return true;
}

bool dbgmon_app_exec(const struct dbgmon_app_desc * desc, bool paused)
{
	void * app = (void *)desc->start_addr;
	int thread_id;

	if (!magic_match(desc->magic, app))
		return false;

	DCC_LOG1(LOG_TRACE, "app=%p", app);

	thread_id = dbgmon_thread_create((int (*)(void *))app_bootstrap, 
									 (void *)app, 
									 &thinkos_main_inf);

	if (!paused)
		dbgmon_thread_resume(thread_id);

	return true;
}


bool dmon_app_suspend(void)
{
	__thinkos_pause_all();

	if (thinkos_rt.active == THINKOS_THREADS_MAX) {
		DCC_LOG(LOG_INFO, "Current is Idle!");
	} else {
		DCC_LOG1(LOG_INFO, "current_thread=%d", thinkos_rt.active);
	}

	/* Make sure the communication channel interrupts are enabled. */

	dbgmon_wait_idle();

	return true;
}

bool dmon_app_continue(void)
{
	struct thinkos_except * xcpt = &thinkos_except_buf;

	if (xcpt->type == 0) {
		DCC_LOG(LOG_TRACE, "....");
		__thinkos_resume_all();
		return true;
	}

	DCC_LOG(LOG_WARNING, "Can't continue with a fault...");

	return false;
}
