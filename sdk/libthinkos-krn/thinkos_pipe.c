/* parse-markup: reST */
  
/* 
 * thinkos_pipe.c
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
 * @file thinkos_pipe
 * @brief ThinkOS pipeunication devices
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

/*
   PIPE devices are simple pipeunication channels comprised of two unidirectional 
   channels (points) or streams and a control point;
   The kernel or a user application can write to the output (TX) stream, 
   reads from the input (RX) stream and control the operation or be notified of
   (out-of-band) events through the control point.

   The console can be directed to a PIPE device...

 */

#include "thinkos_krn-i.h"
#include <sys/dcclog.h>

#define __THINKOS_FIFO__
#include <thinkos/fifo.h>

#define __THINKOS_PIPE__
#include <thinkos/pipe.h>

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

struct thinkos_pipe {
	struct thinkos_fifo tx_fifo;
	struct thinkos_fifo rx_fifo;
};

struct {
	struct thinkos_pipe pipe[THINKOS_PIPE_MAX];
} thinkos_pipe_rt;

#if (THINKOS_ENABLE_PAUSE) && (THINKOS_ENABLE_THREAD_STAT)
bool pipe_rx_resume(struct thinkos_rt * krn,
							   unsigned int th, unsigned int wq, bool tmw) 
{
	__wq_wakeup_return(krn, wq, th, 0);
	
	return true;
}

bool pipe_tx_resume(struct thinkos_rt * krn,
							   unsigned int th, unsigned int wq, bool tmw) 
{
	__wq_wakeup_return(krn, wq, th, 0);
	return true;
}
#endif

#if (THINKOS_PIPE_MAX) > 0

static inline int __attribute__((always_inline)) 
__krn_obj_is_pipetx(struct thinkos_rt * krn, unsigned int oid) {
	return (oid >= THINKOS_PIPE_TX_FIRST) && (oid <= THINKOS_PIPE_TX_LAST);
}

static inline int __attribute__((always_inline)) 
__krn_obj_is_piperx(struct thinkos_rt * krn, unsigned int oid) {
	return (oid >= THINKOS_PIPE_RX_FIRST) && (oid <= THINKOS_PIPE_RX_LAST);
}

int krn_pipe_tx_check(struct thinkos_rt * krn, unsigned int oid) {
	if (!__krn_obj_is_pipetx(krn, oid)) {
		/* FIXME: specific error */
		return THINKOS_ERR_KRN_FAULT;
	}
	return THINKOS_OK;
}

int krn_pipe_rx_check(struct thinkos_rt * krn, unsigned int oid) {
	if (!__krn_obj_is_piperx(krn, oid)) {
		/* FIXME: specific error */
		return THINKOS_ERR_KRN_FAULT;
	}
	return THINKOS_OK;
}

void thinkos_pipe_send_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	struct thinkos_pipe  * pipe;
	struct thinkos_fifo * fifo;
	unsigned int oid = arg[0];
	uint8_t * buf;
	size_t len;
	size_t n;
	uint32_t queue;
	unsigned int wq;
	unsigned int idx;


#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_pipe_tx_check(krn, oid)) != 0) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid pipe %d!", self, oid);
		__THINKOS_ERROR(self, ret);
		arg[0] = THINKOS_EINVAL;
		return;
	}
#endif

	idx = oid - THINKOS_PIPE_TX_BASE;
	wq = oid;
	pipe = &thinkos_pipe_rt.pipe[idx];
	fifo = &pipe->tx_fifo;

	buf = (uint8_t *)arg[1];
	len = arg[2];

	DCC_LOG1(LOG_INFO, "Console write: len=%d", len);

wr_again:
	if ((n = __krn_fifo_write(fifo, buf, len)) > 0) {
		DCC_LOG1(LOG_INFO, "tx_fifo_write: n=%d", n);
		//			__krn_fifo_usr_signal(fifo);
		arg[0] = n;
	} else {
		/* Set the return value to ZERO. The calling thread 
		   should retry sending data. */
		arg[0] = 0;
		/* (1) suspend the thread by removing it from the
		   ready wait queue. The __thinkos_suspend() call cannot be nested
		   inside a LDREX/STREX pair as it may use the exclusive access 
		   itself, in case we have enabled the time sharing option. */
		__krn_thread_suspend(krn, self);
		/* update the thread status in preparation for event wait */
#if (THINKOS_ENABLE_THREAD_STAT)
		__thread_stat_set(krn, self, wq, 0);
#endif
		queue = __ldrex(&krn->wq_lst[wq]);

		/* The fifo may have been flushed while suspending (1).
		   If this is the case roll back and restart. */
		if (!__krn_fifo_isfull(fifo)) {
			/* roll back */
#if (THINKOS_ENABLE_THREAD_STAT)
			__thread_stat_clr(krn, self);
#endif
			/* insert into the ready wait queue */
			__thread_ready_set(krn, self);  
			DCC_LOG2(LOG_INFO, "<%d> rollback 1 %d...", self, wq);
			goto wr_again;
		}
		/* Insert into the event wait queue */
		queue |= (1 << (self - 1));
		/* (3) Try to save back the queue state.
		   The queue may have changed by an interrup handler.
		   If this is the case roll back and restart. */
		if (__strex(&krn->wq_lst[wq], queue)) {
			/* roll back */
#if (THINKOS_ENABLE_THREAD_STAT)
			__thread_stat_clr(krn, self);
#endif
			/* insert into the ready wait queue */
			__thread_ready_set(krn, self);  
			DCC_LOG2(LOG_INFO, "<%d> rollback 2 %d...", self, wq);
			goto wr_again;
		}

		/* -- wait for event ---------------------------------------- */
		DCC_LOG2(LOG_INFO, "<%d> waiting on console write %d...", self, wq);
		/* signal the scheduler ... */
		__krn_sched_defer(krn); 
	}
}

void thinkos_pipe_recv_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	unsigned int pipe = arg[0];
#if (THINKOS_ENABLE_ARG_CHECK)
	int ret;

	if ((ret = krn_pipe_rx_check(krn, pipe)) != THINKOS_OK) {
		DCC_LOG2(LOG_ERROR, "<%2d> invalid pipe %d!", self, pipe);
		arg[0] = THINKOS_EINVAL;
		__THINKOS_ERROR(self, ret);
		return;
	}
#endif

	DCC_LOG1(LOG_INFO, "pipe %d +++++++++++++ ", pipe);

	arg[0] = THINKOS_OK;
	/* signal the scheduler ... */
	__krn_sched_defer(krn);
}

int thinkos_pipe_drain(struct thinkos_pipe * pipe, unsigned int wq,
					   int self, struct thinkos_rt * krn)
{
	struct thinkos_fifo * fifo;
	uint32_t queue;

	fifo = &pipe->tx_fifo;

drain_again:
	if (!__krn_fifo_isempty(fifo)) {
		/* Set the return value to EGAIN. The calling thread 
		   should retry ... */
//		arg[0] = THINKOS_EAGAIN;
		/* (1) suspend the thread by removing it from the
		   ready wait queue. The __thinkos_suspend() call cannot be nested
		   inside a LDREX/STREX pair as it may use the exclusive access 
		   itself, in case we have enabled the time sharing option. */
		__krn_thread_suspend(krn, self);
		/* update the thread status in preparation for event wait */
#if (THINKOS_ENABLE_THREAD_STAT)
		__thread_stat_set(krn, self, wq, 0);
#endif
		/* (2) Save the context pointer. In case an interrupt wakes up
		   this thread before the scheduler is called, this will allow
		   the interrupt handler to locate the return value (r0) address. */

		queue = __ldrex(&krn->wq_lst[wq]);

		/* The fifo may have been drained while suspending (1).
		   If this is the case roll back and restart. */
		if (__krn_fifo_isempty(fifo)) {
			/* roll back */
#if (THINKOS_ENABLE_THREAD_STAT)
			__thread_stat_clr(krn, self);
#endif
			/* insert into the ready wait queue */
			__thread_ready_set(krn, self);  
			DCC_LOG2(LOG_INFO, "<%2d> rollback 1 %d...", self, wq);
			goto drain_again;
		}
		/* Insert into the event wait queue */
		queue |= (1 << (self - 1));
		/* (3) Try to save back the queue state.
		   The queue may have changed by an interrupt handler.
		   If this is the case roll back and restart. */
		if (__strex(&krn->wq_lst[wq], queue)) {
			/* roll back */
#if (THINKOS_ENABLE_THREAD_STAT)
			__thread_stat_clr(krn, self);
#endif
			/* insert into the ready wait queue */
			__thread_ready_set(krn, self);  
			DCC_LOG2(LOG_INFO, "<%2d> rollback 2 %d...", self, wq);
			goto drain_again;
		}

		/* -- wait for event ---------------------------------------- */
		DCC_LOG2(LOG_INFO, "<%2d> waiting on console write %d...", 
				 self, wq);

		/* signal the scheduler ... */
		__krn_sched_defer(krn); 
	}

	return THINKOS_OK;
}

void thinkos_pipe_ctl_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	struct thinkos_pipe  * pipe;
	unsigned int oid = arg[0];
	unsigned int idx;
	int ret;

	if ((ret = krn_pipe_rx_check(krn, oid)) == THINKOS_OK) {
		idx = oid - THINKOS_PIPE_RX_BASE;
	} else if ((ret = krn_pipe_tx_check(krn, oid)) == THINKOS_OK) {
		idx = oid - THINKOS_PIPE_TX_BASE;
	} else {
#if (THINKOS_ENABLE_ARG_CHECK)
		DCC_LOG2(LOG_ERROR, "<%2d> invalid pipe %d!", self, oid);
		__THINKOS_ERROR(self, ret);
#endif
		arg[0] = THINKOS_EINVAL;
		return;
	}

	pipe = &thinkos_pipe_rt.pipe[idx];
	(void)pipe;

	DCC_LOG1(LOG_INFO, "pipe %d +++++++++++++ ", oid);

	arg[0] = THINKOS_OK;
}

#endif /* THINKOS_PIPE_MAX > 0 */


