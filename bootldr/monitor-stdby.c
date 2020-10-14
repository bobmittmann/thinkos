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
/*
#define __THINKOS_DEBUG__
#include <thinkos/debug.h>
*/
#define __THINKOS_CONSOLE__
#include <thinkos/console.h>
#define __THINKOS_KRNSVC__
#include <thinkos/krnsvc.h>

#include <thinkos.h>
#include <vt100.h>

#include <sys/dcclog.h>

#ifndef BOOT_ENABLE_MONITOR
#define BOOT_ENABLE_MONITOR 1
#endif

/* FIXME: the GDB framework for the dbg monitor should be inside a thinkos
   debug library */

void gdb_stub_task(const struct monitor_comm * comm);

#ifndef MONITOR_VT100_ENABLE
#define MONITOR_VT100_ENABLE       1
#endif

#ifndef MONITOR_SELFTEST_ENABLE
#define MONITOR_SELFTEST_ENABLE    0
#endif

#ifndef MONITOR_CONFIGURE_ENABLE
#define MONITOR_CONFIGURE_ENABLE   1
#endif

#ifndef MONITOR_PREBOOT_ENABLE
#define MONITOR_PREBOOT_ENABLE     1
#endif

#ifndef MONITOR_DUMPMEM_ENABLE
#define MONITOR_DUMPMEM_ENABLE     1
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

#ifndef MONITOR_APPUPLOAD_ENABLE
#define MONITOR_APPUPLOAD_ENABLE   0
#endif

#ifndef MONITOR_APPWIPE_ENABLE
#define MONITOR_APPWIPE_ENABLE     0
#endif

#ifndef MONITOR_APPTERM_ENABLE
#define MONITOR_APPTERM_ENABLE     1
#endif

#ifndef MONITOR_APPRESTART_ENABLE
#define MONITOR_APPRESTART_ENABLE  1
#endif

#ifndef MONITOR_FAULT_ENABLE
#define MONITOR_FAULT_ENABLE       THINKOS_ENABLE_EXCEPTIONS
#endif

#ifndef MONITOR_EXCEPTION_ENABLE
#define MONITOR_EXCEPTION_ENABLE   THINKOS_ENABLE_EXCEPTIONS
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

#ifndef MONITOR_BOARDINFO_ENABLE
#define MONITOR_BOARDINFO_ENABLE 1
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

#if !(THINKOS_ENABLE_MONITOR_THREADS)
#error "Need THINKOS_ENABLE_MONITOR_THREADS"
#endif

#if !(THINKOS_ENABLE_IDLE_HOOKS)
#error "Need THINKOS_ENABLE_IDLE_HOOKS"
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

#if (BOOT_ENABLE_GDB)
#include <gdb.h>
#endif

extern int __heap_end;
const void * heap_end = &__heap_end; 
extern int __heap_base;
const void * heap_base = &__heap_base; 

struct monitor {
	const struct monitor_comm * comm;
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
};

static const char monitor_menu[] = 
"  \r\n"
" Debug/Monitor shortcuts:\r\n"
"  \r\n"
#if (MONITOR_BOARDINFO_ENABLE)
"   Ctrl+B - Show board configuration\r\n"
#endif
#if (MONITOR_APPTERM_ENABLE)
"   Ctrl+C - Stop/terminate the application\r\n"
#endif
#if (MONITOR_DUMPMEM_ENABLE)
"   Ctrl+D - Show memory region content\r\n"
#endif
#if (MONITOR_SELFTEST_ENABLE)
"   Ctrl+E - Execute the system selftest/diagnostic applet\r\n"
#endif
#if (MONITOR_BREAKPOINT_ENABLE)
"   Ctrl+F - Set breakpoint\r\n"
#endif

#if (MONITOR_EXCEPTION_ENABLE)
"   Ctrl+G - Show Exception info\r\n"
#endif

#if (MONITOR_CONFIGURE_ENABLE)
"   Ctrl+K - Execute the board configuration applet\r\n"
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
#if (MONITOR_RESTART_MONITOR)
"   Ctrl+Q - Restart monitor\r\n"
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
#if (MONITOR_APPWIPE_ENABLE)
"   Ctrl+W - Wipe/Erase application\r\n"
#endif
"   Ctrl+Y - YMODEM application upload\r\n"
#if (MONITOR_APPRESTART_ENABLE)
"   Ctrl+Z - Restart application\r\n"
#endif
#if (MONITOR_UPGRADE_ENABLE)
"   Ctrl+\\ - Upload ThinkOS/Bootloader\r\n"
#endif
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

#if (MONITOR_VT100_ENABLE)
static const char s_bottom[] = 
	VT100_SET_FONT_G0_ASCII VT100_FONT_SELECT_G0 VT100_SET_SCROLL_ALL 
	VT100_ATTR_NORMAL  VT100_GOTO(199,199) "\r\n";
#endif

static void monitor_show_help(const struct monitor_comm * comm)
{
#if (MONITOR_VT100_ENABLE)
	monitor_printf(comm, s_bottom);
#endif
	DCC_LOG2(LOG_TRACE, "sp=0x%08x comm=0x%08x", cm3_sp_get(), comm);
	monitor_printf(comm, s_hr);
	monitor_printf(comm, "%s-%d.%d.%d (%s):\r\n", 
			 this_board.sw.tag,
			 this_board.sw.ver.major,
			 this_board.sw.ver.minor,
			 this_board.sw.ver.build,
			 this_board.name);
	monitor_printf(comm, monitor_menu);
	monitor_printf(comm, s_hr);
}

#if (MONITOR_SELFTEST_ENABLE)
static void monitor_req_selftest(void) 
{
	monitor_soft_reset();
	monitor_signal(MONITOR_USER_EVENT0);
}
#endif

#if (MONITOR_CONFIGURE_ENABLE)
static void monitor_req_configure(void) 
{
	monitor_soft_reset();
	monitor_signal(MONITOR_USER_EVENT1);
}
#endif

#if (MONITOR_UPGRADE_ENABLE)
static void monitor_req_upgrade(void) 
{
	monitor_soft_reset();
	monitor_signal(MONITOR_USER_EVENT3);
}
#endif

static void monitor_thread_exec(int (* task)(void *), void * arg) 
{
	int thread_id;

	if (task != NULL) {
#if (THINKOS_ENABLE_THREAD_INFO)
		thread_id = monitor_thread_create(task, arg, &thinkos_main_inf);
#else
		thread_id = monitor_thread_create(task, arg, 0);
#endif
		monitor_thread_resume(thread_id);
	}
}

#if (MONITOR_EXCEPTION_ENABLE)
static void monitor_print_fault(const struct monitor_comm * comm)
{
	struct thinkos_except * xcpt = &thinkos_except_buf;

	monitor_print_exception(comm, xcpt);
}

static const char s_crlf[] =  "\r\n";

static void monitor_on_thread_fault(const struct monitor_comm * comm)
{
	struct monitor_thread_inf inf;
	int thread_id;

	/* get the last thread known to be at fault */
	thread_id = monitor_thread_break_get();
	monitor_thread_inf_get(thread_id, &inf);
	__thinkos_pause_all();

	DCC_LOG2(LOG_ERROR, "<%d> fault @ 0x%08x !!", thread_id + 1, inf.pc);

	if (monitor_comm_isconnected(comm)) {
		struct thinkos_except * xcpt = &thinkos_except_buf;

		DCC_LOG(LOG_TRACE, "COMM connected!");
		monitor_printf(comm, s_crlf);
		monitor_printf(comm, s_hr);
		monitor_printf(comm, "* Fault %s [thread=%d errno=%d addr=0x%08x]\r\n", 
						  thinkos_err_name_lut[inf.errno],
						  inf.thread_id + 1,
						  inf.errno,
						  inf.pc);
		if (xcpt->errno != THINKOS_NO_ERROR)
			monitor_print_exception(comm, xcpt);
		else
			monitor_print_thread(comm, thread_id);
		monitor_printf(comm, s_hr);

	}

	/* turn the scheduler back on */
	//thinkos_krn_sched_on();

	DCC_LOG(LOG_TRACE, "done.");
}

#endif

static void monitor_on_krn_except(const struct monitor_comm * comm)
{
	struct monitor_thread_inf inf;
	int thread_id;

	mdelay(500);

	/* get the last thread known to be at fault */
	thread_id = monitor_thread_break_get();
	monitor_thread_inf_get(thread_id, &inf);

	DCC_LOG2(LOG_ERROR, "<%d> fault @ 0x%08x !!", thread_id + 1, inf.pc);

	if (monitor_comm_isconnected(comm)) {
		struct thinkos_except * xcpt = &thinkos_except_buf;

		DCC_LOG(LOG_TRACE, "COMM connected!");
		monitor_printf(comm, s_hr);
	
		if (xcpt->errno == THINKOS_ERR_INVALID_STACK) {
			monitor_printf(comm, 
						  "# Kernel error, possible stack overflow !!!\r\n");
			monitor_printf(comm, " Offended thread: %d\r\n", thread_id + 1);
		} else {
			monitor_printf(comm, "Exception!!!\r\n");
		}

#if (MONITOR_FAULT_ENABLE)
		monitor_print_thread(comm, thread_id);
		monitor_printf(comm, s_hr);
#endif
	} else {
	}

	mdelay(500);

	monitor_soft_reset();

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

	DCC_LOG2(LOG_TRACE, "<%d> breakpoint @ 0x%08x", thread_id + 1, inf.pc);

	if (monitor_comm_isconnected(comm)) {
		monitor_printf(comm, s_hr);
		monitor_printf(mon->comm, "<%d> breakpoint @ 0x%08x\r\n", 
					  thread_id + 1, inf.pc);
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
	struct monitor_thread_inf inf;
	unsigned int thread_id;

	thread_id = monitor_thread_step_get();
	monitor_thread_inf_get(thread_id, &inf);
	__thinkos_pause_all();

	if (monitor_comm_isconnected(comm)) {
		DCC_LOG2(LOG_TRACE, "<%d> step at %08x", thread_id + 1, inf.pc);
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
	DCC_LOG(LOG_WARNING, "__thinkos_pause_all()");
	__thinkos_pause_all();
	if (monitor_wait_idle() < 0) {
		DCC_LOG(LOG_WARNING, "monitor_wait_idle() failed!");
	}
}
#endif

#if (MONITOR_OS_RESUME)
static void monitor_resume_all(const struct monitor_comm * comm)
{
	monitor_printf(comm, "\r\nResuming all threads...\r\n");
	__thinkos_resume_all();
	monitor_printf(comm, "Restarting...\r\n");
}
#endif

#if (MONITOR_APPUPLOAD_ENABLE)
static bool monitor_ymodem_recv(const struct monitor_comm * comm, 
								uint32_t addr, unsigned int size)
{
	monitor_printf(comm, "\r\nYMODEM receive (^X to cancel) ... ");
	if (monitor_ymodem_flash(comm, addr, size) < 0) {
		monitor_printf(comm, "\r\n#ERROR: YMODEM failed!\r\n"); 
		return false;
	}	
	monitor_printf(comm, "\r\nOK\r\n");
	return true;
}
#endif

#if (MONITOR_APPWIPE_ENABLE)
static void monitor_app_erase(const struct monitor_comm * comm, 
							  uint32_t addr, unsigned int size)
{
	monitor_printf(comm, "\r\nErasing application block (%08x)... ", addr);
	monitor_flash_open("APP");
	if (monitor_flash_erase(addr, size) < 0 )
		monitor_printf(comm, "failed!\r\n");
	else	
		monitor_printf(comm, "done.\r\n");
	monitor_flash_close();
}
#endif

#if (MONITOR_DUMPMEM_ENABLE)
void monitor_show_mem(struct monitor * mon)
{
	uint32_t addr = mon->memdump.addr;
	unsigned int size = mon->memdump.size;
	const struct mem_desc * mem;

	monitor_printf(mon->comm, "Addr (0x%08x): ", addr);
	monitor_scanf(mon->comm, "%x", &addr);
	monitor_printf(mon->comm, "Size (%d): ", size);
	monitor_scanf(mon->comm, "%u", &size);

	mem = monitor_mem_lookup(this_board.memory.lst, 3, addr);
	monitor_hexdump(mon->comm, mem, addr, size);
	mon->memdump.addr = addr;
	mon->memdump.size = size;
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

void monitor_task(const struct monitor_comm *, void * arg);

#if (BOOT_ENABLE_GDB)
void __attribute__((naked)) gdb_bootstrap(const struct monitor_comm * comm, 
										  void * arg) 
{
	DCC_LOG1(LOG_TRACE, "sp=0x%08x", cm3_sp_get());
	gdb_stub_task(comm);
	monitor_exec(monitor_task, arg);
}
#endif

#if (BOOT_ENABLE_THIRD)
void third_stub_task(struct monitor_comm *);

void __attribute__((naked)) third_bootstrap(const struct monitor_comm * comm, 
											void * arg) 
{
	/* call the THIRD stub task */
	third_stub_task(comm);
	/* return to the monitor */
	monitor_exec(monitor_task, arg);
}
#endif

#if (MONITOR_BOARDINFO_ENABLE)
static void show_mem_info(const struct monitor_comm * comm, 
						  const struct mem_desc * mem)
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
	for (i = 0; mem->blk[i].cnt != 0; ++i) {
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

static void monitor_board_info(const struct monitor_comm * comm)
{
	monitor_printf(comm, s_hr);
	monitor_printf(comm, "Board: %s <%s>\r\n",
				  this_board.name, this_board.desc);
	monitor_printf(comm, "Hardware: %s revision %d.%d\r\n",
				  this_board.hw.tag, 
				  this_board.hw.ver.major, 
				  this_board.hw.ver.minor
				  );
	/* preprocessor running date and time */
	monitor_printf(comm, "Firmware: %s-%d.%d.%d (%s) " __DATE__ 
				  ", " __TIME__ "\r\n",
				  this_board.sw.tag, 
				  this_board.sw.ver.major, 
				  this_board.sw.ver.minor, 
				  this_board.sw.ver.build,
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

	show_mem_info(comm, this_board.memory.flash);
	show_mem_info(comm, this_board.memory.ram);
	show_mem_info(comm, this_board.memory.periph);

	monitor_printf(comm, "\r\nKernel Profile:\r\n");
	monitor_print_profile(comm, &thinkos_profile);
}
#endif

static bool monitor_process_input(struct monitor * mon, int c)
{
	const struct monitor_comm * comm = mon->comm;

	switch (c) {
#if (BOOT_ENABLE_GDB)
	case '+':
		monitor_exec(gdb_bootstrap, NULL);
		break;
#endif
#if (BOOT_ENABLE_THIRD)
	case '~':
		monitor_exec(third_bootstrap, NULL);
		break;
#endif
#if (MONITOR_BOARDINFO_ENABLE)
   	case CTRL_B:
		monitor_printf(comm, "^B\r\n");
		monitor_board_info(comm);
		break;
#endif
#if (MONITOR_APPTERM_ENABLE)
	case CTRL_C:
		monitor_printf(comm, "^C\r\n");
		monitor_req_app_term();
		break;
#endif
#if (MONITOR_DUMPMEM_ENABLE)
	case CTRL_D:
		monitor_printf(comm, "^D\r\n");
		monitor_show_mem(mon);
		break;
#endif
#if (MONITOR_SELFTEST_ENABLE)
	case CTRL_E:
		monitor_printf(comm, "^E\r\n");
		monitor_req_selftest();
		break;
#endif
#if (MONITOR_BREAKPOINT_ENABLE)
	case CTRL_F:
		monitor_printf(comm, "^F\r\n");
		monitor_breakpoint(mon);
		break;
#endif
#if (MONITOR_CONFIGURE_ENABLE)
	case CTRL_K:
		monitor_printf(comm, "^K\r\n");
		monitor_req_configure();
		break;
#endif
#if (MONITOR_UPGRADE_ENABLE)
	case CTRL_FS:
		monitor_printf(comm, "^\\\r\n");
		monitor_req_upgrade();
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
		monitor_printf(comm, s_hr);
		monitor_print_osinfo(comm);
		break;
#endif
#if (MONITOR_OS_PAUSE)
	case CTRL_P:
		monitor_printf(comm, "^P\r\n");
		monitor_pause_all(comm);
		break;
#endif
#if (MONITOR_RESTART_MONITOR)
	case CTRL_Q:
		monitor_printf(comm, "^Q\r\n");
		monitor_exec(monitor_task, NULL);
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
		monitor_show_help(comm);
		break;
#if (MONITOR_FAULT_ENABLE)
	case CTRL_G:
		monitor_print_fault(comm);
		break;
#endif
	case CTRL_Y:
		monitor_printf(comm, "^Y\r\nUpload application [y]? ");
		if (monitor_getc(comm) == 'y') {
			/* Request app upload */
			monitor_req_app_upload();
		} else {
			monitor_printf(comm, "\r\n");
		}
		break;
#if (MONITOR_APPWIPE_ENABLE)
	case CTRL_W:
		monitor_printf(comm, "^W\r\nErase application [y]? ");
		if (monitor_getc(comm) == 'y') {
			/* Request app erase */
			monitor_req_app_erase(); 
		} else {
			monitor_printf(comm, "\r\n");
		}
		break;
#endif
#if (MONITOR_APPRESTART_ENABLE)
	case CTRL_Z:
		monitor_printf(comm, "^Z\r\n");
		monitor_req_app_exec(); 
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
void __attribute__((noreturn)) monitor_task(const struct monitor_comm * comm,
											void * arg)
{
	struct monitor monitor;
	uint32_t sigmask = 0;
#if (THINKOS_ENABLE_CONSOLE)
  #if (THINKOS_ENABLE_CONSOLE_MODE)
	bool raw_mode = false;
  #endif 
	uint8_t * ptr;
	int cnt;
#endif
	bool startup = false;
#if (MONITOR_SELFTEST_ENABLE)
	bool selftest = false;
#endif
#if (MONITOR_CONFIGURE_ENABLE)
	bool config = false;
#endif
#if (MONITOR_PREBOOT_ENABLE)
	bool preboot = false;
#endif
	uint8_t buf[1];
	int sig;
	bool connected = false;
	
	monitor.comm = comm;
#if (MONITOR_THREADINFO_ENABLE)
	monitor.thread_id = -1;
#endif
#if (MONITOR_DUMPMEM_ENABLE)
	monitor.memdump.addr = this_board.application.start_addr;
	monitor.memdump.size = this_board.application.block_size;
#endif

	sigmask |= (1 << MONITOR_SOFTRST);
	sigmask |= (1 << MONITOR_STARTUP);
#if (MONITOR_EXCEPTION_ENABLE)
	sigmask |= (1 << MONITOR_THREAD_FAULT);
	sigmask |= (1 << MONITOR_KRN_EXCEPT);
#endif
	sigmask |= (1 << MONITOR_COMM_RCV);
#if THINKOS_ENABLE_CONSOLE
	sigmask |= (1 << MONITOR_COMM_CTL);
	sigmask |= (1 << MONITOR_TX_PIPE);
//	sigmask |= (1 << MONITOR_RX_PIPE);
#endif
	sigmask |= (1 << MONITOR_APP_STOP);
	sigmask |= (1 << MONITOR_APP_EXEC);
	sigmask |= (1 << MONITOR_APP_UPLOAD);
	sigmask |= (1 << MONITOR_APP_ERASE);
	sigmask |= (1 << MONITOR_APP_TERM);
	sigmask |= (1 << MONITOR_APP_RESUME);
#if (MONITOR_WATCHPOINT_ENABLE)
	sigmask |= (1 << MONITOR_BREAKPOINT);
#endif
#if (MONITOR_BREAKPOINT_ENABLE)
	sigmask |= (1 << MONITOR_BREAKPOINT);
#endif
#if (MONITOR_THREAD_STEP_ENABLE)
	sigmask |= (1 << MONITOR_THREAD_STEP);
#endif
#if (MONITOR_SELFTEST_ENABLE)
	sigmask |= (1 << MONITOR_USER_EVENT0);
#endif
#if (MONITOR_CONFIGURE_ENABLE)
	sigmask |= (1 << MONITOR_USER_EVENT1);
#endif
#if (MONITOR_PREBOOT_ENABLE)
	sigmask |= (1 << MONITOR_USER_EVENT2);
#endif
#if (MONITOR_UPGRADE_ENABLE)
	sigmask |= (1 << MONITOR_USER_EVENT3);
#endif
	sigmask |= (1 << MONITOR_THREAD_CREATE);
	sigmask |= (1 << MONITOR_THREAD_TERMINATE);

#if 0
	sigmask |= (1 << MONITOR_ALARM);
	monitor_alarm(1000);
#endif

	for(;;) {
		switch ((sig = monitor_select(sigmask))) {

		case MONITOR_STARTUP:
			DCC_LOG1(LOG_TRACE, "/!\\ STARTUP signal (SP=0x%08x)...", 
					 cm3_sp_get());
			monitor_clear(MONITOR_STARTUP);
			startup = true;
			break;

		case MONITOR_COMM_RCV:
			DCC_LOG(LOG_MSG, "COMM_RCV: +++++++++++++++++");
#if (THINKOS_ENABLE_CONSOLE)
  #if (THINKOS_ENABLE_CONSOLE_MODE)
			raw_mode = thinkos_krn_console_is_raw_mode();
			if (raw_mode) {
				goto raw_mode_recv;
			}
  #endif /* THINKOS_ENABLE_CONSOLE_MODE */

			/* receive from the COMM driver one byte at the time */
			if ((cnt = monitor_comm_recv(comm, buf, 1)) > 0) {
				int c = buf[0];

				DCC_LOG1(LOG_MSG, "COMM_RCV: c=0x%02x", c);
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
			}
			break;

  #if (THINKOS_ENABLE_CONSOLE_MODE)
raw_mode_recv:
			/* get a pointer to the head of the pipe.
			   thinkos_console_rx_pipe_ptr() will return the number of 
			   consecutive spaces in the buffer. */
			if ((cnt = thinkos_console_rx_pipe_ptr(&ptr)) > 0) {
				int n;
			
				DCC_LOG1(LOG_MSG, "Raw mode RX, cnt=%d", cnt);

				/* receive from the COMM driver */
				if ((n = monitor_comm_recv(comm, ptr, cnt)) > 0) {
					/* commit the fifo head */
					thinkos_console_rx_pipe_commit(n);
					if (n == cnt) {
						/* Wait for RX_PIPE */
						DCC_LOG(LOG_TRACE, "Wait for RX_PIPE && COMM_RECV");
						sigmask |= (1 << MONITOR_COMM_RCV);
						sigmask &=  ~(1 << MONITOR_RX_PIPE);
						//sigmask &=  ~(1 << MONITOR_RX_PIPE);
					} else {
						/* Wait for COMM_RECV */
						DCC_LOG(LOG_TRACE, "Wait for COMM_RECV");
						sigmask |= (1 << MONITOR_COMM_RCV);
						sigmask &=  ~(1 << MONITOR_RX_PIPE);
					}
				} else {
					DCC_LOG1(LOG_ERROR, "monitor_comm_recv n=%d", n);
					/* Wait for COMM_RECV */
					sigmask |= (1 << MONITOR_COMM_RCV);
					sigmask &=  ~(1 << MONITOR_RX_PIPE);
				}
			} else {
				DCC_LOG(LOG_TRACE, "Raw mode RX wait RX_PIPE");
				/* Wait for RX_PIPE */
				sigmask &= ~(1 << MONITOR_COMM_RCV);
				sigmask |= (1 << MONITOR_RX_PIPE);
			}
			break;

  #endif /* THINKOS_ENABLE_CONSOLE_MODE */
#else
			if (monitor_comm_recv(comm, buf, 1) > 0) {
				/* process the input character */
				monitor_process_input(&monitor, buf[0]);
			}
#endif /* THINKOS_ENABLE_CONSOLE */

			DCC_LOG(LOG_TRACE, "COMM_RCV: ----------------");
			break;

#if (THINKOS_ENABLE_CONSOLE)
		case MONITOR_COMM_CTL:
			monitor_clear(MONITOR_COMM_CTL);
			DCC_LOG(LOG_TRACE, "COMM_CTL !!!!!");
is_connected:
			{
				connected = monitor_comm_isconnected(comm);
				thinkos_krn_console_connect_set(connected);
#if (THINKOS_ENABLE_CONSOLE_MODE)
				raw_mode = thinkos_krn_console_is_raw_mode();
#endif /* THINKOS_ENABLE_CONSOLE_MODE */

				sigmask &= ~((1 << MONITOR_COMM_EOT) | 
							 (1 << MONITOR_COMM_RCV) |
							 (1 << MONITOR_RX_PIPE));
				if (connected) {
					sigmask |= ((1 << MONITOR_COMM_EOT) |
								(1 << MONITOR_COMM_RCV));
#if (THINKOS_ENABLE_CONSOLE_MODE)
					if (raw_mode) {
						DCC_LOG(LOG_TRACE, "Comm connected raw mode!");
					} else 
#endif /* THINKOS_ENABLE_CONSOLE_MODE */
					{
						DCC_LOG(LOG_TRACE, "Comm connected.");
					}
				} else {
					DCC_LOG(LOG_TRACE, "Comm not connected!");
				}

				sigmask |= (1 << MONITOR_TX_PIPE);
			}
			break;

#if 0
		case MONITOR_RX_PIPE:
			monitor_clear(MONITOR_RX_PIPE);
			DCC_LOG(LOG_TRACE, "RX Pipe!");
			break;
#endif

		case MONITOR_COMM_EOT:
			DCC_LOG(LOG_TRACE, "COMM_EOT");
			/* FALLTHROUGH */
		case MONITOR_TX_PIPE:
			if ((cnt = thinkos_console_tx_pipe_ptr(&ptr)) > 0) {
				int n;
				DCC_LOG1(LOG_TRACE, "TX Pipe: cnt=%d, send...", cnt);
				if ((n = monitor_comm_send(comm, ptr, cnt)) > 0) {
					thinkos_console_tx_pipe_commit(n);
					if (n == cnt) {
						/* Wait for TX_PIPE */
						sigmask |= (1 << MONITOR_TX_PIPE);
						sigmask &= ~(1 << MONITOR_COMM_EOT);
					} else {
						/* Wait for COMM_EOT */
						sigmask |= (1 << MONITOR_COMM_EOT);
						sigmask &= ~(1 << MONITOR_TX_PIPE);
					}
				} else {
					/* Wait for COMM_EOT */
					sigmask |= (1 << MONITOR_COMM_EOT);
					sigmask &=  ~(1 << MONITOR_TX_PIPE);
				}
			} else {
				/* Wait for TX_PIPE */
				DCC_LOG1(LOG_TRACE, "TX Pipe: cnt=%d, wait....", cnt);
				sigmask |= (1 << MONITOR_TX_PIPE);
				sigmask &= ~(1 << MONITOR_COMM_EOT);
			}
			break;

		case MONITOR_RX_PIPE:
			/* get a pointer to the head of the pipe.
			   thinkos_console_rx_pipe_ptr() will return the number of 
			   consecutive spaces in the buffer. */
			if ((cnt = thinkos_console_rx_pipe_ptr(&ptr)) > 0) {
				int n;
			
				/* receive from the COMM driver */
				if ((n = monitor_comm_recv(comm, ptr, cnt)) > 0) {
					/* commit the fifo head */
					thinkos_console_rx_pipe_commit(n);
					if (n == cnt) {
						/* Wait for RX_PIPE */
						DCC_LOG(LOG_TRACE, 
								"RX_PIPE: Wait for RX_PIPE && COMM_RECV");
						sigmask |= (1 << MONITOR_COMM_RCV);
						sigmask &=  ~(1 << MONITOR_RX_PIPE);
					} else {
						DCC_LOG(LOG_TRACE, "RX_PIPE: Wait for COMM_RECV");
						/* Wait for COMM_RECV */
						sigmask |= (1 << MONITOR_COMM_RCV);
						sigmask &=  ~(1 << MONITOR_RX_PIPE);
					}
				} else {
					/* Wait for COMM_RECV */
					DCC_LOG(LOG_ERROR, "RX_PIPE: Wait for COMM_RECV");
					sigmask |= (1 << MONITOR_COMM_RCV);
					sigmask &=  ~(1 << MONITOR_RX_PIPE);
				}
			} else {
				DCC_LOG1(LOG_ERROR, "RX_PIPE: RX, cnt=%d", cnt);
				/* Wait for RX_PIPE */
				sigmask &= ~(1 << MONITOR_COMM_RCV);
				sigmask |= (1 << MONITOR_RX_PIPE);
			}
			break;
#endif /* THINKOS_ENABLE_CONSOLE */

#if (MONITOR_SELFTEST_ENABLE)
		case MONITOR_USER_EVENT0:
			DCC_LOG(LOG_TRACE, "MONITOR_USER_EVENT0: self test!");
			monitor_clear(MONITOR_USER_EVENT0);
			monitor_thread_exec(this_board.selftest_task, NULL);
			break;
#endif

#if (MONITOR_CONFIGURE_ENABLE)
		case MONITOR_USER_EVENT1:
			DCC_LOG(LOG_TRACE, "MONITOR_USER_EVENT1: configure!");
			monitor_clear(MONITOR_USER_EVENT1);
			monitor_thread_exec(this_board.configure_task, NULL);
			break;
#endif

#if (MONITOR_PREBOOT_ENABLE)
		case MONITOR_USER_EVENT2:
			DCC_LOG(LOG_TRACE, "MONITOR_USER_EVENT2: preboot!");
			monitor_clear(MONITOR_USER_EVENT2);
			preboot	= true;
			monitor_thread_exec(this_board.preboot_task, NULL);
			break;
#endif
#if (MONITOR_UPGRADE_ENABLE)
		case MONITOR_USER_EVENT3:
			DCC_LOG(LOG_TRACE, "MONITOR_USER_EVENT2: preboot!");
			monitor_clear(MONITOR_USER_EVENT3);
			monitor_printf(comm, "Confirm [y]? ");
			if (monitor_getc(comm) == 'y') {
				this_board.upgrade(comm);
				monitor_printf(comm, "Failed !!!\r\n");
			} else {
				monitor_printf(comm, "\r\n");
			}
			break;
#endif

#if 0
		case MONITOR_ALARM:
			DCC_LOG(LOG_TRACE, "Alarm !");
			monitor_alarm(1000);
			break;
#endif
		}
		DCC_LOG1(LOG_JABBER, "SIG %d!", sig);
	}
}

