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

#define MAINTENANCE_TIMER() ((int32_t)(clk - maintenance_clk))
#define RESET_MAINTENANCE_TIMER() maintenance_clk = clk

void mstp_lnk_master_loop(struct mstp_lnk *lnk)
{
	struct mstp_lnk_comm * comm = lnk->comm;
	struct mstp_lnk_mgmt * mgmt = &lnk->mgmt;
	uint8_t pdu[MSTP_LNK_MTU];
	struct mstp_frm_ref frm;
	unsigned int frm_type;
	unsigned int src_addr;
	unsigned int dst_addr;
	unsigned int pdu_len;
	int frame_count;
	int retry_count;
	int event_count;
	int token_count;
	unsigned int ts;			/* This station */
	unsigned int ns;			/* Next Station */
	unsigned int ps;			/* Poll Station */
	uint32_t silence_clk;
	uint32_t maintenance_clk;
	bool rcvd_valid_frm;
	bool rcvd_invalid_frm;
	bool sole_master;
	uint32_t tx_ack;
	uint32_t rx_seq;
	uint32_t clk = 0;
	uint32_t error_count;
	int32_t dt;
	int ret;

	uint16_t timer_ms[] = {
		[MSTP_INITIALIZE] = 100,
		[MSTP_IDLE] = T_NO_TOKEN,
		[MSTP_ANSWER_DATA_REQUEST] = T_REPLY_DELAY,
		[MSTP_NO_TOKEN] = T_NO_TOKEN + T_SLOT,
		[MSTP_POLL_FOR_MASTER] = T_MIN_OCT_TIMEOUT,	/*T_USAGE_TIMEOUT, */
		[MSTP_PASS_TOKEN] = T_USAGE_TIMEOUT,
		[MSTP_USE_TOKEN] = 2,
		[MSTP_DONE_WITH_TOKEN] = 0,
		[MSTP_WAIT_FOR_REPLY] = T_REPLY_TIMEOUT,
		[MSTP_WAIT_FOR_TEST_REPLY] = T_REPLY_TIMEOUT
	};

	DBGS("[INITIALIZE]");
	lnk->state = MSTP_INITIALIZE;
	MSTP_HW_STATE(lnk->state);
	/* In this state, the device waits for the network layer to 
	   initiate a MSTP data lnk connection or for the physical 
	   layer to indicate the occurrence of a physical 
	   layer connection. */

	memset(pdu, 0, sizeof(pdu));
	memset(&frm, 0, sizeof(struct mstp_frm_ref));

	/* disable PDU transmission */
	sole_master = false;
	rcvd_valid_frm = false;
	rcvd_invalid_frm = false;
	frame_count = 0;
	retry_count = 0;
	/* causing a poll for master to start when this node first receives a
	 * token */
	token_count = N_POLL;
	event_count = 0;
	error_count = 0;
	frm_type = 0;
	src_addr = 0;
	dst_addr = 0;
	pdu_len = 0;

	/* acknowledge role */
	lnk->role_ack = lnk->role_req;
	thinkos_flag_give(mgmt->flag);

	/* wait for link up request ... */
	while (!lnk->up_req)
		thinkos_sleep(8);

	/* set TS to the node's station address, set NS equal to TS 
	   (indicating that the next station is unknown), set PS equal to TS, 
	   set TokenCount to Npoll (thus causing a Poll For Master to be sent 
	   when this node first receives the token), set SoleMaster to FALSE, 
	   set ReceivedValidFrame and ReceivedInvalidFrame to FALSE, and enter 
	   the IDLE state.a */
	ts = lnk->this_station;
	ns = ts;
	ps = ns;
	timer_ms[MSTP_NO_TOKEN] = T_NO_TOKEN + T_SLOT * ts;
	__netmap_reset(mgmt->netmap, ts);
	__list_reset(mgmt->active, MSTP_LNK_MAX_MASTERS);

	/* Get a reference clock */
	clk = thinkos_clock();

	RESET_SILENCE_TIMER();
	RESET_MAINTENANCE_TIMER();

	lnk->state = MSTP_IDLE;
	MSTP_HW_STATE(lnk->state);
	DBGS("[INITIALIZE]  --> [IDLE]");

 again:

	if (lnk->role_req != MSTP_LNK_MASTER) {
		lnk->up_ack = false;
		mgmt->callback(lnk, MSTP_EV_LINK_DOWN);
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
			DBG("MS/TP[%s]: test request (%d) %d --> %d", state_nm[lnk->state],
				pdu_len, src_addr, dst_addr); 
			lnk->stats.rx_mgmt++;
			break;
		case FRM_TEST_RESPONSE:
		case FRM_POLL_FOR_MASTER:
		case FRM_REPLY_POLL_FOR_MASTER:
		case FRM_REPLY_POSTPONED:
			lnk->stats.rx_mgmt++;
			break;
		case FRM_DATA_NO_REPLY:
			DBG("MS/TP[%s]: data(%d) %d --> %d", state_nm[lnk->state],
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
#if 0
			rcvd_invalid_frm = true;
			RESET_SILENCE_TIMER();
#endif		
			lnk->stats.rx_err++;
			WARNS("MS/TP: header decoding error");
			if ((++error_count) == MSTP_LINK_HDR_ERR_MAX) {
				DBGS("MS/TP too many header errors");
				mgmt->callback(lnk, MSTP_EV_LINK_TOO_MANY_ERRORS);
				error_count = 0;
			}
		} else if (ret == MSTP_LINE_BREAK) {
			rcvd_invalid_frm = true;
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

 transition_now:

	switch (lnk->state) {
	case MSTP_IDLE:
		if (rcvd_valid_frm) {
			if (dst_addr == ts || dst_addr == MSTP_ADDR_BCAST) {
				int req;
				int rsp;
				int ret;

				switch (frm_type) {
				case FRM_TOKEN:
					if (dst_addr == MSTP_ADDR_BCAST) {
						ERRS("FRM_TOKEN, dst_addr==MSTP_ADDR_BCAST");
						break;
					}
					rcvd_valid_frm = false;
					frame_count = 0;
					if (sole_master) {
						error_count = 0;
						sole_master = false;
						mgmt->callback(lnk, MSTP_EV_MULTIMASTER);
					}
					if (!lnk->up_ack) {
						lnk->up_ack = true;
						mgmt->callback(lnk, MSTP_EV_LINK_UP);
						/* enable PDU transmission */
						thinkos_gate_open(lnk->tx.gate);
						/* notify management */
						thinkos_flag_give(mgmt->flag);
					}
					lnk->state = MSTP_USE_TOKEN;
					MSTP_HW_STATE(lnk->state);
					YAPS("[IDLE] ReceivedToken --> [USE_TOKEN]");

					goto transition_now;

				case FRM_POLL_FOR_MASTER:
					if (dst_addr == ts) {
						DBG("Replying back to PFM from %d", src_addr);
						mstp_lnk_comm_fast_send(comm, FRM_REPLY_POLL_FOR_MASTER, 
												src_addr);
						lnk->stats.tx_mgmt++;
						rcvd_valid_frm = false;
						RESET_SILENCE_TIMER();
						/* add transmitter's address to address map */
						__netmap_insert(mgmt->netmap, src_addr);
					}
					lnk->state = MSTP_IDLE;
					MSTP_HW_STATE(lnk->state);
					DBGS("[IDLE] ReceivedPFM --> [IDLE]");
					break;

				case FRM_TEST_REQUEST:
					rcvd_valid_frm = false;
					RESET_SILENCE_TIMER();

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

						DBGS("[IDLE] ReceivedDataNeedingReply" 
							 " --> [ANSWER_TEST_REQUEST]");
					} else {
						mgmt->callback(lnk, rsp);
						DBGS("[IDLE] ReceivedBcastMgmtMsg --> [IDLE]");
					}
					break;

				case FRM_BACNET_DATA_NO_REPLY:
				case FRM_TEST_RESPONSE:
				case FRM_DATA_NO_REPLY ... (FRM_DATA_NO_REPLY + 0x3f):
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
				case FRM_DATA_XPCT_REPLY ... (FRM_DATA_XPCT_REPLY + 0x38):
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
						DBGS("[IDLE] ReceivedDataNeedingReply" 
							 " --> [ANSWER_DATA_REQUEST]");
					}
					break;

				default:
					rcvd_valid_frm = false;
					DBGS("[IDLE] ReceivedUnwantedFrame --> [IDLE]");
					break;
				}
			} else {
				DBGS("[IDLE] NotForMe --> [IDLE]");
				rcvd_valid_frm = false;
				if ((frm_type == FRM_POLL_FOR_MASTER) || 
					(frm_type == FRM_REPLY_POLL_FOR_MASTER)) {
					/* keep track of other masters */
					__netmap_insert(mgmt->netmap, src_addr);
				}
			}
			/* For DATA_EXPECTING_REPLY, we will keep the Rx Frame for
			   reference, and the flag will be cleared in the next state */
			if (lnk->state != MSTP_ANSWER_DATA_REQUEST)
				rcvd_valid_frm = false;
		} else if (rcvd_invalid_frm) {
			rcvd_invalid_frm = false;
			DBGS("[IDLE] ReceivedInvalidFrame --> [IDLE]");
			goto transition_now;
		} else if (SILENCE_TIMER() >= T_NO_TOKEN) {
			event_count = 0;	/* Addendum 135-2004d-8 */
			/* lost token */
			lnk->state = MSTP_NO_TOKEN;
			MSTP_HW_STATE(lnk->state);
			DBGS("[IDLE] LostToken --> [NO_TOKEN]");
			rcvd_invalid_frm = false;
			rcvd_valid_frm = false;
			lnk->stats.token_lost++;
			goto transition_now;
		}
		break;

	case MSTP_USE_TOKEN:
		if ((tx_ack = mgmt->tx.ack) != mgmt->tx.seq) {
			uint8_t * pdu_dat;
			/* management data frame pending transmission */
			/* queue is not empty, dequeue and then fill in the information */
			frm_type = mgmt->tx.ftype;
			dst_addr = mgmt->tx.daddr;
			src_addr = mgmt->tx.saddr;
			pdu_len = mgmt->tx.pdu_len;
			pdu_dat = mgmt->tx.pdu;
			if ((frm_type == FRM_TEST_REQUEST) && (dst_addr == ts)) {
				mgmt->callback(lnk, MSTP_EV_PROBE_BEGIN);
				mgmt->loop_probe = true;
			}
			mstp_lnk_comm_frame_send(comm, 
									 MKROUTE(frm_type, dst_addr, src_addr),
							pdu_dat, pdu_len);
			
			mgmt->tx.ack = ++tx_ack;
			thinkos_flag_give(mgmt->flag);
			RESET_SILENCE_TIMER();
			frame_count++;
		} else if ((tx_ack = lnk->tx.ack) != lnk->tx.seq) {
			/* data frame pending transmission */
			/* queue is not empty, dequeue and then fill in the information */
			frm_type = lnk->tx.inf.ftype;
			dst_addr = lnk->tx.inf.daddr;
			src_addr = lnk->tx.inf.saddr;
			pdu_len = lnk->tx.pdu_len;
			mstp_lnk_comm_frame_send(comm, MKROUTE(frm_type, dst_addr, src_addr),
							lnk->tx.pdu, pdu_len);
			lnk->tx.ack = ++tx_ack;
			thinkos_gate_open(lnk->tx.gate);
			RESET_SILENCE_TIMER();
			frame_count++;
		} else {
			/* no data frame awaiting transmission */
			frame_count = N_MAX_INFO_FRAMES;
			lnk->state = MSTP_DONE_WITH_TOKEN;
			MSTP_HW_STATE(lnk->state);
			YAPS("[USE_TOKEN] NothingToSend --> [DONE_WITH_TOKEN]");
			goto transition_now;
		}

		switch (frm_type) {
		case FRM_BACNET_DATA_XPCT_REPLY:
		case FRM_DATA_XPCT_REPLY ... (FRM_DATA_XPCT_REPLY + 0x3f):
			if (dst_addr == MSTP_ADDR_BCAST) {
				lnk->state = MSTP_DONE_WITH_TOKEN;
				MSTP_HW_STATE(lnk->state);
				DBGS("[USE_TOKEN] SendNoWait --> [DONE_WITH_TOKEN]");
				goto transition_now;
			} else {
				lnk->state = MSTP_WAIT_FOR_REPLY;
				MSTP_HW_STATE(lnk->state);
				DBGS("[USE_TOKEN] SendAndWait --> [WAIT_FOR_REPLY]");
			}
			break;

		case FRM_TEST_REQUEST:
			if (dst_addr == MSTP_ADDR_BCAST) {
				lnk->state = MSTP_DONE_WITH_TOKEN;
				MSTP_HW_STATE(lnk->state);
				DBGS("[USE_TOKEN] SendNoWait --> [DONE_WITH_TOKEN]");
				goto transition_now;
			} else if (dst_addr == ts) {
				/* Loopback test requested */
				lnk->state = MSTP_WAIT_FOR_TEST_REPLY;
				MSTP_HW_STATE(lnk->state);
				DBGS("[USE_TOKEN] SendProbeAndWait --> [WAIT_FOR_TEST_REPLY]");
			} else {
				if (mgmt->tx.pdu[0] == LINK_REQ_ISOA) {
					INFS("LINK_REQ_ISOA");
				}

				lnk->state = MSTP_WAIT_FOR_TEST_REPLY;
				MSTP_HW_STATE(lnk->state);
				DBGS("[USE_TOKEN] SendTestAndWait --> [WAIT_FOR_TEST_REPLY]");
			}
			break;

		case FRM_DATA_NO_REPLY ... (FRM_DATA_NO_REPLY + 0x3d):	
			DBG("MS/TP: TX(%d) data %d -> %d", pdu_len, ts, dst_addr);
			/* FALLTHROUGH */
		case FRM_TEST_RESPONSE:
		case FRM_BACNET_DATA_NO_REPLY:
		default:
			lnk->state = MSTP_DONE_WITH_TOKEN;
			MSTP_HW_STATE(lnk->state);
			DBGS("[USE_TOKEN] SendNoWait --> [DONE_WITH_TOKEN]");
			goto transition_now;
		}
		break;

	case MSTP_WAIT_FOR_REPLY:
		if (SILENCE_TIMER() >= T_REPLY_TIMEOUT) {
			frame_count = N_MAX_INFO_FRAMES;
			lnk->state = MSTP_DONE_WITH_TOKEN;
			MSTP_HW_STATE(lnk->state);
			DBGS("[WAIT_FOR_REPLY] ReplyTimeout"
			     " --> [DONE_WITH_TOKEN]");
			goto transition_now;
		} else {
			if (rcvd_invalid_frm == true) {
				rcvd_invalid_frm = false;
				lnk->state = MSTP_DONE_WITH_TOKEN;
				MSTP_HW_STATE(lnk->state);
				INFS("[WAIT_FOR_REPLY] InvalidFrame"
				     " --> [DONE_WITH_TOKEN]");
				goto transition_now;
			} else if (rcvd_valid_frm == true) {
				if (dst_addr == ts) {
					switch (frm_type) {
					case FRM_REPLY_POSTPONED:
						lnk->state = MSTP_DONE_WITH_TOKEN;
						MSTP_HW_STATE(lnk->state);
						DBGS("[WAIT_FOR_REPLY] ReceivedPostponed" 
							 " --> [DONE_WITH_TOKEN]");
						break;
					case FRM_TEST_RESPONSE:
					case FRM_BACNET_DATA_NO_REPLY:
					case FRM_DATA_NO_REPLY ... (FRM_DATA_NO_REPLY + 0x3f):
						if ((rx_seq = lnk->rx.seq) == lnk->rx.ack) {
							lnk->rx.inf.ftype = frm_type;
							lnk->rx.inf.saddr = src_addr;
							lnk->rx.inf.daddr = dst_addr;
							lnk->rx.pdu_len = pdu_len;
							lnk->rx.seq = ++rx_seq;
							memcpy(lnk->rx.pdu, pdu, pdu_len);
							thinkos_sem_post(lnk->rx.sem);
						}
						lnk->state = MSTP_DONE_WITH_TOKEN;
						MSTP_HW_STATE(lnk->state);
						YAP("RCV saddr=%d pdu_len=%d",
						    src_addr, pdu_len);
						DBGS("[WAIT_FOR_REPLY] ReceivedReply --> "
							 "[DONE_WITH_TOKEN]");
						break;

						/* FALLTHROUGH */
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
						lnk->state = MSTP_DONE_WITH_TOKEN;
						MSTP_HW_STATE(lnk->state);
						YAP("RCV saddr=%d pdu_len=%d",
						    src_addr, pdu_len);
						DBGS("[WAIT_FOR_REPLY] ReceivedReply --> "
							 "[DONE_WITH_TOKEN]");
						break;
					default:
						lnk->state = MSTP_IDLE;
						MSTP_HW_STATE(lnk->state);
						INF("[WAIT_FOR_REPLY]"
						    " ReceivedUnexpectedFrame --> [IDLE] %d %d",
						    frm_type, src_addr);
						break;
					}
				} else {
					lnk->state = MSTP_IDLE;
					MSTP_HW_STATE(lnk->state);
					DBGS("[WAIT_FOR_REPLY] ReceivedUnexpectedFrame --> [IDLE]");
				}
				rcvd_valid_frm = false;
				goto transition_now;
			}
		}
		break;

	case MSTP_WAIT_FOR_TEST_REPLY:
		if (rcvd_invalid_frm == true) {
			rcvd_invalid_frm = false;
			WARNS("[WAIT_FOR_TEST_REPLY] InvalidFrame"
				 " --> [DONE_WITH_TOKEN]");
			goto test_reply_error;
		} 
		
		if (rcvd_valid_frm == false) {
			if (SILENCE_TIMER() >= T_REPLY_TIMEOUT) {
				frame_count = N_MAX_INFO_FRAMES;
				if (mgmt->loop_probe) {
					mgmt->callback(lnk, MSTP_EV_PROBE_END);
					mgmt->loop_probe = false;
				}
				DBGS("[WAIT_FOR_TEST_REPLY] TestTimeout"
					 " --> [DONE_WITH_TOKEN]");
				goto test_reply_error;
			} 
			/* again */
			break;
		}

		/* Got a valid frame */

		if (mgmt->loop_probe) {
			mgmt->callback(lnk, MSTP_EV_PROBE_END);
			mgmt->loop_probe = false;
		}

		rcvd_valid_frm = false;

		DBGS("[WAIT_FOR_TEST_REPLY] ReceivedTestResponse --> "
			 "[DONE_WITH_TOKEN]");

		lnk->state = MSTP_DONE_WITH_TOKEN;
		MSTP_HW_STATE(lnk->state);

		if ((rx_seq = mgmt->rx.seq) == mgmt->rx.ack) {
			mgmt->rx.ftype = frm_type;
			mgmt->rx.saddr = src_addr;
			mgmt->rx.daddr = dst_addr;
			mgmt->rx.pdu_len = pdu_len;
			memcpy(mgmt->rx.pdu, pdu, (pdu_len > MSTP_LNK_MGMT_PDU_MAX) ? 
				   MSTP_LNK_MGMT_PDU_MAX : pdu_len);
			mgmt->rx.seq = ++rx_seq;
			thinkos_flag_give(mgmt->flag);
		} else {
			WARN("rx_seq(%d) != rx.ack(%d)", rx_seq, mgmt->rx.ack);
		}

		goto transition_now;

test_reply_error:
		if ((rx_seq = mgmt->rx.seq) == mgmt->rx.ack) {
			/* Dummy frame */
			mgmt->rx.ftype = 0;
			mgmt->rx.saddr = 0;
			mgmt->rx.daddr = 0;
			mgmt->rx.pdu_len = 0;
			mgmt->rx.seq = ++rx_seq;
			thinkos_flag_give(mgmt->flag);
		}

		lnk->state = MSTP_DONE_WITH_TOKEN;
		MSTP_HW_STATE(lnk->state);
		goto transition_now;


	case MSTP_DONE_WITH_TOKEN:
		/* The DONE_WITH_TOKEN state either sends another data frame,  */
		/* passes the token, or initiates a Poll For Master cycle. */

		/* SendAnotherFrame */
		if (frame_count < N_MAX_INFO_FRAMES) {
			lnk->state = MSTP_USE_TOKEN;
			MSTP_HW_STATE(lnk->state);
			DBGS("[DONE_WITH_TOKEN] SendAnotherFrame --> [USE_TOKEN]");
			goto transition_now;
		}
		/* NextStationUnknown - added in Addendum 135-2008v-1 */
		/*  then the next station to which the token
		   should be sent is unknown - so PollForMaster */
		else if ((sole_master == false) && (ns == ts)) {
			ps = (ts + 1) % (N_MAX_MASTER + 1);
			mstp_lnk_comm_fast_send(comm, FRM_POLL_FOR_MASTER, ps);
			lnk->stats.tx_pfm++;
			RESET_MAINTENANCE_TIMER();
			RESET_SILENCE_TIMER();
			retry_count = 0;
			lnk->state = MSTP_POLL_FOR_MASTER;
			MSTP_HW_STATE(lnk->state);
			DBG("[DONE_WITH_TOKEN]"
			    " NextStationUnknown --> [POLL_FOR_MASTER] %d", ps);
		}
		/* SoleMaster */
		else if (token_count < (N_POLL - 1)) {
			if (sole_master == true &&
			    ns != (ts + 1) % (N_MAX_MASTER + 1)) {
//				frame_count = 0;
#if 0
				token_count++;
				thinkos_sleep(1);
				/* FIXME: test only ... */
	//			RESET_SILENCE_TIMER();
#else
				token_count = N_POLL - 1;
#endif
				retry_count = 0;
				event_count = 0;
				lnk->state = MSTP_USE_TOKEN;
				MSTP_HW_STATE(lnk->state);
				DBGS("[DONE_WITH_TOKEN] SoleMaster --> [USE_TOKEN]");
				goto transition_now;
			}
			/* SendToken */
			else {	/* not sole master */
				token_count++;
				mstp_lnk_comm_fast_send(comm, FRM_TOKEN, ns);
				lnk->stats.tx_pfm++;
				RESET_SILENCE_TIMER();
				retry_count = 0;
				event_count = 0;
				lnk->state = MSTP_PASS_TOKEN;
				MSTP_HW_STATE(lnk->state);
				DBG("[DONE_WITH_TOKEN] SendToken --> [PASS_TOKEN] to %d", ns);
			}
		} else if ((ps + 1) % (N_MAX_MASTER + 1) == ns) {
			/* SoleMasterRestartMaintenancePFM */
			if (sole_master == true) {
				ps = (ns + 1) % (N_MAX_MASTER + 1);
				mstp_lnk_comm_fast_send(comm, FRM_POLL_FOR_MASTER, ps);
				lnk->stats.tx_pfm++;
				RESET_SILENCE_TIMER();
				RESET_MAINTENANCE_TIMER();
				ns = ts;
				retry_count = 0;
				/* changed in Errata SSPC-135-2004 */
				token_count = 1;
				/* event_count = 0; removed in Addendum 135-2004d-8 */
				lnk->state = MSTP_POLL_FOR_MASTER;
				MSTP_HW_STATE(lnk->state);
				DBG("[DONE_WITH_TOKEN]"
				    " SoleMasterRestartMaintenancePFM -->"
				    " [POLL_FOR_MASTER] %d", ps);
			}
			/* ResetMaintenancePFM */
			else {
				ps = ts;
				mstp_lnk_comm_fast_send(comm, FRM_TOKEN, ns);
				lnk->stats.tx_token++;
				RESET_SILENCE_TIMER();
				retry_count = 0;
				/* changed in Errata SSPC-135-2004 */
				token_count = 1;
				event_count = 0;
				lnk->state = MSTP_PASS_TOKEN;
				MSTP_HW_STATE(lnk->state);
				DBG("[DONE_WITH_TOKEN] ResetMaintenancePFM"
				    " --> [PASS_TOKEN] %d", ns);
			}
		}
		/* SendMaintenancePFM */
		else {
			ps = (ps + 1) % (N_MAX_MASTER + 1);
			mstp_lnk_comm_fast_send(comm, FRM_POLL_FOR_MASTER, ps);
			lnk->stats.tx_pfm++;
			RESET_MAINTENANCE_TIMER();
			RESET_SILENCE_TIMER();
			retry_count = 0;
			lnk->state = MSTP_POLL_FOR_MASTER;
			MSTP_HW_STATE(lnk->state);
			DBG("[DONE_WITH_TOKEN] SendMaintenancePFM"
			    " --> [POLL_FOR_MASTER] %d", ps);
		}

		break;

	case MSTP_PASS_TOKEN:
		if (retry_count >= N_RETRY_TOKEN) {
			ps = (ns + 1) % (N_MAX_MASTER + 1);
			mstp_lnk_comm_fast_send(comm, FRM_POLL_FOR_MASTER, ps);
			lnk->stats.tx_pfm++;
			RESET_MAINTENANCE_TIMER();
			RESET_SILENCE_TIMER();
			ns = ts;	/* no known successor node */
			retry_count = 0;
			token_count = 0;
			__netmap_reset(mgmt->netmap, ts);
			/* event_count = 0; removed in Addendum 135-2004d-8 */
			lnk->state = MSTP_POLL_FOR_MASTER;
			MSTP_HW_STATE(lnk->state);
			INF("[PASS_TOKEN] FindNewSuccessor --> [POLL_FOR_MASTER] %d", ps);
		} else if (SILENCE_TIMER() <= T_USAGE_TIMEOUT) {
			if (event_count > N_MIN_OCTETS) {
				/* Enter the IDLE state to process the frame. */
				lnk->state = MSTP_IDLE;
				MSTP_HW_STATE(lnk->state);
				YAPS("[PASS_TOKEN] SawTokenUser --> [IDLE]");
				goto transition_now;
			}
		} else {
			if (retry_count < N_RETRY_TOKEN) {
				retry_count++;
				mstp_lnk_comm_fast_send(comm, FRM_TOKEN, ns);
				lnk->stats.tx_token++;
				RESET_SILENCE_TIMER();
				event_count = 0;
				/* stay in current state to listen for NS to begin using
				 *  the token */
				DBG("[PASS_TOKEN] RetrySendToken --> [PASS_TOKEN] to %d", ns);
				goto transition_now;
			} else {
				ps = (ns + 1) % (N_MAX_MASTER + 1);
				mstp_lnk_comm_fast_send(comm, FRM_POLL_FOR_MASTER, ps);
				lnk->stats.tx_pfm++;
				RESET_MAINTENANCE_TIMER();
				RESET_SILENCE_TIMER();
				ns = ts;	/* no known successor node */
				retry_count = 0;
				token_count = 0;
				__netmap_reset(mgmt->netmap, ts);
				/* event_count = 0; removed in Addendum 135-2004d-8 */
				lnk->state = MSTP_POLL_FOR_MASTER;
				MSTP_HW_STATE(lnk->state);
				INF("[PASS_TOKEN] FindNewSuccessor --> "
					"[POLL_FOR_MASTER] %d", ns);
			}
		}
		break;

	case MSTP_NO_TOKEN:

		if ((SILENCE_TIMER() < (int32_t)(T_NO_TOKEN + T_SLOT * ts)) && 
			(MAINTENANCE_TIMER() < T_MAINTENANCE_TIMEOUT)) {
			if (event_count > N_MIN_OCTETS) {
				lnk->state = MSTP_IDLE;
				MSTP_HW_STATE(lnk->state);
				DBGS("[NO_TOKEN] SawFRame 1--> [IDLE]");
				goto transition_now;
			}
			YAP("(event_count(%d) <= N_MIN_OCTETS!!", event_count);
		} else {
			int32_t ns_tmo = T_NO_TOKEN + T_SLOT * (ts + 1);
			if (SILENCE_TIMER() < ns_tmo) {
				ps = (ts + 1) % (N_MAX_MASTER + 1);
				mstp_lnk_comm_fast_send(comm, FRM_POLL_FOR_MASTER, ps);
				lnk->stats.tx_pfm++;
				RESET_MAINTENANCE_TIMER();
				RESET_SILENCE_TIMER();
				ns = ts;	/* next station is unknown */
				token_count = 0;
				/* event_count = 0; removed Addendum 135-2004d-8 */
				retry_count = 0;
				lnk->state = MSTP_POLL_FOR_MASTER;
				MSTP_HW_STATE(lnk->state);
				DBG("[NO_TOKEN] GenerateToken "
				    "--> [POLL_FOR_MASTER] %d", ps);
			}
			/* FIXME: not sure if this is necessary */
			else {
				if (event_count > N_MIN_OCTETS) {
					lnk->state = MSTP_IDLE;
					MSTP_HW_STATE(lnk->state);
					DBGS("[NO_TOKEN] SawFRame 2--> [IDLE]");
					goto transition_now;
				}
			}
		}
		break;

	case MSTP_POLL_FOR_MASTER:
		/* In the POLL_FOR_MASTER state, the node listens for a reply to */
		/* a previously sent Poll For Master frame in order to find  */
		/* a successor node. */
		if (rcvd_valid_frm == true) {
			if (dst_addr == ts
			    && frm_type == FRM_REPLY_POLL_FOR_MASTER) {
				if (ns != src_addr) {
					ns = src_addr;
					DBG("MS/TP next_station=%d", ns);
					__netmap_insert(mgmt->netmap, ns);
				}
				event_count = 0;
				mstp_lnk_comm_fast_send(comm, FRM_TOKEN, ns);
				lnk->stats.tx_token++;
				RESET_SILENCE_TIMER();
				ps = ts;
				token_count = 0;
				retry_count = 0;
				rcvd_valid_frm = false;
				if (sole_master) {
					error_count = 0;
					sole_master =  false;
					mgmt->callback(lnk, MSTP_EV_MULTIMASTER);
					/* enable PDU transmission */
					thinkos_gate_open(lnk->tx.gate);
				}
				lnk->state = MSTP_PASS_TOKEN;
				MSTP_HW_STATE(lnk->state);
				DBGS("[POLL_FOR_MASTER] ReceivedReplyToPFM --> [PASS_TOKEN]");
			} else {
				rcvd_valid_frm = false;
				lnk->state = MSTP_IDLE;
				MSTP_HW_STATE(lnk->state);
				DBGS("[POLL_FOR_MASTER] "
				     "ReceivedUnexpectedFrame --> [IDLE]");
				YAP("frame type = %d, dst_addr = %d, src_addr = %d", 
					frm_type, dst_addr, src_addr);
				goto transition_now;
			}
		} else if (SILENCE_TIMER() >= T_PFM_TIMEOUT ||
			   rcvd_invalid_frm == true) {
			if (sole_master == true) {
				frame_count = 0;
				lnk->state = MSTP_USE_TOKEN;
				MSTP_HW_STATE(lnk->state);
				DBGS("[POLL_FOR_MASTER] SoleMaster --> [USE_TOKEN]");
				/* XXX: hack for testing purposes .... */
				RESET_SILENCE_TIMER();
				/* XXX: end of hack. */
			} else {
				if (ns != ts) {
					event_count = 0;
					mstp_lnk_comm_fast_send(comm, FRM_TOKEN, ns);
					lnk->stats.tx_token++;
					RESET_SILENCE_TIMER();
					retry_count = 0;
					lnk->state = MSTP_PASS_TOKEN;
					MSTP_HW_STATE(lnk->state);
					DBGS("[POLL_FOR_MASTER] DoneWithPFM --> [PASS_TOKEN]");
				} else {
					if ((ps + 1) % (N_MAX_MASTER + 1) != ts) {
						ps = (ps + 1) % (N_MAX_MASTER + 1);
						mstp_lnk_comm_fast_send(comm, FRM_POLL_FOR_MASTER, ps);
						lnk->stats.tx_pfm++;
						RESET_MAINTENANCE_TIMER();
						RESET_SILENCE_TIMER();
						retry_count = 0;
						DBG("[POLL_FOR_MASTER] SendNextPFM --> " 
							"[POLL_FOR_MASTER] to %d", ps);
					} else {
						frame_count = 0;
						rcvd_invalid_frm = false;
						lnk->state = MSTP_USE_TOKEN;
						MSTP_HW_STATE(lnk->state);
						sole_master = true;
						mgmt->callback(lnk, MSTP_EV_SOLE_MASTER);
						if (!lnk->up_ack) {
							error_count = 0;
							lnk->up_ack = true;
							mgmt->callback(lnk, MSTP_EV_LINK_UP);
							/* enable PDU transmission */
							thinkos_gate_open(lnk->tx.gate);
							/* notify management */
							thinkos_flag_give(mgmt->flag);
						}
						INFS("[POLL_FOR_MASTER] DeclareSoleMaster -->" 
							 " [USE_TOKEN]");
				
						goto transition_now;
					}
				}
			}
			rcvd_invalid_frm = false;
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
			mstp_lnk_comm_frame_send(comm, MKROUTE(frm_type, dst_addr, src_addr),
							lnk->tx.pdu, lnk->tx.pdu_len);
			thinkos_gate_open(lnk->tx.gate);
			RESET_SILENCE_TIMER();
			lnk->state = MSTP_IDLE;
			MSTP_HW_STATE(lnk->state);
			INFS("[ANSWER_DATA_REQUEST] Reply --> IDLE[]");
			rcvd_valid_frm = false;
		} else if (SILENCE_TIMER() >= T_REPLY_DELAY) {
			mstp_lnk_comm_fast_send(comm, FRM_REPLY_POSTPONED, src_addr);
			lnk->stats.tx_mgmt++;
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

