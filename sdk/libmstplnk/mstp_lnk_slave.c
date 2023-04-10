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
 * @file mstp_lnk.c
 * @brief MS/TP Link Layer
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

_Pragma("GCC optimize (\"Ofast\")")

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/stm32f.h>
#include <sys/delay.h>
#include "board.h"

#include "mstp_lnk-i.h"

#undef TRACE_LEVEL
#define TRACE_LEVEL TRACE_LVL_INF
#include <trace.h>

#define SILENCE_TIMER() ((int32_t)(clk - silence_clk))
#define RESET_SILENCE_TIMER() silence_clk = clk

void mstp_lnk_slave_loop(struct mstp_lnk *lnk)
{
	struct mstp_lnk_comm * comm = lnk->comm;
	struct mstp_lnk_mgmt * mgmt = &lnk->mgmt;
	uint8_t pdu[MSTP_LNK_MTU];
	struct mstp_frm_ref frm;
	unsigned int frm_type;
	unsigned int src_addr;
	unsigned int dst_addr;
	unsigned int pdu_len;
	int event_count;
	uint32_t error_count;
	unsigned int ts;			/* This station */
	uint32_t silence_clk;
	bool rcvd_valid_frm;
	bool rcvd_invalid_frm;
	uint32_t clk = 0;
	int32_t dt;
	int ret;

	uint16_t timer_ms[] = {
		[MSTP_INITIALIZE] = 100,
		[MSTP_IDLE] = T_NO_TOKEN,
		[MSTP_ANSWER_DATA_REQUEST] = T_REPLY_DELAY
	};


	INFS("[INITIALIZE]");
	lnk->state = MSTP_INITIALIZE;
	MSTP_HW_STATE(lnk->state);
	/* In this state, the device waits for the network layer to 
	   initiate a MSTP data lnk connection or for the physical 
	   layer to indicate the occurrence of a physical 
	   layer connection. */

	memset(pdu, 0, sizeof(pdu));

	memset(&frm, 0, sizeof(struct mstp_frm_ref));

	rcvd_valid_frm = false;
	rcvd_invalid_frm = false;
	/* causing a poll for master to start when this node first receives a
	 * token */
	event_count = 0;
	error_count = 0;
	frm_type = 0;
	src_addr = 0;
	dst_addr = 0;
	pdu_len = 0;

	/* acknowledge role */
	lnk->role_ack = lnk->role_req;
	thinkos_flag_give(mgmt->flag);

	/* wait link-up request */
	while (!lnk->up_req) {
		thinkos_sleep(8);
	}

	/* set TS to the node's station address, set NS equal to TS 
	   (indicating that the next station is unknown), set PS equal to TS, 
	   set TokenCount to Npoll (thus causing a Poll For Master to be sent 
	   when this node first receives the token), set SoleMaster to FALSE, 
	   set ReceivedValidFrame and ReceivedInvalidFrame to FALSE, and enter 
	   the IDLE state.a */
	ts = lnk->this_station;
	timer_ms[MSTP_NO_TOKEN] = T_NO_TOKEN + T_SLOT * ts;
	__netmap_reset(mgmt->netmap, ts);
	__list_reset(mgmt->active, MSTP_LNK_MAX_MASTERS);

	/* Get a reference clock */
	clk = thinkos_clock();

	RESET_SILENCE_TIMER();

	lnk->state = MSTP_IDLE;
	MSTP_HW_STATE(lnk->state);
	INFS("[INITIALIZE]  --> [IDLE]");

again:

	if (lnk->role_req != MSTP_LNK_SLAVE) {
		mgmt->callback(lnk, MSTP_EV_LINK_DOWN);
		lnk->up_ack = false;
		/* disable PDU transmission */
		thinkos_gate_close(lnk->tx.gate);
		/* notify management */
		thinkos_flag_give(mgmt->flag);
		return;
	}

	/* XXX: find the clock that is about to expire first */
	dt = silence_clk - clk + timer_ms[lnk->state];

	ret = mstp_lnk_comm_frame_recv(comm, pdu, &frm, dt);
	if (ret > 0) {
		event_count += ret;
		rcvd_valid_frm = true;
		rcvd_invalid_frm = false;

		frm_type = frm.frm_type;
		dst_addr = frm.dst_addr;
		src_addr = frm.src_addr;
		pdu_len = frm.pdu_len;

		switch (frm_type) {
		case FRM_TOKEN:
			lnk->stats.rx_token++;
			break;
		case FRM_TEST_REQUEST:
			YAP("MS/TP[%s]: test request (%d) %d --> %d", state_nm[lnk->state],
				pdu_len, src_addr, dst_addr); 
			lnk->stats.rx_mgmt++;
			break;
		case FRM_TEST_RESPONSE:
			lnk->stats.rx_mgmt++;
			break;
		case FRM_DATA_NO_REPLY:
			YAP("MS/TP[%s]: data(%d) %d --> %d", state_nm[lnk->state],
				pdu_len, src_addr, dst_addr); 
			if (dst_addr == MSTP_ADDR_BCAST)
				lnk->stats.rx_bcast++;
			else
				lnk->stats.rx_unicast++;
		}

		/* update master active counter */
		if (src_addr < MSTP_LNK_MAX_MASTERS)
			mgmt->active[src_addr]++;

		RESET_SILENCE_TIMER();
	} else {
		if ((ret == MSTP_HDR_SYNC_ERROR) ||
			(ret == MSTP_HDR_CRC_ERROR))  {
#if 1
			rcvd_invalid_frm = true;
			RESET_SILENCE_TIMER();
#endif
			DBGS("MS/TP: header decoding error");
			if ((++error_count) == MSTP_LINK_HDR_ERR_MAX) {
				DBGS("MS/TP too many header errors");
				mgmt->callback(lnk, MSTP_EV_LINK_TOO_MANY_ERRORS);
				error_count = 0;
			}
		} else if (ret == MSTP_LINE_BREAK) {
			error_count = 0;
			rcvd_invalid_frm = true;
			mgmt->callback(lnk, MSTP_EV_LINK_LINE_BREAK);
			RESET_SILENCE_TIMER();
			WARNS("MS/TP: line break...");
		} else if (ret != MSTP_LNK_TIMEOUT) {
			MSTP_HW_STATE(0x0f);
			rcvd_invalid_frm = true;
			RESET_SILENCE_TIMER();
			WARN("MS/TP[%s]: invalid frame", state_nm[lnk->state]);
		}
		event_count = mstp_lnk_comm_pending_events(comm);
	}

	clk = thinkos_clock();

	switch (lnk->state) {
	case MSTP_IDLE:
		if (rcvd_valid_frm) {
			if (dst_addr == ts || dst_addr == MSTP_ADDR_BCAST) {
				uint32_t rx_seq;
				int req;
				int rsp;

				if (!lnk->up_ack) {
					lnk->up_ack = true;
					mgmt->callback(lnk, MSTP_EV_LINK_UP);
					/* enable PDU transmission */
					thinkos_gate_open(lnk->tx.gate);
					/* notify management */
					thinkos_flag_give(mgmt->flag);
				}
				
				switch (frm_type) {

				case FRM_BACNET_DATA_NO_REPLY:
				case FRM_TEST_RESPONSE:
				case FRM_DATA_NO_REPLY ... (FRM_DATA_NO_REPLY + 0x3a):
					if ((rx_seq = lnk->rx.seq) == lnk->rx.ack) {
						lnk->rx.inf.ftype = frm_type;
						lnk->rx.inf.saddr = src_addr;
						lnk->rx.inf.daddr = dst_addr;
						lnk->rx.pdu_len = pdu_len;
						memcpy(lnk->rx.pdu, pdu, pdu_len);
						lnk->rx.seq = ++rx_seq;
						thinkos_sem_post(lnk->rx.sem);
						DBG("MS/TP: RX data %d->%d (%d) ok..",
							src_addr, dst_addr, pdu_len);
					} else {
						WARN("MS/TP: RX data %d->%d (%d) drop..",
							 src_addr, dst_addr, pdu_len);
					}
					DBGS("[IDLE] ReceivedDataNoReply --> [IDLE]");
					break;
				case FRM_BACNET_DATA_XPCT_REPLY:
				case FRM_TEST_REQUEST:
					RESET_SILENCE_TIMER();
					rcvd_valid_frm = false;

					/* FIXME: remove callback and use a thread instead */
					req = pdu[0];
					ret = mgmt->callback(lnk, req);
					rsp = req | 0x80;

					mgmt->tst.req = req;
					thinkos_flag_give(mgmt->flag);
					mgmt->tst.rsp = rsp;
					mgmt->tst.ret = ret;

					if (dst_addr != MSTP_ADDR_BCAST) {
						lnk->state = MSTP_ANSWER_TEST_REQUEST;
						MSTP_HW_STATE(lnk->state);

						INFS("[IDLE] ReceivedDataNeedingReply" 
							 " --> [ANSWER_TEST_REQUEST]");
					} else {
						mgmt->callback(lnk, rsp);
						INFS("[IDLE] ReceivedBcastMgmtMsg --> [IDLE]");
					}
					break;

				case FRM_DATA_XPCT_REPLY ... (FRM_DATA_XPCT_REPLY + 0x3f):
					if ((rx_seq = lnk->rx.seq) == lnk->rx.ack) {
						lnk->rx.inf.ftype = frm_type;
						lnk->rx.inf.saddr = src_addr;
						lnk->rx.inf.daddr = dst_addr;
						lnk->rx.pdu_len = pdu_len;
						memcpy(lnk->rx.pdu, pdu, pdu_len);
						lnk->rx.seq = ++rx_seq;
						thinkos_sem_post(lnk->rx.sem);
					}
					YAP("RCV saddr=%d pdu_len=%d", src_addr,
						pdu_len);
					//bacnet_dl_pdu_recv_notify(lnk->addr.netif);
					if (dst_addr != MSTP_ADDR_BCAST) {
						lnk->state = MSTP_ANSWER_DATA_REQUEST;
						MSTP_HW_STATE(lnk->state);
						INFS("[IDLE] ReceivedDataNeedingReply" 
							 " --> [ANSWER_DATA_REQUEST]");
					}
					break;
				default:
					rcvd_valid_frm = false;
					YAPS("[IDLE] ReceivedUnwantedFrame --> [IDLE]");
					break;
				}
			} else {
				YAPS("[IDLE] NotForMe --> [IDLE]");
				rcvd_valid_frm = false;
			}
			/* For DATA_EXPECTING_REPLY, we will keep the Rx Frame for
			   reference, and the flag will be cleared in the next state */
			if (lnk->state != MSTP_ANSWER_DATA_REQUEST)
				rcvd_valid_frm = false;
		} else if (rcvd_invalid_frm) {
			rcvd_invalid_frm = false;
			YAPS("[IDLE] ReceivedInvalidFrame --> [IDLE]");
		} else {
			YAPS("[IDLE] Timeout [IDLE]");
		}
		break;

	case MSTP_ANSWER_DATA_REQUEST:
		frm_type = lnk->tx.inf.ftype;
		src_addr = lnk->tx.inf.saddr;
		dst_addr = ts;
		if ((SILENCE_TIMER() < T_REPLY_DELAY) && (lnk->tx.pdu_len > 0) &&
			(frm_type == FRM_TEST_RESPONSE ||
			 frm_type == FRM_BACNET_DATA_NO_REPLY ||
			 (frm_type >= FRM_DATA_NO_REPLY &&
			  frm_type < (FRM_DATA_NO_REPLY + 0x3f)))) {
			mstp_lnk_comm_frame_send(comm, MKROUTE(frm_type, 
												   dst_addr, src_addr),
									 lnk->tx.pdu, lnk->tx.pdu_len);
			thinkos_gate_open(lnk->tx.gate);
			RESET_SILENCE_TIMER();
			lnk->state = MSTP_IDLE;
			MSTP_HW_STATE(lnk->state);
			INFS("[ANSWER_DATA_REQUEST] Reply --> [IDLE]");
			rcvd_valid_frm = false;
		} else if (SILENCE_TIMER() >= T_REPLY_DELAY) {
			mstp_lnk_comm_fast_send(comm, FRM_REPLY_POSTPONED, src_addr);
			RESET_SILENCE_TIMER();
			lnk->state = MSTP_IDLE;
			MSTP_HW_STATE(lnk->state);
			INFS("[ANSWER_DATA_REQUEST] DeferredReply --> [IDLE]");
			rcvd_valid_frm = false;
		}
		break;

		case MSTP_ANSWER_TEST_REQUEST:
		{
			int rsp = mgmt->tst.rsp;

			pdu[0] = rsp;
			pdu[1] = mgmt->tst.ret;
			mstp_lnk_comm_frame_send(comm, MKROUTE(FRM_TEST_RESPONSE, 
												   src_addr, ts),
									 pdu, pdu_len);

			mgmt->callback(lnk, rsp);
			lnk->state = MSTP_IDLE;
			MSTP_HW_STATE(lnk->state);
			INFS("[ANSWER_TEST_REQUEST] Reply --> [IDLE]");
		}
		break;
	}

	goto again;
}

