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

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <sys/dcclog.h>

static const char * const app_argv[] = {
	"thinkos_app"
};

#if 0
static int __attribute__((naked, noreturn)) app_bootstrap(void * arg)
{
	int (* app_reset)(int argc, char ** argv);
	uintptr_t thumb_call = (uintptr_t)arg | 1;

	app_reset = (int (*)(int argc, char ** argv))thumb_call;

	for (;;) {
		DCC_LOG2(LOG_TRACE, "sp=0x%08x app_reset=0x%08x", 
				 cm3_sp_get(), app_reset);
		app_reset(1, (char **)app_argv);
	}
}
#endif

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

bool monitor_app_exec(const struct monitor_app_desc * desc, bool paused)
{
	void * ptr = (void *)desc->start_addr;
	uintptr_t thumb_call = (uintptr_t)ptr | 1;
//	int (* app)(int argc, char ** argv);
	int (* app)(void *, unsigned int);
	int thread_id;

//	app = (int (*)(int argc, char ** argv))thumb_call;
	app = C_TASK(thumb_call);

	if (!magic_match(desc->magic, ptr))
		return false;

	DCC_LOG1(LOG_TRACE, "app=%p", app);

#if (THINKOS_ENABLE_THREAD_INFO)
	thread_id = monitor_thread_create(app, C_ARG(app_argv), &thinkos_main_inf);
#else
	thread_id = monitor_thread_create(app, C_ARG(app_argv), 0);
#endif

	if (!paused)
		monitor_thread_resume(thread_id);

	return true;
}


bool monitor_app_suspend(void)
{
	__thinkos_pause_all();

	if (thinkos_rt.active == THINKOS_THREADS_MAX) {
		DCC_LOG(LOG_INFO, "Current is Idle!");
	} else {
		DCC_LOG1(LOG_INFO, "current_thread=%d", thinkos_rt.active);
	}

	/* Make sure the communication channel interrupts are enabled. */

	monitor_wait_idle();

	return true;
}

bool monitor_app_continue(void)
{
	struct thinkos_except * xcpt = &thinkos_except_buf;

	if (xcpt->errno == 0) {
		DCC_LOG(LOG_TRACE, "....");
		__thinkos_resume_all();
		return true;
	}

	DCC_LOG(LOG_WARNING, "Can't continue with a fault...");

	return false;
}

