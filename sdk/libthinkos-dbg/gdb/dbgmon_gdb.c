/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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
 * @file gdb-rsp.c
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include <gdbrsp.h>
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>

#include <sys/dcclog.h>
#include <vt100.h>

#define CTRL_C 0x03

#ifndef RSP_BUFFER_LEN
#define RSP_BUFFER_LEN 256
#endif

#ifndef GDB_DEBUG_PACKET
#define GDB_DEBUG_PACKET 1
#endif

#ifndef GDB_IDLE_TIMEOUT_MS 
#define GDB_IDLE_TIMEOUT_MS 1000
#endif

#ifndef GDB_ENABLE_CONSOLE
#define GDB_ENABLE_CONSOLE 0
#endif

//struct gdbrspd gdbrspd;
int dbgmon_gdbrsp_comm_send(void * attr, const void * buf, size_t len)
{
	return dbgmon_comm_send(attr, buf, len);
}

int dbgmon_gdbrsp_comm_recv(void * attr, void * buf, size_t len)
{
	return dbgmon_comm_recv(attr, buf, len);
}

const struct gdbrsp_comm_op dbgmon_gdbrsp_comm_op  = {
	.send = (int (*)(void *, const void *, size_t))dbgmon_gdbrsp_comm_send,
	.recv = (int (*)(void *, void *, size_t))dbgmon_gdbrsp_comm_recv 
};

const struct gdbrsp_target_op dbgmon_gdbrsp_target_op  = {
	.init = (int (*)(void *))NULL,
	.mem_write = (int (*)(void *, uint32_t addr, const void * ptr, unsigned int len)) NULL,
	.mem_read = (int (*)(void *, uint32_t addr, void * ptr, unsigned int len)) NULL,
	.file_read = (int (*)(void *, const char * name, char * dst, 
					  unsigned int offs, unsigned int size)) NULL,
	.cpu_halt = (int (*)(void *)) NULL,
	.cpu_continue = (int (*)(void *)) NULL,
	.cpu_goto = (int (*)(void *, uint32_t addr, int opt)) NULL,
	.cpu_run = (int (*)(void *, uint32_t addr, int opt)) NULL,
	.cpu_reset = (int (*)(void *)) NULL,
	.app_exec = (int (*)(void *)) NULL,
	.thread_getnext = (int (*)(void *, int thread_id)) NULL,
	.thread_active = (int (*)(void *)) NULL,
	.thread_break_id = (int (*)(void *)) NULL,
	.thread_any = (int (*)(void *)) NULL,
	.thread_isalive = (bool (*)(void *, int thread_id)) NULL,
	.thread_register_get = (int (*)(void *, int thread_id, int reg, uint64_t * val)) NULL,
	.thread_register_set = (int (*)(void *, unsigned int thread_id, int reg, uint64_t val)) NULL,
	.thread_goto = (int (*)(void *, unsigned int thread_id, uint32_t addr)) NULL,
	.thread_step_req = (int (*)(void *, unsigned int thread_id)) NULL,
	.thread_continue = (int (*)(void *, unsigned int thread_id)) NULL,
	.thread_info = (int (*)(void *, unsigned int gdb_thread_id, char * buf)) NULL,
	.breakpoint_clear_all = (int (*)(void *)) NULL,
	.watchpoint_clear_all = (int (*)(void *)) NULL,
	.breakpoint_set = (int (*)(void *, uint32_t addr, unsigned int size)) NULL,
	.breakpoint_clear = (int (*)(void *, uint32_t addr, unsigned int size)) NULL,
	.watchpoint_set = (int (*)(void *, uint32_t addr, unsigned int size, 
						  unsigned int opt)) NULL,
	.watchpoint_clear = (int (*)(void *, uint32_t addr, unsigned int size)) NULL
};

void gdb_stub_task(struct dbgmon_comm * comm)
{
//	struct gdbrsp * gdb = &gdbrspd;
	struct gdbrsp_comm * gdb_comm;
	struct gdbrsp_target * gdb_target;
	struct gdbrsp_agent * gdb_agent;
	char pkt[RSP_BUFFER_LEN];
	uint32_t sigmask;
	char buf[4];
	int sig;
	int len;
#if THINKOS_ENABLE_CONSOLE
	uint8_t * ptr;
	int cnt;
#endif

	__thinkos_memset32(pkt, 0, sizeof(pkt));

	gdb_agent = gdbrsp_agent_getinstance();
	gdb_target = gdbrsp_target_getinstance();
	gdb_comm = gdbrsp_comm_getinstance();

	gdbrsp_comm_init(gdb_comm, &dbgmon_gdbrsp_comm_op, (void *)comm);
	gdbrsp_target_init(gdb_target, &dbgmon_gdbrsp_target_op, (void *)NULL);
	gdbrsp_agent_init(gdb_agent, gdb_comm, gdb_target);

	dbgmon_breakpoint_clear_all();
	dbgmon_watchpoint_clear_all();

	//	dbgmon_comm_connect(comm);

	//	DCC_LOG(LOG_INFO, "Comm connected..");

	sigmask = (1 << DBGMON_KRN_EXCEPT);
	sigmask |= (1 << DBGMON_THREAD_FAULT);
	sigmask |= (1 << DBGMON_THREAD_STEP);
	sigmask |= (1 << DBGMON_BREAKPOINT);
	sigmask |= (1 << DBGMON_APP_STOP);
	sigmask |= (1 << DBGMON_APP_EXEC);
	sigmask |= (1 << DBGMON_APP_UPLOAD);
	sigmask |= (1 << DBGMON_APP_ERASE);
	sigmask |= (1 << DBGMON_APP_TERM);
	sigmask |= (1 << DBGMON_APP_RESUME);
	sigmask |= (1 << DBGMON_COMM_RCV);
	sigmask |= (1 << DBGMON_COMM_CTL);
#if (THINKOS_ENABLE_CONSOLE)
	sigmask |= (1 << DBGMON_TX_PIPE);
	sigmask |= (1 << DBGMON_RX_PIPE);
#endif
	sigmask |= (1 << DBGMON_SOFTRST);
	sigmask |= (1 << DBGMON_THREAD_CREATE);
	sigmask |= (1 << DBGMON_THREAD_TERMINATE);
	sigmask |= (1 << DBGMON_ALARM);


#if (GDB_IDLE_TIMEOUT_MS > 0)
	dbgmon_alarm(GDB_IDLE_TIMEOUT_MS);
#endif		

	for(;;) {
		DCC_LOG(LOG_MSG, "dbgmon_select()...");
		sig = dbgmon_select(sigmask);
		DCC_LOG1(LOG_INFO, "sig=%d", sig);

		switch (sig) {

#if (GDB_IDLE_TIMEOUT_MS > 0)
		case DBGMON_ALARM:
			DCC_LOG(LOG_INFO, "alarm!");
			dbgmon_alarm(GDB_IDLE_TIMEOUT_MS);
		break;
#endif

		case DBGMON_SOFTRST:
			DCC_LOG(LOG_INFO, "Soft reset.");
//			this_board.softreset();
			dbgmon_clear(DBGMON_SOFTRST);
#if THINKOS_ENABLE_CONSOLE
//			thinkos_console_reset();
			/* Update the console connection flag which was cleared
			   by __console_reset(). */
			thinkos_console_connect_set(dbgmon_comm_isconnected(comm));
#endif
			break;

		case DBGMON_APP_UPLOAD:
			dbgmon_clear(DBGMON_APP_UPLOAD);
			DCC_LOG(LOG_TRACE, "/!\\ APP_UPLOAD signal !");
			break;


		case DBGMON_APP_EXEC:
			dbgmon_clear(DBGMON_APP_EXEC);
			DCC_LOG(LOG_TRACE, "/!\\ APP_EXEC signal !");
#if 0
			if (!dbgmon_app_exec(&this_board.application, true)) {
				DCC_LOG(LOG_ERROR, "/!\\ dbgmon_app_exec() failed!");
				gdb->active_app = false;
			} else {
				gdb->active_app = true;
			}
#endif
			break;

		case DBGMON_APP_ERASE:
			dbgmon_clear(DBGMON_APP_ERASE);
			DCC_LOG(LOG_TRACE, "/!\\ APP_ERASE signal !");
			break;

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

		case DBGMON_KRN_EXCEPT:
			DCC_LOG(LOG_INFO, "Exception.");
			dbgmon_clear(DBGMON_KRN_EXCEPT);
//			gdbrsp_on_fault(gdb, pkt);
			break;

		case DBGMON_THREAD_FAULT:
			DCC_LOG(LOG_INFO, "Thread fault.");
			dbgmon_clear(DBGMON_THREAD_FAULT);
//			gdbrsp_on_fault(gdb, pkt);
			break;

		case DBGMON_THREAD_STEP:
			DCC_LOG(LOG_INFO, "DBGMON_THREAD_STEP");
			dbgmon_clear(DBGMON_THREAD_STEP);
//			gdbrsp_on_step(gdb, pkt);
			break;

		case DBGMON_BREAKPOINT:
			DCC_LOG(LOG_INFO, "DBGMON_BREAKPOINT");
			dbgmon_clear(DBGMON_BREAKPOINT);
//			gdbrsp_on_breakpoint(gdb, pkt);
			break;

		case DBGMON_THREAD_CREATE:
			dbgmon_clear(DBGMON_THREAD_CREATE);
			DCC_LOG(LOG_TRACE, "/!\\ THREAD_CREATE signal !");
			break;

		case DBGMON_THREAD_TERMINATE:
			dbgmon_clear(DBGMON_THREAD_TERMINATE);
			DCC_LOG(LOG_TRACE, "/!\\ THREAD_TERMINATE signal !");
			break;


		case DBGMON_COMM_CTL:
			DCC_LOG(LOG_INFO, "Comm Ctl.");
			dbgmon_clear(DBGMON_COMM_CTL);
			if (!dbgmon_comm_isconnected(comm)) {
				DCC_LOG(LOG_WARNING, "Debug Monitor Comm closed!");
				return;
			}
			break;

#if (THINKOS_ENABLE_CONSOLE)
		case DBGMON_COMM_EOT:
			DCC_LOG(LOG_MSG, "COMM_EOT");
			/* FALLTHROUGH */

		case DBGMON_TX_PIPE:
			DCC_LOG(LOG_MSG, "TX Pipe.");
			if ((cnt = thinkos_console_tx_pipe_ptr(&ptr)) > 0) {
#if 0
				int n;
				DCC_LOG1(LOG_MSG, "TX Pipe, %d pending chars.", cnt);
				if ((n = gdbrsp_console_output(gdb, pkt, 
								ptr, cnt)) > 0) {

					/* enable COMM_EOT event to continue sending 
					   data */
					sigmask |= (1 << DBGMON_COMM_EOT);
					thinkos_console_tx_pipe_commit(n); 
				} else {
					DCC_LOG(LOG_WARNING, "rsp_console_output() failed!!!");
					dbgmon_clear(DBGMON_TX_PIPE);
					sigmask &= ~(1 << DBGMON_COMM_EOT);
				}
#endif
			} else {
				DCC_LOG(LOG_MSG, "TX Pipe empty!!!");
				dbgmon_clear(DBGMON_TX_PIPE);
				sigmask &= ~(1 << DBGMON_COMM_EOT);
			}
			break;
#endif

#if (THINKOS_ENABLE_CONSOLE)
		case DBGMON_RX_PIPE:
			dbgmon_clear(DBGMON_RX_PIPE);
			DCC_LOG(LOG_WARNING, "RX Pipe empty!!!");
			break;
#endif

		case DBGMON_COMM_RCV:
			DCC_LOG(LOG_MSG, "DBGMON_COMM_RCV +++++++++++");
			if (dbgmon_comm_recv(comm, buf, 1) != 1) {
				DCC_LOG(LOG_WARNING, "dbgmon_comm_recv() failed!");
				continue;
			}
			DCC_LOG(LOG_MSG, "DBGMON_COMM_RCV --------");

			switch (buf[0]) {

			case '+':
				DCC_LOG(LOG_INFO, "[ACK]");
				break;

			case '-':
#if GDB_ENABLE_RXMIT
				DCC_LOG(LOG_WARNING, "[NACK] rxmit!");
				gdbrsp_pkt_rxmit(gdb);
#else
				DCC_LOG(LOG_WARNING, "[NACK]!");
#endif
				break;

			case '$':
				DCC_LOG(LOG_MSG, "Comm RX: '$'");
				dbgmon_alarm(1000);
				len = gdbrsp_pkt_recv(gdb_comm, pkt, RSP_BUFFER_LEN);
				dbgmon_alarm_stop();
				if (len <= 0) {
//					if (!gdb->noack_mode)
//						gdbrsp_ack(gdb);
					if (gdbrsp_pkt_input(gdb_agent, pkt, len) == (int)GDB_RSP_QUIT)
						return;
				}
				break;

			case CTRL_C:
				DCC_LOG(LOG_INFO, "[BREAK]");
				gdbrsp_on_break(gdb_agent, pkt);
				break;


			default:
				DCC_LOG1(LOG_WARNING, "invalid: %02x", buf[0]);
				return;
			}

#if (GDB_IDLE_TIMEOUT_MS > 0)
			dbgmon_alarm(GDB_IDLE_TIMEOUT_MS);
#endif
			break;
		}
	}
}


