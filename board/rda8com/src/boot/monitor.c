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
#include <sys/delay.h>

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_DEBUG__
#include <thinkos/debug.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#define __THINKOS_KRNSVC__
#include <thinkos/krnsvc.h>

#include <thinkos.h>
#include <vt100.h>

#include <sys/dcclog.h>

int monitor_console_shell(const struct monitor_comm * comm);
void __attribute__((noreturn)) app_task(void *, unsigned int);

/* FIXME: the GDB framework for the dbg monitor should be inside a thinkos
   debug library */

void gdb_stub_task(const struct monitor_comm * comm);

#ifndef MONITOR_GDB_ENABLE
#define MONITOR_GDB_ENABLE         0
#endif

#ifndef MONITOR_UPGRADE_ENABLE
#define MONITOR_UPGRADE_ENABLE     0
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
#define MONITOR_APPWIPE_ENABLE     0
#endif

#ifndef MONITOR_FAULT_ENABLE
#define MONITOR_FAULT_ENABLE       1
#endif

#ifndef MONITOR_EXCEPTION_ENABLE
#define MONITOR_EXCEPTION_ENABLE   1
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

#ifndef MONITOR_THREAD_STEP_ENABLE
#define MONITOR_THREAD_STEP_ENABLE 0
#endif

#ifndef MONITOR_PROFILE_ENABLE
#define MONITOR_PROFILE_ENABLE 0
#endif

#ifndef MONITOR_WATCHPOINT_ENABLE
#define MONITOR_WATCHPOINT_ENABLE  0
#endif

#ifndef BOOT_ENABLE_GDB
#define BOOT_ENABLE_GDB 0
#endif

#ifndef MONITOR_BREAKPOINT_ENABLE
#define MONITOR_BREAKPOINT_ENABLE  0
#endif


/* ---------------------------------------------------------------------------
 * Configuration options sanity check
 */

#if (MONITOR_THREAD_STEP_ENABLE)
  #undef MONITOR_THREADINFO_ENABLE
  #define MONITOR_THREADINFO_ENABLE  1
#endif

#if !(THINKOS_ENABLE_MONITOR)
#error "Need THINKOS_ENABLE_MONITOR"
#endif

/*
#if !(THINKOS_ENABLE_MONITOR_THREADS)
#error "Need THINKOS_ENABLE_MONITOR_THREADS"
#endif

#if !(THINKOS_ENABLE_IDLE_HOOKS)
#error "Need THINKOS_ENABLE_IDLE_HOOKS"
#endif
*/

#if !(THINKOS_ENABLE_CONSOLE_MODE)
#error "Need THINKOS_ENABLE_CONSOLE_MODE"
#endif

#if (MONITOR_THREAD_STEP_ENABLE) & !(THINKOS_ENABLE_DEBUG_STEP)
#error "MONITOR_THREAD_STEP_ENABLE requires THINKOS_ENABLE_DEBUG_STEP"
#endif

#if (MONITOR_WATCHPOINT_ENABLE) & !(THINKOS_ENABLE_DEBUG_WPT)
#error "MONITOR_WATCHPOINT_ENABLE requires THINKOS_ENABLE_DEBUG_WPT"
#endif

#if (MONITOR_BREAKPOINT_ENABLE) & !(THINKOS_ENABLE_DEBUG_BKPT)
#error "MONITOR_BREAKPOINT_ENABLE requires THINKOS_ENABLE_DEBUG_BKPT"
#endif

/* ---------------------------------------------------------------------------
 * 
 */

#include <ascii.h>

#if (MONITOR_GDB_ENABLE)
#include <gdb.h>
#endif

extern int __heap_end;
const void * heap_end = &__heap_end; 
extern int __heap_base;
const void * heap_base = &__heap_base; 

struct monitor {
	const struct thinkos_board * board;
	const struct monitor_comm * comm;
	intptr_t test_status; 
#if (MONITOR_THREADINFO_ENABLE)
	int8_t thread_id;
#endif
#if (MONITOR_WATCHPOINT_ENABLE)
	struct {
		uint32_t addr;
	} wp[4];
#endif
#if (MONITOR_BREAKPOINT_ENABLE)
	struct {
		uint32_t addr;
	} bp[4];
#endif
};

static const char monitor_menu[] = 
"  \r\n"
" Debug/Monitor shortcuts:\r\n"
"  \r\n"
"   Ctrl+C - Stop/terminate the application\r\n"
#if (MONITOR_BREAKPOINT_ENABLE)
"   Ctrl+F - Set breakpoint\r\n"
#endif

#if (MONITOR_EXCEPTION_ENABLE)
"   Ctrl+G - Show Exception info\r\n"
#endif

#if (MONITOR_THREADINFO_ENABLE)
"   Ctrl+N - Select Next Thread\r\n"
#endif
#if (MONITOR_OSINFO_ENABLE)
"   Ctrl+O - Show ThinkOS state info\r\n"
#endif
#if (MONITOR_OS_PAUSE)
"   Ctrl+P - Pause all threads\r\n"
#endif
#if (MONITOR_OS_RESUME)
"   Ctrl+R - Resume all threads\r\n"
#endif
#if (MONITOR_THREAD_STEP_ENABLE)
"   Ctrl+S - Single step thread execution \r\n"
#endif
#if (MONITOR_THREADINFO_ENABLE)
"   Ctrl+T - Show thread info\r\n"
#endif
#if (MONITOR_STACKUSAGE_ENABLE)
"   Ctrl+U - Stack usage info\r\n"
#endif
"   Ctrl+V - Show this Help\r\n"
#if (MONITOR_WATCHPOINT_ENABLE)
"   Ctrl+] - Set watchpoint\r\n"
#endif
#if 0
"   Ctrl+^ - \r\n"
"   Ctrl+_ - \r\n"
#endif
;

static const char s_hr[] = 
"-----------------------------------------------------------------------\r\n";

static void monitor_show_help(const struct monitor_comm * comm,
							  const struct thinkos_board * board)
{
	DCC_LOG2(LOG_TRACE, "sp=0x%08x comm=0x%08x", cm3_sp_get(), comm);
	monitor_printf(comm, s_hr);
	monitor_printf(comm, "%s-%d.%d.%d (%s):\r\n", 
			 board->sw.tag,
			 board->sw.ver.major,
			 board->sw.ver.minor,
			 board->sw.ver.build,
			 board->name);
	monitor_puts(monitor_menu, comm);
	monitor_printf(comm, s_hr);
}

#if (MONITOR_EXCEPTION_ENABLE)
static void monitor_print_fault(const struct monitor_comm * comm)
{
	struct thinkos_except * xcpt = __thinkos_except_buf();

	monitor_print_exception(comm, xcpt);
}

static const char s_crlf[] =  "\r\n";

static void monitor_on_thread_fault(const struct monitor_comm * comm)
{
	int thread_id;
	int32_t errno;

	/* get the last thread known to be at fault */
	thread_id = monitor_thread_break_get(&errno);
	if (thread_id <= 0) {
		DCC_LOG(LOG_WARNING, "No break thread!!!");
		return;
	}

	DCC_LOG2(LOG_ERROR, "<%d> fault %d !!", thread_id, errno);

	if (monitor_comm_isconnected(comm)) {
		struct monitor_thread_inf inf;

		DCC_LOG(LOG_TRACE, "COMM connected!");

		monitor_thread_inf_get(thread_id, &inf);
		monitor_printf(comm, s_crlf);
		monitor_printf(comm, s_hr);
		monitor_printf(comm, "* Error %s [thread=%d errno=%d addr=0x%08x]\r\n", 
					   thinkos_krn_err_tag(inf.errno),
					   inf.thread_id,
					   inf.errno,
					   inf.pc);
		monitor_print_thread(comm, thread_id);
		monitor_printf(comm, s_hr);
	}

	DCC_LOG(LOG_TRACE, "thinkos_dbg_thread_break_clr().");
	thinkos_dbg_thread_break_clr();

	DCC_LOG(LOG_TRACE, "done.");
}

#endif

static void monitor_on_krn_fault(const struct monitor_comm * comm)
{
	int thread_id;
	int32_t errno;

//	mdelay(500);

	/* get the last thread known to be at fault */
	thread_id = monitor_thread_break_get(&errno);
	if (thread_id <= 0) {
		DCC_LOG(LOG_WARNING, "No break thread!!!");
	} else {
		DCC_LOG2(LOG_ERROR, "<%d> fault %d !!", thread_id, errno);
	}

	if (monitor_comm_isconnected(comm)) {
		monitor_printf(comm, 
					  "# Kernel error, possible stack overflow !!!\r\n");
		if (thread_id > 0) {
			monitor_printf(comm, " Offended thread: %d\r\n", thread_id);
			monitor_print_thread(comm, thread_id);
		}

		monitor_printf(comm, s_hr);
	}

//	mdelay(500);

	DCC_LOG(LOG_TRACE, "done.");
}

#if (MONITOR_BREAKPOINT_ENABLE)
static void monitor_on_bkpt(struct monitor * mon)
{
	const struct monitor_comm * comm = mon->comm;
	struct monitor_thread_inf inf;
	unsigned int thread_id;

	thread_id = monitor_thread_break_get();
	monitor_thread_inf_get(thread_id, &inf);
	__thinkos_pause_all();

	DCC_LOG2(LOG_TRACE, "<%d> breakpoint @ 0x%08x", thread_id, inf.pc);

	if (monitor_comm_isconnected(comm)) {
		monitor_printf(comm, s_hr);
		monitor_printf(mon->comm, "<%d> breakpoint @ 0x%08x\r\n", 
					  thread_id, inf.pc);
		mon->thread_id = thread_id;
		monitor_print_thread(comm, thread_id);
		monitor_breakpoint_clear(inf.pc, 4);
		monitor_printf(comm, s_hr);
	}
}
#endif

#if (MONITOR_THREAD_STEP_ENABLE)
static void monitor_on_step(struct monitor * mon)
{
	const struct monitor_comm * comm = mon->comm;
	struct thinkos_rt * krn = &thinkos_rt;
	struct monitor_thread_inf inf;
	unsigned int thread_id;

	thread_id = monitor_thread_step_get();
	monitor_thread_inf_get(thread_id, &inf);
	__thinkos_krn_pause_all(krn);

	if (monitor_comm_isconnected(comm)) {
		DCC_LOG2(LOG_TRACE, "<%d> step at %08x", thread_id, inf.pc);
		monitor_printf(comm, s_hr);
		mon->thread_id = thread_id;
		monitor_print_thread(comm, thread_id);
		monitor_printf(comm, s_hr);
	}
}
#endif

#if (MONITOR_OS_PAUSE)
static void monitor_pause_all(const struct monitor_comm * comm)
{
	monitor_printf(comm, "\r\nPausing all threads...\r\n");
	DCC_LOG(LOG_WARNING, "thinkos_dbg_pause_all()");
	thinkos_dbg_pause_all();
}
#endif

#if (MONITOR_OS_RESUME)
static void monitor_resume_all(const struct monitor_comm * comm)
{
	monitor_printf(comm, "\r\nResuming all threads...\r\n");
	DCC_LOG(LOG_WARNING, "thinkos_dbg_resume_all()");
	thinkos_dbg_resume_all();
}
#endif

#if (MONITOR_BREAKPOINT_ENABLE)
void monitor_breakpoint(struct monitor * mon)
{
	unsigned int no = 0;
	uint32_t addr;

	monitor_printf(mon->comm, "No (0..3): ");
	monitor_scanf(mon->comm, "%u", &no);
	if (no > 3) {
		monitor_printf(mon->comm, "Invalid!\r\n");
		return;
	}
	addr = mon->bp[no].addr;
	monitor_printf(mon->comm, "Addr (0x%08x): ", addr);
	monitor_scanf(mon->comm, "%x", &addr);
	mon->bp[no].addr = addr;
	monitor_breakpoint_set(addr, 4);
	monitor_printf(mon->comm, "Breakpoint %d @ 0x%08x\r\n", no, addr);
}
#endif

#if (MONITOR_WATCHPOINT_ENABLE)
void monitor_watchpoint(struct monitor * mon)
{
	unsigned int no = 0;
	uint32_t addr;

	monitor_printf(mon->comm, "No (0..3): ");
	monitor_scanf(mon->comm, "%u", &no);
	if (no > 3) {
		monitor_printf(mon->comm, "Invalid!\r\n");
		return;
	}
	addr = mon->wp[no].addr;
	monitor_printf(mon->comm, "Addr (0x%08x): ", addr);
	monitor_scanf(mon->comm, "%x", &addr);
	mon->wp[no].addr = addr;
	monitor_watchpoint_set(addr, 4, 0);
}
#endif

void boot_monitor_task(const struct monitor_comm *, void * arg);

#if (MONITOR_GDB_ENABLE)
void __attribute__((naked)) gdb_bootstrap(const struct monitor_comm * comm, 
										  void * arg) 
{
	DCC_LOG1(LOG_TRACE, "sp=0x%08x", cm3_sp_get());
	gdb_stub_task(comm);
	monitor_exec(boot_monitor_task, arg);
}
#endif

static bool monitor_process_input(struct monitor * mon, int c)
{
	const struct monitor_comm * comm = mon->comm;
	const struct thinkos_board * board = mon->board;

	switch (c) {
#if (MONITOR_GDB_ENABLE)
	case '+':
		monitor_exec(gdb_bootstrap, NULL);
		break;
#endif
	case CTRL_C:
		monitor_printf(comm, "^C\r\n");
		DCC_LOG(LOG_WARNING, "^C monitor_req_app_term()!");
		monitor_req_app_term();
		break;
#if (MONITOR_DUMPMEM_ENABLE)
	case CTRL_D:
		monitor_printf(comm, "^D\r\n");
		monitor_show_mem(mon, board);
		break;
#endif
#if (MONITOR_BREAKPOINT_ENABLE)
	case CTRL_F:
		monitor_printf(comm, "^F\r\n");
		monitor_breakpoint(mon);
		break;
#endif
#if (MONITOR_THREADINFO_ENABLE)
	case CTRL_N:
		mon->thread_id = __thinkos_thread_getnext(mon->thread_id);
		if (mon->thread_id == - 1)
			mon->thread_id = __thinkos_thread_getnext(mon->thread_id);
		monitor_print_thread(comm, mon->thread_id);
		break;
#endif
#if (MONITOR_OSINFO_ENABLE)
	case CTRL_O:
		monitor_printf(comm, "^O\r\n");
		monitor_signal(MONITOR_USER_EVENT4);
		break;
#endif
#if (MONITOR_OS_PAUSE)
	case CTRL_P:
		monitor_printf(comm, "^P\r\n");
		monitor_pause_all(comm);
		break;
#endif
#if (MONITOR_OS_RESUME)
	case CTRL_R:
		monitor_printf(comm, "^R\r\n");
		monitor_resume_all(comm);
		break;
#endif
#if (MONITOR_THREAD_STEP_ENABLE)
	case CTRL_S:
		monitor_printf(comm, "^S\r\n");
		monitor_printf(comm, s_hr);
		monitor_thread_step(mon->thread_id, false);
		break;
#endif
#if (MONITOR_THREADINFO_ENABLE)
	case CTRL_T:
		monitor_print_thread(comm, mon->thread_id);
		break;
#endif
#if (MONITOR_STACKUSAGE_ENABLE)
	case CTRL_U:
		monitor_print_stack_usage(comm);
		break;
#endif
	case CTRL_V:
		monitor_show_help(comm, board);
		break;
#if (MONITOR_FAULT_ENABLE)
	case CTRL_G:
		monitor_print_fault(comm);
		break;
#endif
#if (MONITOR_WATCHPOINT_ENABLE)
	case CTRL_GS:
		monitor_printf(comm, "^]\r\n");
		monitor_watchpoint(mon);
		break;
#endif
	default:
		return false;
	}

	return true;
}

/*
   Default Monitor Task
 */
void __attribute__((noreturn)) 
boot_monitor_task(const struct monitor_comm * comm, void * arg)
{
#if (MONITOR_OSINFO_ENABLE)
	uint32_t cycref[THINKOS_THREAD_LAST + 1];
#endif
	const struct thinkos_board * board;
	struct monitor monitor;
	bool raw_mode = false;
	uint32_t sigmask = 0;
	uint8_t * ptr;
	int cnt;
	uint8_t buf[1];
	int sig;

	board = (const struct thinkos_board *)arg;
	monitor.board = board;
	monitor.comm = comm;
#if (MONITOR_THREADINFO_ENABLE)
	monitor.thread_id = -1;
#endif

	sigmask |= (1 << MONITOR_SOFTRST);
	sigmask |= (1 << MONITOR_KRN_ABORT);
#if (MONITOR_EXCEPTION_ENABLE)
	sigmask |= (1 << MONITOR_THREAD_FAULT);
	sigmask |= (1 << MONITOR_THREAD_BREAK);
	sigmask |= (1 << MONITOR_KRN_FAULT);
#endif
	sigmask |= (1 << MONITOR_COMM_RCV);
	sigmask |= (1 << MONITOR_COMM_BRK);
	sigmask |= (1 << MONITOR_COMM_CTL);
	sigmask |= (1 << MONITOR_TX_PIPE);
#if (MONITOR_WATCHPOINT_ENABLE)
	sigmask |= (1 << MONITOR_BREAKPOINT);
#endif
#if (MONITOR_BREAKPOINT_ENABLE)
	sigmask |= (1 << MONITOR_BREAKPOINT);
#endif
#if (MONITOR_THREAD_STEP_ENABLE)
	sigmask |= (1 << MONITOR_THREAD_STEP);
#endif
#if (MONITOR_OSINFO_ENABLE)
	sigmask |= (1 << MONITOR_USER_EVENT4);
#endif
	sigmask |= (1 << MONITOR_USER_EVENT3);

	DCC_LOG(LOG_TRACE, "================= ThinkOS Monitor ================="); 

	for(;;) {
		DCC_LOG1(LOG_MSG, "sigmask=%08x", sigmask); 
		switch ((sig = monitor_select(sigmask))) {

		case MONITOR_SOFTRST:
			/* Acknowledge the signal */
			monitor_clear(MONITOR_SOFTRST);
			DCC_LOG(LOG_WARNING, "/!\\ SOFTRST signal !");
			board->softreset();
			thinkos_krn_console_raw_mode_set(raw_mode = false);
			goto is_connected;
			break;

		case MONITOR_KRN_ABORT:
			monitor_clear(MONITOR_KRN_ABORT);
			DCC_LOG(LOG_TRACE, "/!\\ KRN_ABORT signal...");
			monitor_signal(MONITOR_USER_EVENT3);
			break;

		case MONITOR_COMM_BRK:
			/* Acknowledge the signal */
			DCC_LOG(LOG_WARNING, "/!\\ COMM_BREAK signal !");
			monitor_comm_break_ack(comm);
			monitor_soft_reset();
			monitor_signal(MONITOR_USER_EVENT3);
			break;

		case MONITOR_THREAD_BREAK:
			monitor_clear(MONITOR_THREAD_BREAK);
			DCC_LOG(LOG_WARNING, "/!\\ THREAD_BREAK signal !");
			break;

#if (MONITOR_EXCEPTION_ENABLE)
		case MONITOR_THREAD_FAULT:
			monitor_clear(MONITOR_THREAD_FAULT);
			DCC_LOG(LOG_TRACE, "Thread fault !.");
			monitor_on_thread_fault(comm);
			break;

		case MONITOR_KRN_FAULT:
			monitor_clear(MONITOR_KRN_FAULT);
			DCC_LOG(LOG_TRACE, "!! Kernel fault !!");
			monitor_on_krn_fault(comm);
			break;
#endif

#if (MONITOR_BREAKPOINT_ENABLE)
		case MONITOR_BREAKPOINT:
			monitor_clear(MONITOR_BREAKPOINT);
			monitor_on_bkpt(&monitor);
			break;
#endif

#if (MONITOR_THREAD_STEP_ENABLE)
		case MONITOR_THREAD_STEP:
			monitor_clear(MONITOR_THREAD_STEP);
			DCC_LOG(LOG_INFO, "MONITOR_THREAD_STEP");
			monitor_on_step(&monitor);
			break;
#endif

		case MONITOR_COMM_RCV:
			raw_mode = thinkos_krn_console_is_raw_mode();
			if (raw_mode) {
				sigmask = monitor_on_comm_rcv(comm, sigmask);
				break;
			}

			/* receive from the COMM driver one byte at the time */
			if ((cnt = monitor_comm_recv(comm, buf, 1)) > 0) {
				int c = buf[0];

				DCC_LOG1(LOG_INFO, "COMM_RCV: c=0x%02x", c);
				/* process the input character */
				if (!monitor_process_input(&monitor, c)) {
					int n;
					/* if the character was not consumed by the monitor 
					   insert into the console pipe */
					/* get a pointer to the head of the pipe.
					 thinkos_console_rx_pipe_ptr() will return the number of 
					 consecutive spaces in the buffer. We need only one. */
					if ((n = thinkos_console_rx_pipe_ptr(&ptr)) > 0) {
						/* copy the character into the RX fifo */
						ptr[0] = c;
						/* commit the fifo head */
						thinkos_console_rx_pipe_commit(1);
					} else {
						/* discard */
					}
				}

			} else {
				DCC_LOG1(LOG_INFO, "monitor_comm_recv() = %d", cnt);
			}
			break;

		case MONITOR_COMM_CTL:
is_connected:
			sigmask = monitor_on_comm_ctl(comm, sigmask);
			break;

		case MONITOR_COMM_EOT:
			/* FALLTHROUGH */
		case MONITOR_TX_PIPE:
			sigmask = monitor_on_tx_pipe(comm, sigmask);
			break;

		case MONITOR_RX_PIPE:
			sigmask = monitor_on_rx_pipe(comm, sigmask);
			break;

		case MONITOR_USER_EVENT3:
			monitor_clear(MONITOR_USER_EVENT3);
			DCC_LOG(LOG_WARNING, "MONITOR_USER_EVENT3!");
			if (board->on_break)
				board->on_break(comm);
			else {
				DCC_LOG(LOG_WARNING, "board->on_break() NULL pointer!");
			}
			break;

#if (MONITOR_OSINFO_ENABLE)
		case MONITOR_USER_EVENT4:
			monitor_clear(MONITOR_USER_EVENT4);
			monitor_puts(s_hr, comm);
			monitor_print_osinfo(comm, cycref);
			break;
#endif

		default:
			DCC_LOG1(LOG_WARNING, "unhandled SIG %d!", sig);
			break;
		}
	}
}


