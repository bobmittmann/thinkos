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
 * @file third-stub.c
 * @brief ThinkOS Remote Debugger Stub
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#include <stdlib.h>
#include <stdbool.h>

#include <sys/dcclog.h>
#include <sys/stm32f.h>

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_BOOTLDR__
#include <thinkos/bootldr.h>
#include <thinkos.h>

#include <trdp.h>

/* THIRD - ThinkOS Remote Debugger  */

struct third_stub {
	struct dmon_comm * comm;
	uint32_t remote;
	uint32_t session;
	uint32_t sequence;
	uint32_t idle_timer;
};

static void trdp_rand_init(void)
{
	struct stm32_rng  * rng = STM32_RNG;

	/* Enable RNG clock */
	stm32_clk_enable(STM32_RCC, STM32_CLK_RNG);
	/* Enable the random number generator */
	rng->cr = RNG_RNGEN;
}

static uint32_t trdp_rand32(void)
{
	struct stm32_rng  * rng = STM32_RNG;

	while ((rng->sr & RNG_DRDY) == 0);

	return rng->dr;
}


#define THIRD_STUB_PKTBUF_LEN 512

static int third_on_fault(struct third_stub * stub, uint32_t * pkt)
{
	return 0;
/*
	int thread_id;

	if (third->stopped) {
		DCC_LOG(LOG_WARNING, "on fault, suspended already!");
		return 0;
	}

	thread_id = thread_break_id();
	gdb->thread_id.g = thread_id; 

	DCC_LOG1(LOG_TRACE, "suspending (current=%d) ... ...", thread_id);

	target_halt();
	gdb->stopped = true;
	gdb->last_signal = TARGET_SIGNAL_SEGV;

	return rsp_thread_stop_reply(gdb, pkt, thread_id);
*/
}

int third_on_break(struct third_stub * stub, uint32_t * pkt)
{
	return 0;
/*
	int thread_id;

	DCC_LOG(LOG_TRACE, "on break, suspending... ... ...");

	//gdb->thread_id.g = thread_active();

	target_halt();
	thread_id = thread_any();
	gdb->thread_id.g = thread_id;
	gdb->stopped = true;
	gdb->last_signal = TARGET_SIGNAL_INT;
	
	return rsp_thread_stop_reply(gdb, pkt, thread_id);
*/
}

static int third_on_step(struct third_stub * stub, uint32_t * pkt)
{
	return 0;
}


static int third_on_breakpoint(struct third_stub * stub, uint32_t * pkt)
{
	return 0;
}

static int third_greeting(struct third_stub * stub, uint32_t * pkt)
{
	__thinkos_memcpy(pkt, "\r\nTRDPLE\0\0\0\0", 12);
	dmon_comm_send(stub->comm, pkt, 12);
	return 0;
}

#define TRDRPC_HDR_OPC(_HDR) ((_HDR) >> 9)
#define TRDRPC_HDR_DATALEN(_HDR) ((_HDR) & 0x1ff)

#define TRDRPC_MKHDR(_OPC, _LEN) (((_OPC & 0x7f) << 9) | (_LEN & 0x1ff))

int trdp_lnk_pkt_recv(struct third_stub * stub, uint32_t * pkt)
{
	struct dmon_comm * comm = stub->comm;
	unsigned int rem;
	uint32_t sigmask;
	uint32_t sigset;
	uint8_t * cp;
	int hdr;

	SIG_ZERO(sigmask);
	SIG_SET(sigmask, DBGMON_COMM_RCV);
	SIG_SET(sigmask, DBGMON_COMM_CTL);
	SIG_SET(sigmask, DBGMON_ALARM);
	dbgmon_alarm(100);

	/* get the packet header */
	cp = (uint8_t *)pkt;
	rem = 4;
	while (rem) {
		int n;
		sigset = dbgmon_select(sigmask);
		if (SIG_ISSET(sigset, DBGMON_COMM_RCV)) {
			if ((n = dmon_comm_recv(comm, cp, rem)) <= 0) {
				DCC_LOG(LOG_WARNING, "dmon_comm_recv() failed!");
				return -1;
			}
			cp += n;
			rem -= n;
		} else if (SIG_ISSET(sigset, DBGMON_ALARM)) {
			dbgmon_clear(DBGMON_ALARM);
			DCC_LOG(LOG_WARNING, "timeout!");
			return -1;
		} else {
			DCC_LOG(LOG_WARNING, "unexpected signal!");
			return -1;
		}
	}

	hdr = (pkt[1] << 8) | pkt[0];

	/* get the packet payload */
	cp = (uint8_t *)pkt;
	rem = TRDRPC_HDR_DATALEN(hdr);
	while (rem) {
		int n;
		sigset = dbgmon_select(sigmask);
		if (SIG_ISSET(sigset, DBGMON_COMM_RCV)) {
			if ((n = dmon_comm_recv(comm, cp, rem)) <= 0) {
				DCC_LOG(LOG_WARNING, "dmon_comm_recv() failed!");
				return -1;
			}
			cp += n;
			rem -= n;
		} else if (SIG_ISSET(sigset, DBGMON_ALARM)) {
			dbgmon_clear(DBGMON_ALARM);
			DCC_LOG(LOG_WARNING, "timeout!");
			return -1;
		} else {
			DCC_LOG(LOG_WARNING, "unexpected signal!");
			return -1;
		}
	}

	return hdr;
}

int trdp_lnk_pkt_send(struct third_stub * stub, unsigned int opc,
					  uint32_t * pkt, unsigned int len)
{
	uint32_t * hdr= (uint32_t *)pkt;

	*hdr = TRDRPC_MKHDR(opc, len);

	return dmon_comm_send(stub->comm, hdr, len + 4);
}

static int trdrpc_auth_init_svc(struct third_stub * stub, 
							  uint32_t * pkt, int len)
{
	struct trdp_auth_init * init;
	struct trdp_auth_desc * desc;
	uint32_t agent;
	uint32_t session;

	if (len != sizeof(struct trdp_auth_init))
		return -1;

	init = (struct trdp_auth_init  *)pkt;
	(void)init;
	agent = init->agent;	
	stub->remote = agent;

	/* generte a session ID */
	session = trdp_rand32();
	/* store the session ID */
	stub->session = session;
	/* Start the session idle timer (1 minute for the authentication) */
	stub->idle_timer = __thinkos_ticks() + (60 * 1000);
	
	desc = (struct trdp_auth_desc *)&pkt[1];
	desc->session = session;
	desc->method[0] = TRDP_AUTH_RSA1024;
	desc->method[1] = TRDP_AUTH_PASSWORD;
	desc->method[2] = TRDP_AUTH_PLAIN;
	desc->method[3] = TRDP_AUTH_NONE;

	return trdp_lnk_pkt_send(stub, TRDRPC_AUTH_DESC, 
							 pkt, sizeof(struct trdp_auth_desc));
}

void third_stub_task(struct dmon_comm * comm)
{
	struct third_stub third_stub;
	struct third_stub * stub = &third_stub;
	uint32_t pkt[THIRD_STUB_PKTBUF_LEN / 4];
	uint32_t sigmask;
	uint32_t sigset;

	stub->comm = comm;
	stub->session = 0;
	stub->sequence = 0;
	/* initialize the idle timer */
	stub->idle_timer =  __thinkos_ticks() + (60 * 60 * 1000);

	/* initialize the random number engine */
	trdp_rand_init();

	/* Send a greeting packet */
	third_greeting(stub, pkt);
	
	dmon_breakpoint_clear_all();
	dmon_watchpoint_clear_all();

	SIG_ZERO(sigmask);
	SIG_SET(sigmask, DBGMON_EXCEPT);
	SIG_SET(sigmask, DBGMON_THREAD_FAULT);
	SIG_SET(sigmask, DBGMON_THREAD_STEP);
	SIG_SET(sigmask, DBGMON_COMM_RCV);
	SIG_SET(sigmask, DBGMON_COMM_CTL);
	SIG_SET(sigmask, DBGMON_BREAKPOINT);
#if (THINKOS_ENABLE_CONSOLE)
	SIG_SET(sigmask, DBGMON_TX_PIPE);
#endif
	SIG_SET(sigmask, DBGMON_SOFTRST);

	for(;;) {
		sigset = dbgmon_select(sigmask);

		DCC_LOG1(LOG_MSG, "sig=%08x", sigset);

		if (SIG_ISSET(sigset, DBGMON_SOFTRST)) {
			dbgmon_clear(DBGMON_SOFTRST);
			this_board.softreset();
		}

		if (SIG_ISSET(sigset, DBGMON_COMM_CTL)) {
			DCC_LOG(LOG_TRACE, "Comm Ctl.");
			dbgmon_clear(DBGMON_COMM_CTL);
			if (!dmon_comm_isconnected(comm)) {
				DCC_LOG(LOG_WARNING, "Debug Monitor Comm closed!");
				return;
			}
		}

#if (THINKOS_ENABLE_CONSOLE)
		if (SIG_ISSET(sigset, DBGMON_TX_PIPE)) {
			DCC_LOG(LOG_MSG, "TX Pipe.");
//			if (rsp_console_output(gdb, pkt) <= 0) {
				dbgmon_clear(DBGMON_TX_PIPE);
//			}
		}
#endif


		if (SIG_ISSET(sigset, DBGMON_EXCEPT)) {
			DCC_LOG(LOG_TRACE, "Exception.");
			dbgmon_clear(DBGMON_EXCEPT);
			third_on_fault(stub, pkt);
		}

		if (SIG_ISSET(sigset, DBGMON_THREAD_FAULT)) {
			DCC_LOG(LOG_TRACE, "Thread fault.");
			dbgmon_clear(DBGMON_THREAD_FAULT);
			third_on_fault(stub, pkt);
		}

		if (SIG_ISSET(sigset, DBGMON_THREAD_STEP)) {
			DCC_LOG(LOG_INFO, "DBGMON_THREAD_STEP");
			dbgmon_clear(DBGMON_THREAD_STEP);
			third_on_step(stub, pkt);
		}

		if (SIG_ISSET(sigset, DBGMON_BREAKPOINT)) {
			DCC_LOG(LOG_INFO, "DBGMON_BREAKPOINT");
			dbgmon_clear(DBGMON_BREAKPOINT);
			third_on_breakpoint(stub, pkt);
		}

		if (SIG_ISSET(sigset, DBGMON_COMM_RCV)) {
			unsigned int opc;
			unsigned int len;
			int hdr;
			int ret = 0;

			/* get a packet form link layer */
			if ((hdr = trdp_lnk_pkt_recv(stub, pkt)) < 0) {
				DCC_LOG(LOG_WARNING, "trdp_lnk_pkt_recv() failed!");
				return;
			}

			opc = TRDRPC_HDR_OPC(hdr);
			len = TRDRPC_HDR_DATALEN(hdr);

			switch (opc) {
			case TRDRPC_AUTH_INIT:
				ret = trdrpc_auth_init_svc(stub, pkt, len);
				break;

			case TRDRPC_SUSPEND:
				break;

			case TRDRPC_RESUME:
				break;

			case TRDRPC_EXEC:
				break;

			case TRDRPC_REBOOT:
				break;

			case TRDRPC_KERNELINFO:
				break;

			case TRDRPC_MEM_LOCK:
				break;

			case TRDRPC_MEM_UNLOCK:
				break;

			case TRDRPC_MEM_ERASE:
				break;

			case TRDRPC_MEM_READ:
				break;

			case TRDRPC_MEM_WRITE:
				break;

			case TRDRPC_MEM_SEEK:
				break;

			case TRDRPC_MEM_CRC32:
				break;

			case TRDRPC_EXCEPTINFO:
				break;

			case TRDRPC_THREADINFO:
				break;

			default:
				DCC_LOG(LOG_ERROR, "TRDP protocol error!");
				ret = -1;
			}

			if (ret < 0) {
				/* disconnect this session */
				return;
			}

			/* check the session timer for expiration */
			if ((int32_t)(stub->idle_timer - __thinkos_ticks()) < 0) {
				DCC_LOG(LOG_TRACE, "Idle timer timeout!");
				/* disconnect this session */
				return;
			}

		}
	}
}
 
