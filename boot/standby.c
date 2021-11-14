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
 * @file standby.c
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
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
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

void __attribute__((noreturn)) app_task(void *, unsigned int);

void boot_monitor_task(const struct monitor_comm * comm, void * arg);
void boot_monitor_task(const struct monitor_comm * comm, void * arg);

/*
   Default Monitor Task
 */
void __attribute__((noreturn)) 
standby_monitor_task(const struct monitor_comm * comm, void * arg)
{
	const struct thinkos_board * board;
	uint32_t sigmask = 0;
	int sig;

	board = (const struct thinkos_board *)arg;

	sigmask |= (1 << MONITOR_KRN_FAULT);
	sigmask |= (1 << MONITOR_KRN_ABORT);
	sigmask |= (1 << MONITOR_SOFTRST);
	sigmask |= (1 << MONITOR_COMM_BRK);

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
	sigmask |= (1 << MONITOR_THREAD_BREAK);
	sigmask |= (1 << MONITOR_THREAD_CREATE);
	sigmask |= (1 << MONITOR_THREAD_TERMINATE);

#if 0
	sigmask |= (1 << MONITOR_ALARM);
	monitor_alarm(1000);
#endif

	DCC_LOG(LOG_TRACE, "================= ThinkOS Monitor ================="); 

	for(;;) {
		DCC_LOG1(LOG_TRACE, "sigmask=%08x", sigmask); 
		switch ((sig = monitor_select(sigmask))) {

		case MONITOR_KRN_ABORT:
			monitor_clear(MONITOR_KRN_ABORT);
			DCC_LOG1(LOG_TRACE, "/!\\ KRN_ABORT signal (SP=0x%08x)...", 
					 cm3_sp_get());
			break;

		case MONITOR_SOFTRST:
			/* Acknowledge the signal */
			monitor_clear(MONITOR_SOFTRST);
			DCC_LOG(LOG_WARNING, "/!\\ SOFTRST signal !");
			board->on_softreset();
			break;

		case MONITOR_APP_EXEC:
			monitor_clear(MONITOR_APP_EXEC);
			DCC_LOG(LOG_TRACE, "/!\\ APP_EXEC signal !");


			if (!monitor_app_exec(comm)) {
				monitor_printf(comm, "Can't run application!\r\n");
				/* XXX: this event handler could be optionally compiled
				   to save some resources. As a matter of fact I don't think
				   they are useful at all */
				DCC_LOG(LOG_TRACE, "monitor_app_exec() failed!");
//				if (board->default_task != NULL) {
//					DCC_LOG(LOG_TRACE, "default_task()...!");
//					monitor_thread_create(comm, C_TASK(board->default_task), 
//										 C_ARG(NULL), true);
//				} else {
//					DCC_LOG(LOG_TRACE, "no default app set!");
//				}
			}
			DCC_LOG(LOG_TRACE, "APP_EXEC done");
			break;

		case MONITOR_THREAD_TERMINATE:
			monitor_clear(MONITOR_THREAD_TERMINATE);
			break;

		case MONITOR_COMM_RCV:
			sigmask = monitor_on_comm_rcv(comm, sigmask);
			break;

		case MONITOR_COMM_CTL:
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

			/* FALLTHROUGH */
		default:
			monitor_exec(boot_monitor_task, arg);
			DCC_LOG1(LOG_WARNING, "unhandled signal: %d", sig);
		}
	}
}


void __attribute__((noreturn)) 
init_monitor_task(const struct monitor_comm * comm, void * arg)
{
	const struct thinkos_board * board;
	uint32_t sigmask = 0;
	bool startup = false;
	int sig;

	board = (const struct thinkos_board *)arg;

	sigmask |= (1 << MONITOR_SOFTRST);
	sigmask |= (1 << MONITOR_THREAD_FAULT);
	sigmask |= (1 << MONITOR_KRN_FAULT);
	sigmask |= (1 << MONITOR_COMM_BRK);
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
	sigmask |= (1 << MONITOR_THREAD_CREATE);
	sigmask |= (1 << MONITOR_THREAD_TERMINATE);

#if 0
	sigmask |= (1 << MONITOR_ALARM);
	monitor_alarm(1000);
#endif

	DCC_LOG(LOG_TRACE, "================= ThinkOS Monitor ================="); 

	for(;;) {
		DCC_LOG1(LOG_TRACE, "sigmask=%08x", sigmask); 
		switch ((sig = monitor_select(sigmask))) {

		case MONITOR_KRN_ABORT:
			monitor_clear(MONITOR_KRN_ABORT);
			DCC_LOG1(LOG_TRACE, "/!\\ KRN_ABORT signal (SP=0x%08x)...", 
					 cm3_sp_get());
			break;

		case MONITOR_SOFTRST:
			/* Acknowledge the signal */
			monitor_clear(MONITOR_SOFTRST);
			DCC_LOG(LOG_WARNING, "/!\\ SOFTRST signal !");
			board->on_softreset();
			break;

		case MONITOR_APP_EXEC:
			monitor_clear(MONITOR_APP_EXEC);
			DCC_LOG(LOG_TRACE, "/!\\ APP_EXEC signal !");


			if (!monitor_app_exec(comm)) {
				monitor_printf(comm, "Can't run application!\r\n");
				/* XXX: this event handler could be optionally compiled
				   to save some resources. As a matter of fact I don't think
				   they are useful at all */
				DCC_LOG(LOG_TRACE, "monitor_app_exec() failed!");
//				if (board->default_task != NULL) {
//					DCC_LOG(LOG_TRACE, "default_task()...!");
//					monitor_thread_create(comm, C_TASK(board->default_task), 
//										 C_ARG(NULL), true);
//				} else {
//					DCC_LOG(LOG_TRACE, "no default app set!");
//				}
			}
			DCC_LOG(LOG_TRACE, "APP_EXEC done");
			break;

		case MONITOR_THREAD_TERMINATE:
			DCC_LOG(LOG_TRACE, "/!\\ THREAD_TERMINATE");

			monitor_wait_idle();

			if (!startup) {
				int thread_id;
				int code;

				monitor_clear(MONITOR_THREAD_TERMINATE);
				thread_id = monitor_thread_terminate_get(&code);
				(void)thread_id; 
				(void)code; 
				DCC_LOG2(LOG_TRACE, "/!\\ THREAD_TERMINATE id=%d code=%d",
						 thread_id, code);
				break;
			}

			startup = false;
			DCC_LOG(LOG_TRACE, "APP exec request");
			monitor_req_app_exec();
			break;


		case MONITOR_COMM_RCV:
			sigmask = monitor_on_comm_rcv(comm, sigmask);
			break;

		case MONITOR_COMM_CTL:
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

			/* FALLTHROUGH */
		default:
			monitor_exec(boot_monitor_task, arg);
			DCC_LOG1(LOG_WARNING, "unhandled signal: %d", sig);
		}
	}
}



