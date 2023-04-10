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
 * @file mstp_lnk_task.c
 * @brief MS/TP Link Layer
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

_Pragma("GCC optimize (\"Ofast\")")


#include "mstp_lnk-i.h"

#undef TRACE_LEVEL
#define TRACE_LEVEL TRACE_LVL_DBG
#include <trace.h>

const char * const state_nm[] = {
	[MSTP_INITIALIZE] = "INITIALIZE",
	[MSTP_IDLE] = "IDLE",
	[MSTP_ANSWER_DATA_REQUEST] = "ANSWER_DATA_REQUEST",
	[MSTP_NO_TOKEN] = "NO_TOKEN",
	[MSTP_POLL_FOR_MASTER] = "POLL_FOR_MASTER",
	[MSTP_PASS_TOKEN] = "PASS_TOKEN",
	[MSTP_USE_TOKEN] = "USE_TOKEN",
	[MSTP_DONE_WITH_TOKEN] = "DONE_WITH_TOKEN",
	[MSTP_WAIT_FOR_REPLY] = "WAIT_FOR_REPLY",
	[MSTP_WAIT_FOR_TEST_REPLY] = "WAIT_FOR_TEST_REPLY"
};

/* -------------------------------------------------------------------------
 * MS/TP 
 * ------------------------------------------------------------------------- */

//void __attribute__ ((noreturn)) mstp_lnk_task(struct mstp_lnk *lnk)
int mstp_lnk_task(struct mstp_lnk *lnk)
{
	int role;

	INF("MS/TP task started...");

	while (lnk->alive) {

		role = mstp_lnk_role_get(lnk);

		switch (role) {
		case MSTP_LNK_SLAVE: 
			DBGS("MS/TP slave role...");
			mstp_lnk_slave_loop(lnk);
			break;

		case MSTP_LNK_MASTER: 
			DBGS("MS/TP master role...");
			mstp_lnk_master_loop(lnk);
			break;

		case MSTP_LNK_SNIFFER: 
			DBGS("MS/TP sniffer role...");
			mstp_lnk_sniffer_loop(lnk);
			break;
		};
	}

	return 0;
}

//uint32_t mstp_stack[512] __attribute__ ((aligned(8), section(".stack")));
uint32_t mstp_stack[512] __attribute__ ((aligned(16)));

const struct thinkos_thread_inf mstp_lnk_thread_inf = {
	.stack_ptr = mstp_stack,
	.stack_size = sizeof(mstp_stack),
	.priority = 4,
	.thread_id = 4,
	.paused = 0,
	.tag = "MS/TP"
};

int mstp_lnk_init(struct mstp_lnk *lnk, const char *tag,
				  unsigned int addr, enum mstp_lnk_role role, 
				  struct mstp_lnk_comm * comm)
{
	int ret;

	if (lnk == NULL)
		return -EINVAL;

	if ((role == MSTP_LNK_MASTER) && (addr > N_MAX_MASTER))
		return -EINVAL;

	if (lnk->thread != 0)  {
		/* Already initialized */
		return -EINVAL;
	}

	if (comm == NULL)  {
		/* Already initialized */
		return -EINVAL;
	}

#if MSTP_HW_TRACE_ENABLED
	/* outputs the state of the MSTP state machine to GPIO pins - for
	 * debugging purposes */
	mstp_trace_init();
#endif


	lnk->comm = comm;
	mstp_lnk_comm_init(comm, MSTP_BAUDRATE, MSTP_LNK_TX_IDLE_BITS, addr);

	lnk->this_station = addr;
#if (MSTP_LNK_HALF_DUPLEX)
	lnk->half_duplex = true;
#endif

	INF("MS/TP: MSTP_BAUDRATE=%d", MSTP_BAUDRATE);
	INF("MS/TP: T_NO_TOKEN=%d", T_NO_TOKEN);
	INF("MS/TP: T_REPLY_DELAY=%d", T_REPLY_DELAY);
	INF("MS/TP: T_REPLY_TIMEOUT=%d", T_REPLY_TIMEOUT);
	INF("MS/TP: T_LOOPBACK_TIMEOUT=%d", (uint32_t)T_LOOPBACK_TIMEOUT);
	INF("MS/TP: T_SLOT=%d", (uint32_t)T_SLOT);
	INF("MS/TP: T_USAGE_DELAY=%d", (uint32_t)T_USAGE_DELAY);
	INF("MS/TP: T_USAGE_TIMEOUT=%d", (uint32_t)T_USAGE_TIMEOUT);
	INF("MS/TP: T_MIN_OCT_TIMEOUT=%d", (uint32_t)T_MIN_OCT_TIMEOUT);
	INF("MS/TP: T_PFM_TIMEOUT=%d", (uint32_t)T_PFM_TIMEOUT);
	INF("MS/TP: T_SEND_WAIT_TIMEOUT=%d", (uint32_t)T_SEND_WAIT_TIMEOUT);
	DBG("MS/TP: CPU latency = %d.. %d", 
		(uint32_t)T_CPU_LATENCY_MIN_NS, (uint32_t)T_CPU_LATENCY_MAX_NS);
	DBG("MS/TP: roundtrip = %d.. %d", 
		(uint32_t)T_ROUNDTRIP_MIN, (uint32_t)T_ROUNDTRIP_MAX);
					

	lnk->rx.sem = thinkos_sem_alloc(0);
	lnk->tx.gate = thinkos_gate_alloc();
	lnk->mutex = thinkos_mutex_alloc();
	lnk->tag = tag;
	lnk->role = role;
	lnk->state = MSTP_INITIALIZE;
	lnk->enabled = false;

	/* set the management callback function to default  */
	lnk->mgmt.callback = mstp_lnk_default_callback;
	/* include this node's address in the address map array */
	__netmap_reset(lnk->mgmt.netmap, addr);

	lnk->alive = true;
	ret = thinkos_thread_create_inf(C_TASK(mstp_lnk_task), (void *)lnk, 
									&mstp_lnk_thread_inf);

	if (ret > 0) { 
		lnk->thread = ret;
	} 

	return 0;
}

int mstp_lnk_done(struct mstp_lnk * lnk)
{
	if (lnk->thread == 0) {
		return -1;
	}

	lnk->alive = false;
	lnk->role = MSTP_LNK_NONE;

	thinkos_join(lnk->thread);
	lnk->thread = 0;
	
	thinkos_mutex_free(lnk->mutex);
	thinkos_sem_free(lnk->rx.sem);
	thinkos_gate_free(lnk->tx.gate);

	return 0;
}

int mstp_lnk_addr_set(struct mstp_lnk *lnk, unsigned int addr)
{
	if (lnk == NULL)
		return -EINVAL;

	if (lnk->state != MSTP_INITIALIZE)
		return -EINVAL;

	INF("MS/TP: mstp_lnk_addr_set=%d", addr);

	if (lnk->this_station != addr) {
		__netmap_reset(lnk->mgmt.netmap, addr);
		lnk->this_station = addr;
	}

	return 0;
}

int mstp_lnk_resume(struct mstp_lnk *lnk)
{
	if (lnk == NULL)
		return -EINVAL;

	if (lnk->state != MSTP_INITIALIZE)
		return -EINVAL;

	INFS("Enabling BACnet MS/TP Data Link");

	lnk->enabled = true;

	return 0;
}

int mstp_lnk_stop(struct mstp_lnk *lnk)
{
	if (lnk == NULL)
		return -EINVAL;

	if (lnk->state == MSTP_INITIALIZE)
		return -EINVAL;

	DBGS("Pausing BACnet MS/TP Data Link");
	lnk->enabled = false;

	return 0;
}

int mstp_lnk_stats_get(struct mstp_lnk *lnk, struct mstp_lnk_stats * stats,
		     bool reset)
{
	if (lnk == NULL) {
		return -EINVAL;
	}

	if (stats != NULL) {
		*stats = lnk->stats;
	}

	if (reset) {
		INFS("\n---- Restarting all the stats ----\n");
		lnk->stats.rx_err = 0;
		lnk->stats.rx_token = 0;
		lnk->stats.rx_mgmt = 0;
		lnk->stats.rx_unicast = 0;
		lnk->stats.rx_bcast = 0;
		lnk->stats.tx_token = 0;
		lnk->stats.tx_mgmt = 0;
		lnk->stats.tx_pfm = 0;
		lnk->stats.tx_unicast = 0;
		lnk->stats.tx_bcast = 0;
		lnk->stats.token_lost = 0;
	}

	return 0;
}

int mstp_lnk_half_duplex_set(struct mstp_lnk *lnk, bool on)
{
	int ret = lnk->half_duplex;

	lnk->half_duplex = on;

	return ret;
}

int mstp_lnk_addr_get(struct mstp_lnk *lnk)
{
	return lnk->this_station;
}

int mstp_lnk_role_get(struct mstp_lnk *lnk)
{
	return lnk->role;
}

int mstp_lnk_role_set(struct mstp_lnk *lnk, enum mstp_lnk_role role)
{
	int prev = lnk->role;

	lnk->role = role;

	return prev;
}


/* -------------------------------------------------------------------------
 *
 * Pool of resources
 * ------------------------------------------------------------------------- */

static struct mstp_lnk mstp_lnk_pool[MSTP_LNK_POOL_SIZE]; 

struct mstp_lnk * mstp_lnk_alloc(void)
{
	struct mstp_lnk * lnk;
	int i;

	for (i = 0; i < MSTP_LNK_POOL_SIZE; ++i) { 
		lnk = &mstp_lnk_pool[i];
		if (!lnk->pool_alloc) {
			memset(lnk, 0, sizeof(struct mstp_lnk));
			lnk->pool_alloc = true;
			return lnk;
		}
	}

	return NULL;
}


#if MSTP_HW_TRACE_ENABLED
/* -------------------------------------------------------------------------
 * States of the MSTP state machine
 * ------------------------------------------------------------------------- */

#define PIN0 STM32_GPIOA, 13
#define PIN1 STM32_GPIOA, 15
#define PIN2 STM32_GPIOB, 3
#define PIN3 STM32_GPIOA, 14
#define PIN4 STM32_GPIOA, 5

void mstp_trace_state(int state)
{
	stm32_gpio_clr(PIN4);

	stm32_gpio_clr(PIN0);
	stm32_gpio_clr(PIN1);
	stm32_gpio_clr(PIN2);
	stm32_gpio_clr(PIN3);

	if (state & (1 << 0)) 
		stm32_gpio_set(PIN0);
	if (state & (1 << 1)) 
		stm32_gpio_set(PIN1);
	if (state & (1 << 2)) 
		stm32_gpio_set(PIN2);
	if (state & (1 << 3)) 
		stm32_gpio_set(PIN3);

	stm32_gpio_set(PIN4);

	udelay(20);
}

void mstp_trace_init(void)
{
	stm32_gpio_clr(PIN0);
	stm32_gpio_mode(PIN0, OUTPUT, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_clr(PIN1);
	stm32_gpio_mode(PIN1, OUTPUT, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_clr(PIN2);
	stm32_gpio_mode(PIN2, OUTPUT, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_clr(PIN3);
	stm32_gpio_mode(PIN3, OUTPUT, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_set(PIN4);
	stm32_gpio_mode(PIN4, OUTPUT, PUSH_PULL | SPEED_HIGH);
}

#endif

int mstp_lnk_recv(struct mstp_lnk *lnk, void *buf, unsigned int max,
				  struct mstp_frame_inf *inf)
{
	int len;
	int ret;

	if ((ret = thinkos_sem_timedwait(lnk->rx.sem, 
									 T_RECV_WAIT_TIMEOUT)) < 0) {
		if (ret == THINKOS_ETIMEDOUT) {
			DBGS("MSTPLnkRcv: thinkos_sem_timedwait() timeout!");
			return -ETIMEDOUT;
		} else {
			ERRS("MSTPLnkRcv: thinkos_sem_wait() error!");
			abort();
		}
	}

	len = MIN(lnk->rx.pdu_len, max);
	*inf = lnk->rx.inf;
	memcpy(buf, lnk->rx.pdu, len);
	lnk->rx.ack++;

	YAP("MSTPLnkRcv: %d", len);
	return len;
}

/* XXX: this call will block */
int mstp_lnk_send(struct mstp_lnk *lnk, const void *buf, 
				  unsigned int cnt, const struct mstp_frame_inf *inf)
{
	uint32_t seq;

	if (cnt > MSTP_LNK_PDU_MAX) {
		ERR("too large:  %d > %d", cnt, MSTP_LNK_PDU_MAX);
		return -EINVAL;
	}

	if ((inf->type != FRM_BACNET_DATA_XPCT_REPLY) &&
	    (inf->type != FRM_BACNET_DATA_NO_REPLY) &&
	    (inf->type != FRM_TEST_REQUEST) &&
	    (inf->type != FRM_TEST_RESPONSE) &&
	    (inf->type < FRM_DATA_XPCT_REPLY)) {
		ERR("frame type: %d", inf->type);
		return -EINVAL;
	}

	if (thinkos_gate_timedwait(lnk->tx.gate, T_SEND_WAIT_TIMEOUT) < 0) {
		ERR("timeout");
		return -ETIMEDOUT;
	}	

	seq = lnk->tx.seq;
	if (seq != lnk->tx.ack) {
		ERR("MSTPLnkSnd: seq=%d ack=%d", seq, lnk->tx.ack);
		thinkos_gate_exit(lnk->tx.gate, 1);
		return -EAGAIN;
	}

	if (cnt) {
		lnk->tx.pdu_len = cnt;
		memcpy(lnk->tx.pdu, buf, cnt);
	}
	lnk->tx.inf = *inf;
	lnk->tx.seq = ++seq;

	INF("MSTPLnkSnd(%d): %d", seq, cnt);

	thinkos_gate_exit(lnk->tx.gate, 0);

	return cnt;
}
