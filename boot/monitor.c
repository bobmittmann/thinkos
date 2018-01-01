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

void gdb_stub_task(struct dmon_comm * comm);

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
#define MONITOR_SELFTEST_ENABLE    1
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

#if (BOOT_ENABLE_GDB)
#include <gdb.h>
#endif

/* ASCII Keyboard codes */

#define _NULL_  0x00 /* Null (Ctrl+@) */
#define CTRL_A  0x01 /* SOH */
#define CTRL_B  0x02 /* STX */
#define CTRL_C  0x03 /* ETX */
#define CTRL_D  0x04 /* EOT */
#define CTRL_E  0x05 /* ENQ */
#define CTRL_F  0x06 /* ACK */
#define CTRL_G  0x07 /* BEL */
#define CTRL_H  0x08 /* BS */
#define CTRL_I  0x09 /* TAB */
#define CTRL_J  0x0a /* LF */
#define CTRL_K  0x0b /* VT */
#define CTRL_L  0x0c /* FF */
#define CTRL_M  0x0d /* CR */
#define CTRL_N  0x0e /* SO */
#define CTRL_O  0x0f /* SI */
#define CTRL_P  0x10 /* DLE */
#define CTRL_Q  0x11 /* DC1 */
#define CTRL_R  0x12 /* DC2 */
#define CTRL_S  0x13 /* DC3 */
#define CTRL_T  0x14 /* DC4 */
#define CTRL_U  0x15 /* NAK */
#define CTRL_V  0x16 /* SYN */
#define CTRL_W  0x17 /* ETB */
#define CTRL_X  0x18 /* CAN */
#define CTRL_Y  0x19 /* EM */
#define CTRL_Z  0x1a /* SUB */
#define _ESC_   0x1b /* ESC (Ctrl+[) */
#define CTRL_FS 0x1c /* FS  (Ctrl+\) */
#define CTRL_GS 0x1d /* GS  (Ctrl+]) */
#define CTRL_RS 0x1e /* RS  (Ctrl+^) */
#define CTRL_US 0x1f /* US  (Ctrl+_) */

extern int __heap_end;
const void * heap_end = &__heap_end; 
extern int __heap_base;
const void * heap_base = &__heap_base; 

struct monitor {
	struct dmon_comm * comm;
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
" Ctrl+S - Show memory\r\n"
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

static void monitor_show_help(struct dmon_comm * comm)
{
	dmprintf(comm, s_hr);
	dmprintf(comm, "ThinkOS-%d.%d.%d (%s):\r\n", 
			 this_board.sw_ver.major,
			 this_board.sw_ver.minor,
			 this_board.sw_ver.build,
			 this_board.name);
	dmprintf(comm, monitor_menu);
	dmprintf(comm, s_hr);
}

#if (MONITOR_EXCEPTION_ENABLE)
static void monitor_print_fault(struct dmon_comm * comm)
{
	struct thinkos_except * xcpt = &thinkos_except_buf;

	if (xcpt->type == 0) {
		dmprintf(comm, "No fault!");
		return;
	}

	dmon_print_exception(comm, xcpt);
}

static void monitor_on_fault(struct dmon_comm * comm)
{
	struct thinkos_except * xcpt = &thinkos_except_buf;

	DCC_LOG(LOG_TRACE, "dmon_wait_idle()...");

	if (dbgmon_wait_idle() < 0) {
		DCC_LOG(LOG_WARNING, "dmon_wait_idle() failed!");
	}

	DCC_LOG(LOG_TRACE, "<<IDLE>>");

	if (dmon_comm_isconnected(comm)) {
		DCC_LOG(LOG_TRACE, "COMM connected!");
		dmprintf(comm, s_hr);
		dmon_print_exception(comm, xcpt);
		dmprintf(comm, s_hr);
	}

	DCC_LOG(LOG_TRACE, "done.");
}
#endif

#if (MONITOR_WATCHPOINT_ENABLE)
static void monitor_on_bkpt(struct monitor * mon)
{
	struct dmon_comm * comm = mon->comm;
	struct thinkos_except * xcpt = &thinkos_except_buf;

	DCC_LOG(LOG_TRACE, "dmon_wait_idle()...");

	if (dbgmon_wait_idle() < 0) {
		DCC_LOG(LOG_WARNING, "dmon_wait_idle() failed!");
	}

	DCC_LOG(LOG_TRACE, "<<IDLE>>");

	if (dmon_comm_isconnected(comm)) {
		dmprintf(comm, s_hr);
		dmon_print_exception(comm, xcpt);
		dmprintf(comm, s_hr);
	}
}
#endif

#if (MONITOR_OS_PAUSE)
static void monitor_pause_all(struct dmon_comm * comm)
{
	dmprintf(comm, "\r\nPausing all threads...\r\n");
	DCC_LOG(LOG_WARNING, "__thinkos_pause_all()");
	__thinkos_pause_all();
	if (dbgmon_wait_idle() < 0) {
		DCC_LOG(LOG_WARNING, "dmon_wait_idle() failed!");
	}
}
#endif

#if (MONITOR_OS_RESUME)
static void monitor_resume_all(struct dmon_comm * comm)
{
	dmprintf(comm, "\r\nResuming all threads...\r\n");
	__thinkos_resume_all();
	dmprintf(comm, "Restarting...\r\n");
}
#endif

static void monitor_ymodem_recv(struct dmon_comm * comm, 
								uint32_t addr, unsigned int size)
{
	dmprintf(comm, "\r\nYMODEM receive (^X to cancel) ... ");
	if (dmon_ymodem_flash(comm, addr, size) < 0) {
		dmprintf(comm, "\r\n#ERROR: YMODEM failed!\r\n"); 
		return;
	}	
	dmprintf(comm, "\r\nOK\r\n");
}

#if (MONITOR_APPWIPE_ENABLE)
static void monitor_app_erase(struct dmon_comm * comm, 
							  uint32_t addr, unsigned int size)
{
	dmprintf(comm, "\r\nErasing application block (%08x)... ", addr);
	if (dmon_app_erase(comm, addr, size))
		dmprintf(comm, "done.\r\n");
	else	
		dmprintf(comm, "failed!\r\n");
}
#endif

#if (MONITOR_DUMPMEM_ENABLE)
int long2hex_le(char * s, unsigned long val);
int char2hex(char * s, int c);

void monitor_dump_mem(struct dmon_comm * comm, 
					  uint32_t addr, unsigned int size)
{
	char buf[14 + 16 * 3];
	unsigned int rem = size;
	uint8_t * cmp = (uint8_t *)-1;
	bool eq = false;

	while (rem) {
		int n = rem < 16 ? rem : 16;
		uint8_t * src = (uint8_t *)addr;
		char * cp = buf;
		unsigned int i;
	
		if (cmp != (uint8_t *)-1) {
			for (i = 0; i < n; ++i) {
				if (src[i] != cmp[i]) {
					eq = false;
					goto dump_line;
				}
			}

			if (!eq) {
				dmon_comm_send(comm, " ...\r\n", 6);
				eq = true;
			}
		} else {	

dump_line:
			cp += long2hex_le(cp, addr);
			*cp++ = ':';

			for (i = 0; i < n; ++i) {
				*cp++ = ' ';
				cp += char2hex(cp, src[i]);
			}

			*cp++ = '\r';
			*cp++ = '\n';

			dmon_comm_send(comm, buf, cp - buf);
		}

		addr += n;
		rem -= n;
		cmp = src;
	}
}

void monitor_show_mem(struct monitor * mon)
{
	uint32_t addr = mon->memdump.addr;
	unsigned int size = mon->memdump.size;

	dmprintf(mon->comm, "Addr (0x%08x): ", addr);
	dmscanf(mon->comm, "%x", &addr);
	dmprintf(mon->comm, "Size (%d): ", size);
	dmscanf(mon->comm, "%u", &size);

	monitor_dump_mem(mon->comm, addr, size);
	mon->memdump.addr = addr;
	mon->memdump.size = size;
}

#endif

#if (MONITOR_WATCHPOINT_ENABLE)
void monitor_watchpoint(struct monitor * mon)
{
	unsigned int no = 0;
	uint32_t addr;

	dmprintf(mon->comm, "No (0..3): ");
	dmscanf(mon->comm, "%u", &no);
	if (no > 3)
		dmprintf(mon->comm, "Invalid!\r\n");
	addr = mon->wp[no].addr;
	dmprintf(mon->comm, "Addr (0x%08x): ", addr);
	dmscanf(mon->comm, "%x", &addr);
	mon->wp[no].addr = addr;
	dmon_watchpoint_set(addr, 4, 0);
}
#endif

void monitor_task(struct dmon_comm *);

#if (MONITOR_SELFTEST_ENABLE)
void __attribute__((naked)) selftest_bootstrap(struct dmon_comm * comm) 
{
	if (this_board.selftest)
		this_board.selftest(comm);
	dbgmon_exec(monitor_task);
}
#endif

#if (BOOT_ENABLE_GDB)
void __attribute__((naked)) gdb_bootstrap(struct dmon_comm * comm) 
{
	DCC_LOG1(LOG_TRACE, "sp=0x%08x", cm3_sp_get());
	gdb_stub_task(comm);
	dbgmon_exec(monitor_task);
}
#endif

#if (BOOT_ENABLE_THIRD)
void third_stub_task(struct dmon_comm *);

void __attribute__((naked)) third_bootstrap(struct dmon_comm * comm) 
{
	/* call the THIRD stub task */
	third_stub_task(comm);
	/* return to the monitor */
	dbgmon_exec(monitor_task);
}
#endif

static bool monitor_process_input(struct monitor * mon, int c)
{
	struct dmon_comm * comm = mon->comm;

	switch (c) {
#if (BOOT_ENABLE_GDB)
	case '+':
		dbgmon_exec(gdb_bootstrap);
		break;
#endif
#if (BOOT_ENABLE_THIRD)
	case '~':
		dbgmon_exec(third_bootstrap);
		break;
#endif
#if (MONITOR_APPTERM_ENABLE)
	case CTRL_C:
		dmprintf(comm, "^C\r\n");
		dbgmon_soft_reset();
		break;
#endif
#if (MONITOR_SELFTEST_ENABLE)
	case CTRL_E:
		dmprintf(comm, "^E\r\n");
		dbgmon_soft_reset();
		dbgmon_exec(selftest_bootstrap);
		break;
#endif
#if (MONITOR_CONFIGURE_ENABLE)
	case CTRL_K:
		dmprintf(comm, "^K\r\n");
		dbgmon_soft_reset();
		this_board.configure(comm);
		break;
#endif
#if (MONITOR_UPGRADE_ENABLE)
	case CTRL_FS:
		dbgmon_soft_reset();
		dmprintf(comm, "^\\\r\nConfirm [y]? ");
		if (dmgetc(comm) == 'y') {
			this_board.upgrade(comm);
			dmprintf(comm, "Failed !!!\r\n");
		} else {
			dmprintf(comm, "\r\n");
		}
		break;
#endif
#if (MONITOR_THREADINFO_ENABLE)
	case CTRL_N:
		mon->thread_id = __thinkos_thread_getnext(mon->thread_id);
		if (mon->thread_id == - 1)
			mon->thread_id = __thinkos_thread_getnext(mon->thread_id);
		dmprintf(comm, "Thread = %d\r\n", mon->thread_id);
		dmon_print_thread(comm, mon->thread_id);
		break;
#endif
#if (MONITOR_OSINFO_ENABLE)
	case CTRL_O:
		dmprintf(comm, "^O\r\n");
		dmprintf(comm, s_hr);
		dmon_print_osinfo(comm);
		break;
#endif
#if (MONITOR_OS_PAUSE)
	case CTRL_P:
		dmprintf(comm, "^P\r\n");
		monitor_pause_all(comm);
		break;
#endif
#if (MONITOR_OS_RESUME)
	case CTRL_R:
		dmprintf(comm, "^R\r\n");
		monitor_resume_all(comm);
		break;
#endif
#if (MONITOR_RESTART_MONITOR)
	case CTRL_Q:
		dmprintf(comm, "^Q\r\n");
		dbgmon_exec(monitor_task);
		break;
#endif
#if (MONITOR_DUMPMEM_ENABLE)
	case CTRL_S:
		dmprintf(comm, "^S\r\n");
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
		dmprintf(comm, "^\\\r\nUpload application [y]? ");
		if (dmgetc(comm) == 'y') {
			dbgmon_soft_reset();
			/* Request app upload */
			dbgmon_signal(DBGMON_APP_UPLOAD); 
			/* Request app exec */
			dbgmon_signal(DBGMON_APP_EXEC); 
		} else {
			dmprintf(comm, "\r\n");
		}
		break;
#if (MONITOR_APPWIPE_ENABLE)
	case CTRL_W:
		dmprintf(comm, "^W\r\n");
		dmprintf(comm, "^\\\r\nErase application [y]? ");
		if (dmgetc(comm) == 'y') {
			dbgmon_soft_reset();
			/* Request app erase */
			dbgmon_signal(DBGMON_APP_ERASE); 
		} else {
			dmprintf(comm, "\r\n");
		}
		break;
#endif
#if (MONITOR_APPRESTART_ENABLE)
	case CTRL_Z:
		dmprintf(comm, "^Z\r\n");
		dbgmon_soft_reset();
		/* Request app exec */
		dbgmon_signal(DBGMON_APP_EXEC); 
		break;
#endif
#if (MONITOR_WATCHPOINT_ENABLE)
	case CTRL_GS:
		dmprintf(comm, "^]\r\n");
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

void __attribute__((noreturn)) monitor_task(struct dmon_comm * comm)
{
	struct monitor monitor;
	uint32_t sigmask = 0;
	uint32_t sigset;
#if THINKOS_ENABLE_CONSOLE
	uint8_t * ptr;
	int cnt;
#endif
	int tick_cnt = 0;
	uint8_t buf[1];
	
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
	sigmask |= (1 << DBGMON_COMM_CTL);
	sigmask |= (1 << DBGMON_TX_PIPE);
	sigmask |= (1 << DBGMON_RX_PIPE);
#endif
	sigmask |= (1 << DBGMON_APP_STOP);
	sigmask |= (1 << DBGMON_APP_EXEC);
	sigmask |= (1 << DBGMON_APP_UPLOAD);
	sigmask |= (1 << DBGMON_APP_ERASE);
#if (MONITOR_WATCHPOINT_ENABLE)
	sigmask |= (1 << DBGMON_BREAKPOINT);
#endif

	for(;;) {
		sigset = dbgmon_select(sigmask);

		DCC_LOG1(LOG_MSG, "sigset=%08x", sigset);

		if (sigset & (1 << DBGMON_STARTUP)) {
			dbgmon_clear(DBGMON_STARTUP);
			DCC_LOG1(LOG_TRACE, "/!\\ STARTUP signal (SP=0x%08x)...", 
					 cm3_sp_get());
			/* first time we run the monitor, start a timer to call the 
			   board_tick() periodically */
			sigmask |= (1 << DBGMON_ALARM);
			dbgmon_alarm(125);
		}

		if (sigset & (1 << DBGMON_SOFTRST)) {
			DCC_LOG(LOG_WARNING, "/!\\ SOFTRST signal !");
			this_board.softreset();
			dbgmon_clear(DBGMON_SOFTRST);
		}

		if (sigset & (1 << DBGMON_APP_UPLOAD)) {
			dbgmon_clear(DBGMON_APP_UPLOAD);
			DCC_LOG(LOG_TRACE, "/!\\ APP_UPLOAD signal !");
			monitor_ymodem_recv(comm, this_board.application.start_addr, 
								this_board.application.block_size);
		}

		if (sigset & (1 << DBGMON_APP_EXEC)) {
			dbgmon_clear(DBGMON_APP_EXEC);
			DCC_LOG(LOG_TRACE, "/!\\ APP_EXEC signal !");
			if (!dmon_app_exec(this_board.application.start_addr, false)) {
				dmprintf(comm, "Can't run application!\r\n");
				/* XXX: this event handler could be optionally compiled
				   to save some resources. As a matter of fact I don't think
				   they are useful at all */
				DCC_LOG(LOG_TRACE, "dmon_app_exec() failed!");
				if (this_board.on_error != NULL)
					dmon_thread_exec((void *)this_board.on_error, NULL);
			}
		}

#if (MONITOR_APPWIPE_ENABLE)
		if (sigset & (1 << DBGMON_APP_ERASE)) {
			dbgmon_clear(DBGMON_APP_ERASE);
			DCC_LOG(LOG_TRACE, "/!\\ APP_ERASE signal !");
			monitor_app_erase(comm, this_board.application.start_addr, 
							  this_board.application.block_size);
		}
#endif

#if THINKOS_ENABLE_CONSOLE
		if (sigset & (1 << DBGMON_COMM_CTL)) {
			dbgmon_clear(DBGMON_COMM_CTL);
			DCC_LOG(LOG_MSG, "Comm Ctl.");
		}
#endif

#if (MONITOR_EXCEPTION_ENABLE)
		if (sigset & (1 << DBGMON_THREAD_FAULT)) {
			dbgmon_clear(DBGMON_THREAD_FAULT);
			DCC_LOG(LOG_TRACE, "Thread fault.");
			monitor_on_fault(comm);
		}

		if (sigset & (1 << DBGMON_EXCEPT)) {
			dbgmon_clear(DBGMON_EXCEPT);
			DCC_LOG(LOG_TRACE, "System exception.");
			monitor_on_fault(comm);
		}
#endif

#if (MONITOR_WATCHPOINT_ENABLE)
		if (sigset & (1 << DBGMON_BREAKPOINT)) {
			dbgmon_clear(DBGMON_BREAKPOINT);
			monitor_on_bkpt(&monitor);
		}
#endif

		if (sigset & (1 << DBGMON_COMM_RCV)) {
#if THINKOS_ENABLE_CONSOLE
			/* receive from the COMM driver one bye at the time */
			if (dmon_comm_recv(comm, buf, 1) > 0) {
				int c = buf[0];
				/* process the input character */
				if (!monitor_process_input(&monitor, c)) {
					/* if the character was not consumed by the monitor 
					   insert into the console pipe */
					/* get a pointer to the head of the pipe.
					 __console_rx_pipe_ptr() will return the number of 
					 consecutive spaces in the buffer. We need only one. */
					if (__console_rx_pipe_ptr(&ptr) > 0) {
						/* copy the character into the RX fifo */
						ptr[0] = c;
						/* commit the fifo head */
						__console_rx_pipe_commit(1);
					} else {
						/* discard */
					}
				}
			}
#else
			if (dmon_comm_recv(comm, buf, 1) > 0) {
				/* process the input character */
				monitor_process_input(&monitor, buf[0]);
			}
#endif
		}

#if THINKOS_ENABLE_CONSOLE
		if (sigset & (1 << DBGMON_RX_PIPE)) {
			dbgmon_clear(DBGMON_RX_PIPE);
			if ((cnt = __console_rx_pipe_ptr(&ptr)) > 0) {
				DCC_LOG1(LOG_TRACE, "RX Pipe. rx_pipe.free=%d. "
						 "Unmaksing DBGMON_COMM_RCV!", cnt);
				sigmask |= (1 << DBGMON_COMM_RCV);
			} else {
				DCC_LOG(LOG_TRACE, "RX Pipe empty!!!");
			}
		}

		if (sigset & (1 << DBGMON_TX_PIPE)) {
			DCC_LOG(LOG_MSG, "TX Pipe.");
			if ((cnt = __console_tx_pipe_ptr(&ptr)) > 0) {
				DCC_LOG1(LOG_INFO, "TX Pipe, %d pending chars.", cnt);
				cnt = dmon_comm_send(comm, ptr, cnt);
				__console_tx_pipe_commit(cnt); 
			} else {
				DCC_LOG(LOG_INFO, "TX Pipe empty!!!");
				dbgmon_clear(DBGMON_TX_PIPE);
			}
		}
#endif

		if (sigset & (1 << DBGMON_ALARM)) {
			/* Aclknowledge the signal */
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
		}
	}
}


