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
 * @file mstp_lnk-i.h
 * @brief MST/TP link layer private header
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#ifndef __MSTP_LNK_I_H__
#define __MSTP_LNK_I_H__

#ifdef CONFIG_H
#include "config.h"
#endif

#include <bacnet/mstp_lnk.h>

#include <stdbool.h>
#include <thinkos.h>
#include <errno.h>

#include "uart_dma.h"

#ifndef MSTP_LNK_POOL_SIZE
#define MSTP_LNK_POOL_SIZE 1
#endif

/* ???: Addresses start from 0 so maximum number of masters is equal to
 * MSTP_LNK_MAX_MASTERS + 1
 */
#ifndef MSTP_LNK_MAX_MASTERS
#define MSTP_LNK_MAX_MASTERS 10	/* addresses 0 to 9 */
#endif

#ifndef MSTP_LNK_MAX_NODES
#define MSTP_LNK_MAX_NODES MSTP_LNK_MAX_MASTERS
#endif

/* Pulse GPIO to trigger oscilloscope capture */
#ifndef	MSTP_SCOPE_TRIGGER_ENABLED
#define MSTP_SCOPE_TRIGGER_ENABLED 0
#endif

#ifndef mstp_link_hdr_err_max 
#define MSTP_LINK_HDR_ERR_MAX 32
#endif

//#define MSTP_LNK_MTU       511
#define MSTP_LNK_MTU       127
#define MSTP_LNK_PDU_MAX   (MSTP_LNK_MTU - 11)
#define MSTP_OVERHEAD		10
#define MSTP_TOKEN_SIZE		8

/* definitions that depend on bit time - milliseconds */
//#define MSTP_BAUDRATE		 88200
//#define MSTP_BAUDRATE		 176400
//#define MSTP_BAUDRATE		 352800
//#define MSTP_BAUDRATE		 705600
#define MSTP_BAUDRATE		 500000

#define MSTP_LNK_MGMT_PDU_MIN 6
#define MSTP_LNK_MGMT_PDU_MAX 54
#define MSTP_LNK_MGMT_DATA_MAX ((MSTP_LNK_MGMT_PDU_MAX)  - 4)

struct uart_dma_op;

struct mstp_lnk_mgmt {
	int (*callback) (struct mstp_lnk * lnk, unsigned int ev);
	uint32_t netmap[(MSTP_LNK_MAX_MASTERS + 31) / 32];
	uint32_t active[MSTP_LNK_MAX_MASTERS];
	uint8_t loop_probe;
	uint8_t flag;

	struct {
		uint8_t req;  /* Test request code */
		uint8_t rsp;  /* Test response code */
		uint8_t ret;  /* Test return value */
		uint16_t seq; /* Test sequence number */
	} tst;

	struct {
		volatile uint32_t seq;
		volatile uint32_t ack;
		uint8_t ftype;
		uint8_t saddr;
		uint8_t daddr;
		uint8_t pdu_len;
		uint8_t pdu[MSTP_LNK_MGMT_PDU_MAX];
	} rx;

	struct {
		volatile uint32_t seq;
		volatile uint32_t ack;
		uint8_t ftype;
		uint8_t saddr;
		uint8_t daddr;
		uint8_t pdu_len;
		uint8_t pdu[MSTP_LNK_MGMT_PDU_MAX];
	} tx;

};

struct mstp_lnk {
	uint8_t thread;
	volatile uint8_t alive;

	uint8_t mutex;
	uint8_t sole_master;

	volatile uint8_t prev_state;
	volatile uint8_t state;
	uint8_t this_station;	/* this station address */
	uint8_t next_station;	/* next station address */

	volatile uint8_t role_req;
	volatile uint8_t role_ack;

	volatile uint8_t up_req;
	volatile uint8_t up_ack;

	const char * tag;
	struct mstp_lnk_comm * comm;


	struct {
		/* PDU */
		volatile uint32_t seq;
		volatile uint32_t ack;
		struct mstp_frame_inf inf;
		int16_t sem;
		uint16_t pdu_len;
		uint8_t pdu[MSTP_LNK_PDU_MAX];
	} rx;

	struct {
		/* PDU */
		volatile uint32_t seq;
		volatile uint32_t ack;
		struct mstp_frame_inf inf;
		int16_t gate;
		uint16_t pdu_len;
		uint8_t pdu[MSTP_LNK_PDU_MAX];
	} tx;

	struct {
		struct {
			volatile uint32_t seq;
			volatile uint32_t ack;
		} probe;
		volatile uint8_t status; /* class A status */
		volatile uint8_t prev;
		uint8_t sup_flag;
		uint32_t interval;
	} class_a;

	struct mstp_lnk_stats stats;

	struct mstp_lnk_mgmt mgmt;

	struct {
		uint32_t state_cnt[11];
		uint32_t event_cnt[36];
	} dbg;
};

/* MSTP Connection State Machine states */
enum mstp_state {
	MSTP_IDLE                = 0,
	MSTP_NO_TOKEN            = 1,
	MSTP_POLL_FOR_MASTER     = 2,
	MSTP_PASS_TOKEN          = 3,
	MSTP_USE_TOKEN           = 4,
	MSTP_DONE_WITH_TOKEN     = 5,

	MSTP_ANSWER_DATA_REQUEST = 6,
	MSTP_WAIT_FOR_REPLY      = 7,
	MSTP_ANSWER_TEST_REQUEST = 8,
	MSTP_WAIT_FOR_TEST_REPLY = 9,
	MSTP_INITIALIZE          = 10
} ;

extern const uint8_t mstp_crc8_lut[256];

static inline unsigned int __mstp_crc8(unsigned int crc, int c)
{
	return mstp_crc8_lut[crc ^ c];
}

#if MSTP_SCOPE_TRIGGER_ENABLED
static inline void mstp_trace_trigger(void)
{
	// PA3 clear
	stm32_gpio_clr(IO_PA3);
	// PA3 set
	stm32_gpio_set(IO_PA3);
}
#endif

static inline void __netmap_insert(uint32_t bmp[], unsigned int __i) {
	bmp[__i / 32] |= (1 << (__i % 32));
}

static inline void __netmap_reset(uint32_t bmp[], unsigned int __ts) {
		bmp[0] = (1 << __ts);
#if MSTP_LNK_MAX_MASTERS > 32
		__ts -= 32;
		bmp[1] = (1 << __ts);
#if MSTP_LNK_MAX_MASTERS > 64
		__ts -= 32;
		bmp[2] = (1 << __ts);
#if MSTP_LNK_MAX_MASTERS > 96
		__ts -= 32;
		bmp[3] = (1 << __ts);
#endif
#endif
#endif
}

static inline void __list_reset(uint32_t __lst[], unsigned int __len) 
{
	int i;

	for (i = 0; i < __len; ++i) 
		__lst[i] = 0;
}

struct mstp_frm_ref {
	uint8_t frm_type;
	uint8_t dst_addr;
	uint8_t src_addr;
	uint16_t pdu_len;
};

#define MSTP_RCVD_VALID_FRAME  0
#define MSTP_LNK_TIMEOUT      -1
#define MSTP_HDR_SYNC_ERROR   -2
#define MSTP_HDR_CRC_ERROR    -3
#define MSTP_DATA_CRC_ERROR   -4
#define MSTP_FRAME_LEN_ERROR  -5
#define MSTP_LINE_BREAK		  -6
#define MSTP_LNK_ERROR		  -7


/* Get the information about MSTP states */
#ifndef	MSTP_HW_TRACE_ENABLED
#define MSTP_HW_TRACE_ENABLED	0
#endif

#if MSTP_HW_TRACE_ENABLED
  #define MSTP_HW_STATE(__STATE) mstp_trace_state(__STATE)
#else
  #define MSTP_HW_STATE(__STATE)
#endif

/* Number of bits between frames */
#define N_FRAME_GAP         40

/* Time bit in nanoseconds */
#define T_BIT_NS            (1000000000LL / MSTP_BAUDRATE)
/* Delay in the hub */
#define T_HUB_DELAY_NS      (T_BIT_NS * 2)

#define T_CPU_NS_PER_OCTET 1000LL
#define T_OCTETS_NS(__OCTETS__)  (T_BIT_NS * 10LL * (__OCTETS__))
#define T_OCTETS_MS(__OCTETS__)  ((T_OCTETS_NS(__OCTETS__) + \
										500000LL) / 1000000)
/* This latency represent the time it takes to process a frame,
 it comprises a fixed value and a payload dependant value. 
 This value is the sum of encode and decode times. */
#define T_CPU_MIN_NS       200000LL
#define T_CPU_NS_PER_OCTET 1000LL
#define T_CPU_LATENCY_NS(__OCTETS__)  (T_CPU_MIN_NS + \
									   T_CPU_NS_PER_OCTET * (__OCTETS__))

/* CPU latency in milliseconds */
#define T_CPU_LATENCY_MS(__OCTETS__)  ((T_CPU_LATENCY_NS(__OCTETS__) + \
										500000LL) / 1000000)

/* Minimum and maximum latencies in nanoseconds */
#define T_CPU_LATENCY_MIN_NS   T_CPU_LATENCY_NS(MSTP_OVERHEAD)
#define T_CPU_LATENCY_MAX_NS   T_CPU_LATENCY_NS(MSTP_LNK_MTU)

/* Worst case propagation time: 1200 meters cable and VP = 0.75 * C. 
 * C = 300000000 [m/s]
 * T_NS = 1000000000 *  L [m] / VP [m/s]
 * T_NS = L * 4.444444 
 * */
#define T_PROPAGATION_MAX_NS  5333

#define T_FRAME_OCTETS_NS(__OCTETS__) (((N_FRAME_GAP + (__OCTETS__) * 10LL) * \
	1000000000LL) / MSTP_BAUDRATE) 


#define T_FRAME_DELAY_MAX_NS(__OCTETS__)  (T_FRAME_OCTETS_NS(__OCTETS__) + \
										   T_CPU_LATENCY_NS(__OCTETS__) + \
										   (MSTP_LNK_MAX_NODES * \
											(T_PROPAGATION_MAX_NS + \
											 T_HUB_DELAY_NS)))

#define T_ROUNDTRIP_MS(__OCTETS__)  ((T_FRAME_DELAY_MAX_NS(__OCTETS__) + \
									  500000LL) / 1000000)

#define T_ROUNDTRIP_MIN T_ROUNDTRIP_MS(MSTP_OVERHEAD)
#define T_ROUNDTRIP_MAX T_ROUNDTRIP_MS(MSTP_LNK_MTU)
					

#define FRM_WAIT_FOR_TEST_REPLY    (FRM_DATA_XPCT_REPLY + 0x3f)

/* -------------------------------------------------------------------------
 * MS/TP Link Layer
 * ------------------------------------------------------------------------- */

#define N_MAX_INFO_FRAMES 1
#define N_MAX_MASTER      MSTP_LNK_MAX_MASTERS
/* Number of tokens between Maintenance PFM */
#define N_POLL            1024
#define N_RETRY_TOKEN     2
#define N_MIN_OCTETS      4

/* The minimum time without a DataAvailable or ReceiveError event within a 
   frame before a receiving node may discard the frame: 60 bit times. 
   (Implementations may use larger values for this timeout, not to exceed 
   100 milliseconds.) */
#define T_FRAME_ABORT_MS 2

/* The time without a DataAvailable or ReceiveError event before declaration 
   of loss of token: 500 milliseconds */
#define T_NO_TOKEN      25
/* The maximum time a node may wait after reception of a frame that expects
 * a reply before sending the first octet of a reply or Reply
 * Postponed frame: 250 milliseconds. */
#define T_REPLY_DELAY    12
/* The minimum time without a DataAvailable or ReceiveError event that a
 * node must wait for a station to begin replying to a confirmed
 * request: 255 milliseconds. (Implementations may use larger values for
 * this timeout, not to exceed 300 milliseconds.) */
#define T_REPLY_TIMEOUT  15
//#define T_REPLY_TIMEOUT     ((N_ROUNDTRIP_MAX * 1000) / (MSTP_BAUDRATE))

#define T_LOOPBACK_TIMEOUT  T_ROUNDTRIP_MS(64)

#define T_SLOT           (2 * T_ROUNDTRIP_MIN)
#define T_USAGE_DELAY    T_ROUNDTRIP_MIN
#define T_USAGE_TIMEOUT	 (T_ROUNDTRIP_MIN + T_ROUNDTRIP_MAX)
#define T_PFM_TIMEOUT    (2 * T_ROUNDTRIP_MIN)

/* the time it takes to send out 4 octets */
#define T_MIN_OCT_TIMEOUT T_ROUNDTRIP_MS(4) 

#define T_SEND_WAIT_TIMEOUT (MSTP_LNK_MAX_NODES * T_USAGE_TIMEOUT)
#define T_RECV_WAIT_TIMEOUT (MSTP_LNK_MAX_NODES * T_USAGE_TIMEOUT)

#define  T_MAINTENANCE_TIMEOUT 5000


#ifndef MSTP_LNK_HALF_DUPLEX
#define MSTP_LNK_HALF_DUPLEX 1
#endif

#ifndef MSTP_LNK_TX_IDLE_BITS
#define MSTP_LNK_TX_IDLE_BITS  N_FRAME_GAP
#endif


#define ROUTE_FRM_TYPE(_ROUTE) ((_ROUTE) & 0xff)
#define ROUTE_DST_ADDR(_ROUTE) (((_ROUTE) >> 8) & 0xff)
#define ROUTE_SRC_ADDR(_ROUTE) (((_ROUTE) >> 16) & 0xff)

#define MKROUTE(_TYP, _DST, _SRC) ((_TYP) + ((_DST) << 8) + ((_SRC) << 16))

extern const char * const state_nm[];


#ifndef	MSTP_COMM_DEBUG_ENABLED
#define MSTP_COMM_DEBUG_ENABLED 1
#endif


#if (MSTP_COMM_DEBUG_ENABLED)

#define PIN0 IO_1
#define PIN1 IO_2
#define PIN2 IO_SAI1_FS
#define PIN3 IO_SAI1_SD
#define PIN4 IO_SAI1_SCK

#define MSTP_COMM_DBG0_CLR() stm32_gpio_clr(PIN0)
#define MSTP_COMM_DBG0_SET() stm32_gpio_set(PIN0)

#define MSTP_COMM_DBG1_CLR() stm32_gpio_clr(PIN1)
#define MSTP_COMM_DBG1_SET() stm32_gpio_set(PIN1)

#else

#define MSTP_COMM_DBG0_CLR() do { } while (0)
#define MSTP_COMM_DBG0_SET() do { } while (0)
#define MSTP_COMM_DBG1_CLR() do { } while (0)
#define MSTP_COMM_DBG1_SET() do { } while (0)

#endif

#ifdef __cplusplus
extern "C" {
#endif

void mstp_trace_init(void);

void mstp_trace_state(int state);

unsigned int mstp_crc8(unsigned int crc, const void *buf, int len);

unsigned int mstp_crc16(unsigned int crc, const void *buf, int len);

void mstp_lnk_sniffer_loop(struct mstp_lnk *lnk);
void mstp_lnk_master_loop(struct mstp_lnk *lnk);
void mstp_lnk_slave_loop(struct mstp_lnk *lnk);

int mstp_lnk_comm_rx_en_set(struct mstp_lnk_comm *comm, bool enable);

int mstp_lnk_comm_idle_bits_set(struct mstp_lnk_comm *comm, unsigned int idle_bits);

int mstp_lnk_comm_pending_events(struct mstp_lnk_comm *comm);

int mstp_lnk_default_callback(struct mstp_lnk *lnk, unsigned int ev);

int mstp_lnk_comm_uart_bind(struct mstp_lnk_comm * comm, 
							const struct uart_dma_drv * drv);

/* COMM device driver instances */
struct mstp_lnk_comm * mstp_lnk_comm_instance_get(int oid);

int mstp_lnk_comm_init(struct mstp_lnk_comm *comm, unsigned int baudrate, 
					   unsigned int idle_bits, int ts);

int mstp_lnk_comm_frame_recv(struct mstp_lnk_comm *comm, void * dst, 
							 struct mstp_frm_ref *frm, unsigned int tmo);

int mstp_lnk_comm_frame_send(struct mstp_lnk_comm *comm, unsigned int route,
							 uint8_t * pdu, unsigned int len);

int mstp_lnk_comm_fast_send(struct mstp_lnk_comm *comm, unsigned int type,
							unsigned int daddr);

int mstp_lnk_comm_half_duplex_set(struct mstp_lnk_comm *comm, bool on);

int mstp_lnk_comm_stats_fmt(struct mstp_lnk_comm *comm, char * buf, size_t max);

void mstp_lnk_mgmt_reset(struct mstp_lnk_mgmt * mgmt);

void mstp_lnk_debug_init(void);
	
void mstp_dbg_io_0_set(void); 
void mstp_dbg_io_0_clr(void); 

void mstp_dbg_io_0_toggle(void); 
void mstp_dbg_io_1_toggle(void); 
void mstp_dbg_io_2_toggle(void); 
void mstp_dbg_io_3_toggle(void); 
void mstp_dbg_io_4_toggle(void); 

void mstp_debug_init(void);

#ifdef __cplusplus
}
#endif
#endif				/* __MSTP_LNK_I_H__ */

