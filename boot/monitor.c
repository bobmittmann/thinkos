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

#include <thinkos.h>
#include <vt100.h>
#include <ascii.h>

#include <sys/dcclog.h>

#ifndef BOOT_ENABLE_MONITOR
#define BOOT_ENABLE_MONITOR 1
#endif

int monitor_console_shell(const struct monitor_comm * comm);
void __attribute__((noreturn)) app_task(void *, unsigned int);

/* FIXME: the GDB framework for the dbg monitor should be inside a thinkos
   debug library */

void gdb_stub_task(const struct monitor_comm * comm);

#ifndef MONITOR_VT100_ENABLE
#define MONITOR_VT100_ENABLE       1
#endif

#ifndef MONITOR_DUMPMEM_ENABLE
#define MONITOR_DUMPMEM_ENABLE     1
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

#ifndef MONITOR_EXCEPTION_ENABLE
#define MONITOR_EXCEPTION_ENABLE   (THINKOS_ENABLE_EXCEPTIONS)
#endif

#ifndef MONITOR_OS_PAUSE
#define MONITOR_OS_PAUSE 1
#endif

#ifndef MONITOR_OS_RESUME
#define MONITOR_OS_RESUME 1
#endif

#ifndef MONITOR_THREAD_STEP_ENABLE
#define MONITOR_THREAD_STEP_ENABLE 1
#endif

#ifndef MONITOR_BOARDINFO_ENABLE
#define MONITOR_BOARDINFO_ENABLE 1
#endif

#ifndef MONITOR_PROFILE_ENABLE
#define MONITOR_PROFILE_ENABLE 1
#endif

#ifndef MONITOR_WATCHPOINT_ENABLE
#define MONITOR_WATCHPOINT_ENABLE  0
#endif

#ifndef MONITOR_GDB_ENABLE
#define MONITOR_GDB_ENABLE 0
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

#if !(THINKOS_ENABLE_CONSOLE)
#error "Need THINKOS_ENABLE_CONSOLE"
#endif

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
#if (MONITOR_BREAKPOINT_ENABLE)
	struct {
		uint32_t addr;
	} bp[4];
#endif
#if (MONITOR_OSINFO_ENABLE)
	uint32_t cycref[THINKOS_THREAD_LAST + 1];
#endif
};

static const char monitor_menu[] = 
"\r\n"
" Monitor shortcuts:\r\n"
"\r\n"
#if (MONITOR_BOARDINFO_ENABLE)
"\tCtrl+B - Show board configuration\r\n"
#endif
"\tCtrl+C - Stop/terminate the application\r\n"
#if (MONITOR_DUMPMEM_ENABLE)
"\tCtrl+D - Show memory region content\r\n"
#endif

#if (MONITOR_BREAKPOINT_ENABLE)
"\tCtrl+F - Set breakpoint\r\n"
#endif

"\tCtrl+G - Show Exception info\r\n"

#if (MONITOR_THREADINFO_ENABLE)
"\tCtrl+N - Select Next Thread\r\n"
#endif
#if (MONITOR_OSINFO_ENABLE)
"\tCtrl+O - Show ThinkOS state info\r\n"
#endif
#if (MONITOR_OS_PAUSE)
"\tCtrl+P - Pause all threads\r\n"
#endif
#if (MONITOR_RESTART_MONITOR)
"\tCtrl+Q - Restart monitor\r\n"
#endif
#if (MONITOR_OS_RESUME)
"\tCtrl+R - Resume all threads\r\n"
#endif
#if (MONITOR_THREAD_STEP_ENABLE)
"\tCtrl+S - Single step thread execution \r\n"
#endif
#if (MONITOR_THREADINFO_ENABLE)
"\tCtrl+T - Show thread info\r\n"
#endif
#if (MONITOR_STACKUSAGE_ENABLE)
"\tCtrl+U - Stack usage info\r\n"
#endif
"\tCtrl+V - Show this Help\r\n"

#if (MONITOR_WATCHPOINT_ENABLE)
"\tCtrl+] - Set watchpoint\r\n"
#endif
#if 0
"\tCtrl+^ - \r\n"
"\tCtrl+_ - \r\n"
#endif
;

static const char s_hr[] = 
"-----------------------------------------------------------------------\r\n";

static void monitor_hbar(const struct monitor_comm * comm)
{
	monitor_puts(s_hr, comm);
}


#if (MONITOR_VT100_ENABLE)
static const char s_bottom[] = 
	VT100_SET_FONT_G0_ASCII VT100_FONT_SELECT_G0 VT100_SET_SCROLL_ALL 
	VT100_ATTR_NORMAL  VT100_GOTO(199,199) "\r\n";
#endif

static void monitor_show_help(const struct monitor_comm * comm,
							  const struct thinkos_board * board)
{
#if (MONITOR_VT100_ENABLE)
	monitor_printf(comm, s_bottom);
#endif
	DCC_LOG2(LOG_TRACE, "sp=0x%08x comm=0x%08x", cm3_sp_get(), comm);
	monitor_hbar(comm);
	monitor_printf(comm, "%s-%d.%d.%d (%s):\r\n", 
			 board->sw.tag,
			 board->sw.ver.major,
			 board->sw.ver.minor,
			 board->sw.ver.build,
			 board->name);
	monitor_puts(monitor_menu, comm);
	monitor_hbar(comm);
}

static void monitor_on_print_fault(const struct monitor_comm * comm, 
								   struct thinkos_krn * krn) 
{
	struct thinkos_fault * fault = __thinkos_fault_rt();
//	const struct thinkos_mem_desc * mem = &sram_desc;
	struct krn_thread_state inf;
	int thread_id;
	int32_t errno;

	/* get the last thread known to be at fault */
	thread_id = monitor_thread_break_get(&errno);

	monitor_print_newln(comm);
	monitor_hbar(comm);

	if  (thinkos_krn_thread_state_get(krn, thread_id, &inf)) {
		monitor_printf(comm, "* Error %s [thread=%d errno=%d addr=0x%08x]\r\n", 
					   thinkos_krn_err_tag(inf.errno),
					   inf.thread_id, inf.errno, inf.ctx->pc);

		monitor_print_thread_state(comm, &inf);
		monitor_print_newln(comm);
	}

	monitor_print_fault(comm, fault);
	monitor_print_newln(comm);


}

void monitor_on_thread_fault(const struct monitor_comm * comm,
							 struct thinkos_krn * krn) 
{
	int thread_id;
	int32_t errno;

	/* get the last thread known to be at fault */
	thread_id = monitor_thread_break_get(&errno);
	if (thread_id <= 0) {
		DCC_LOG(LOG_WARNING, "No break thread!!!");
		return;
	}

	DCC_LOG2(LOG_ERROR, "<%d> error %d !!", thread_id, errno);

	if ((errno >= THINKOS_ERR_APP_INVALID) && 
		(errno <= THINKOS_ERR_APP_BSS_INVALID)) {
		DCC_LOG(LOG_ERROR, "Invalid application !");
		monitor_thread_break_clr();
		return;
	}

	if (!monitor_comm_isconnected(comm)) {
		DCC_LOG(LOG_ERROR, "Restarting!");
		thinkos_krn_sysrst();
		return;
	}
	struct krn_thread_state inf;


	if (errno == THINKOS_ERR_SYSCALL_INVALID) {
    	struct thinkos_context * ctx = monitor_thread_ctx_get(thread_id);
       	uint8_t * pc = (uint8_t *)ctx->pc;
		uint8_t syscall = pc[-2];;
		DCC_LOG1(LOG_ERROR, "Invalid System Call %d!", syscall);
 		monitor_thread_break_clr();

		monitor_printf(comm, "* Invalid SysCall %d [thread=%d errno=%d "
					   "addr=0x%08x]\r\n", syscall, thread_id, errno, (uintptr_t)pc); 

	} else {
		if (errno && thinkos_krn_thread_state_get(krn, thread_id, &inf)) {
			monitor_printf(comm, "* Error %s [thread=%d errno=%d "
						   "addr=0x%08x]\r\n", 
						   thinkos_krn_err_tag(inf.errno),
						   inf.thread_id, inf.errno, inf.ctx->pc);

			monitor_print_thread_state(comm, &inf);
		}
		struct thinkos_fault * fault = __thinkos_fault_rt();
		if (__thinkos_fault_is_valid(fault)) {
			monitor_print_fault(comm, fault);
		}
	}
	monitor_hbar(comm);
	monitor_print_newln(comm);
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
		monitor_newln(comm);
		monitor_hbar(comm);
		monitor_printf(mon->comm, "<%d> breakpoint @ 0x%08x\r\n", 
					  thread_id, inf.pc);
		mon->thread_id = thread_id;
		monitor_print_thread_state(comm, &inf);
		monitor_breakpoint_clear(inf.pc, 4);
		monitor_hbar(comm);
	}
}
#endif

#if (MONITOR_THREADINFO_ENABLE)
static void monitor_on_print_thread(struct monitor * mon, bool next,
									struct thinkos_krn * krn)
{
	struct krn_thread_state inf;
	int thread_id = mon->thread_id;

	if (next || thinkos_krn_thread_state_get(krn, thread_id, &inf)) { 
		thread_id = __thinkos_thread_getnext(thread_id);
		if (thread_id < 0)
			thread_id = __thinkos_thread_getnext(thread_id);
		mon->thread_id = thread_id;
		if (!thinkos_krn_thread_state_get(krn, thread_id, &inf)) {
			return;
		}
	}
	monitor_print_thread_state(mon->comm, &inf);
}
#endif

#if (MONITOR_THREAD_STEP_ENABLE)
static void monitor_on_step(struct monitor * mon, struct thinkos_krn * krn)
{
	const struct monitor_comm * comm = mon->comm;
	struct krn_thread_state inf;
	unsigned int thread_id;

	thread_id = monitor_thread_step_get();
	thinkos_krn_thread_state_get(thread_id, &inf);
	__thinkos_krn_pause_all(krn);

	if (monitor_comm_isconnected(comm)) {
		DCC_LOG2(LOG_TRACE, "<%d> step at %08x", thread_id, inf.pc);
		monitor_hbar(comm);
		mon->thread_id = thread_id;
		monitor_print_thread_state(comm, &inf);
		monitor_hbar(comm);
	}
}
#endif

#if (MONITOR_OS_PAUSE)
static void monitor_pause_all(const struct monitor_comm * comm, struct thinkos_krn * krn)
{
	monitor_thread_break_clr();

	monitor_printf(comm, "\r\nPausing all threads...\r\n");
	DCC_LOG(LOG_WARNING, "thinkos_dbg_pause_all()");
	thinkos_dbg_pause_all();
}
#endif

#if (MONITOR_OS_RESUME)
static void monitor_resume_all(const struct monitor_comm * comm, struct thinkos_krn * krn)
{
	monitor_printf(comm, "\r\nResuming all threads...\r\n");
	DCC_LOG(LOG_WARNING, "thinkos_dbg_resume_all()");
	thinkos_dbg_resume_all();
}
#endif

#if (MONITOR_DUMPMEM_ENABLE)
void monitor_show_mem(struct monitor * mon,
					  const struct thinkos_board * board)
{
	uint32_t addr = mon->memdump.addr;
	unsigned int size = mon->memdump.size;
	const struct thinkos_mem_desc * mem;

	monitor_printf(mon->comm, "Addr (0x%08x): ", addr);
	monitor_scanf(mon->comm, "%x", &addr);
	monitor_printf(mon->comm, "Size (%d): ", size);
	monitor_scanf(mon->comm, "%u", &size);

	mem = monitor_mem_lookup(board->memory->desc, board->memory->cnt, addr);
	monitor_hexdump(mon->comm, mem, addr, size);
	mon->memdump.addr = addr;
	mon->memdump.size = size;
}
#endif

#if (MONITOR_BREAKPOINT_ENABLE)
void monitor_breakpoint(struct monitor * mon, struct thinkos_krn * krn)
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
void monitor_watchpoint(struct monitor * mon, struct thinkos_krn * krn)
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

void boot_monitor_task(const struct monitor_comm * comm, void * arg, 
					   struct thinkos_krn * krn);

#if (MONITOR_GDB_ENABLE)
void __attribute__((naked)) gdb_bootstrap(const struct monitor_comm * comm, 
										  void * arg) 
{
	DCC_LOG1(LOG_TRACE, "sp=0x%08x", cm3_sp_get());
	gdb_stub_task(comm);
	monitor_exec(boot_monitor_task, comm, arg, NULL);
}
#endif

#if (MONITOR_BOARDINFO_ENABLE)
static void show_mem_info(const struct monitor_comm * comm, 
						  const struct thinkos_mem_desc * mem)
{
	const char * tag;
	uint32_t base;
	uint32_t size;
	int align;
	bool ro;
	int i;

	if (mem== NULL)
		return;

	monitor_printf(comm, "  %s:\r\n", mem->tag);
	for (i = 0; i < mem->cnt; ++i) {
		tag = mem->blk[i].tag;
		size = mem->blk[i].cnt << mem->blk[i].siz;
		base = mem->base + mem->blk[i].off;
		ro = (mem->blk[i].opt == M_RO) ? 1 : 0;
		align = ((mem->blk[i].opt & 3) + 1) * 8;

		monitor_printf(comm, "    %8s %08x-%08x %8d %6s %2d\r\n",
					  tag, base, base + size - 4, size, 
					  ro ? "RO" : "RW", align);
	}

}

static void monitor_board_info(const struct monitor_comm * comm, 
							   const struct thinkos_board * board)
{
	unsigned int i;

	monitor_printf(comm, s_hr);
	monitor_printf(comm, "Board: %s <%s>\r\n",
				  board->name, board->desc);
	monitor_printf(comm, "Hardware: %s revision %d.%d\r\n",
				  board->hw.tag, 
				  board->hw.ver.major, 
				  board->hw.ver.minor
				  );
	/* preprocessor running date and time */
	monitor_printf(comm, "Firmware: %s-%d.%d.%d (%s) " __DATE__ 
				  ", " __TIME__ "\r\n",
				  board->sw.tag, 
				  board->sw.ver.major, 
				  board->sw.ver.minor, 
				  board->sw.ver.build,
#if DEBUG
				  "debug"
#else
				  "release"
#endif
				  ); 
	/* compiler version string */
	monitor_printf(comm, "Compiler: GCC-" __VERSION__ "\r\n");

	/* memory blocks */
	monitor_printf(comm, "\r\nMemory Map:\r\n");
	monitor_printf(comm, "         Tag       Adress span"
				  "     Size  Flags  Align \r\n");

	for (i = 0; i < board->memory->cnt; ++i) {
		show_mem_info(comm, board->memory->desc[i]);
	}

#if (MONITOR_PROFILE_ENABLE)
	monitor_printf(comm, "\r\nKernel Profile:\r\n");
	monitor_print_profile(comm, &thinkos_profile);
#endif
}
#endif

static bool monitor_process_input(struct monitor * mon, int c, 
								  struct thinkos_krn * krn)
{
	const struct monitor_comm * comm = mon->comm;
	const struct thinkos_board * board = mon->board;

	switch (c) {
#if (MONITOR_GDB_ENABLE)
	case '+':
		monitor_exec(gdb_bootstrap, NULL);
		break;
#endif
#if (MONITOR_BOARDINFO_ENABLE)
   	case CTRL_B:
		monitor_printf(comm, "^B\r\n");
		monitor_board_info(comm, board);
		break;
#endif
	case CTRL_C:
		monitor_puts("^C\r\n", comm);
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
		monitor_on_print_thread(mon, true, krn);
		break;
#endif
#if (MONITOR_OSINFO_ENABLE)
	case CTRL_O:
		monitor_puts("^O\r\n", comm);
		monitor_puts(s_hr, comm);
		monitor_print_osinfo(comm, mon->cycref);
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
		monitor_puts("^S\r\n", comm);
		monitor_hbar(comm);
		monitor_thread_step(mon->thread_id, false);
		break;
#endif
#if (MONITOR_THREADINFO_ENABLE)
	case CTRL_T:
		monitor_on_print_thread(mon, true, krn);
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
	case CTRL_G:
		monitor_on_print_fault(comm, krn);
		break;
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
void __attribute__((noreturn)) boot_monitor_task(const struct monitor_comm * comm,
												void * arg, 
												struct thinkos_krn * krn)
{
	const struct thinkos_board * board;
	struct monitor monitor;
	uint32_t sigmask = 0;
	bool raw_mode = false;
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
#if (MONITOR_DUMPMEM_ENABLE)
	monitor.memdump.addr = 0;
	monitor.memdump.size = 1024;
#endif

	sigmask |= (1 << MONITOR_THREAD_FAULT);
	sigmask |= (1 << MONITOR_THREAD_BREAK);
	sigmask |= (1 << MONITOR_COMM_RCV);
	sigmask |= (1 << MONITOR_COMM_EOT);
	sigmask |= (1 << MONITOR_TX_PIPE);
	sigmask |= (1 << MONITOR_RX_PIPE);
	sigmask |= (1 << MONITOR_COMM_BRK);
	sigmask |= (1 << MONITOR_COMM_CTL);
	sigmask |= (1 << MONITOR_APP_TERM);

	sigmask |= (1 << MONITOR_THREAD_BREAK);
	sigmask |= (1 << MONITOR_APP_EXEC);
 	sigmask |= (1 << MONITOR_ON_CORE_RST);

#if (MONITOR_WATCHPOINT_ENABLE)
	sigmask |= (1 << MONITOR_BREAKPOINT);
#endif
#if (MONITOR_BREAKPOINT_ENABLE)
	sigmask |= (1 << MONITOR_BREAKPOINT);
#endif
#if (MONITOR_THREAD_STEP_ENABLE)
	sigmask |= (1 << MONITOR_THREAD_STEP);
#endif
	sigmask |= (1 << MONITOR_USER_EVENT3);

	monitor_unmask(MONITOR_COMM_BRK);
	monitor_unmask(MONITOR_COMM_CTL);

	DCC_LOG(LOG_TRACE, "================= ThinkOS Monitor ================="); 

	for(;;) {
		DCC_LOG1(LOG_MSG, "sigmask=%08x", sigmask); 
		switch ((sig = monitor_select(sigmask))) {
#if 0
			/* request a soft reset */
		case MONITOR_SOFTRST:
			/* Acknowledge the signal */
			monitor_clear(MONITOR_SOFTRST);
			DCC_LOG(LOG_WARNING, "/!\\ SOFTRST signal !");
			thinkos_krn_console_raw_mode_set(raw_mode = false);
			board->on_softreset();
			goto is_connected;
			break;
#endif
		case MONITOR_USR_ABORT:
			monitor_clear(MONITOR_USR_ABORT);
			monitor_puts("\r\n/!\\ USR ABORT\r\n", comm);
			DCC_LOG(LOG_TRACE, "/!\\ USR_ABORT signal...");
			monitor_signal(MONITOR_USER_EVENT3);
			break;

		case MONITOR_APP_TERM:
			monitor_clear(MONITOR_APP_TERM);
			DCC_LOG(LOG_TRACE, "/!\\ APP_TERM signal !");
     		thinkos_krn_req_core_rst(krn);
			break;

		case MONITOR_COMM_BRK:
			/* Acknowledge the signal */
			monitor_clear(MONITOR_COMM_BRK);
			DCC_LOG(LOG_WARNING, "/!\\ COMM_BREAK signal !");
			monitor_puts("\r\n+++\r\nBRK\r\n", comm);
			monitor_comm_break_ack(comm);
			thinkos_krn_req_core_rst(krn);
			break;

		case MONITOR_ON_CORE_RST:
			monitor_clear(MONITOR_ON_CORE_RST);
			thinkos_krn_console_raw_mode_set(raw_mode = false);
			board->on_softreset();
			if (board->on_break) {
				board->on_break(comm);
			}
			break;
#if 0
		case MONITOR_APP_STOP:
			monitor_clear(MONITOR_APP_STOP);
			raw_mode = false;
			DCC_LOG(LOG_TRACE, "/!\\ APP_STOP signal !");
			break;

		case MONITOR_APP_RESUME:
			monitor_clear(MONITOR_APP_RESUME);
			DCC_LOG(LOG_TRACE, "/!\\ APP_RESUME signal !");
			break;

		case MONITOR_THREAD_BREAK:
			monitor_clear(MONITOR_THREAD_BREAK);
			DCC_LOG(LOG_WARNING, "/!\\ THREAD_BREAK signal !");
			break;
#endif

#if (MONITOR_FAULT_ENABLE)
		case MONITOR_THREAD_FAULT:
			monitor_clear(MONITOR_THREAD_FAULT);
			DCC_LOG(LOG_TRACE, "Thread fault !.");
			thinkos_krn_console_raw_mode_set(raw_mode = false);
			/* Restore critical NVIC interrupts */
			monitor_on_thread_fault(comm, krn);
			break;
#endif

#if (MONITOR_BREAKPOINT_ENABLE)
		case MONITOR_BREAKPOINT:
			monitor_clear(MONITOR_BREAKPOINT);
			thinkos_krn_console_raw_mode_set(raw_mode = false);
			monitor_on_bkpt(&monitor);
			break;
#endif

#if (MONITOR_THREAD_STEP_ENABLE)
		case MONITOR_THREAD_STEP:
			monitor_clear(MONITOR_THREAD_STEP);
			DCC_LOG(LOG_INFO, "MONITOR_THREAD_STEP");
			monitor_on_step(&monitor, krn);
			break;
#endif

		case MONITOR_COMM_RCV:
			if ((raw_mode = thinkos_krn_console_is_raw_mode())) {
				DCC_LOG(LOG_INFO, "COMM_RCV...");
				sigmask = monitor_on_rx_pipe(comm, sigmask);
				break;
			}
			/* receive from the COMM driver one byte at the time */
			if ((cnt = monitor_comm_recv(comm, buf, 1)) > 0) {
				int c = buf[0];

				DCC_LOG1(LOG_INFO, "COMM_RCV: c=0x%02x", c);
				/* process the input character */
				if (!monitor_process_input(&monitor, c, krn)) {
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
			}
			break;

		case MONITOR_COMM_CTL:
			DCC_LOG(LOG_MSG, "/!\\ MONITOR_COMM_CTL");
			monitor_clear(MONITOR_COMM_CTL);
			{
				int status = monitor_comm_status_get(comm);

	        	if (status & COMM_ST_CONNECTED) {
					DCC_LOG(LOG_TRACE, "connected....");
				}

	        	if (status & COMM_ST_BREAK_REQ ) {
					monitor_comm_break_ack(comm);
					DCC_LOG(LOG_TRACE, "break_req....");
				}
			}

			sigmask = monitor_on_comm_ctl(comm, sigmask);
			DCC_LOG1(LOG_MSG, "sigmask=%08x", sigmask);
			break;

		case MONITOR_RX_PIPE:
			sigmask = monitor_on_rx_pipe(comm, sigmask);
			break;

		case MONITOR_COMM_EOT:
			/* FALLTHROUGH */
		case MONITOR_TX_PIPE:
			sigmask = monitor_on_tx_pipe(comm, sigmask);
			break;

		default:
			monitor_clear(sig);
			DCC_LOG1(LOG_WARNING, "Unhandled SIG %d!", sig);
			break;
		}
	}
}

