/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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
 * @file monitor.h
 * @brief ThinkOS monitor example
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 


#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>
#include <arch/cortex-m3.h>
#include <sys/param.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>
#include <sys/dcclog.h>

/* FIXME: the GDB framework for the dbg monitor should be inside a thinkos
   dbgmon library */

void gdb_stub_task(const struct dbgmon_comm * comm);

#ifndef MONITOR_CONFIGURE_ENABLE
#define MONITOR_CONFIGURE_ENABLE   1
#endif

#ifndef MONITOR_DUMPMEM_ENABLE
#define MONITOR_DUMPMEM_ENABLE     1
#endif

#ifndef MONITOR_WATCHPOINT_ENABLE
#define MONITOR_WATCHPOINT_ENABLE  0
#endif

#ifndef MONITOR_UPGRADE_ENABLE
#define MONITOR_UPGRADE_ENABLE     1
#endif

#ifndef MONITOR_STACKUSAGE_ENABLE
#define MONITOR_STACKUSAGE_ENABLE  1
#endif

#ifndef MONITOR_THREADINFO_ENABLE
#define MONITOR_THREADINFO_ENABLE  1
#endif

#ifndef MONITOR_OSINFO_ENABLE  
#define MONITOR_OSINFO_ENABLE      1
#endif

#ifndef MONITOR_APPWIPE_ENABLE
#define MONITOR_APPWIPE_ENABLE     1
#endif

#ifndef MONITOR_APPTERM_ENABLE
#define MONITOR_APPTERM_ENABLE     1
#endif

#ifndef MONITOR_APPRESTART_ENABLE
#define MONITOR_APPRESTART_ENABLE  1
#endif

#ifndef MONITOR_SELFTEST_ENABLE
#define MONITOR_SELFTEST_ENABLE    0
#endif

#ifndef MONITOR_FAULT_ENABLE
#define MONITOR_FAULT_ENABLE       THINKOS_ENABLE_EXCEPTIONS
#endif

#ifndef MONITOR_EXCEPTION_ENABLE
#define MONITOR_EXCEPTION_ENABLE   THINKOS_ENABLE_EXCEPTIONS
#endif

#ifndef BOOT_ENABLE_GDB
#define BOOT_ENABLE_GDB 0
#endif

#ifndef BOOT_ENABLE_THIRD
#define BOOT_ENABLE_THIRD 0
#endif

#ifndef MONITOR_OS_PAUSE
#define MONITOR_OS_PAUSE 0
#endif

#ifndef MONITOR_OS_RESUME
#define MONITOR_OS_RESUME 0
#endif

#ifndef MONITOR_RESTART_MONITOR
#define MONITOR_RESTART_MONITOR 0
#endif

#ifndef MONITOR_THREAD_STEP_ENABLE
#define MONITOR_THREAD_STEP_ENABLE 1
#endif

/* ---------------------------------------------------------------------------
 * Configuration options sanity check
 */

#if (MONITOR_THREAD_STEP_ENABLE)
  #undef MONITOR_THREADINFO_ENABLE
  #define MONITOR_THREADINFO_ENABLE  1
#endif

/* ---------------------------------------------------------------------------
 * 
 */

#include <ascii.h>

#if (BOOT_ENABLE_GDB)
#include <gdb.h>
#endif

extern int __heap_end;
const void * heap_end = &__heap_end; 
extern int __heap_base;
const void * heap_base = &__heap_base; 

struct monitor {
	const struct dbgmon_comm * comm;
#if (MONITOR_THREADINFO_ENABLE)
	int8_t thread_id;
#endif
#if (MONITOR_DUMPMEM_ENABLE)
	struct {
		uint32_t addr;
		unsigned int size;
	} memdump;
#endif
#if (MONITOR_WATCHPOINT_ENABLE)
	struct {
		uint32_t addr;
	} wp[4];
#endif
};

static const char monitor_menu[] = 
#if (MONITOR_APPTERM_ENABLE)
" Ctrl+C - Stop app\r\n"
#endif
#if (MONITOR_SELFTEST_ENABLE)
" Ctrl+E - Run selftest\r\n"
#endif
#if (MONITOR_CONFIGURE_ENABLE)
" Ctrl+K - Configure Board\r\n"
#endif
#if (MONITOR_THREADINFO_ENABLE)
" Ctrl+N - Select Next Thread\r\n"
#endif
#if (MONITOR_OSINFO_ENABLE)
" Ctrl+O - ThinkOS info\r\n"
#endif
#if (MONITOR_OS_PAUSE)
" Ctrl+P - Pause all threads\r\n"
#endif
#if (MONITOR_RESTART_MONITOR)
" Ctrl+Q - Restart monitor\r\n"
#endif
#if (MONITOR_OS_RESUME)
" Ctrl+R - Resume all threads\r\n"
#endif
#if (MONITOR_DUMPMEM_ENABLE)
" Ctrl+D - Dump memory\r\n"
#endif
#if (MONITOR_THREAD_STEP_ENABLE)
" Ctrl+S - Thread step\r\n"
#endif
#if (MONITOR_THREADINFO_ENABLE)
" Ctrl+T - Thread info\r\n"
#endif
#if (MONITOR_STACKUSAGE_ENABLE)
" Ctrl+U - Stack usage info\r\n"
#endif
" Ctrl+V - Help\r\n"
#if (MONITOR_APPWIPE_ENABLE)
" Ctrl+W - Wipe application\r\n"
#endif
#if (MONITOR_EXCEPTION_ENABLE)
" Ctrl+X - Exception info\r\n"
#endif
" Ctrl+Y - YMODEM app upload\r\n"
#if (MONITOR_APPRESTART_ENABLE)
" Ctrl+Z - Restart app\r\n"
#endif
#if (MONITOR_UPGRADE_ENABLE)
" Ctrl+\\ - Upload ThinkOS\r\n"
#endif
#if (MONITOR_WATCHPOINT_ENABLE)
" Ctrl+] - Set watchpoint\r\n"
#endif
;

static const char s_hr[] = "-----------------------\r\n";

static void monitor_show_help(const struct dbgmon_comm * comm)
{
	DCC_LOG2(LOG_TRACE, "sp=0x%08x comm=0x%08x", cm3_sp_get(), comm);
	dbgmon_printf(comm, s_hr);
	dbgmon_printf(comm, "ThinkOS-%d.%d.%d (%s):\r\n", 
			 this_board.sw_ver.major,
			 this_board.sw_ver.minor,
			 this_board.sw_ver.build,
			 this_board.name);
	dbgmon_printf(comm, monitor_menu);
	dbgmon_printf(comm, s_hr);
}

#if (MONITOR_EXCEPTION_ENABLE)
static void monitor_print_fault(const struct dbgmon_comm * comm)
{
	struct thinkos_except * xcpt = &thinkos_except_buf;

	if (xcpt->type == 0) {
		dbgmon_printf(comm, "No fault!");
		return;
	}

	dmon_print_exception(comm, xcpt);
}

static void monitor_on_fault(const struct dbgmon_comm * comm)
{
	struct thinkos_except * xcpt = &thinkos_except_buf;

#if !THINKOS_DBGMON_ENABLE_IRQ_MGMT
	/* Restore interrupts */
	this_board.softreset();
#endif

	DCC_LOG(LOG_TRACE, "dmon_wait_idle()...");

	__thinkos_pause_all();
	if (dbgmon_wait_idle() < 0) {
		DCC_LOG(LOG_WARNING, "dmon_wait_idle() failed!");
	}

	DCC_LOG(LOG_TRACE, "<<IDLE>>");

	if (dbgmon_comm_isconnected(comm)) {
		DCC_LOG(LOG_TRACE, "COMM connected!");
		dbgmon_printf(comm, s_hr);
		dmon_print_exception(comm, xcpt);
		dbgmon_printf(comm, s_hr);
	}

	DCC_LOG(LOG_TRACE, "done.");
}
#endif

#if (MONITOR_WATCHPOINT_ENABLE)
static void monitor_on_bkpt(struct monitor * mon)
{
	const struct dbgmon_comm * comm = mon->comm;
	struct thinkos_except * xcpt = &thinkos_except_buf;

	DCC_LOG(LOG_TRACE, "dmon_wait_idle()...");

	if (dbgmon_wait_idle() < 0) {
		DCC_LOG(LOG_WARNING, "dmon_wait_idle() failed!");
	}

	DCC_LOG(LOG_TRACE, "<<IDLE>>");

	if (dbgmon_comm_isconnected(comm)) {
		dbgmon_printf(comm, s_hr);
		dmon_print_exception(comm, xcpt);
		dbgmon_printf(comm, s_hr);
	}
}
#endif

#if (MONITOR_THREAD_STEP_ENABLE)
static void monitor_on_step(struct monitor * mon)
{
	const struct dbgmon_comm * comm = mon->comm;
	struct thinkos_except * xcpt = &thinkos_except_buf;

	DCC_LOG(LOG_TRACE, "dmon_wait_idle()...");

	if (dbgmon_wait_idle() < 0) {
		DCC_LOG(LOG_WARNING, "dmon_wait_idle() failed!");
	}

	DCC_LOG(LOG_TRACE, "<<IDLE>>");

	if (dbgmon_comm_isconnected(comm)) {
		dbgmon_printf(comm, s_hr);
		dmon_print_exception(comm, xcpt);
		dbgmon_printf(comm, s_hr);
	}
}
#endif

#if (MONITOR_OS_PAUSE)
static void monitor_pause_all(const struct dbgmon_comm * comm)
{
	dbgmon_printf(comm, "\r\nPausing all threads...\r\n");
	DCC_LOG(LOG_WARNING, "__thinkos_pause_all()");
	__thinkos_pause_all();
	if (dbgmon_wait_idle() < 0) {
		DCC_LOG(LOG_WARNING, "dmon_wait_idle() failed!");
	}
}
#endif

#if (MONITOR_OS_RESUME)
static void monitor_resume_all(const struct dbgmon_comm * comm)
{
	dbgmon_printf(comm, "\r\nResuming all threads...\r\n");
	__thinkos_resume_all();
	dbgmon_printf(comm, "Restarting...\r\n");
}
#endif

static void monitor_ymodem_recv(const struct dbgmon_comm * comm, 
								uint32_t addr, unsigned int size)
{
	dbgmon_printf(comm, "\r\nYMODEM receive (^X to cancel) ... ");
	if (dmon_ymodem_flash(comm, addr, size) < 0) {
		dbgmon_printf(comm, "\r\n#ERROR: YMODEM failed!\r\n"); 
		return;
	}	
	dbgmon_printf(comm, "\r\nOK\r\n");
}

#if (MONITOR_APPWIPE_ENABLE)
static void monitor_app_erase(const struct dbgmon_comm * comm, 
							  uint32_t addr, unsigned int size)
{
	dbgmon_printf(comm, "\r\nErasing application block (%08x)... ", addr);
	if (dmon_app_erase(comm, addr, size))
		dbgmon_printf(comm, "done.\r\n");
	else	
		dbgmon_printf(comm, "failed!\r\n");
}
#endif

#if (MONITOR_DUMPMEM_ENABLE)
void monitor_show_mem(struct monitor * mon)
{
	uint32_t addr = mon->memdump.addr;
	unsigned int size = mon->memdump.size;

	dbgmon_printf(mon->comm, "Addr (0x%08x): ", addr);
	dbgmon_scanf(mon->comm, "%x", &addr);
	dbgmon_printf(mon->comm, "Size (%d): ", size);
	dbgmon_scanf(mon->comm, "%u", &size);

	dbgmon_hexdump(mon->comm, addr, (uintptr_t *)addr, size);
	mon->memdump.addr = addr;
	mon->memdump.size = size;
}
#endif

#if (MONITOR_WATCHPOINT_ENABLE)
void monitor_watchpoint(struct monitor * mon)
{
	unsigned int no = 0;
	uint32_t addr;

	dbgmon_printf(mon->comm, "No (0..3): ");
	dbgmon_scanf(mon->comm, "%u", &no);
	if (no > 3)
		dbgmon_printf(mon->comm, "Invalid!\r\n");
	addr = mon->wp[no].addr;
	dbgmon_printf(mon->comm, "Addr (0x%08x): ", addr);
	dbgmon_scanf(mon->comm, "%x", &addr);
	mon->wp[no].addr = addr;
	dmon_watchpoint_set(addr, 4, 0);
}
#endif

void monitor_task(const struct dbgmon_comm *, void * arg);

#if (MONITOR_SELFTEST_ENABLE)
void __attribute__((naked)) selftest_bootstrap(const struct dbgmon_comm * comm, 
											   void * arg) 
{
	if (this_board.selftest)
		this_board.selftest(comm);
	dbgmon_exec(monitor_task, NULL);
}
#endif

#if (BOOT_ENABLE_GDB)
void __attribute__((naked)) gdb_bootstrap(const struct dbgmon_comm * comm, void * arg) 
{
	DCC_LOG1(LOG_TRACE, "sp=0x%08x", cm3_sp_get());
	gdb_stub_task(comm);
	dbgmon_exec(monitor_task, arg);
}
#endif

#if (BOOT_ENABLE_THIRD)
void third_stub_task(struct dbgmon_comm *);

void __attribute__((naked)) third_bootstrap(const struct dbgmon_comm * comm, 
											void * arg) 
{
	/* call the THIRD stub task */
	third_stub_task(comm);
	/* return to the monitor */
	dbgmon_exec(monitor_task, arg);
}
#endif

static bool monitor_process_input(struct monitor * mon, int c)
{
	const struct dbgmon_comm * comm = mon->comm;

	switch (c) {
#if (BOOT_ENABLE_GDB)
	case '+':
		dbgmon_exec(gdb_bootstrap, NULL);
		break;
#endif
#if (BOOT_ENABLE_THIRD)
	case '~':
		dbgmon_exec(third_bootstrap, NULL);
		break;
#endif
#if (MONITOR_APPTERM_ENABLE)
	case CTRL_C:
		dbgmon_printf(comm, "^C\r\n");
		dbgmon_req_app_term();
		break;
#endif
#if (MONITOR_SELFTEST_ENABLE)
	case CTRL_E:
		dbgmon_printf(comm, "^E\r\n");
		dbgmon_soft_reset();
		dbgmon_exec(selftest_bootstrap);
		break;
#endif
#if (MONITOR_CONFIGURE_ENABLE)
	case CTRL_K:
		dbgmon_printf(comm, "^K\r\n");
		dbgmon_soft_reset();
		this_board.configure(comm);
		break;
#endif
#if (MONITOR_UPGRADE_ENABLE)
	case CTRL_FS:
		dbgmon_soft_reset();
		dbgmon_printf(comm, "^\\\r\nConfirm [y]? ");
		if (dbgmon_getc(comm) == 'y') {
			this_board.upgrade(comm);
			dbgmon_printf(comm, "Failed !!!\r\n");
		} else {
			dbgmon_printf(comm, "\r\n");
		}
		break;
#endif
#if (MONITOR_THREADINFO_ENABLE)
	case CTRL_N:
		mon->thread_id = __thinkos_thread_getnext(mon->thread_id);
		if (mon->thread_id == - 1)
			mon->thread_id = __thinkos_thread_getnext(mon->thread_id);
		dbgmon_printf(comm, "Thread = %d\r\n", mon->thread_id);
		dmon_print_thread(comm, mon->thread_id);
		break;
#endif
#if (MONITOR_THREAD_STEP_ENABLE)
	case CTRL_S:
		dbgmon_printf(comm, "^S\r\n");
		dbgmon_printf(comm, s_hr);
		dmon_thread_step(mon->thread_id, false);
		break;
#endif
#if (MONITOR_OSINFO_ENABLE)
	case CTRL_O:
		dbgmon_printf(comm, "^O\r\n");
		dbgmon_printf(comm, s_hr);
		dmon_print_osinfo(comm);
		break;
#endif
#if (MONITOR_OS_PAUSE)
	case CTRL_P:
		dbgmon_printf(comm, "^P\r\n");
		monitor_pause_all(comm);
		break;
#endif
#if (MONITOR_OS_RESUME)
	case CTRL_R:
		dbgmon_printf(comm, "^R\r\n");
		monitor_resume_all(comm);
		break;
#endif
#if (MONITOR_RESTART_MONITOR)
	case CTRL_Q:
		dbgmon_printf(comm, "^Q\r\n");
		dbgmon_exec(monitor_task, NULL);
		break;
#endif
#if (MONITOR_DUMPMEM_ENABLE)
	case CTRL_D:
		dbgmon_printf(comm, "^D\r\n");
		monitor_show_mem(mon);
		break;
#endif
#if (MONITOR_THREADINFO_ENABLE)
	case CTRL_T:
		dmon_print_thread(comm, mon->thread_id);
		break;
#endif
#if (MONITOR_STACKUSAGE_ENABLE)
	case CTRL_U:
		dmon_print_stack_usage(comm);
		break;
#endif
	case CTRL_V:
		monitor_show_help(comm);
		break;
#if (MONITOR_FAULT_ENABLE)
	case CTRL_X:
		monitor_print_fault(comm);
		break;
#endif
	case CTRL_Y:
		dbgmon_printf(comm, "^\\\r\nUpload application [y]? ");
		if (dbgmon_getc(comm) == 'y') {
			/* Request app upload */
			dbgmon_req_app_upload();
		} else {
			dbgmon_printf(comm, "\r\n");
		}
		break;
#if (MONITOR_APPWIPE_ENABLE)
	case CTRL_W:
		dbgmon_printf(comm, "^W\r\n");
		dbgmon_printf(comm, "^\\\r\nErase application [y]? ");
		if (dbgmon_getc(comm) == 'y') {
			/* Request app erase */
			dbgmon_req_app_erase(); 
		} else {
			dbgmon_printf(comm, "\r\n");
		}
		break;
#endif
#if (MONITOR_APPRESTART_ENABLE)
	case CTRL_Z:
		dbgmon_printf(comm, "^Z\r\n");
		dbgmon_req_app_exec(); 
		break;
#endif
#if (MONITOR_WATCHPOINT_ENABLE)
	case CTRL_GS:
		dbgmon_printf(comm, "^]\r\n");
		monitor_watchpoint(mon);
		break;
#endif
	default:
		return false;
	}

	return true;
}

/*
   Dafault Monitor Task
 */

void __attribute__((noreturn)) monitor_task(const struct dbgmon_comm * comm,
											void * arg)
{
	struct monitor monitor;
	uint32_t sigmask = 0;
#if THINKOS_ENABLE_CONSOLE
	uint8_t * ptr;
	int cnt;
#endif
	int tick_cnt = 0;
	uint8_t buf[1];
	int sig;
	
	monitor.comm = comm;
#if (MONITOR_THREADINFO_ENABLE)
	monitor.thread_id = -1;
#endif
#if (MONITOR_DUMPMEM_ENABLE)
	monitor.memdump.addr = this_board.application.start_addr;
	monitor.memdump.size = this_board.application.block_size;
#endif


	DCC_LOG1(LOG_TRACE, "Monitor sp=%08x ...", cm3_sp_get());

	sigmask |= (1 << DBGMON_SOFTRST);
	sigmask |= (1 << DBGMON_STARTUP);
#if (MONITOR_EXCEPTION_ENABLE)
	sigmask |= (1 << DBGMON_THREAD_FAULT);
	sigmask |= (1 << DBGMON_EXCEPT);
#endif
	sigmask |= (1 << DBGMON_COMM_RCV);
#if THINKOS_ENABLE_CONSOLE
	sigmask |= (1 << DBGMON_COMM_EOT);
	sigmask |= (1 << DBGMON_COMM_CTL);
	sigmask |= (1 << DBGMON_TX_PIPE);
	sigmask |= (1 << DBGMON_RX_PIPE);
#endif
	sigmask |= (1 << DBGMON_APP_STOP);
	sigmask |= (1 << DBGMON_APP_EXEC);
	sigmask |= (1 << DBGMON_APP_UPLOAD);
	sigmask |= (1 << DBGMON_APP_ERASE);
	sigmask |= (1 << DBGMON_APP_TERM);
	sigmask |= (1 << DBGMON_APP_RESUME);
#if (MONITOR_WATCHPOINT_ENABLE)
	sigmask |= (1 << DBGMON_BREAKPOINT);
#endif
#if (MONITOR_THREAD_STEP_ENABLE)
	sigmask |= (1 << DBGMON_THREAD_STEP);
#endif

	for(;;) {
		switch ((sig = dbgmon_sched_select(sigmask))) {

		case DBGMON_STARTUP:
			dbgmon_clear(DBGMON_STARTUP);
			DCC_LOG1(LOG_TRACE, "/!\\ STARTUP signal (SP=0x%08x)...", 
					 cm3_sp_get());
			/* first time we run the monitor, start a timer to call the 
			   board_tick() periodically */
			sigmask |= (1 << DBGMON_ALARM);
			dbgmon_alarm(125);
			break;

		case DBGMON_SOFTRST:
			dbgmon_clear(DBGMON_SOFTRST);
			DCC_LOG(LOG_WARNING, "/!\\ SOFTRST signal !");
			this_board.softreset();
#if THINKOS_ENABLE_CONSOLE
			__console_reset();
			/* Update the console connection flag which was cleared
			 by __console_reset(). */
			__console_connect_set(dbgmon_comm_isconnected(comm));
#endif
			break;

		case DBGMON_APP_UPLOAD:
			dbgmon_clear(DBGMON_APP_UPLOAD);
			DCC_LOG(LOG_TRACE, "/!\\ APP_UPLOAD signal !");
			monitor_ymodem_recv(comm, this_board.application.start_addr, 
								this_board.application.block_size);
			/* Request app exec */
	//		dbgmon_req_app_exec(); 
			break;

		case DBGMON_APP_EXEC:
			dbgmon_clear(DBGMON_APP_EXEC);
			DCC_LOG(LOG_TRACE, "/!\\ APP_EXEC signal !");
			if (!dbgmon_app_exec(&this_board.application, false)) {
				dbgmon_printf(comm, "Can't run application!\r\n");
				/* XXX: this event handler could be optionally compiled
				   to save some resources. As a matter of fact I don't think
				   they are useful at all */
				DCC_LOG(LOG_TRACE, "dbgmon_app_exec() failed!");
				if (this_board.on_error != NULL)
					dmon_thread_exec((void *)this_board.on_error, NULL);
			}
			break;

#if (MONITOR_APPWIPE_ENABLE)
		case DBGMON_APP_ERASE:
			dbgmon_clear(DBGMON_APP_ERASE);
			DCC_LOG(LOG_TRACE, "/!\\ APP_ERASE signal !");
			monitor_app_erase(comm, this_board.application.start_addr, 
							  this_board.application.block_size);
			break;
#endif

		case DBGMON_APP_TERM:
			dbgmon_clear(DBGMON_APP_TERM);
			DCC_LOG(LOG_TRACE, "/!\\ APP_TERM signal !");
			break;

		case DBGMON_APP_STOP:
			dbgmon_clear(DBGMON_APP_STOP);
			DCC_LOG(LOG_TRACE, "/!\\ APP_STOP signal !");
			break;

		case DBGMON_APP_RESUME:
			dbgmon_clear(DBGMON_APP_RESUME);
			DCC_LOG(LOG_TRACE, "/!\\ APP_RESUME signal !");
			break;


#if THINKOS_ENABLE_CONSOLE
		case DBGMON_COMM_CTL:
			dbgmon_clear(DBGMON_COMM_CTL);
			DCC_LOG(LOG_MSG, "Comm Ctl.");
			__console_connect_set(dbgmon_comm_isconnected(comm));
			break;
#endif

#if (MONITOR_EXCEPTION_ENABLE)
		case DBGMON_THREAD_FAULT:
			dbgmon_clear(DBGMON_THREAD_FAULT);
			DCC_LOG(LOG_TRACE, "Thread fault.");
			monitor_on_fault(comm);
			break;

		case DBGMON_EXCEPT:
			dbgmon_clear(DBGMON_EXCEPT);
			DCC_LOG(LOG_TRACE, "System exception.");
			monitor_on_fault(comm);
			break;
#endif

#if (MONITOR_WATCHPOINT_ENABLE)
		case DBGMON_BREAKPOINT:
			dbgmon_clear(DBGMON_BREAKPOINT);
			monitor_on_bkpt(&monitor);
			break;
#endif

#if (MONITOR_THREAD_STEP_ENABLE)
		case DBGMON_THREAD_STEP:
			dbgmon_clear(DBGMON_THREAD_STEP);
			DCC_LOG(LOG_INFO, "DBGMON_THREAD_STEP");
			monitor_on_step(&monitor);
			break;
#endif

		case DBGMON_COMM_RCV:
			dbgmon_clear(DBGMON_COMM_RCV);
			DCC_LOG(LOG_TRACE, "COMM_RCV: +++++++++++++++++");
#if THINKOS_ENABLE_CONSOLE
			/* receive from the COMM driver one byte at the time */
			if ((cnt = dbgmon_comm_recv(comm, buf, 1)) > 0) {
				int c = buf[0];

				DCC_LOG1(LOG_TRACE, "COMM_RCV: c=0x%02x", c);
				/* process the input character */
				if (!monitor_process_input(&monitor, c)) {
					int n;
					/* if the character was not consumed by the monitor 
					   insert into the console pipe */
					/* get a pointer to the head of the pipe.
					 __console_rx_pipe_ptr() will return the number of 
					 consecutive spaces in the buffer. We need only one. */
					if ((n = __console_rx_pipe_ptr(&ptr)) > 0) {
						/* copy the character into the RX fifo */
						ptr[0] = c;
						/* commit the fifo head */
						__console_rx_pipe_commit(1);
						if (n == 1) {
							/* no more space on rx fifo ... */
							DCC_LOG(LOG_TRACE, "rx_pipe.free=0 "
									"maksing DBGMON_COMM_RCV?");
//							sigmask &= ~(1 << DBGMON_COMM_RCV);
						}
					} else {
						/* discard */
					}
				}
			} else {
				DCC_LOG1(LOG_WARNING, "COMM_RCV: %d!", cnt);
				sigmask &= ~(1 << DBGMON_COMM_RCV);
			}
#else
			if (dbgmon_comm_recv(comm, buf, 1) > 0) {
				/* process the input character */
				monitor_process_input(&monitor, buf[0]);
			}
#endif
			DCC_LOG(LOG_TRACE, "COMM_RCV: ----------------");
			break;

#if THINKOS_ENABLE_CONSOLE
		case DBGMON_RX_PIPE:
			dbgmon_clear(DBGMON_RX_PIPE);
			if ((cnt = __console_rx_pipe_ptr(&ptr)) > 0) {
				DCC_LOG1(LOG_MSG, "RX_PIPE. rx_pipe.free=%d. "
						 "Unmaksing DBGMON_COMM_RCV!", cnt);
				sigmask |= (1 << DBGMON_COMM_RCV);
			} else {
				DCC_LOG(LOG_MSG, "RX Pipe empty!!!");
			}
			break;

		case DBGMON_COMM_EOT:
			DCC_LOG(LOG_MSG, "COMM_EOT");
		//	break;

		case DBGMON_TX_PIPE:
			DCC_LOG(LOG_MSG, "TX Pipe.");
			if ((cnt = __console_tx_pipe_ptr(&ptr)) > 0) {
				int n;
				DCC_LOG1(LOG_MSG, "TX Pipe, %d pending chars.", cnt);
				n = dbgmon_comm_send(comm, ptr, cnt);
				__console_tx_pipe_commit(n); 
				sigmask |= (1 << DBGMON_COMM_EOT);
			} else {
				DCC_LOG(LOG_MSG, "TX Pipe empty!!!");
				dbgmon_clear(DBGMON_TX_PIPE);
				sigmask &= ~(1 << DBGMON_COMM_EOT);
			}
			break;
#endif

		case DBGMON_ALARM:
			/* Acknowledge the signal */
			dbgmon_clear(DBGMON_ALARM);
			/* Query the board for app boot up */
			if (this_board.autoboot(tick_cnt++)) {
				DCC_LOG(LOG_TRACE, "autoboot app_exec()...");
				dbgmon_signal(DBGMON_APP_EXEC); 
				sigmask &= ~(1 << DBGMON_ALARM);
			} else {
				/* reastart the alarm timer */
				dbgmon_alarm(125);
			}
			break;
		}
	}
}

