/* parse-markup: reST */
  
/* 
 * thinkos_comm.c
 *
 * Copyright(C) 2021 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
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

/* 
 * @file thinkos_comm
 * @brief ThinkOS communication devices
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

/*
   COMM devices are simple communication channels comprised of two unidirectional 
   channels (points) or streams and a control point;
   The kernel or a user application can write to the output (TX) stream, 
   reads from the input (RX) stream and control the operation or be notified of
   (out-of-band) events through the control point.

   The console can be directed to a COMM device...

 */

#include "thinkos_krn-i.h"
#include <sys/dcclog.h>

#define __THINKOS_FIFO__
#include <thinkos/fifo.h>

#define __THINKOS_COMM__
#include <thinkos/comm.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

struct {
	struct thinkos_comm * comm[THINKOS_COMM_MAX];
} thinkos_comm_rt;

#if (THINKOS_ENABLE_PAUSE) && (THINKOS_ENABLE_THREAD_STAT)
bool comm_rx_resume(struct thinkos_rt * krn,
							   unsigned int th, unsigned int wq, bool tmw) 
{
	__wq_wakeup_return(krn, wq, th, 0);
	
	return true;
}

bool comm_tx_resume(struct thinkos_rt * krn,
							   unsigned int th, unsigned int wq, bool tmw) 
{
	__wq_wakeup_return(krn, wq, th, 0);
	return true;
}
#endif

#if (THINKOS_COMM_MAX) > 0

static inline int __attribute__((always_inline)) 
__krn_obj_is_commtx(struct thinkos_rt * krn, unsigned int oid) {
	return (oid >= THINKOS_COMM_TX_FIRST) && (oid <= THINKOS_COMM_TX_LAST);
}

static inline int __attribute__((always_inline)) 
__krn_obj_is_commrx(struct thinkos_rt * krn, unsigned int oid) {
	return (oid >= THINKOS_COMM_RX_FIRST) && (oid <= THINKOS_COMM_RX_LAST);
}

int krn_comm_tx_check(struct thinkos_rt * krn, unsigned int oid) {
	if (!__krn_obj_is_commtx(krn, oid)) {
		/* FIXME: specific error */
		return THINKOS_ERR_KRN_FAULT;
	}
	return THINKOS_OK;
}

int krn_comm_rx_check(struct thinkos_rt * krn, unsigned int oid) {
	if (!__krn_obj_is_commrx(krn, oid)) {
		/* FIXME: specific error */
		return THINKOS_ERR_KRN_FAULT;
	}
	return THINKOS_OK;
}

struct comm_tx_req {
	uint32_t cnt;
	uint8_t * ptr;
	uint32_t len;
};

int krn_comm_tx_getc(struct thinkos_rt * krn, unsigned int tx_wq)
{
	int c = -1;
	int th;

	th = __krn_wq_head(krn, tx_wq);
	if (th != THINKOS_THREAD_NULL) {
		struct comm_tx_req * req;
		unsigned int len;
		unsigned int cnt;

		req = (struct comm_tx_req *)__thread_frame_get(krn, th);

		if ((len = req->len) > (cnt = req->cnt)) {
			c = req->ptr[cnt++];
			req->cnt = cnt;
		} 

		if (len == cnt) {
			/* wakeup from the wait queue */
			__wq_wakeup_return(krn, tx_wq, th, cnt);
			/* signal the scheduler ... */
			__krn_sched_defer(krn);
		}
	}

	return c;
}

void thinkos_comm_send_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	struct comm_tx_req * req = (struct comm_tx_req *)arg;
	const struct thinkos_comm  * comm;
	unsigned int oid = arg[0];
	unsigned int wq;
	unsigned int idx;

	DCC_LOG2(LOG_TRACE, "<%d> comm_oid=%d", self, oid);

#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_comm_tx_check(krn, oid)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid comm %d!", self, oid);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#if 0
	if (!__thinkos_mem_usr_rd_chk((uint32_t)req->ptr, req->len)) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid user memory: %p!", self, req->ptr);
		__THINKOS_ERROR(self, THINKOS_ERR_MEMORY_INVALID);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	idx = oid - THINKOS_COMM_TX_BASE;
	comm = krn->comm[idx];
	wq = oid;

//	DCC_LOG1(LOG_INFO, "<%2d> sleeping...", self);
	req->cnt = 0;
	/* wait for event ... */
	__krn_thread_wait(krn, self, wq);

	/* signal driver */
	comm->drv_op->signal(comm->drv, COMM_TX_PEND);
}

void thinkos_comm_recv_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	unsigned int comm = arg[0];
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_comm_rx_check(krn, comm)) != THINKOS_OK) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid comm %d!", self, comm);
		arg[0] = THINKOS_EINVAL;
		__THINKOS_ERROR(self, ret);
		return;
	}
#endif

	arg[0] = THINKOS_OK;
	/* signal the scheduler ... */
	__krn_sched_defer(krn);
}

int thinkos_comm_drain(struct thinkos_comm * comm, unsigned int wq,
					   int self, struct thinkos_rt * krn)
{
	return THINKOS_OK;
}

struct comm_ctl_req {
	struct {
		uint8_t idx;
		uint8_t res1;
		uint8_t res2;
		uint8_t oper;
	};
};

void thinkos_comm_ctl_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	const struct thinkos_comm  * comm;
	struct comm_ctl_req * req;
	unsigned int oid;
	unsigned int idx;
	unsigned int op;
	
	req = (struct comm_ctl_req *)arg;
	op = req->oper;
	
	DCC_LOG3(LOG_TRACE, "<%d> oper=%d devno=%d", self, op, req->idx);

	if (op == THINKOS_COMM_OPEN) {
		idx = req->idx;
		if (idx > THINKOS_COMM_MAX) {
			__THINKOS_ERROR(self, THINKOS_ERR_COMM_INVALID);
			arg[0] = THINKOS_EINVAL;
			return;
		}

		comm = krn->comm[idx];
		(void)comm;

		oid = idx + THINKOS_COMM_TX_BASE;
		(void)oid;
#if (THINKOS_ENABLE_SANITY_CHECK)
		if (comm == NULL) {
			__THINKOS_ERROR(self, THINKOS_ERR_KRN_FAULT);
			arg[0] = THINKOS_EINVAL;
			return;
		}
#endif

		comm->drv_op->open(comm->drv);
		comm->krn_op->reset(comm, IRQ_HIGH_PRIORITY);

		arg[0] = oid;
		return;
	}

	arg[0] = THINKOS_ENOSYS;
}

int thinkos_krn_comm_init(struct thinkos_rt * krn,
						  const struct thinkos_comm * comm, 
						  unsigned int idx)
{
	int tx_wq;
	int rx_wq;
	
	tx_wq = THINKOS_COMM_TX_FIRST + idx;
	rx_wq = THINKOS_COMM_RX_FIRST + idx;

	krn->comm[idx] = comm;
	comm->krn_op->init(comm, tx_wq, rx_wq);
	comm->krn_op->reset(comm, IRQ_HIGH_PRIORITY);

	return 0;
}

#endif
