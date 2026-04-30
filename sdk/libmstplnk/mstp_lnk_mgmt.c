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

/* -------------------------------------------------------------------------
 * Link Management
 * ------------------------------------------------------------------------- */

int mstp_lnk_default_callback(struct mstp_lnk *lnk, unsigned int ev)
{
	(void)lnk;
	(void)ev;

	switch (ev) {
	case MSTP_EV_SOLE_MASTER:
		INFS("MSTP_EV_SOLE_MASTER");
		break;
	case MSTP_EV_MULTIMASTER:
		INFS("MSTP_EV_MULTIMASTER");
	break;
	}

	return 0;
}

int mstp_lnk_mgmt_set(struct mstp_lnk *lnk, 
					  int (*cbk) (struct mstp_lnk *, unsigned int))
{
	if (lnk == NULL)
		return -EINVAL;

	lnk->mgmt.callback = cbk;
	return 0;
}

/* -------------------------------------------------------------------------
 * MSTP Network Map
 * ------------------------------------------------------------------------- */

unsigned int mstp_lnk_getnetmap(struct mstp_lnk *lnk, uint8_t map[],
				unsigned int max)
{
	unsigned int cnt = 0;
	int addr;
	int i_addr;
	int j_addr;

	if (max > MSTP_LNK_MAX_MASTERS)
		max = MSTP_LNK_MAX_MASTERS;

	for (addr = 0; addr < MSTP_LNK_MAX_MASTERS; ++addr) {
		i_addr = addr / 32;
		j_addr = addr % 32;
		if (lnk->mgmt.netmap[i_addr] & (1 << j_addr)) {
			if (cnt < max)
				map[cnt++] = addr;
		}
	}

	return cnt;
}

unsigned int mstp_lnk_active_get(struct mstp_lnk *lnk, uint8_t map[],
								 unsigned int max)
{
	struct mstp_lnk_mgmt * mgmt = &lnk->mgmt;
	unsigned int cnt = 0;
	int i;

	if (max > MSTP_LNK_MAX_MASTERS)
		max = MSTP_LNK_MAX_MASTERS;

	for (i = 0; i < MSTP_LNK_MAX_MASTERS; ++i) {
		if (mgmt->active[i]) {
			if (cnt < max)
				map[cnt++] = i;
		}
		mgmt->active[i] = 0;
	}

	return cnt;
}

bool mstp_lnk_firstaddr(struct mstp_lnk *mstp)
{
	uint8_t map[16];
	unsigned int cnt;
	unsigned int i;
	bool firstAddr = true;

	cnt = mstp_lnk_getnetmap(mstp, map, sizeof(map));

	/*for (i = 0; i < cnt; ++i) {
	   printf("\n\t%2d - Node Address %2d", i, map[i]);
	   } */

	/* if this node is the smallest address on the network */
	for (i = 0; i < cnt; ++i) {
		if (map[i] < mstp->this_station) {
			firstAddr = false;
			break;
		}
	}

	YAP("first address or not: %d", firstAddr);
	return firstAddr;
}

ssize_t mstp_lnk_mgmt_send(struct mstp_lnk *lnk, 
						   const struct mstp_lnk_mgmt_msg * msg, 
						   const void *buf, size_t cnt)
{
	struct mstp_lnk_mgmt * mgmt = &lnk->mgmt;
	int ts = lnk->this_station;
	int src_addr;
	int dst_addr;
	int frm_type;
	uint32_t tx_seq;
	uint32_t rx_ack;

	uint8_t q_opc;
	uint8_t q_arg;
	uint16_t q_id;

	uint8_t p_opc;
	uint8_t p_arg;
	uint16_t p_id;
	int  p_len;
	uint8_t * pdu;

	int q_len;
	int ret;

	if (cnt > MSTP_LNK_MGMT_DATA_MAX) {
		ERR("too large:  %d > %d", cnt, MSTP_LNK_MGMT_DATA_MAX);
		return -EINVAL;
	}

	/* mutex_lock(mgmt->mutex); */
	tx_seq = mgmt->tx.seq;
	ret = 0;
	while (tx_seq != mgmt->tx.ack) {
		if (ret < 0) {
			if (ret == THINKOS_ETIMEDOUT) {
				WARNS("Mgmt Tx queue timeout...");
			}
			return ret;
		}
		ret = thinkos_flag_timedtake(mgmt->flag, T_SEND_WAIT_TIMEOUT);
	}

	/* mutex_unlock(mgmt->mutex); */
	rx_ack = mgmt->rx.ack;
	while (rx_ack != mgmt->rx.seq) {
		mgmt->rx.ack = ++rx_ack;
		INFS("MgmtReq: out of sync ...");
	}

	DBGS("MgmtReq: send request...");

	pdu = mgmt->tx.pdu;

	while ((q_id = rand()) == 0);

	q_opc = msg->req;
	q_arg = msg->opt;

	pdu[0] = q_opc;
	pdu[1] = q_arg;
	pdu[2] = q_id >> 8;
	pdu[3] = q_id;
	q_len = 4;
	if (cnt) {
		memcpy(&pdu[4], buf, cnt);
		q_len += cnt;
	}
	while ((q_len % 4) != 2) 
		pdu[q_len++] = 0;

	mgmt->tx.saddr = ts;
	mgmt->tx.daddr = msg->daddr;
	mgmt->tx.ftype = FRM_TEST_REQUEST;
	mgmt->tx.pdu_len = q_len;
	mgmt->tx.seq = ++tx_seq;

	YAPS("MgmtReq: wait for xfer complete ...");
	ret = 0;
	while (tx_seq != mgmt->tx.ack) {
		/* Wait for transfer complete */
//		thinkos_flag_take(mgmt->flag);
		if (ret < 0) {
			/* mutex_unlock(mgmt->mutex); */
			WARNS("MgmtReq Tx flag timeout 2 ...");
			return ret;
		}	
		ret = thinkos_flag_timedtake(mgmt->flag, T_SEND_WAIT_TIMEOUT);
	}

	if (msg->daddr == MSTP_ADDR_BCAST) {
		/* mutex_unlock(mgmt->mutex); */
		DBGS("MgmtReq broadcast ...");
		return q_len;
	}

	mstp_dbg_io_0_toggle();

	/* Not broadcast, wait for response... */
	DBGS("MgmtReq: wait for reply...");

	ret = 0;
	while (rx_ack == mgmt->rx.seq) {
//		thinkos_flag_take(mgmt->flag);
		if (ret < 0) {
			/* mutex_unlock(mgmt->mutex); */
			WARNS("MgmtReq: Rx flag timeout");
			return ret;
		}
		ret = thinkos_flag_timedtake(mgmt->flag, 2 * T_REPLY_TIMEOUT);
	}
	mgmt->rx.ack = ++rx_ack;

	pdu = mgmt->rx.pdu;
	p_len = mgmt->rx.pdu_len;
	p_opc = pdu[0];
	p_arg = pdu[1];
	p_id = (pdu[2] << 8) | pdu[3];
	src_addr = mgmt->rx.saddr;
	dst_addr = mgmt->rx.daddr;
	frm_type = mgmt->rx.ftype;


	if (p_len == 0)  {
		DBGS("MgmtReq: pdu_len==0 !!!.");
		return -ETIMEDOUT;
	}

	if (q_len != p_len)  {
		WARNS("MgmtReq: PDU length invalid...");
		return -EIO;
	}

	DBG("Mgmt: RX(%d, %d) 0x%02x --> 0x%02x", 
		frm_type, p_len, src_addr, dst_addr);

	if ((frm_type == FRM_TEST_REQUEST) && (src_addr == dst_addr)) {
		/*Loopback test */
		DBGS("MgmtReq: loop test...");
		ret = 1;
	} else {
		if ((frm_type == FRM_TEST_RESPONSE) && (dst_addr == ts) &&
			(p_opc == (q_opc | 0x80)) && (p_id == q_id)) {
			ret = p_arg;
			DBG("MgmtReq: test response = %d ...", ret);
		} else {
			ERR("MgmtReq: REQ(%d) %02x %02x %04x...", q_len, q_opc, q_arg, q_id);
			ERR("       : RSP(%d) %02x %02x %04x...", p_len, p_opc, p_arg, p_id);
			ret = -1;
		}
	}

	return ret;
}

void mstp_lnk_mgmt_reset(struct mstp_lnk_mgmt * mgmt)
{
	/* initialize queues */
	mgmt->rx.seq = 0;
	mgmt->rx.ack = 0;

	mgmt->tx.seq = 0;
	mgmt->tx.ack = 0;

}

int mstp_lnk_role_get(struct mstp_lnk *lnk)
{
	return lnk->role_ack;
}

int mstp_lnk_role_req_get(struct mstp_lnk *lnk)
{
	return lnk->role_req;
}

int mstp_lnk_role_set(struct mstp_lnk *lnk, enum mstp_lnk_role role)
{
	struct mstp_lnk_mgmt * mgmt = &lnk->mgmt;
	int ret = 0;
	int self = thinkos_thread_self();

	if (role != lnk->role_req) {

		lnk->up_req = false;

		lnk->role_req = role;
		if (self == lnk->thread)
			return 0;

		while (role != lnk->role_ack) {
			if ((ret = thinkos_flag_timedtake(mgmt->flag, 2000)) < 0) {
				DBGS("management flag timeout");
				break;
			}
		}
	}
	return ret;
}

int mstp_lnk_up_timedwait(struct mstp_lnk *lnk, unsigned int tmo_ms)
{
	struct mstp_lnk_mgmt * mgmt = &lnk->mgmt;
	int ret = 0;

	while (!lnk->up_ack) {

		if ((ret = thinkos_flag_timedtake(mgmt->flag, tmo_ms)) < 0) {
			DBGS("management flag timeout");
			break;
		}
	}

	return ret;
}

