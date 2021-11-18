/* 
 * thinkos_console.c
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

#include "thinkos_krn-i.h"
#include <sys/dcclog.h>

#include <sys/param.h>

#define SVC_RETCODE 5

void tp12_on(void);
void tp12_off(void);
void tp13_on(void);
void tp13_off(void);

#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif

inline static void console_signal_tx_req(void)
{
#if (THINKOS_ENABLE_MONITOR)
	monitor_signal(SIG_CONSOLE_TX);
#else
#endif
}

inline static void console_clear_tx_pipe(void)
{
#if (THINKOS_ENABLE_MONITOR)
	monitor_clear(MONITOR_TX_PIPE);
#else
#endif
}

inline static void console_signal_tx_pipe(void)
{
#if (THINKOS_ENABLE_MONITOR)
	monitor_signal(MONITOR_TX_PIPE);
#else
#endif
}

inline static void console_clear_rx_pipe(void)
{
#if (THINKOS_ENABLE_MONITOR)
	monitor_clear(MONITOR_RX_PIPE);
#else
#endif
}

inline static void console_signal_rx_pipe(void)
{
#if (THINKOS_ENABLE_MONITOR)
	monitor_signal(MONITOR_RX_PIPE);
#else
#endif
}

inline static void console_clear_ctl(void)
{
#if (THINKOS_ENABLE_MONITOR)
	/* FIXME: the raw mode console mechanism has a circular dependency 
	   between console and debug monitor */
	monitor_clear(MONITOR_COMM_CTL);
#else
#endif
}


inline static void console_signal_ctl(void)
{
#if (THINKOS_ENABLE_MONITOR)
	/* FIXME: the raw mode console mechanism has a circular dependency 
	   between console and debug monitor */
	monitor_signal(MONITOR_COMM_CTL);
#else
#endif
}


#if (THINKOS_ENABLE_CONSOLE)

/* Console lockout debug ... */
#ifndef ENABLE_CONSOLE_DEBUG
#define ENABLE_CONSOLE_DEBUG 0
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

#define CONSOLE_FLAG_CONNECTED (1  << 0)
#define CONSOLE_FLAG_RD_BREAK  (1  << 1)
#define CONSOLE_FLAG_WR_BREAK  (1  << 2)
#define CONSOLE_FLAG_RAW       (1  << 3)

struct {
//	volatile uint8_t comm;
	struct console_tx_pipe tx_pipe;
	struct console_rx_pipe rx_pipe;
	volatile union {
		struct {
			uint32_t connected: 1;
			uint32_t rd_break: 1;
			uint32_t wr_break: 1;
			uint32_t raw_mode: 1;
			uint32_t rd_nonblock: 1;
			uint32_t wr_nonblock: 1;
			uint32_t reserved: 18;
			uint32_t open_cnt: 8;
		};
		uint32_t flags;
	};
} thinkos_console_rt;

#if 0
static int tx_pipe_write(const uint8_t * buf, unsigned int len)
{
	struct console_tx_pipe * pipe = &thinkos_console_rt.tx_pipe;
	uint8_t * cp = (uint8_t *)buf;
	uint32_t head;
	uint32_t tail;
	uint32_t max;
	uint32_t pos;
	int cnt;

	/* pipe->head is declared as volatile, 
	   for performance reasons we read it only once at 
	   the beginning and write it back at the end. */
	head = pipe->head;
	tail = pipe->tail;
	/* get the maximum number of chars we can write into buffer */
	if ((max = tail + THINKOS_CONSOLE_TX_FIFO_LEN - head) == 0) {
		DCC_LOG2(LOG_INFO, "fifo full: head=%u tail=%u", head, tail);
		return 0;
	}
	/* cnt is the number of chars we will write to the buffer,
	   it should be the minimum of max and len */
	cnt = MIN(max, len);
	/* get the head position in the buffer */
	pos = (head % THINKOS_CONSOLE_TX_FIFO_LEN);
	DCC_LOG6(LOG_INFO, "head=%u tail=%u max=%d len=%d cnt=%d pos=%d", 
			 head, tail, max, len, cnt, pos);

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
#endif

static inline bool tx_pipe_isfull(void)
{
	struct console_tx_pipe * pipe = &thinkos_console_rt.tx_pipe;
	return (pipe->tail + THINKOS_CONSOLE_TX_FIFO_LEN) == pipe->head;
}

static inline bool tx_pipe_isempty(void)
{
	struct console_tx_pipe * pipe = &thinkos_console_rt.tx_pipe;
	return pipe->tail == pipe->head;
}

static inline bool rx_pipe_isempty(void)
{
	struct console_rx_pipe * pipe = &thinkos_console_rt.rx_pipe;
	return pipe->tail == pipe->head;
}

#if 0
static unsigned int __rx_pipe_avail(struct console_rx_pipe * pipe) 
{
	int32_t cnt;

	/* get the used space */
	cnt = pipe->head - pipe->tail;

	return cnt;
}
#endif

/* get the a pointer from the next available character in the
   queue and return the number of available chars */ 
int thinkos_console_tx_pipe_ptr(uint8_t ** ptr) 
{
	struct console_tx_pipe * pipe = &thinkos_console_rt.tx_pipe;
	uint32_t tail;
	uint32_t head;
	int32_t cnt;
	int32_t pos;

#if ENABLE_CONSOLE_DEBUG
	if (thinkos_console_rt.tx_pipe.tail > 0x40000000) {
		DCC_LOG1(LOG_PANIC, "tail=%u", thinkos_console_rt.tx_pipe.tail);
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

	DCC_LOG4(LOG_INFO, "head=%u tail=%u cnt=%d pos=%d", 
			 head, tail, cnt, pos);
	*ptr = &pipe->buf[pos];

	return cnt;
}

#if (THINKOS_ENABLE_CONSOLE_BREAK)
static int __console_rd_break(struct thinkos_rt * krn) 
{
	unsigned int wq = THINKOS_WQ_CONSOLE_RD;
	int ret;
	int th;

	if ((th = __krn_wq_head(krn, wq)) == THINKOS_THREAD_NULL) {
		thinkos_console_rt.rd_break = 1;
		DCC_LOG(LOG_MSG, "no thread waiting.");
		ret = 0;
	} else {
		DCC_LOG1(LOG_MSG, "break %d", th);

		thinkos_console_rt.rd_break = 0;
		/* wakeup from the console read wait queue setting the return 
		   value to THINKOS_EINTR.
		   The calling thread should retry the operation. */
		__wq_wakeup_return(krn, wq, th, THINKOS_EINTR);
		ret = 1;
	}

	return ret;
}

static int __console_wr_break(struct thinkos_rt * krn) 
{
	unsigned int wq = THINKOS_WQ_CONSOLE_WR;
	int ret;
	int th;

	if ((th = __krn_wq_head(krn, wq)) == THINKOS_THREAD_NULL) {
		DCC_LOG(LOG_INFO, "no thread waiting.");
		thinkos_console_rt.wr_break = 1;
		ret = 0;
	} else {
		thinkos_console_rt.rd_break = 0;
		/* wakeup from the console write wait queue setting the return 
		   value to 0.
		   The calling thread should retry the operation. */
		__wq_wakeup_return(krn, wq, th, 0);
		ret = 1;
	}

	return ret;
}
#endif

void thinkos_console_tx_pipe_commit(int cnt) 
{
	unsigned int wq = THINKOS_WQ_CONSOLE_WR;
	struct thinkos_rt * krn = &thinkos_rt;
	uint32_t tail;
	int th;

#if (THINKOS_ENABLE_SANITY_CHECK)	
	if ((cnt <= 0) || (cnt > THINKOS_CONSOLE_TX_FIFO_LEN)) {
		DCC_LOG1(LOG_PANIC, "cnt=%d !!!", cnt);
		return;
	}
#endif

	tail = thinkos_console_rt.tx_pipe.tail + cnt;
	thinkos_console_rt.tx_pipe.tail = tail;
#if 0
	if (tail == thinkos_console_rt.tx_pipe.head) {
		DCC_LOG(LOG_TRACE, "TX_PIPE empty");
		console_clear_tx_pipe();
	}
#endif

	if ((th = __krn_wq_head(krn, wq)) == THINKOS_THREAD_NULL) {
		DCC_LOG(LOG_INFO, "no thread waiting.");
		return;
	}

	DCC_LOG1(LOG_TRACE, "thread_id=%d", th);

	/* XXX: To avoid a race condition when writing to the 
	   pipe from the service call and this function (invoked
	   from an interrupt handler), let the thread to
	   wake up and retry. */
	/* wakeup from the console wait queue */
	__krn_wq_wakeup(krn, wq, th);
	/* signal the scheduler ... */
	__krn_sched_defer(krn); 
}



int thinkos_console_rx_pipe_ptr(uint8_t ** ptr) 
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

#if 0
void __con_rx_dump(void) 
{
	struct console_rx_pipe * pipe = &thinkos_console_rt.rx_pipe;
	DCC_LOG2(LOG_PANIC, "Rx pipe: head=%u tail=%u", pipe->head, pipe->tail);
}
#endif

#if 0
/* Write into fifo from.  */
ssize_t thinkos_console_rx_pipe_write(struct thinkos_rt * krn, 
									  const uint8_t * buf, size_t len)
{
	struct console_rx_pipe * pipe = &thinkos_console_rt.rx_pipe;
	int wq = THINKOS_WQ_CONSOLE_RD;
	uint32_t head;
	uint32_t tail;
	int cnt = 0;
	int pos;
	int max;
	int th;

	/* get the head position in the buffer */
	head = pipe->head;
	tail = pipe->tail;

	/* get the maximum number of chars we can write into buffer */
	if ((max = (tail - head) + THINKOS_CONSOLE_RX_FIFO_LEN) >= 0) {
		uint8_t * cp = (uint8_t *)buf;

		/* cnt is the number of chars we will write to the buffer,
		   it should be the minimum of max and len */
		cnt = MIN(max, len);
		/* get the head position in the buffer */
		pos = head % THINKOS_CONSOLE_RX_FIFO_LEN;
		/* check whether to wrap around or on not */
		if ((pos + cnt) > THINKOS_CONSOLE_RX_FIFO_LEN) {
			/* we need to perform two writes */
			int n;

			/* get the number of chars from head pos until the end of buffer */
			n = THINKOS_CONSOLE_RX_FIFO_LEN - pos;
			/* get the number of chars from tail pos until the end of buffer */
			__thinkos_memcpy(&pipe->buf[pos], cp, n);
			/* the remaining chars are at the beginning of the buffer */
			__thinkos_memcpy(&pipe->buf[0], cp + n, cnt - n);
		} else {
			__thinkos_memcpy(&pipe->buf[pos], cp, cnt);
		}

		DCC_LOG2(LOG_INFO, "wq=%d cnt=%d", wq, cnt);
		pipe->head = head + cnt;
	}

	if ((th = __krn_wq_head(krn, wq)) != THINKOS_THREAD_NULL) {
		DCC_LOG3(LOG_INFO, "wakeup wq=%d cnt=%d th=%d", wq, cnt, th);
		/* Wakeup from the console read wait queue setting the 
		   return value to 0.
		   The calling thread should retry the operation. */
		__wq_wakeup_return(krn, wq, th, 0);
		/* signal the scheduler ... */
		__krn_sched_defer(krn); 

	}

	return cnt;
}
#endif

#if (THINKOS_ENABLE_PAUSE) && (THINKOS_ENABLE_THREAD_STAT)
bool thinkos_console_rd_resume(struct thinkos_rt * krn,
							   unsigned int th, unsigned int wq, bool tmw) 
{
	DCC_LOG1(LOG_INFO, "PC=%08x ...........", __thread_pc_get(krn, th)); 
	/* wakeup from the console read wait queue setting the return value to 0.
	   The calling thread should retry the operation. */
	__wq_wakeup_return(krn, wq, th, 0);
	
	return true;
}

bool thinkos_console_wr_resume(struct thinkos_rt * krn,
							   unsigned int th, unsigned int wq, bool tmw) 
{
	if (!tx_pipe_isempty()) {
		DCC_LOG1(LOG_INFO, "PC=%08x pipe full ..",  
				 __thread_pc_get(krn, th)); 
		console_signal_tx_pipe();
	} else {
		DCC_LOG1(LOG_INFO, "PC=%08x ...........",
				 __thread_pc_get(krn, th)); 
	}
	/* wakeup from the console write wait queue setting the return value to 0.
	   The calling thread should retry the operation. */
	__wq_wakeup_return(krn, wq, th, 0);
	return true;
}
#endif

#if 0
void thinkos_console_send_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{	
	struct comm_tx_req * req = (struct comm_tx_req *)arg;
	unsigned int wq = THINKOS_WQ_CONSOLE_WR;

	DCC_LOG2(LOG_TRACE, "<%2d> console tx req... %d", self, wq);

	req->cnt = 0;

	/* wait for event ... */
	__krn_thread_wait(krn, self, wq);

	/* signal driver */
	console_signal_tx_req();
}
#endif

void thinkos_console_send_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	struct console_tx_pipe * pipe = &thinkos_console_rt.tx_pipe;
	unsigned int wq = THINKOS_WQ_CONSOLE_WR;
	unsigned int len = arg[1];
	uint8_t * buf = (uint8_t *)arg[0];
	uint8_t * cp = (uint8_t *)buf;
	uint32_t head;
	uint32_t tail;
	uint32_t max;
	uint32_t pos;
	int cnt;

	/* (1) suspend the thread by removing it from the
	   ready wait queue. The __thinkos_suspend() call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access 
	   itself, in case we have enabled the time sharing option. */
	__krn_thread_suspend(krn, self);

	/* Insert into the event wait queue */
	__krn_wq_insert(krn, wq, self);

	/* Set the return value to ZERO. The calling thread 
	   should retry sending data. */
	arg[SVC_RETCODE] = 0;

	/* pipe->head is declared as volatile, 
	   for performance reasons we read it only once at 
	   the beginning and write it back at the end. */
	head = pipe->head;
	tail = pipe->tail;
	/* get the maximum number of chars we can write into buffer */
	if ((max = (tail - head + THINKOS_CONSOLE_TX_FIFO_LEN)) == 0) {
		/* -- wait for event ---------------------------------------- */
		DCC_LOG2(LOG_TRACE, "<%d> fifo full: waiting %d...", self, wq);
		/* signal the scheduler ... */
		__krn_sched_defer(krn); 
	} else {
		/* cnt is the number of chars we will write to the buffer,
		   it should be the minimum of max and len */
		cnt = MIN(max, len);

		/* get the head position in the buffer */
		pos = (head % THINKOS_CONSOLE_TX_FIFO_LEN);
		DCC_LOG6(LOG_INFO, "head=%u tail=%u max=%d len=%d cnt=%d pos=%d", 
				 head, tail, max, len, cnt, pos);

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

		/* remove from console wait queue */
		__krn_wq_remove(krn, wq, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);  

		/* signal driver */
		console_signal_tx_pipe();

		arg[SVC_RETCODE] = cnt;
	}
}

void thinkos_console_rx_pipe_commit(int cnt) 
{
	struct thinkos_rt * krn = &thinkos_rt;
	int wq = THINKOS_WQ_CONSOLE_RD;
	uint32_t * ptr = (uint32_t *)&thinkos_console_rt.rx_pipe.head;
	uint32_t queue;
	uint32_t head;
	int idx;
	int th;

	do {
		head = __ldrex(ptr);
		head += cnt;
	} while (__strex(ptr, head));

	wq = THINKOS_WQ_CONSOLE_RD;
	do {
		queue = __ldrex(&krn->wq_lst[wq]);
		/* get a thread from the queue bitmap */
		if ((idx = __thinkos_ffs(queue)) == 32) {
			return;
		} 
		/* remove from the wait queue */
		queue &= ~(1 << idx);
	} while (__strex(&krn->wq_lst[wq], queue));
	th = idx + 1; 

	/* insert the thread into ready queue */
	__thread_ready_set(krn, th);  
#if (THINKOS_ENABLE_TIMED_CALLS)
	/* possibly remove from the time wait queue */
	__thread_clk_disable(krn, th);
	/* set the thread's return value */
	__thread_r0_set(krn, th, 0);
#endif
#if (THINKOS_ENABLE_THREAD_STAT)
	/* update status */
	__thread_stat_clr(krn, th);
#endif
	/* signal the scheduler ... */
	__krn_sched_defer(krn);
}

#if (THINKOS_ENABLE_CONSOLE_READ)
static int rx_pipe_read(uint8_t * buf, unsigned int len)
{
	struct console_rx_pipe * pipe = &thinkos_console_rt.rx_pipe;
	unsigned int max;
	uint32_t tail;
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
#endif


#if (THINKOS_ENABLE_CONSOLE_READ)
void thinkos_console_recv_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
#if (THINKOS_ENABLE_TIMED_CALLS) || (THINKOS_ENABLE_CONSOLE_NONBLOCK)
	int nonblock;
#endif
#if (THINKOS_ENABLE_TIMED_CALLS) 
	int32_t tmo;
#endif
	unsigned int len = arg[1];
	uint8_t * buf = (uint8_t *)arg[0];
	unsigned int wq = THINKOS_WQ_CONSOLE_RD;
	uint32_t queue;
	int n;

rd_again:
	if ((n = rx_pipe_read(buf, len)) > 0) {
		DCC_LOG3(LOG_INFO, "<%2d> rx_pipe_read(len=%d) => %d", 
				 self, len, n);
		console_signal_rx_pipe();
		arg[SVC_RETCODE] = n;
		return;
	}

#if (THINKOS_ENABLE_TIMED_CALLS) || (THINKOS_ENABLE_CONSOLE_NONBLOCK)
#if (THINKOS_ENABLE_TIMED_CALLS) 
	tmo = (int32_t)arg[2];
  #if (THINKOS_ENABLE_CONSOLE_NONBLOCK)
	nonblock = (tmo == 0) || (thinkos_console_rt.rd_nonblock);
  #else
	nonblock = tmo == 0;
  #endif
#elif (THINKOS_ENABLE_CONSOLE_NONBLOCK)
	nonblock = thinkos_console_rt.rd_nonblock;
#endif
	if (nonblock) {
		/* if timeout is 0 do not block */
		arg[SVC_RETCODE] = THINKOS_EAGAIN;
		DCC_LOG2(LOG_WARNING, "<%2d> tmo=%d !!!", self, tmo);
		return;
	}
#endif

	/* (1) suspend the thread by removing it from the
	   ready wait queue. The __thinkos_suspend() call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access 
	   itself, in case we have enabled the time sharing option. */
	__krn_thread_suspend(krn, self);

	queue = __ldrex(&krn->wq_lst[wq]);

#if 0
	/* The pipe may have been flushed while suspending (1).
	   If this is the case roll back and restart. */
	if (!rx_pipe_isempty()) {
		/* roll back */
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);  
		DCC_LOG1(LOG_WARNING, "<%d> rollback 1 ...", self);
		goto rd_again;
	}
#endif

	/* -- wait for event ---------------------------------------- */
	DCC_LOG1(LOG_INFO, "<%d> prepare ...", self);

#if (THINKOS_ENABLE_TIMED_CALLS) 
	if (tmo > 0) {
		/* Set the default return value to timeout. The
		   thinkos_console_rx_pipe_commit() call will change this to 0 */
		arg[SVC_RETCODE] = THINKOS_ETIMEDOUT;
		/* set the clock */
		__thread_clk_itv_set(krn, self, tmo);
		/* update the thread status in preparation for event wait */
#if (THINKOS_ENABLE_THREAD_STAT)
		__thread_stat_set(krn, self, wq, 1);
#endif
		/* insert into the clock wait queue */
		__thread_clk_enable(krn, self);
	} else 
#endif	
	{
		/* Set the return value to ZERO. The calling thread 
		   should retry sending data. */
		arg[SVC_RETCODE] = 0;
		/* update the thread status in preparation for event wait */
#if (THINKOS_ENABLE_THREAD_STAT)
		__thread_stat_set(krn, self, wq, 0);
#endif
	}

	/* Insert into the event wait queue */
	queue |= (1 << (self - 1));

	/* (3) Try to save back the queue state.
	   The queue may have changed by an interrup handler.
	   If this is the case roll back and restart. */
	if ((__strex(&krn->wq_lst[wq], queue)) || !rx_pipe_isempty()) {
		/* roll back */
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);  
#if (THINKOS_ENABLE_THREAD_STAT)
		__thread_stat_clr(krn, self);
#endif
#if (THINKOS_ENABLE_TIMED_CALLS) 
		/* remove from the clock wait queue */
		__thread_clk_disable(krn, self);
#endif
		DCC_LOG1(LOG_WARNING, "<%d> rollback 2 ...", self);
		goto rd_again;
	}

	/* signal the scheduler ... */
	__krn_sched_defer(krn);

	/* -- wait for event ---------------------------------------- */
	DCC_LOG1(LOG_INFO, "<%d> sleeping ...", self);

	return;
}
#endif

#if (THINKOS_ENABLE_CONSOLE_DRAIN)
void thinkos_console_drain_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	struct console_tx_pipe * pipe = &thinkos_console_rt.tx_pipe;
	unsigned int wq = THINKOS_WQ_CONSOLE_WR;
	uint32_t head;
	uint32_t tail;
	uint32_t cnt;

	/* (1) suspend the thread by removing it from the
	   ready wait queue. The __thinkos_suspend() call cannot be nested
	   inside a LDREX/STREX pair as it may use the exclusive access 
	   itself, in case we have enabled the time sharing option. */
	__krn_thread_suspend(krn, self);

	/* Insert into the event wait queue */
	__krn_wq_insert(krn, wq, self);

	/* Set the return value to THINKOS_EAGAIN. The calling thread 
	   should retry sending data. */
	arg[SVC_RETCODE] = THINKOS_EAGAIN;

	/* pipe->head is declared as volatile, 
	   for performance reasons we read it only once at 
	   the beginning and write it back at the end. */
	head = pipe->head;
	tail = pipe->tail;
	/* get the maximum number of chars we can write into buffer */
	if ((cnt = (head - tail)) > 0) {
		/* -- wait for event ---------------------------------------- */
		DCC_LOG2(LOG_TRACE, "<%d> fifo full: waiting %d...", self, wq);
		/* signal the scheduler ... */
		__krn_sched_defer(krn); 
	} else {
		/* remove from console wait queue */
		__krn_wq_remove(krn, wq, self);
		/* insert into the ready wait queue */
		__thread_ready_set(krn, self);  

		arg[SVC_RETCODE] = 0;
	}

}
#endif

void thinkos_console_timed_fixup_svc(int32_t arg[], int self, 
									 struct thinkos_rt * krn)
{
	DCC_LOG(LOG_TRACE, "time_fixup");
}

void thinkos_console_ctl_svc(int32_t arg[], int self, struct thinkos_rt * krn)
{
	unsigned int req = arg[0];
	
#if (ENABLE_CONSOLE_DEBUG)
	if (thinkos_console_rt.tx_pipe.tail > 0x40000000) {
		DCC_LOG1(LOG_PANIC, "tail=%u", thinkos_console_rt.tx_pipe.tail);
		__THINKOS_ERROR(self, THINKOS_ERR_CONSOLE_FAULT);
	}
#endif

	/* (2) Save the context pointer. In case an interrupt wakes up
	   this thread before the scheduler is called, this will allow
	   the interrupt handler to locate the return value (r0) address. */
/* NEW: 2020-12-02 performed by the svc call entry stub 
	__thread_ctx_flush(krn, arg, self);
*/	
	switch (req) {

#if (THINKOS_ENABLE_CONSOLE_OPEN)
	case CONSOLE_OPEN:
		thinkos_console_rt.open_cnt++;
		arg[SVC_RETCODE] = THINKOS_OK;
		break;

	case CONSOLE_CLOSE:
		if (thinkos_console_rt.open_cnt > 0)
			arg[SVC_RETCODE] = THINKOS_EBADF;
		else
			arg[SVC_RETCODE] = THINKOS_OK;
		break;
#endif

#if (THINKOS_ENABLE_CONSOLE_MISC)
	case CONSOLE_IS_CONNECTED:
		DCC_LOG1(LOG_MSG, "CONSOLE_IS_CONNECTED(%d)", 
				thinkos_console_rt.connected);
		arg[SVC_RETCODE] = thinkos_console_rt.connected;
		break;
#endif

#if (THINKOS_ENABLE_CONSOLE_BREAK)
	case CONSOLE_IO_BREAK: 
		{
			unsigned int io = arg[1];
			unsigned int cnt = 0;

			if (io & CONSOLE_IO_WR)
				cnt += __console_wr_break(krn); 
			if (io & CONSOLE_IO_RD)
				cnt += __console_rd_break(krn); 
			if (cnt)
				__krn_sched_defer(krn); /* signal the scheduler ... */
		}
		break;
#endif

#if (THINKOS_ENABLE_CONSOLE_NONBLOCK)
	case CONSOLE_RD_NONBLOCK_SET: 
		thinkos_console_rt.rd_nonblock = arg[1] ? 1 : 0;
		break;

	case CONSOLE_WR_NONBLOCK_SET: 
		thinkos_console_rt.wr_nonblock = arg[1] ? 1 : 0;
		break;
#endif

	case CONSOLE_DRAIN:
#if (THINKOS_ENABLE_CONSOLE_DRAIN)
		thinkos_console_drain_svc(arg, self, krn);
#else
		arg[SVC_RETCODE] = THINKOS_OK;
#endif
		break;

#if (THINKOS_ENABLE_CONSOLE_MODE)
	case CONSOLE_RAW_MODE_SET:
		{
			bool val = arg[1];

			thinkos_console_rt.raw_mode = val ? 1 : 0;
			DCC_LOG1(LOG_INFO, "CONSOLE_RAW_MODE %s", val ? "true" : "false");
			arg[SVC_RETCODE] = THINKOS_OK;
			console_signal_ctl();
		}
		break;
#endif

	default:
		DCC_LOG1(LOG_ERROR, "invalid console request %d!", req);
		__THINKOS_ERROR(self, THINKOS_ERR_CONSOLE_REQINV);
		arg[SVC_RETCODE] = THINKOS_EINVAL;
		break;
	}
}

#if (THINKOS_ENABLE_CONSOLE_MODE)
bool thinkos_krn_console_is_raw_mode(void) 
{
	return thinkos_console_rt.raw_mode ? true : false;
}

void thinkos_krn_console_raw_mode_set(bool val) 
{
	DCC_LOG1(LOG_INFO, "raw_mode=%s", val ? "true" : "false");
	thinkos_console_rt.raw_mode = val;
}
#endif

void thinkos_krn_console_connect_set(bool val) 
{
	DCC_LOG1(LOG_INFO, "connected=%s", val ? "true" : "false");
	thinkos_console_rt.connected = val;
}

void thinkos_krn_console_reset(void)
{
	DCC_LOG1(LOG_INFO, "thinkos_console_rt=0x%08x", &thinkos_console_rt);
	__thinkos_memset32(&thinkos_console_rt, 0x00000000, 
					 sizeof(thinkos_console_rt));
	console_clear_tx_pipe();
	console_clear_rx_pipe();
	console_clear_ctl();
}

void thinkos_krn_console_init(void)
{
	thinkos_krn_console_reset();
}


#endif /* (THINKOS_ENABLE_CONSOLE) */


