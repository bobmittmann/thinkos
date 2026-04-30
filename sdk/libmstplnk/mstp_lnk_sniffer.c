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

void mstp_lnk_sniffer_loop(struct mstp_lnk *lnk)
{
	struct mstp_lnk_comm * comm = lnk->comm;
	uint8_t pdu[MSTP_LNK_MTU];
	struct mstp_frm_ref frm;
	unsigned int frm_type;
	unsigned int src_addr;
	unsigned int dst_addr;
	unsigned int pdu_len;
	int32_t dt;
	int ret;

	while (lnk->alive) {
		dt = 10;

		ret = mstp_lnk_comm_frame_recv(comm, pdu, &frm, dt);
		if (ret > 0) {

			frm_type = frm.frm_type;
			dst_addr = frm.dst_addr;
			src_addr = frm.src_addr;
			pdu_len = frm.pdu_len;

			switch (frm_type) {
			case FRM_TOKEN:
				lnk->stats.rx_token++;
				break;
			case FRM_TEST_REQUEST:
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

		} else {
			if (ret != MSTP_LNK_TIMEOUT) {
			}
		}
	}
}

