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
   COMM devices are simple communication channels comprising two unidirectional 
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

#if (THINKOS_COMM_MAX) > 0

struct {
	struct thinkos_comm comm[THINKOS_COMM_MAX];
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
		return THINKOS_ERR_GENERAL;
	}
	return THINKOS_OK;
}

int krn_comm_rx_check(struct thinkos_rt * krn, unsigned int oid) {
	if (!__krn_obj_is_commrx(krn, oid)) {
		/* FIXME: specific error */
		return THINKOS_ERR_GENERAL;
	}
	return THINKOS_OK;
}

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
			__krn_wq_ready_thread_ins(krn, th);
			__krn_wq_remove(krn, tx_wq, th);
			/* signal the scheduler ... */
			__krn_sched_defer(krn);
		}
	}

	return c;
}

ssize_t krn_comm_tx_wq_req_process(struct thinkos_rt * krn, 
								   unsigned int tx_wq,
								   uint8_t * dst, size_t max)
{
	struct comm_tx_req * req = NULL;
	int th;

	th = __krn_wq_head(krn, tx_wq);
	if (th != THINKOS_THREAD_NULL) {
		req = (struct comm_tx_req *)__thread_frame_get(krn, th);
		size_t len = req->len;
		size_t cnt = req->cnt;
		ssize_t rem;
		uint8_t * src;

		if ((rem = (len - cnt)) > 0) {
			int i;
			if (rem > max)
				rem = max;
		
			src = &req->ptr[cnt];
			for (i = 0; i < rem; ++i) {
				dst[i] = src[i];
			}
			cnt += rem;
			req->cnt = cnt + rem;
		}

		if (len == cnt) {
			/* wakeup from the wait queue */
			__krn_wq_ready_thread_ins(krn, th);
			__krn_wq_remove(krn, tx_wq, th);
			/* signal the scheduler ... */
			__krn_sched_defer(krn);
		}
	
		return rem;
	}

	return 0;
}


void thinkos_comm_timed_fixup_svc(int32_t arg[], int self, 
								  struct thinkos_rt * krn)
{
	unsigned int wq;

	/* Adjust return if necessary */
	if (!__thread_clk_is_enabled(krn, self)) {
		if (arg[0] == 0)  {
			arg[SVC_RETURN] = THINKOS_ETIMEDOUT;      
			DCC_LOG2(LOG_TRACE, "<%d> wq=%d timeout", self, arg[4]);
		} else {
			DCC_LOG3(LOG_TRACE, "<%d> wq=%d cnt=%d", self, arg[4], arg[0]);
		}
	} else {
		DCC_LOG3(LOG_TRACE, "<%d> wq=%d cnt=%d", self, arg[4], arg[0]);
	}

	/* remove from event wait queue */
	wq = arg[4];
	__krn_wq_remove(krn, wq, self);  

}

void thinkos_krn_comm_on_eot(struct thinkos_rt * krn, unsigned int wq)
{
	/* wakeup all threads waiting on the condition */
	//__krn_wq_wakeup_all(krn, wq);
	int th = __krn_wq_wakeup_head(krn, wq);
	(void)th;
	DCC_LOG1(LOG_TRACE, "<%2d> wakeup...", th);
}

void thinkos_comm_send_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	struct comm_tx_req * req = (struct comm_tx_req *)arg;
	const struct thinkos_comm  * comm;
	unsigned int tx_wq = arg[0];
	int32_t rem;
	ssize_t cnt;
	size_t len;

#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_comm_tx_check(krn, tx_wq)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid comm %d!", self, tx_wq);
		__THINKOS_ERROR(self, ret);
		arg[SVC_RETURN] = THINKOS_EINVAL;
		return;
	}
#if 0
	if (!__thinkos_mem_usr_rd_chk((uint32_t)req->ptr, req->len)) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid user memory: %p!", self, req->ptr);
		__THINKOS_ERROR(self, THINKOS_ERR_MEMORY_INVALID);
		arg[SVC_RETURN] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	len = req->len;
	cnt = req->cnt;

	DCC_LOG2(LOG_MSG, "<%2d> comm %d", self, tx_wq);
	DCC_LOG3(LOG_YAP, "<%2d> len=%d cnt=%d.", self, len, cnt);

	if ((rem = (int32_t)(len - cnt)) > 0) {
		unsigned int idx;
		ssize_t ret;
	
		idx = tx_wq - THINKOS_COMM_TX_BASE;
		comm = krn->comm[idx];

		__krn_thread_suspend(krn, self);
		__krn_wq_insert(krn, tx_wq, self);

		/* signal driver */
		ret = comm->drv_op->send(comm->drv, &req->ptr[cnt], rem);
		if (ret >= 0) {
			cnt += ret;
			req->cnt = cnt;

			if (len > cnt) {
				uint32_t pc;
				/* repeat the operation */
				pc = arg[SVC_ARG_PC];      
				pc -= 2;
				arg[SVC_ARG_PC] = pc;      
				/* signal the scheduler ... */
				__krn_sched_defer(krn);
				return;
			}
		} else {
			arg[SVC_RETURN] = ret;
		}
		/* roll back */
		__thread_ready_set(krn, self);
		__krn_wq_remove(krn, tx_wq, self);
	}
}

void thinkos_comm_timedsend_svc(int32_t arg[], int self, 
							   struct thinkos_rt * krn)
{
	thinkos_comm_send_svc(arg, self, krn);
}

int krn_comm_rx_putc(struct thinkos_rt * krn, unsigned int rx_wq, int c)
{
	int ret = -1;
	int th;

	th = __krn_wq_head(krn, rx_wq);
	if (th != THINKOS_THREAD_NULL) {
		struct comm_rx_req * req;
		unsigned int len;
		unsigned int cnt;

		req = (struct comm_rx_req *)__thread_frame_get(krn, th);
		len = req->len;
		cnt = req->cnt;

		if (cnt < len) {
			req->ptr[cnt++] = c;
			req->cnt = cnt;
		} 

		DCC_LOG3(LOG_TRACE, "<%2d> cnt=%d len=%d", th, cnt, len);

		/* wakeup from the wait queue */
		__krn_wq_ready_thread_ins(krn, th);
		__krn_wq_remove(krn, rx_wq, th);
		/* signal the scheduler ... */
		__krn_sched_defer(krn);
	}

	return ret;
}

void thinkos_comm_timedrecv_svc(int32_t arg[], int self, 
								struct thinkos_rt * krn)
{
	struct comm_rx_req * req = (struct comm_rx_req *)arg;
	const struct thinkos_comm  * comm;
	unsigned int oid = arg[0];
	unsigned int wq;
	unsigned int idx;
	unsigned int tmo;

	idx = oid - THINKOS_COMM_TX_BASE;
	wq = idx + THINKOS_COMM_RX_BASE;
	tmo = req->tmo;

	DCC_LOG4(LOG_TRACE, "<%d> idx=%d wq=%d tmo=%d", self, idx, wq, tmo);

#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_comm_rx_check(krn, wq)) != THINKOS_OK) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid comm %d!", self, wq);
		arg[SVC_RETURN] = THINKOS_EINVAL;
		__THINKOS_ERROR(self, ret);
		return;
	}
#endif

	comm = krn->comm[idx];

	arg[SVC_RETURN] = wq;
	req->cnt = 0;

	/* wait for event ... */
	__krn_thread_timedwait(krn, self, wq, tmo);

	/* signal driver */
	comm->drv_op->signal(comm->drv, COMM_RX_WAIT);
}

void thinkos_krn_comm_on_rcv(struct thinkos_rt * krn, unsigned int wq)
{
	int th = __krn_wq_wakeup_head(krn, wq);
	(void)th;
	DCC_LOG1(LOG_TRACE, "<%2d> wakeup...", th);
}


void thinkos_comm_recv_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	struct comm_rx_req * req = (struct comm_rx_req *)arg;
	const struct thinkos_comm  * comm;
	unsigned int oid = arg[0];
	unsigned int rx_wq;
	unsigned int idx;
	size_t len;

	idx = oid - THINKOS_COMM_TX_BASE;
	rx_wq = idx + THINKOS_COMM_RX_BASE;

	DCC_LOG3(LOG_TRACE, "<%d> idx=%d wq=%d", self, idx, rx_wq);

#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_comm_rx_check(krn, rx_wq)) != THINKOS_OK) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid comm %d!", self, rx_wq);
		arg[SVC_RETURN] = THINKOS_EINVAL;
		__THINKOS_ERROR(self, ret);
		return;
	}
#if 0
	if (!__thinkos_mem_usr_wr_chk((uint32_t)req->ptr, req->len)) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid user memory: %p!", self, req->ptr);
		__THINKOS_ERROR(self, THINKOS_ERR_MEMORY_INVALID);
		arg[SVC_RETURN] = THINKOS_EINVAL;
		return;
	}
#endif
#endif

	__krn_thread_suspend(krn, self);
	__krn_wq_insert(krn, rx_wq, self);

	len = req->len;

	if (len > 0) {
		ssize_t ret;

		comm = krn->comm[idx];

		ret = comm->drv_op->recv(comm->drv, req->ptr, len);
		if (ret == 0) {
			uint32_t pc;
			/* repeat the operation */
			pc = arg[SVC_ARG_PC];      
			pc -= 2;
			arg[SVC_ARG_PC] = pc;      
			/* signal the scheduler ... */
			__krn_sched_defer(krn);
			return;
		}
		arg[SVC_RETURN] = ret;
		/* roll back */
		__thread_ready_set(krn, self);
		__krn_wq_remove(krn, rx_wq, self);
	}
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
	
	if (op == THINKOS_COMM_OPEN) {
		idx = req->idx;
		if (idx > THINKOS_COMM_MAX) {
			__THINKOS_ERROR(self, THINKOS_ERR_COMM_INVALID);
			arg[SVC_RETURN] = THINKOS_EINVAL;
			return;
		}

		comm = krn->comm[idx];
		(void)comm;

		oid = idx + THINKOS_COMM_TX_BASE;
		(void)oid;
#if (THINKOS_ENABLE_SANITY_CHECK)
		if (comm == NULL) {
			__THINKOS_ERROR(self, THINKOS_ERR_COMM_INVALID);
			arg[SVC_RETURN] = THINKOS_EINVAL;
			return;
		}
#endif

		DCC_LOG2(LOG_TRACE, "<%d> open(%p)", self, comm->drv);
		comm->drv_op->open(comm->drv);

		arg[SVC_RETURN] = oid;
		return;
	} else {
		DCC_LOG3(LOG_TRACE, "<%d> oper=%d devno=%d", self, op, req->idx);
	}

	arg[SVC_RETURN] = THINKOS_ENOSYS;
}

int thinkos_krn_comm_init(struct thinkos_rt * krn,
						  unsigned int idx,
						  const struct thinkos_comm * comm, 
						  void * lowlvldrv)
{
	int tx_wq;
	int rx_wq;
	
	if (idx >= THINKOS_COMM_MAX) {
		DCC_LOG1(LOG_ERROR, "invalid COMM interface: %d", idx);
		return -1;
	}

	tx_wq = THINKOS_COMM_TX_FIRST + idx;
	rx_wq = THINKOS_COMM_RX_FIRST + idx;

	krn->comm[idx] = comm;
	/* Initializes driver */
	comm->drv_op->init(krn, comm->drv, lowlvldrv, tx_wq, rx_wq);

	return idx;
}

#endif

