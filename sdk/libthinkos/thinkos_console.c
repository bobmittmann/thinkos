/* 
 * thikos_util.c
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without flagen the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif
#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#include <thinkos.h>

#include <sys/param.h>

#if THINKOS_ENABLE_CONSOLE

/* Console lockout debug ... */
#ifndef ENABLE_CONSOLE_DEBUG
#define ENABLE_CONSOLE_DEBUG 1
#endif

#if (!THINKOS_ENABLE_MONITOR)
#error "Need THINKOS_ENABLE_MONITOR!"
#endif

#ifndef THINKOS_CONSOLE_FIFO_LEN
#define THINKOS_CONSOLE_FIFO_LEN 64
#endif

#ifndef THINKOS_CONSOLE_RX_FIFO_LEN
#define THINKOS_CONSOLE_RX_FIFO_LEN THINKOS_CONSOLE_FIFO_LEN
#endif

#ifndef THINKOS_CONSOLE_TX_FIFO_LEN
#define THINKOS_CONSOLE_TX_FIFO_LEN THINKOS_CONSOLE_FIFO_LEN
#endif

struct console_rx_pipe {
	volatile uint32_t head;
	volatile uint32_t tail;
	uint8_t buf[THINKOS_CONSOLE_RX_FIFO_LEN];
};

struct console_tx_pipe {
	volatile uint32_t head;
	volatile uint32_t tail;
	uint8_t buf[THINKOS_CONSOLE_TX_FIFO_LEN];
};

struct {
	uint8_t open_cnt;
	uint8_t connected: 1;
#if ENABLE_CONSOLE_DEBUG
	volatile uint16_t tx_last_cnt;
#endif
	struct console_tx_pipe tx_pipe;
	struct console_rx_pipe rx_pipe;
} thinkos_console_rt;

static int rx_pipe_read(uint8_t * buf, unsigned int len)
{
	struct console_rx_pipe * pipe = &thinkos_console_rt.rx_pipe;
	uint32_t tail;
	int max;
	int cnt;
	int pos;

	/* pipe->tail is declared as volatile, 
	   for performance reasons we read it only once at 
	   the beginning and write it back at the end. */
	tail = pipe->tail;
	/* get the maximum number of chars we can read from the buffer */
	if ((max = pipe->head - tail) == 0)
		return 0;
	/* cnt is the number of chars we will read from the buffer,
	   it should be the minimum of max and len */
	cnt = MIN(max, len);
	/* get the tail position in the buffer */
	pos = (tail % THINKOS_CONSOLE_RX_FIFO_LEN);
	/* check whether to wrap around or on not */
	if ((pos + cnt) > THINKOS_CONSOLE_RX_FIFO_LEN) {
		/* we need to perform two reads */
		int n;
		int m;
		/* get the number of chars from tail pos until the end of buffer */
		n = THINKOS_CONSOLE_RX_FIFO_LEN - pos;
		/* the remaining chars are at the beginning of the buffer */
		m = cnt - n;
		__thinkos_memcpy(buf, &pipe->buf[pos], n);
		__thinkos_memcpy(buf + n, &pipe->buf[0], m);
	} else {
		__thinkos_memcpy(buf, &pipe->buf[pos], cnt);
	}

	pipe->tail = tail + cnt;

	return cnt;
}

static int tx_pipe_write(const uint8_t * buf, unsigned int len)
{
	struct console_tx_pipe * pipe = &thinkos_console_rt.tx_pipe;
	uint8_t * cp = (uint8_t *)buf;
	uint32_t head;
	uint32_t tail;
	uint32_t max;
	int cnt;
	uint32_t pos;

#if ENABLE_CONSOLE_DEBUG
	if (thinkos_console_rt.tx_pipe.tail > 0x40000000) {
		DCC_LOG1(LOG_PANIC, "tail=%u", thinkos_console_rt.tx_pipe.tail);
		__THINKOS_ERROR(THINKOS_ERR_CONSOLE_FAULT);
	}
#endif
	/* pipe->head is declared as volatile, 
	   for performance reasons we read it only once at 
	   the beginning and write it back at the end. */
	head = pipe->head;
	tail = pipe->tail;
	/* get the maximum number of chars we can write into buffer */
	if ((max = tail + THINKOS_CONSOLE_TX_FIFO_LEN - head) == 0) {
		DCC_LOG2(LOG_TRACE, "fifo empty: head=%u tail=%u", head, tail);
		return 0;
	}
	/* cnt is the number of chars we will write to the buffer,
	   it should be the minimum of max and len */
	cnt = MIN(max, len);
	/* get the head position in the buffer */
	pos = (head % THINKOS_CONSOLE_TX_FIFO_LEN);
	DCC_LOG6(LOG_INFO, "head=%u tail=%u max=%d len=%d cnt=%d pos=%d", 
			 head, tail, max, len, cnt, pos);

#if ENABLE_CONSOLE_DEBUG
	if (cnt > THINKOS_CONSOLE_TX_FIFO_LEN) {
		DCC_LOG4(LOG_PANIC, "head=%u tail=%u len=%d cnt = %d", 
			 head, tail, THINKOS_CONSOLE_TX_FIFO_LEN, cnt);
		__THINKOS_ERROR(THINKOS_ERR_CONSOLE_FAULT);
	}
#endif

	/* check whether to wrap around or on not */
	if ((pos + cnt) > THINKOS_CONSOLE_TX_FIFO_LEN) {
		/* we need to perform two writes */
		int n;
		int m;
		/* get the number of chars from tail pos until the end of buffer */
		n = THINKOS_CONSOLE_TX_FIFO_LEN - pos;
		/* the remaining chars are at the beginning of the buffer */
		m = cnt - n;
		__thinkos_memcpy(&pipe->buf[pos], cp, n);
		__thinkos_memcpy(&pipe->buf[0], cp + n, m);
	} else {
		__thinkos_memcpy(&pipe->buf[pos], cp, cnt);
	}

	pipe->head = head + cnt;

	return cnt;
}

static bool tx_pipe_isfull(void)
{
	struct console_tx_pipe * pipe = &thinkos_console_rt.tx_pipe;
	return (pipe->tail + THINKOS_CONSOLE_TX_FIFO_LEN) == pipe->head;
}

static bool tx_pipe_isempty(void)
{
	struct console_tx_pipe * pipe = &thinkos_console_rt.tx_pipe;
	return pipe->tail == pipe->head;
}

/* get the a pointer from the next available character in the
   queue and return the number of available chars */ 
int __console_tx_pipe_ptr(uint8_t ** ptr) 
{
	struct console_tx_pipe * pipe = &thinkos_console_rt.tx_pipe;
	uint32_t tail;
	uint32_t head;
	int32_t cnt;
	int pos;

#if ENABLE_CONSOLE_DEBUG
	if (thinkos_console_rt.tx_pipe.tail > 0x40000000) {
		DCC_LOG1(LOG_PANIC, "tail=%u", thinkos_console_rt.tx_pipe.tail);
		__THINKOS_ERROR(THINKOS_ERR_CONSOLE_FAULT);
	}
#endif

	/* get the tail position in the buffer */
	tail = pipe->tail;
	head = pipe->head;
	pos = (tail % THINKOS_CONSOLE_TX_FIFO_LEN);
	/* get the used space */
	cnt = head - tail;
	/* check whether to wrap around or on not */
	if ((pos + cnt) > THINKOS_CONSOLE_TX_FIFO_LEN) {
		/* get the number of chars from tail pos until the end of buffer */
		cnt = THINKOS_CONSOLE_TX_FIFO_LEN - pos;
	}

#if 0
	/* check whether to wrap around or on not */
	if (head >= tail) {
		/* get the used space */
		cnt = head - tail;
#if ENABLE_CONSOLE_DEBUG
		if (cnt > THINKOS_CONSOLE_TX_FIFO_LEN) {
			DCC_LOG3(LOG_PANIC, "head=%u tail=%u cnt=%d", 
					 head, tail, cnt);
			__THINKOS_ERROR(THINKOS_ERR_CONSOLE_FAULT);
		}
#endif
	} else {
		/* get the number of chars from tail pos until the end of buffer */
		cnt = THINKOS_CONSOLE_TX_FIFO_LEN - pos;
#if ENABLE_CONSOLE_DEBUG
		if (cnt > THINKOS_CONSOLE_TX_FIFO_LEN) {
			DCC_LOG3(LOG_PANIC, "head=%u tail=%u cnt=%d", 
					 head, tail, cnt);
			__THINKOS_ERROR(THINKOS_ERR_CONSOLE_FAULT);
		}
#endif
	}
#endif

#if ENABLE_CONSOLE_DEBUG
	thinkos_console_rt.tx_last_cnt = cnt;
#endif
	DCC_LOG4(LOG_INFO, "head=%u tail=%u cnt=%d pos=%d", 
			 head, tail, cnt, pos);
	*ptr = &pipe->buf[pos];

	return cnt;
}

void __console_tx_pipe_commit(int cnt) 
{
	int wq = THINKOS_WQ_CONSOLE_WR;
	int th;

#if ENABLE_CONSOLE_DEBUG
	if (cnt != thinkos_console_rt.tx_last_cnt) {
		DCC_LOG1(LOG_PANIC, "cnt=%d !!!", cnt);
		__THINKOS_ERROR(THINKOS_ERR_CONSOLE_FAULT);
	}
#endif
	if ((cnt <= 0) || (cnt > THINKOS_CONSOLE_TX_FIFO_LEN)) {
		DCC_LOG1(LOG_PANIC, "cnt=%d !!!", cnt);
		__THINKOS_ERROR(THINKOS_ERR_CONSOLE_FAULT);
	}

	thinkos_console_rt.tx_pipe.tail += cnt;
	if ((th = __thinkos_wq_head(wq)) == THINKOS_THREAD_NULL) {
		DCC_LOG(LOG_INFO, "no thread waiting.");
		return;
	}

	DCC_LOG1(LOG_MSG, "thread_id=%d", th);

	/* XXX: To avoid a race condition when writing to the 
	   pipe from the service call and this function (invoked
	   from an interrupt handler), let the thread to
	   wake up and retry. */
	/* wakeup from the console wait queue */
	__thinkos_wakeup(wq, th);
	/* signal the scheduler ... */
	__thinkos_defer_sched();
}


int __console_rx_pipe_ptr(uint8_t ** ptr) 
{
	struct console_rx_pipe * pipe = &thinkos_console_rt.rx_pipe;
	uint32_t head;
	int cnt;
	int pos;

	/* get the head position in the buffer */
	head = pipe->head;
	pos = (head % THINKOS_CONSOLE_RX_FIFO_LEN);
	/* get the free space */
	cnt = pipe->tail + THINKOS_CONSOLE_RX_FIFO_LEN - head;
	/* check whether to wrap around or on not */
	if ((pos + cnt) > THINKOS_CONSOLE_RX_FIFO_LEN) {
		/* get the number of chars from head pos until the end of buffer */
		cnt = THINKOS_CONSOLE_RX_FIFO_LEN - pos;
	}
	*ptr = &pipe->buf[pos];

	return cnt;
}

void __console_rx_pipe_commit(int cnt) 
{
	int wq = THINKOS_WQ_CONSOLE_RD;
	unsigned int max;
	uint8_t * buf;
	int th;
	int n;

	thinkos_console_rt.rx_pipe.head += cnt;

	DCC_LOG2(LOG_INFO, "wq=%d -> 0x%08x", wq, thinkos_rt.wq_lst[wq]);

	if ((th = __thinkos_wq_head(wq)) == THINKOS_THREAD_NULL) {
		DCC_LOG(LOG_INFO, "no thread waiting.");
		return;
	}

	DCC_LOG1(LOG_INFO, "thread_id=%d", th);

	buf = (uint8_t *)thinkos_rt.ctx[th]->r1;
	max = thinkos_rt.ctx[th]->r2;
	/* read from the RX pipe into the thread's read buffer */
	if ((n = rx_pipe_read(buf, max)) == 0) {
		DCC_LOG(LOG_INFO, "_pipe_read() == 0");
		return;
	}

	/* wakeup from the console wait queue */
	__thinkos_wakeup_return(wq, th, n);
	/* signal the scheduler ... */
	__thinkos_defer_sched();
}


#if (THINKOS_ENABLE_PAUSE && THINKOS_ENABLE_THREAD_STAT)
void __console_rd_resume(unsigned int th, unsigned int wq, bool tmw) 
{
	DCC_LOG1(LOG_TRACE, "PC=%08x ...........", thinkos_rt.ctx[th]->pc); 
	/* wakeup from the console read wait queue setting the return value to 0.
	   The calling thread should retry the operation. */
	__thinkos_wakeup_return(wq, th, 0);
}

void __console_wr_resume(unsigned int th, unsigned int wq, bool tmw) 
{
	if (tx_pipe_isfull()) {
		DCC_LOG1(LOG_TRACE, "PC=%08x pipe full ..", thinkos_rt.ctx[th]->pc); 
		dbgmon_signal(DBGMON_TX_PIPE);
	} else {
		DCC_LOG1(LOG_TRACE, "PC=%08x ...........", thinkos_rt.ctx[th]->pc); 
	}
	/* wakeup from the console write wait queue setting the return value to 0.
	   The calling thread should retry the operation. */
	__thinkos_wakeup_return(wq, th, 0);
}
#endif

void thinkos_console_svc(int32_t * arg, int self)
{
	unsigned int req = arg[0];
	unsigned int wq;
	unsigned int len;
	uint32_t queue;
	uint8_t * buf;
	int n;
	
#if ENABLE_CONSOLE_DEBUG
	if (thinkos_console_rt.tx_pipe.tail > 0x40000000) {
		DCC_LOG1(LOG_PANIC, "tail=%u", thinkos_console_rt.tx_pipe.tail);
		__THINKOS_ERROR(THINKOS_ERR_CONSOLE_FAULT);
	}
#endif

	switch (req) {

	case CONSOLE_OPEN:
		thinkos_console_rt.open_cnt++;
		arg[0] = THINKOS_OK;
		break;

	case CONSOLE_CLOSE:
		if (thinkos_console_rt.open_cnt > 0)
			arg[0] = THINKOS_EBADF;
		else
			arg[0] = THINKOS_OK;
		break;

	case CONSOLE_IOCTL:
		DCC_LOG(LOG_TRACE, "CONSOLE_IOCTL");
		arg[0] = THINKOS_OK;
		break;

	case CONSOLE_IS_CONNECTED:
		DCC_LOG1(LOG_TRACE, "CONSOLE_IS_CONNECTED(%d)", 
				thinkos_console_rt.connected);
		arg[0] = thinkos_console_rt.connected;
		break;

#if THINKOS_ENABLE_TIMED_CALLS
	case CONSOLE_TIMEDREAD:
		buf = (uint8_t *)arg[1];
		len = arg[2];
		DCC_LOG1(LOG_INFO, "Console timed read: len=%d", len);
		if ((n = rx_pipe_read(buf, len)) > 0) {
			dbgmon_signal(DBGMON_RX_PIPE);
			arg[0] = n;
			break;
		}
		DCC_LOG(LOG_INFO, "Console timed read wait...");
		/* wait for event */
		__thinkos_suspend(self);
		/* insert into the mutex wait queue */
		__thinkos_tmdwq_insert(THINKOS_WQ_CONSOLE_RD, self, arg[3]);
		/* Set the default return value to timeout. */
		arg[0] = THINKOS_ETIMEDOUT;
		/* signal the scheduler ... */
		__thinkos_defer_sched(); 
		break;
#endif

	case CONSOLE_READ:
		buf = (uint8_t *)arg[1];
		len = arg[2];
		DCC_LOG1(LOG_INFO, "Console read: len=%d", len);
		if ((n = rx_pipe_read(buf, len)) > 0) {
			dbgmon_signal(DBGMON_RX_PIPE);
			arg[0] = n;
			break;
		}
		DCC_LOG(LOG_INFO, "Console read wait...");
		/* wait for event */
		__thinkos_suspend(self);
		/* insert into the mutex wait queue */
		__thinkos_wq_insert(THINKOS_WQ_CONSOLE_RD, self);
		/* signal the scheduler ... */
		__thinkos_defer_sched(); 
		break;

	case CONSOLE_WRITE:
		buf = (uint8_t *)arg[1];
		len = arg[2];
		wq = THINKOS_WQ_CONSOLE_WR;
		DCC_LOG1(LOG_MSG, "Console write: len=%d", len);
wr_again:
		if ((n = tx_pipe_write(buf, len)) > 0) {
			DCC_LOG1(LOG_INFO, "tx_pipe_write: n=%d", n);
			dbgmon_signal(DBGMON_TX_PIPE);
			arg[0] = n;
		} else {
			/* Set the return value to ZERO. The calling thread 
			   shuould retry sending data. */
			arg[0] = 0;
			/* (1) suspend the thread by removing it from the
			   ready wait queue. The __thinkos_suspend() call cannot be nested
			   inside a LDREX/STREX pair as it may use the exclusive access 
			   itself, in case we have anabled the time sharing option. */
			__thinkos_suspend(self);
			/* update the thread status in preparation for event wait */
#if THINKOS_ENABLE_THREAD_STAT
			thinkos_rt.th_stat[self] = wq << 1;
#endif
			/* (2) Save the context pointer. In case an interrupt wakes up
			   this thread before the scheduler is called, this will allow
			   the interrupt handler to locate the return value (r0) address. */
			thinkos_rt.ctx[self] = (struct thinkos_context *)&arg[-CTX_R0];

			queue = __ldrex(&thinkos_rt.wq_lst[wq]);
			
			/* The pipe may have been flushed while suspending (1).
			   If this is the case roll back and restart. */
			if (!tx_pipe_isfull()) {
				/* roll back */
#if THINKOS_ENABLE_THREAD_STAT
				thinkos_rt.th_stat[self] = 0;
#endif
				/* insert into the ready wait queue */
				__bit_mem_wr(&thinkos_rt.wq_ready, self, 1);  
				DCC_LOG2(LOG_INFO, "<%d> rollback 1 %d...", self + 1, wq);
				goto wr_again;
			}
			/* Insert into the event wait queue */
			queue |= (1 << self);
			/* (3) Try to save back the queue state.
			   The queue may have changed by an interrup handler.
			   If this is the case roll back and restart. */
			if (__strex(&thinkos_rt.wq_lst[wq], queue)) {
				/* roll back */
#if THINKOS_ENABLE_THREAD_STAT
				thinkos_rt.th_stat[self] = 0;
#endif
				/* insert into the ready wait queue */
				__bit_mem_wr(&thinkos_rt.wq_ready, self, 1);  
				DCC_LOG2(LOG_INFO, "<%d> rollback 2 %d...", self + 1, wq);
				goto wr_again;
			}

			/* -- wait for event ---------------------------------------- */
			DCC_LOG2(LOG_INFO, "<%d> waiting on console write %d...", 
					 self + 1, wq);
			/* signal the scheduler ... */
			__thinkos_defer_sched(); 

		}
		break;

	case CONSOLE_DRAIN:
		DCC_LOG(LOG_TRACE, "CONSOLE_DRAIN");
		wq = THINKOS_WQ_CONSOLE_WR;
drain_again:
#if ENABLE_CONSOLE_DEBUG
		if (thinkos_console_rt.tx_pipe.tail > 0x40000000) {
			__THINKOS_ERROR(THINKOS_ERR_CONSOLE_FAULT);
		}
#endif
		if (tx_pipe_isempty()) {
			DCC_LOG(LOG_INFO, "pipe empty.");
			arg[0] = 0;
		} else {
#if 0
		arg[0] = THINKOS_EAGAIN;
		/* wait for event */
		__thinkos_suspend(self);
		/* insert into the mutex wait queue */
		__thinkos_wq_insert(THINKOS_WQ_CONSOLE_WR, self);
		/* -- wait for event ---------------------------------------- */
//		DCC_LOG(LOG_INFO, "waiting on console write");
		/* signal the scheduler ... */
		__thinkos_defer_sched(); 
#endif
			/* Set the return value to EGAIN. The calling thread 
			   shuould retry ... */
			arg[0] = THINKOS_EAGAIN;
			/* (1) suspend the thread by removing it from the
			   ready wait queue. The __thinkos_suspend() call cannot be nested
			   inside a LDREX/STREX pair as it may use the exclusive access 
			   itself, in case we have anabled the time sharing option. */
			__thinkos_suspend(self);
			/* update the thread status in preparation for event wait */
#if THINKOS_ENABLE_THREAD_STAT
			thinkos_rt.th_stat[self] = wq << 1;
#endif
			/* (2) Save the context pointer. In case an interrupt wakes up
			   this thread before the scheduler is called, this will allow
			   the interrupt handler to locate the return value (r0) address. */
			thinkos_rt.ctx[self] = (struct thinkos_context *)&arg[-CTX_R0];

			queue = __ldrex(&thinkos_rt.wq_lst[wq]);

			/* The pipe may have been dranned while suspending (1).
			   If this is the case roll back and restart. */
			if (tx_pipe_isempty()) {
				/* roll back */
#if THINKOS_ENABLE_THREAD_STAT
				thinkos_rt.th_stat[self] = 0;
#endif
				/* insert into the ready wait queue */
				__bit_mem_wr(&thinkos_rt.wq_ready, self, 1);  
				DCC_LOG2(LOG_INFO, "<%d> rollback 1 %d...", self + 1, wq);
				goto drain_again;
			}
			/* Insert into the event wait queue */
			queue |= (1 << self);
			/* (3) Try to save back the queue state.
			   The queue may have changed by an interrup handler.
			   If this is the case roll back and restart. */
			if (__strex(&thinkos_rt.wq_lst[wq], queue)) {
				/* roll back */
#if THINKOS_ENABLE_THREAD_STAT
				thinkos_rt.th_stat[self] = 0;
#endif
				/* insert into the ready wait queue */
				__bit_mem_wr(&thinkos_rt.wq_ready, self, 1);  
				DCC_LOG2(LOG_INFO, "<%d> rollback 2 %d...", self + 1, wq);
				goto drain_again;
			}

			/* -- wait for event ---------------------------------------- */
			DCC_LOG2(LOG_INFO, "<%d> waiting on console write %d...", 
					 self + 1, wq);
			/* signal the scheduler ... */
			__thinkos_defer_sched(); 
		}
		break;

	default:
		DCC_LOG1(LOG_ERROR, "invalid console request %d!", req);
		__THINKOS_ERROR(THINKOS_ERR_CONSOLE_REQINV);
		arg[0] = THINKOS_EINVAL;
		break;
	}
}

void __console_reset(void)
{
	DCC_LOG(LOG_TRACE, "clearing pipes and signals.");
	dbgmon_clear(DBGMON_TX_PIPE);
	dbgmon_clear(DBGMON_RX_PIPE);

	thinkos_console_rt.tx_last_cnt = 0;
	thinkos_console_rt.tx_pipe.head = 0;
	thinkos_console_rt.tx_pipe.tail = 0;
	thinkos_console_rt.rx_pipe.head = 0;
	thinkos_console_rt.rx_pipe.tail = 0;
	thinkos_console_rt.connected = 0;
	thinkos_console_rt.open_cnt = 0;
}

void __console_connect_set(bool val) 
{
#if ENABLE_CONSOLE_DEBUG
	if (thinkos_console_rt.tx_pipe.tail > 0x40000000) {
		DCC_LOG1(LOG_PANIC, "tail=%u", thinkos_console_rt.tx_pipe.tail);
		__THINKOS_ERROR(THINKOS_ERR_CONSOLE_FAULT);
	}
#endif

	DCC_LOG1(LOG_TRACE, "connected=%d.", val);
	thinkos_console_rt.connected = val;
}

void thinkos_console_init(void)
{
	__console_reset();
}

#endif /* THINKOS_ENABLE_CONSOLE */

