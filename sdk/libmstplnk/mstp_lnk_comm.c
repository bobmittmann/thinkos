/* 
 * Copyright(C) 2015 Robinson Mittmann. All Rights Reserved.
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
 * @file mstp_lnk_comm.c
 * @brief RDA Communcation Link Layer
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

_Pragma("GCC optimize (\"Ofast\")")

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stm32f.h>
#include <sys/serial.h>
#include <assert.h>

#include <thinkos.h>

#include "mstp_lnk-i.h"
#include "io.h"
#include "board.h"

#undef TRACE_LEVEL
#define TRACE_LEVEL TRACE_LVL_INF
#include <trace.h>

#if COMMLNK_TEST_ENABLED
#define COMMLNK_TEST_ENABLED 0
#endif

#ifndef MSTP_COMM_THREAD_ENABLED
#define MSTP_COMM_THREAD_ENABLED 0
#endif

#ifndef MSTP_COMM_SYNC_RETRY
#define MSTP_COMM_SYNC_RETRY 0
#endif


#define COMMLNK_MTU         512
#define COMMLNK_XFER_MAX    ((COMMLNK_MTU) + 2)
#define COMMLNK_TX_BUFSIZE	(COMMLNK_XFER_MAX)
#define COMMLNK_RX_BUFSIZE	((COMMLNK_XFER_MAX) * 2)

/* -------------------------------------------------------------------------
 * Serial Driver Structure 
 */

struct mstp_lnk_comm {
	uint8_t addr;

#if (MSTP_LNK_HALF_DUPLEX)
	volatile uint8_t half_duplex;
#endif

#if (MSTP_COMM_THREAD_ENABLED)
	uint8_t thread;
	volatile uint8_t alive;
	uint8_t rx_flag;
#endif

	struct {
		const struct uart_dma_op *op;
	} dev;

	struct {
		uint8_t token[MSTP_TOKEN_SIZE];
		volatile uint8_t idle_bits;
		uint8_t buf[COMMLNK_TX_BUFSIZE];
	} tx;

	struct {
		uint32_t pre_pos;	/* previous position of the start of buffer */
		volatile uint32_t cur_pos;	/* current position of the 
									   start of buffer */
		uint8_t buf[COMMLNK_RX_BUFSIZE];
	} rx;

	struct {
		uint32_t frame_err;
		uint32_t tx_bcast;
		uint32_t tx_unicast;
		uint32_t tx_token;
		uint32_t tx_mgmt;
		uint32_t tx_pfm;


		uint32_t rx_err_datcrc;
		uint32_t rx_err_datlen;
		uint32_t rx_err_crc;
		uint32_t rx_err_sync;
		uint32_t rx_err_uart;
		uint32_t rx_short;
		uint32_t rx_zero;
		uint32_t rx_break;
		uint32_t rx_idle;
		uint32_t rx_timeout;

	} stats;

};

static void comm_bit_timer_init(unsigned int baudrate)
{
	struct stm32f_tim *tim = STM32F_TIM6;
	unsigned int div;

	/* Timer clock enable */
	stm32_clk_enable(STM32_RCC, STM32_CLK_TIM6);
	/* get the total divisior */
	div = (stm32f_tim2_hz + (baudrate / 2)) / baudrate;
	/* Timer configuration */
	tim->psc = div - 1;
	tim->arr = 0;
	tim->cnt = 0;
	tim->dier = TIM_UIE;	/* Update interrupt enable */
	tim->cr1 = TIM_CMS_EDGE | TIM_OPM | TIM_URS;
	INF("Tim6: div=%d stm32f_tim2_hz=%d freq=%d",
	    div, stm32f_tim2_hz, baudrate);
}

static void nbit_sleep(unsigned int nbit)
{
	struct stm32f_tim *tim = STM32F_TIM6;

#if 1
	/* Disable timer */
	tim->cr1 = TIM_CMS_EDGE | TIM_OPM | TIM_URS;
	tim->cnt = 0;
#endif
	tim->arr = nbit - 1;
	/* Enable timer */
	tim->cr1 = TIM_CMS_EDGE | TIM_OPM | TIM_URS | TIM_CEN;

	do {
		thinkos_irq_wait(STM32_IRQ_TIM6);
	} while ((tim->sr & TIM_UIF) == 0);
	tim->sr = 0;
}

/* -------------------------------------------------------------------------
 * Serial Link API 
 * ------------------------------------------------------------------------- */

/* Single COMM device driver instance */
struct mstp_lnk_comm mstp_lnk_comm_singleton_rt;

struct mstp_lnk_comm * mstp_lnk_comm_instance_get(int oid) {
	return &mstp_lnk_comm_singleton_rt;
}

int mstp_lnk_comm_uart_bind(struct mstp_lnk_comm * comm, 
							const struct uart_dma_drv * drv)
{
	if (comm == NULL) {
		ERRS("Null pointer!");
		return -EINVAL;
	}

	if (drv == NULL) {
		ERRS("Null pointer!");
		return -EINVAL;
	}

	INF("Biding COMM to driver %s", drv->tag);
	comm->dev.op = drv->op;

	return 0;
}


int mstp_lnk_comm_frame_send(struct mstp_lnk_comm *comm, unsigned int route,
			   uint8_t * pdu, unsigned int len)
{
	const struct uart_dma_op * uart_op = comm->dev.op;
	uint8_t *buf = comm->tx.buf;
	unsigned int crc;
	unsigned int type;
	unsigned int daddr;
	unsigned int cnt;
	unsigned int i;

	/* encode header */
	buf[0] = 0x55;
	buf[1] = 0xff;
	buf[2] = ROUTE_FRM_TYPE(route);
	buf[3] = ROUTE_DST_ADDR(route);
	buf[4] = ROUTE_SRC_ADDR(route);
	buf[5] = (len >> 8) & 0xff;
	buf[6] = len & 0xff;
	crc = __mstp_crc8(0xff, buf[2]);
	crc = __mstp_crc8(crc, buf[3]);
	crc = __mstp_crc8(crc, buf[4]);
	crc = __mstp_crc8(crc, buf[5]);
	buf[7] = ~__mstp_crc8(crc, buf[6]);

	if (len > 0) {
		/* encode PDU */
		uint8_t *cp = &buf[8];
		crc = ~mstp_crc16(0xffff, pdu, len);
		for (i = 0; i < len; ++i)
			cp[i] = pdu[i];

		cp[i++] = (crc & 0xff);
		cp[i++] = (crc >> 8) & 0xff;

		cnt = len + 10;
	} else {
		buf[8] = 0xff;
		buf[9] = 0xff;
		cnt = 8;
	}

	type = buf[2];
	daddr = buf[3];
	(void)type;

	if (cnt > COMMLNK_TX_BUFSIZE + 2) {
		ERRS("MS/TP COMM: too long!");
		return -EINVAL;
	}

	if (cnt <= 2) {
		ERRS("MS/TP COMM: too short!");
		return -EINVAL;
	}

#if (MSTP_LNK_HALF_DUPLEX)
	if (comm->half_duplex) 
		uart_op->rx_en_set(false);
#endif

	nbit_sleep(40);
	/* start the DMA to transfer */
	uart_op->tx_dma_start(comm->tx.buf, cnt);

	/* Wait for transmission complete */
	nbit_sleep(cnt * 10 + comm->tx.idle_bits);


#if (MSTP_LNK_HALF_DUPLEX)
	uart_op->rx_en_set(true);
#endif

#if 0
	switch (type) {
	case FRM_TOKEN:
		comm->stats.tx_token++;
		break;
		/* FIXME: to get accurate stats, separate the poll for master,
		 * and reply poll for master */
	case FRM_TEST_REQUEST:
		comm->stats.tx_mgmt++;
		break;

	case FRM_POLL_FOR_MASTER:
	case FRM_REPLY_POLL_FOR_MASTER:
	case FRM_TEST_RESPONSE:
	case FRM_REPLY_POSTPONED:
	default:
		break;
	}
#endif
	if (daddr == MSTP_ADDR_BCAST)
		comm->stats.tx_bcast++;
	else
		comm->stats.tx_unicast++;

	return cnt;
}

int mstp_lnk_comm_fast_send(struct mstp_lnk_comm *comm, unsigned int type,
							unsigned int daddr)
{
	const struct uart_dma_op * uart_op = comm->dev.op;
	uint8_t *buf = comm->tx.token;
	unsigned int crc;

	/* encode token */
	buf[2] = type;
	buf[3] = daddr;
	crc = __mstp_crc8(0xff, buf[2]);
	crc = __mstp_crc8(crc, buf[3]);
	crc = __mstp_crc8(crc, buf[4]);
	crc = __mstp_crc8(crc, 0);
	buf[7] = ~__mstp_crc8(crc, 0);

#if (MSTP_LNK_HALF_DUPLEX)
	if (comm->half_duplex) 
		uart_op->rx_en_set(false);
#endif

	nbit_sleep(40);

	/* start the DMA to transfer */
	uart_op->tx_dma_start(comm->tx.token, 8);

	/* Wait for transmission complete */
	nbit_sleep(8 * 10 + comm->tx.idle_bits);


#if (MSTP_LNK_HALF_DUPLEX)
	uart_op->rx_en_set(true);
#endif

	switch (type) {
	case FRM_TOKEN:
		comm->stats.tx_token++;
		break;
	case FRM_TEST_REQUEST:
	case FRM_TEST_RESPONSE:
	case FRM_REPLY_POLL_FOR_MASTER:
	case FRM_REPLY_POSTPONED:
		comm->stats.tx_mgmt++;
		break;
	case FRM_POLL_FOR_MASTER:
		comm->stats.tx_pfm++;
	default:
		break;
	}

	return 8;
}

#if (MSTP_COMM_THREAD_ENABLED)
/* -------------------------------------------------------------------------
 * Thread to handle errors, break and idle line detection 
 */
int mstp_comm_aux_task(struct mstp_lnk_comm *comm)
{
	const struct uart_dma_op * uart_op = comm->dev.op;

	while(comm->alive) {
		int event;

		event = uart_op->event_wait();

		if (event == UART_EVENT_RX_BREAK) {
			ERRS("COMM: RX break ...");
			return -1;
		} else if (event == UART_EVENT_RX_IDLE) {
			uint32_t head;

			/* update DMA ring head */
			head = COMMLNK_RX_BUFSIZE - uart_op->rx_dma_rem();
			comm->rx.cur_pos = head;
			thinkos_flag_give(comm->rx_flag);
		} else {
			ERRS("COMM: Event invalid...");
			return -1;
		} 

	}

	return 0;
}

/* -------------------------------------------------------------------------
 * Serial link initialization
 * ------------------------------------------------------------------------- */

uint32_t mstp_comm_aux_stack[128] __attribute__ ((aligned(8), 
												  section(".stack")));

const struct thinkos_thread_inf mstp_comm_aux_inf = {
	.stack_ptr = mstp_comm_aux_stack,
	.stack_size = sizeof(mstp_comm_aux_stack),
	.priority = 2,
	.thread_id = 2,
	.paused = 0,
	.tag = "MSTPCOM"
};

#endif


int mstp_lnk_comm_init(struct mstp_lnk_comm * comm, unsigned int baudrate, 
				   unsigned int idle_bits, int ts)
{
	const struct uart_dma_op * uart_op;
	int ret = 0;

	if (comm == NULL) {
		ERRS("Null pointer!");
		return -EINVAL;
	}

	if ((uart_op = comm->dev.op) == NULL) {
		ERRS("COMM not bound to a UART!");
		return -1;
	}

#if (MSTP_COMM_THREAD_ENABLED)
	if (comm->thread != 0) {
		ERRS("Module already initialized!");
		return -1;
	}
#endif

	/* initialize TX DMA */
	uart_op->tx_dma_init(comm->tx.buf, COMMLNK_TX_BUFSIZE);

	/* initialize RX DMA */
	uart_op->rx_dma_init(comm->rx.buf, COMMLNK_RX_BUFSIZE);

	uart_op->init(baudrate);

	comm->tx.idle_bits = idle_bits;
	comm->rx.cur_pos = 0;
	comm->rx.pre_pos = 0;

#if (MSTP_LNK_HALF_DUPLEX)
	comm->half_duplex = false;
#endif

	/* prepare the token buffer for fast transfers */
	comm->addr = ts;
	comm->tx.token[0] = 0x55;
	comm->tx.token[1] = 0xff;
	comm->tx.token[2] = 0;
	comm->tx.token[3] = 0;
	comm->tx.token[4] = ts;
	comm->tx.token[5] = 0;
	comm->tx.token[6] = 0;

	comm_bit_timer_init(baudrate);

#if (MSTP_COMM_THREAD_ENABLED)
	comm->rx_flag = thinkos_flag_alloc();
	comm->alive = true;
	ret = thinkos_thread_create_inf(C_TASK(mstp_comm_aux_task), 
									(void *)comm, &mstp_comm_aux_inf);
	if (ret < 0) {
		ERRS("thinkos_thread_create_inf() failed!");
	} else {
		comm->thread = ret;
	}
#endif

	return ret;
}


int mstp_lnk_comm_done(struct mstp_lnk_comm *comm)
{
	int ret = 0;

#if (MSTP_COMM_THREAD_ENABLED)
	if (comm->thread == 0) {
		ERRS("Module not initialized!");
		return -1;
	}

	comm->alive = false;

	thinkos_join(comm->thread);
	comm->thread = 0;
#endif

	return ret;
}

int mstp_lnk_comm_frame_recv(struct mstp_lnk_comm *comm, void * buf, 
							 struct mstp_frm_ref *frm, unsigned int tmo)
{
	uint8_t * rx_buf = comm->rx.buf;
	unsigned int sync;
	unsigned int crc;
	unsigned int chk;
	unsigned int type;
	unsigned int daddr;
	unsigned int saddr;
	unsigned int pdu_len;
	uint32_t head;
	uint32_t tail;
	int cnt;
	int ret;

#if (MSTP_COMM_THREAD_ENABLED)
	tail = comm->rx.pre_pos;
	if (tmo == 0) {
		WARNS("mo==0");
	}
	/* Trying to take the rx_available flag */
	if (thinkos_flag_timedtake(comm->rx_flag, tmo) < 0) {
		if ((head = comm->rx.cur_pos) == tail) {
			return MSTP_LNK_TIMEOUT;
		}

		/* Pending data on DMA ring, process it */
		YAPS("MS/TP COM: pending data ?");
	} 
#else
	const struct uart_dma_op * uart_op = comm->dev.op;
	int event;

	event = uart_op->event_timedwait(tmo);

	tail = comm->rx.pre_pos;
	/* update DMA ring head */
	head = COMMLNK_RX_BUFSIZE - uart_op->rx_dma_rem();
	comm->rx.cur_pos = head;

	if (event == UART_EVENT_RX_IDLE) {
		comm->stats.rx_idle++;
	} else if (event == UART_EVENT_RX_BREAK) {
		comm->stats.rx_break++;
		return MSTP_LINE_BREAK;
	} else if (event == UART_EVENT_RX_ERROR) {
		comm->stats.rx_err_uart++;
//		return -1;
	} else if (event == UART_EVENT_TIMEOUT) {
		comm->stats.rx_timeout++;
//		return MSTP_LNK_TIMEOUT;
	} else {
		ERRS("COMM: Event invalid...");
		return -1;
	} 
#endif
	
	do {
		head = comm->rx.cur_pos;
		cnt = (head >= tail) ? (head - tail) : 
			(head + COMMLNK_RX_BUFSIZE - tail);

		if (cnt == 0) {
			comm->stats.rx_zero++;
			YAPS("COMM: buffer empty !!!!.");
			ret = MSTP_LNK_TIMEOUT;
			break;
		}

		if (cnt < 6) {
			YAP("MS/TP garbage=%d", cnt);
			tail = (tail + cnt) % COMMLNK_RX_BUFSIZE;
			comm->stats.rx_short++;
			comm->rx.pre_pos = tail;
			mstp_dbg_io_1_toggle();
			ret = MSTP_LNK_TIMEOUT;
			break;
		}

		/*
		   Robust preambule detection:

		   0x55 0xFF:
		   S 0 1 2 3 4 5 6 7 E S 0 1 2 3 4 5 6 7 E S
		   __   _   _   _   _   _   _ _ _ _ _ _ _ _ _   _ _ _ _ _
		   |_| |_| |_| |_| |_| |_|                 |_|_|_|_|_|_|_

		   0x55 ... :
		   S 0 1 2 3 4 5 6 7 E                 S 0 1 2 3 4 
		   __ _ _   _   _   _   _   _ _ _ _ _ _ _ _ _   _ _ _ _ _
		   |_| |_| |_| |_| |_|                 |_|_|_|_|_|_|_

		   0xD5 ... :
		   S 0 1 2 3 4 5 6 7 E             S 0 1 2 3 4 
		   __ _ _ _ _   _   _   _   _ _ _ _ _ _ _ _ _   _ _ _ _ _
		   |_| |_| |_| |_|                 |_|_|_|_|_|_|_

		   0xF5 ... :
		   S 0 1 2 3 4 5 6 7 E         S 0 1 2 3 4 
		   __ _ _ _ _ _ _   _   _   _ _ _ _ _ _ _ _ _   _ _ _ _ _
		   |_| |_| |_|                 |_|_|_|_|_|_|_
		   0xFD ... :
		   S 0 1 2 3 4 5 6 7 E     S 0 1 2 3 4 
		   __ _ _ _ _ _ _ _ _   _   _ _ _ _ _ _ _ _ _   _ _ _ _ _
		   |_| |_|                 |_|_|_|_|_|_|_
		   0xFF ... :
		   S 0 1 2 3 4 5 6 7 E S 0 1 2 3 4 
		   __ _ _ _ _ _ _ _ _ _ _   _ _ _ _ _ _ _ _ _   _ _ _ _ _
		   |_|                 |_|_|_|_|_|_|_
		   ...  ... :
		   S 0 1 2 3 4 
		   __ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _   _ _ _ _ _
		   |_|_|_|_|_|_|_
		   */
		sync = rx_buf[tail];
#if 0
		if (cnt > 8) {
			INF("MS/TP cnt=%d", cnt);
		}
#endif
		if (sync == 0x55) { 
			tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
			cnt--;
			sync = rx_buf[tail];
			if (sync == 0xff) {
				tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
				cnt--;
			} else {
				DBGS("MS/TP sync 0x55 ...");
			}
		} else if ((sync == 0xd5) || (sync == 0xf5) || 
				   (sync == 0xfd) || (sync == 0xff)) {
			tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
			mstp_dbg_io_0_toggle();
			DBG("MS/TP sync 0x%02x (%d)...", sync, cnt);
			cnt--;
		} else {
//			mstp_dbg_io_1_toggle();
			DBG("MS/TP sync 0x%02x (%d) ???", sync, cnt);
		}

		/* need at least 6 bytes for MS/TP header */
		if (cnt < 6) {
			/* skip all */
			tail = (tail + cnt) % COMMLNK_RX_BUFSIZE;
			comm->rx.pre_pos = tail;
			DBG("MS/TP sync error 0x%02x (%d)!!!", sync, cnt);
			comm->stats.rx_err_sync++;
			ret = MSTP_HDR_SYNC_ERROR;
			break;
		}

		/* Read header from DMA ring */
		type = rx_buf[tail];
		tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
		daddr = rx_buf[tail];
		tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
		saddr = rx_buf[tail];
		tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
		pdu_len = rx_buf[tail] << 8; 
		tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
		pdu_len += rx_buf[tail];
		tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
		chk = rx_buf[tail]; 
		tail = (tail + 1) % COMMLNK_RX_BUFSIZE;

		/* header sequence check */
		crc = __mstp_crc8(0xff, type);
		crc = __mstp_crc8(crc, daddr);
		crc = __mstp_crc8(crc, saddr);
		crc = __mstp_crc8(crc, pdu_len >> 8);
		crc = (~__mstp_crc8(crc, pdu_len & 0xff) & 0xff);
		cnt -= 6;

		if (crc != chk) {
#if (MSTP_COMM_SYNC_RETRY)
			/* skip 1 char */
			tail = comm->rx.pre_pos;
			tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
			comm->rx.pre_pos = tail;
			continue;
#endif
			/* skip all */
			tail = (tail + cnt) % COMMLNK_RX_BUFSIZE;
			comm->rx.pre_pos = tail;
			DBGS("MS/TP: HDR CRC error");
			comm->stats.rx_err_crc++;
			ret = MSTP_HDR_CRC_ERROR;
			break;
		}

		/* skip header */
		comm->rx.pre_pos = tail;

		frm->frm_type = type;
		frm->dst_addr = daddr;
		frm->src_addr = saddr;
		frm->pdu_len = pdu_len;

		if (pdu_len > 0) {
			unsigned int chk;
			uint32_t end;
			uint8_t * pdu;

			if ((unsigned int)cnt < pdu_len + 2) {
				WARN("MS/TP: PDU error, frame=%d < cnt=%d", pdu_len + 2, cnt);
				comm->stats.rx_err_datlen++;
				ret = MSTP_FRAME_LEN_ERROR;
				break;
			}

			if ((unsigned int)cnt > pdu_len + 2) {
				YAP("MS/TP: frame=%d > cnt=%d", pdu_len + 2, cnt);
			}

			pdu = (uint8_t *)buf;
			cnt = pdu_len;
			end = (tail + cnt) % COMMLNK_RX_BUFSIZE;

			/* copy PDU into buffer */
			if (end >= tail) {
				memcpy(pdu, &rx_buf[tail], cnt);
			} else {
				int n = COMMLNK_RX_BUFSIZE - tail;
				memcpy(pdu, &rx_buf[tail], n);
				memcpy(&pdu[n], rx_buf, end);
			}

			/* get frame sequence check */
			tail = end;
			chk = rx_buf[tail];
			tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
			chk += rx_buf[tail] << 8;
			tail = (tail + 1) % COMMLNK_RX_BUFSIZE;
			crc = (~mstp_crc16(0xffff, pdu, pdu_len)) & 0xffff;

			/* skip the frame */
			comm->rx.pre_pos = tail;

			if (crc == chk) {
				ret = pdu_len + 10;
			} else {
				WARN("Data CRC error %04x != %04x", crc, chk);
				comm->stats.rx_err_datcrc++;
				ret = MSTP_DATA_CRC_ERROR;
			}
		}  else {
			ret = 8;
		}
		break;
	} while (1);

	/* return the number of events */
	return ret;
}

int mstp_lnk_comm_pending_events(struct mstp_lnk_comm *comm)
{
	const struct uart_dma_op * uart_op = comm->dev.op;
	uint32_t head;
	uint32_t tail;
	int cnt;

	head = COMMLNK_RX_BUFSIZE - uart_op->rx_dma_rem();

	tail = comm->rx.pre_pos;

	cnt = (head >= tail) ? (head - tail) : (head + COMMLNK_RX_BUFSIZE - tail);

	return cnt;
}

int mstp_lnk_comm_half_duplex_set(struct mstp_lnk_comm *comm, bool on)
{
	int ret = comm->half_duplex;

	if (on)
		YAPS("MSCom: half duplex ON");
	else
		YAPS("MSCom: half duplex OFF");

	comm->half_duplex = on;

	return ret;
}

int mstp_lnk_comm_idle_bits_set(struct mstp_lnk_comm *comm, unsigned int idle_bits)
{
	comm->tx.idle_bits = idle_bits;

	return 0;
}

int mstp_lnk_comm_stats_fmt(struct mstp_lnk_comm *comm, char * buf, size_t max)
{
	char * cp = buf;
	int rem = max;
	int n;

	cp += n = snprintf(cp, rem, "Link-comm:\n");
	cp += n = snprintf(cp, rem, "  -  idle = %9d\n", comm->stats.rx_idle);
	rem -= n;
	cp += n = snprintf(cp, rem, "  - break = %9d\n", comm->stats.rx_break);
	rem -= n;
	cp += n = snprintf(cp, rem, "  -  zero = %9d\n", comm->stats.rx_zero);
	rem -= n;
	cp += n = snprintf(cp, rem, "  -   tmo = %9d\n", comm->stats.rx_timeout);
	rem -= n;
	cp += n = snprintf(cp, rem, "  -  sync = %9d\n", comm->stats.rx_err_sync);
	rem -= n;
	cp += n = snprintf(cp, rem, "  -   crc = %9d\n", comm->stats.rx_err_crc);
	rem -= n;
	cp += n = snprintf(cp, rem, "  - short = %9d\n", comm->stats.rx_short);
	rem -= n;
	cp += n = snprintf(cp, rem, "  - dtlen = %9d\n", comm->stats.rx_err_datlen);
	rem -= n;
	cp += n = snprintf(cp, rem, "  - dtcrc = %9d\n", comm->stats.rx_err_datcrc);
	rem -= n;
	cp += n = snprintf(cp, rem, "  - error = %9d\n", comm->stats.rx_err_uart);
	rem -= n;

	return cp - buf;
}

void mstp_lnk_comm_stats_clr(struct mstp_lnk_comm *comm)
{
	comm->stats.rx_idle = 0;
	comm->stats.rx_break = 0;
	comm->stats.rx_zero = 0;
	comm->stats.rx_timeout = 0;
	comm->stats.rx_err_sync = 0;
	comm->stats.rx_err_crc = 0;
	comm->stats.rx_short = 0;
	comm->stats.rx_err_datlen = 0;
	comm->stats.rx_err_datcrc = 0;
	comm->stats.rx_err_uart = 0;
}

