/* 
 * File:	 /thinkos/comm.h
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011-2018 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __THINKOS_COMM_H__
#define __THINKOS_COMM_H__

#ifndef __THINKOS_COMM__
#error "Never use <thinkos/comm.h> directly; include <thinkos.h> instead."
#endif 

#ifndef __ASSEMBLER__

/* ----------------------------------------------------------------------------
 *  Communication interface
 * ----------------------------------------------------------------------------
 */

/* Status bits */
#define COMM_ST_BREAK_REQ (1 << 0)
#define COMM_ST_CONNECTED (1 << 1)

/* Control operations */
enum thinkos_comm_ctrl {
	COMM_CTRL_STATUS_GET = 0,
	COMM_CTRL_CONNECT = 1,
	COMM_CTRL_DISCONNECT = 2,
	COMM_CTRL_BREAK_ACK = 3
};

/* Signals */
enum thinkos_comm_signal {
	COMM_TX_FIFO = 0,
	COMM_TX_PEND = 1,
	COMM_RX_WAIT = 2
};

struct thinkos_comm;
struct thinkos_comm_drv;


struct thinkos_comm_drv_op {
	int (*open)(struct thinkos_comm_drv *);
	int (*send)(struct thinkos_comm_drv *, const void * buf, unsigned int len);
	int (*recv)(struct thinkos_comm_drv *, void * buf, unsigned int len);
	int (*ctrl)(struct thinkos_comm_drv *, unsigned int opc);
	void (*signal)(struct thinkos_comm_drv *, unsigned int sig);
	void (*reset)(struct thinkos_comm_drv *, int priority);
	int (*init)(struct thinkos_rt *, struct thinkos_comm_drv *, void * lld, int tx_wq, int rx_wq);
};

struct thinkos_comm_krn_op {
	int (*done)(const struct thinkos_comm * comm);
};

struct thinkos_comm {
	union {
		char tag[8];
		uint64_t hash;
	};
	struct thinkos_comm_drv * drv;
	const struct thinkos_comm_drv_op * drv_op;
};

struct comm_rx_req {
	uint32_t wq;
	uint8_t * ptr;
	uint32_t len;
	uint32_t tmo;
	int32_t cnt;
};

struct comm_tx_req {
	uint32_t wq;
	uint8_t * ptr;
	uint32_t len;
	uint32_t tmo;
	uint32_t cnt;
};

#ifdef __cplusplus
extern "C" {
#endif

int krn_comm_rx_putc(struct thinkos_rt * krn, unsigned int rx_wq, int c);

int krn_comm_tx_getc(struct thinkos_rt * krn, unsigned int tx_wq);

int thinkos_krn_comm_init(struct thinkos_rt * krn, unsigned int idx,
						  const struct thinkos_comm * comm, void * parm);

ssize_t krn_comm_tx_wq_req_process(struct thinkos_rt * krn, unsigned int tx_wq,
								   uint8_t * dst, size_t max);

static inline int krn_comm_send(const struct thinkos_comm * comm, 
								   const void * buf, unsigned int len) {
	return comm->drv_op->send(comm->drv, buf, len);
}

static inline int krn_comm_recv(const struct thinkos_comm * comm,
								   void * buf, unsigned int len) {
	return comm->drv_op->recv(comm->drv, buf, len);
}
static inline int krn_comm_connect(const struct thinkos_comm * comm) {
	return comm->drv_op->ctrl(comm->drv, COMM_CTRL_CONNECT);
}

static inline int krn_comm_disconnect(const struct thinkos_comm * comm) {
	return comm->drv_op->ctrl(comm->drv, COMM_CTRL_DISCONNECT);
}

static inline int krn_comm_break_ack(const struct thinkos_comm * comm) {
	return comm->drv_op->ctrl(comm->drv, COMM_CTRL_BREAK_ACK);
}

static inline int krn_comm_status_get(const struct thinkos_comm * comm) {
	return comm->drv_op->ctrl(comm->drv, COMM_CTRL_STATUS_GET);
}

static inline bool krn_comm_isconnected(const struct thinkos_comm * comm) {
	return (comm->drv_op->ctrl(comm->drv, COMM_CTRL_STATUS_GET) & 
	        COMM_ST_CONNECTED) ? true : false;
}

void thinkos_krn_comm_on_eot(struct thinkos_rt * krn, unsigned int wq);

void thinkos_krn_comm_on_rcv(struct thinkos_rt * krn, unsigned int wq);

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_COMM_H__ */

