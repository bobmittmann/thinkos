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
 * @file mstp_lnk_debug.c
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

const char * const state_nm[] = {
	[MSTP_IDLE] = "IDLE",
	[MSTP_NO_TOKEN] = "NO_TOKEN",
	[MSTP_POLL_FOR_MASTER] = "POLL_FOR_MASTER",
	[MSTP_PASS_TOKEN] = "PASS_TOKEN",
	[MSTP_USE_TOKEN] = "USE_TOKEN",
	[MSTP_DONE_WITH_TOKEN] = "DONE_WITH_TOKEN",

	[MSTP_WAIT_FOR_REPLY] = "WAIT_FOR_REPLY",
	[MSTP_WAIT_FOR_TEST_REPLY] = "WAIT_FOR_TEST_REPLY",
	[MSTP_ANSWER_DATA_REQUEST] = "ANSWER_DATA_REQUEST",
	[MSTP_ANSWER_TEST_REQUEST] = "MSTP_ANSWER_TEST_REQUEST",
	[MSTP_INITIALIZE] = "INITIALIZE",
};

void mstp_dbg_io_0_set(void)
{
	stm32_gpio_set(PIN0);
}

void mstp_dbg_io_0_clr(void)
{
	stm32_gpio_clr(PIN0); 
}

void mstp_dbg_io_0_toggle(void) {
	if (stm32_gpio_stat(PIN0)) stm32_gpio_clr(PIN0); 
	else stm32_gpio_set(PIN0);
}

void mstp_dbg_io_1_toggle(void) {
	if (stm32_gpio_stat(PIN1)) stm32_gpio_clr(PIN1); 
	else stm32_gpio_set(PIN1);
}

void mstp_dbg_io_2_toggle(void) {
	if (stm32_gpio_stat(PIN2)) stm32_gpio_clr(PIN2); 
	else stm32_gpio_set(PIN2);
}

void mstp_dbg_io_3_toggle(void) 
{
	if (stm32_gpio_stat(PIN3)) stm32_gpio_clr(PIN3); 
	else stm32_gpio_set(PIN3);
}

void mstp_dbg_io_4_toggle(void) 
{
	if (stm32_gpio_stat(PIN4)) stm32_gpio_clr(PIN4); 
	else stm32_gpio_set(PIN4);
}

void mstp_lnk_debug_init(void)
{
	stm32_gpio_clr(PIN0);
	stm32_gpio_mode(PIN0, OUTPUT, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_clr(PIN1);
	stm32_gpio_mode(PIN1, OUTPUT, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_clr(PIN2);
	stm32_gpio_mode(PIN2, OUTPUT, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_clr(PIN3);
	stm32_gpio_mode(PIN3, OUTPUT, PUSH_PULL | SPEED_HIGH);

	stm32_gpio_clr(PIN4);
	stm32_gpio_mode(PIN4, OUTPUT, PUSH_PULL | SPEED_HIGH);
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

void mstp_lnk_dbg_state(struct mstp_lnk *lnk, unsigned int nst)
{
	unsigned int pst = lnk->state;

	switch (pst) {

	case MSTP_IDLE:
		lnk->state = MSTP_IDLE;
		DBGS("[IDLE] ReceivedPFM --> [IDLE]");

		lnk->state = MSTP_ANSWER_TEST_REQUEST;
		INFS("[IDLE] BeceivedBcastMgmtMsg --> [IDLE]");

		lnk->state = MSTP_IDLE;
		DBGS("[IDLE] ReceivedDataNoReply --> [IDLE]");

		lnk->state = MSTP_IDLE;
		DBGS("[IDLE] ReceivedUnwantedFrame --> [IDLE]");

		lnk->state = MSTP_IDLE;
		DBGS("[IDLE] NotForMe --> [IDLE]");

		lnk->state = MSTP_IDLE;
		DBGS("[IDLE] ReceivedInvalidFrame --> [IDLE]");

		lnk->state = MSTP_USE_TOKEN;
		YAPS("[IDLE] ReceivedToken --> [USE_TOKEN]");

		lnk->state = MSTP_ANSWER_TEST_REQUEST;
		INFS("[IDLE] ReceivedDataNeedingReply --> [ANSWER_TEST_REQUEST]");

		lnk->state = MSTP_ANSWER_DATA_REQUEST;
		INFS("[IDLE] ReceivedDataNeedingReply --> [ANSWER_DATA_REQUEST]");

		lnk->state = MSTP_NO_TOKEN;
		DBGS("[IDLE] LostToken --> [NO_TOKEN]");
		break;

	case MSTP_USE_TOKEN:
		lnk->state = MSTP_DONE_WITH_TOKEN;
		YAPS("[USE_TOKEN] NothingToSend --> [DONE_WITH_TOKEN]");

		lnk->state = MSTP_DONE_WITH_TOKEN;
		DBGS("[USE_TOKEN] SendNoWait --> [DONE_WITH_TOKEN]");

		lnk->state = MSTP_WAIT_FOR_REPLY;
		DBGS("[USE_TOKEN] SendAndWait --> [WAIT_FOR_REPLY]");
		break;

		lnk->state = MSTP_DONE_WITH_TOKEN;
		DBGS("[USE_TOKEN] SendNoWait --> [DONE_WITH_TOKEN]");

		lnk->state = MSTP_WAIT_FOR_TEST_REPLY;
		DBGS("[USE_TOKEN] SendProbeAndWait --> [WAIT_FOR_TEST_REPLY]");

		lnk->state = MSTP_WAIT_FOR_TEST_REPLY;
		DBGS("[USE_TOKEN] SendTestAndWait --> [WAIT_FOR_TEST_REPLY]");

		lnk->state = MSTP_DONE_WITH_TOKEN;
		DBGS("[USE_TOKEN] SendNoWait --> [DONE_WITH_TOKEN]");
		break;

	case MSTP_WAIT_FOR_REPLY:
		lnk->state = MSTP_DONE_WITH_TOKEN;
		INFS("[WAIT_FOR_REPLY] ReplyTimeout --> [DONE_WITH_TOKEN]");

		lnk->state = MSTP_DONE_WITH_TOKEN;
		INFS("[WAIT_FOR_REPLY] InvalidFrame --> [DONE_WITH_TOKEN]");

		lnk->state = MSTP_DONE_WITH_TOKEN;
		DBGS("[WAIT_FOR_REPLY] ReceivedPostponed --> [DONE_WITH_TOKEN]");

		lnk->state = MSTP_DONE_WITH_TOKEN;
		DBGS("[WAIT_FOR_REPLY] ReceivedReply --> [DONE_WITH_TOKEN]");

		lnk->state = MSTP_DONE_WITH_TOKEN;
		DBGS("[WAIT_FOR_REPLY] ReceivedReply --> [DONE_WITH_TOKEN]");

		lnk->state = MSTP_IDLE;
		INF("[WAIT_FOR_REPLY] ReceivedUnexpectedFrame --> [IDLE]");

		lnk->state = MSTP_IDLE;
		DBGS("[WAIT_FOR_REPLY] ReceivedUnexpectedFrame --> [IDLE]");
		break;

	case MSTP_WAIT_FOR_TEST_REPLY:
		lnk->state = MSTP_DONE_WITH_TOKEN;
		DBGS("[WAIT_FOR_TEST_REPLY] TestTimeout --> [DONE_WITH_TOKEN]");

		lnk->state = MSTP_DONE_WITH_TOKEN;
		WARNS("[WAIT_FOR_TEST_REPLY] InvalidFrame --> [DONE_WITH_TOKEN]");

		lnk->state = MSTP_DONE_WITH_TOKEN;
		DBGS("[WAIT_FOR_TEST_REPLY] ReceivedTestResponse --> [DONE_WITH_TOKEN]");
		break;

	case MSTP_DONE_WITH_TOKEN:
		lnk->state = MSTP_USE_TOKEN;
		INFS("[DONE_WITH_TOKEN] SendAnotherFrame --> [USE_TOKEN]");

		lnk->state = MSTP_POLL_FOR_MASTER;
		DBG("[DONE_WITH_TOKEN] NextStationUnknown --> [POLL_FOR_MASTER]");

		lnk->state = MSTP_USE_TOKEN;
		DBGS("[DONE_WITH_TOKEN] SoleMaster --> [USE_TOKEN]");

		lnk->state = MSTP_PASS_TOKEN;
		DBG("[DONE_WITH_TOKEN] SendToken --> [PASS_TOKEN]");

		lnk->state = MSTP_POLL_FOR_MASTER;
		DBG("[DONE_WITH_TOKEN] SoleMasterRestartMaintenancePFM --> [POLL_FOR_MASTER]");

		lnk->state = MSTP_PASS_TOKEN;
		DBG("[DONE_WITH_TOKEN] ResetMaintenancePFM --> [PASS_TOKEN]");

		lnk->state = MSTP_POLL_FOR_MASTER;
		DBG("[DONE_WITH_TOKEN] SendMaintenancePFM --> [POLL_FOR_MASTER]");
		break;

	case MSTP_PASS_TOKEN:
		lnk->state = MSTP_POLL_FOR_MASTER;
		INF("[PASS_TOKEN] FindNewSuccessor --> [POLL_FOR_MASTER]");

		lnk->state = MSTP_IDLE;
		MSTP_HW_STATE(lnk->state);
		YAPS("[PASS_TOKEN] SawTokenUser --> [IDLE]");

		lnk->state = MSTP_PASS_TOKEN;
		DBG("[PASS_TOKEN] RetrySendToken --> [PASS_TOKEN]");

		lnk->state = MSTP_POLL_FOR_MASTER;
		INF("[PASS_TOKEN] FindNewSuccessor --> [POLL_FOR_MASTER]");
		break;

	case MSTP_NO_TOKEN:
		lnk->state = MSTP_IDLE;
		DBGS("[NO_TOKEN] SawFrame 1--> [IDLE]");

		lnk->state = MSTP_IDLE;
		INFS("[NO_TOKEN] SawFrame 2--> [IDLE]");

		lnk->state = MSTP_POLL_FOR_MASTER;
		DBG("[NO_TOKEN] GenerateToken --> [POLL_FOR_MASTER]");

		break;

	case MSTP_POLL_FOR_MASTER:
		lnk->state = MSTP_PASS_TOKEN;
		DBGS("[POLL_FOR_MASTER] ReceivedReplyToPFM --> [PASS_TOKEN]");

		lnk->state = MSTP_IDLE;
		DBGS("[POLL_FOR_MASTER] ReceivedUnexpectedFrame --> [IDLE]");

		lnk->state = MSTP_USE_TOKEN;
		DBGS("[POLL_FOR_MASTER] SoleMaster --> [USE_TOKEN]");

		lnk->state = MSTP_PASS_TOKEN;
		DBGS("[POLL_FOR_MASTER] DoneWithPFM --> [PASS_TOKEN]");

		lnk->state = MSTP_USE_TOKEN;
		INFS("[POLL_FOR_MASTER] DeclareSoleMaster --> [USE_TOKEN]");
		break;


	case MSTP_ANSWER_DATA_REQUEST:
		lnk->state = MSTP_IDLE;
		INFS("[ANSWER_DATA_REQUEST] Reply --> IDLE[]");

		lnk->state = MSTP_IDLE;
		INFS("[ANSWER_DATA_REQUEST] DeferredReply --> [IDLE]");
		break;

	case MSTP_ANSWER_TEST_REQUEST:
		lnk->state = MSTP_IDLE;
		INFS("[ANSWER_TEST_REQUEST] TestReply --> [IDLE]");

		lnk->state = MSTP_IDLE;
		INFS("[ANSWER_TEST_REQUEST] DeferredTestReply --> [IDLE]");
		break;
	}
}

void mstp_lnk_debug_dump(void)
{
	INF("MS/TP:   MSTP_LNK_MAX_NODES = %d", (uint32_t)MSTP_LNK_MAX_NODES);
	INF("MS/TP:        MSTP_BAUDRATE = %d bps", (uint32_t)MSTP_BAUDRATE);
	INF("MS/TP:        MSTP_LNK_MTU  = %d octets", (uint32_t)MSTP_LNK_MTU);
	INF("MS/TP:        MSTP_OVERHEAD = %d octets", (uint32_t)MSTP_OVERHEAD);

	INF("MS/TP:             T_BIT_NS = %d ns", (uint32_t)T_BIT_NS);
	INF("MS/TP: T_PROPAGATION_MAX_NS = %d ns", (uint32_t)T_PROPAGATION_MAX_NS);
	INF("MS/TP: T_CPU_LATENCY_MIN_NS = %d ns", (uint32_t)T_CPU_LATENCY_MIN_NS);
	INF("MS/TP: T_CPU_LATENCY_MAX_NS = %d ns", (uint32_t)T_CPU_LATENCY_MAX_NS);
	INF("MS/TP:      T_ROUNDTRIP_MIN = %d ms", (uint32_t)T_ROUNDTRIP_MIN);
	INF("MS/TP:      T_ROUNDTRIP_MAX = %d ms", (uint32_t)T_ROUNDTRIP_MAX);
	INF("MS/TP:           T_NO_TOKEN = %d ms", (uint32_t)T_NO_TOKEN);
	INF("MS/TP:        T_REPLY_DELAY = %d ms", (uint32_t)T_REPLY_DELAY);
	INF("MS/TP:      T_REPLY_TIMEOUT = %d ms", (uint32_t)T_REPLY_TIMEOUT);
	INF("MS/TP:   T_LOOPBACK_TIMEOUT = %d ms", (uint32_t)T_LOOPBACK_TIMEOUT);
	INF("MS/TP:               T_SLOT = %d ms", (uint32_t)T_SLOT);
	INF("MS/TP:        T_USAGE_DELAY = %d ms", (uint32_t)T_USAGE_DELAY);
	INF("MS/TP:      T_USAGE_TIMEOUT = %d ms", (uint32_t)T_USAGE_TIMEOUT);
	INF("MS/TP:    T_MIN_OCT_TIMEOUT = %d ms", (uint32_t)T_MIN_OCT_TIMEOUT);
	INF("MS/TP:        T_PFM_TIMEOUT = %d ms", (uint32_t)T_PFM_TIMEOUT);
	INF("MS/TP:  T_SEND_WAIT_TIMEOUT = %d ms", (uint32_t)T_SEND_WAIT_TIMEOUT);
}

