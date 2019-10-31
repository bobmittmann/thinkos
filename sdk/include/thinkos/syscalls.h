/* 
 * thikos_svc.h
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
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */


#ifndef __THINKOS_SYSCALLS_H__
#define __THINKOS_SYSCALLS_H__

#define __THINKOS_PROFILE__
#include <thinkos/profile.h>
#include <stdint.h>
#include <sys/types.h>

/* -------------------------------------------------------------------------- 
 * Service numbers
 * --------------------------------------------------------------------------*/

#define THINKOS_THREAD_SELF            0
#define THINKOS_THREAD_CREATE          1

#define THINKOS_CLOCK                  2
#define THINKOS_ALARM                  3
#define THINKOS_SLEEP                  4

#define THINKOS_MUTEX_LOCK             5
#define THINKOS_MUTEX_TRYLOCK          6
#define THINKOS_MUTEX_TIMEDLOCK        7
#define THINKOS_MUTEX_UNLOCK           8

#define THINKOS_SEM_INIT               9
#define THINKOS_SEM_WAIT              10
#define THINKOS_SEM_TRYWAIT           11
#define THINKOS_SEM_TIMEDWAIT         12
#define THINKOS_SEM_POST              13

#define THINKOS_COND_WAIT             14
#define THINKOS_COND_TIMEDWAIT        15
#define THINKOS_COND_SIGNAL           16
#define THINKOS_COND_BROADCAST        17

#define THINKOS_FLAG_TAKE             18
#define THINKOS_FLAG_TIMEDTAKE        19
#define THINKOS_FLAG_GIVE             20

#define THINKOS_FLAG_VAL              21
#define THINKOS_FLAG_CLR              22
#define THINKOS_FLAG_SET              23
#define THINKOS_FLAG_WATCH            24
#define THINKOS_FLAG_TIMEDWATCH       25

#define THINKOS_GATE_WAIT             26
#define THINKOS_GATE_TIMEDWAIT        27
#define THINKOS_GATE_EXIT             28
#define THINKOS_GATE_OPEN             29
#define THINKOS_GATE_CLOSE            30

#define THINKOS_EVENT_WAIT            31
#define THINKOS_EVENT_TIMEDWAIT       32
#define THINKOS_EVENT_RAISE           33
#define THINKOS_EVENT_MASK            34
#define THINKOS_EVENT_CLEAR           35

#define THINKOS_CONSOLE               36

#define THINKOS_IRQ_WAIT              37
#define THINKOS_IRQ_TIMEDWAIT         38
#define THINKOS_IRQ_CTL               39

#define THINKOS_OBJ_ALLOC             40
#define THINKOS_OBJ_FREE              41

#define THINKOS_JOIN                  42
#define THINKOS_PAUSE                 43
#define THINKOS_RESUME                44

#define THINKOS_CANCEL                45
#define THINKOS_EXIT                  46
#define THINKOS_TERMINATE             47

#define THINKOS_CTL                   48

#define THINKOS_COMM                  49

#define THINKOS_IRQ_TIMEDWAIT_CLEANUP 50

#define THINKOS_ESCALATE              51

#define THINKOS_CRITICAL_ENTER        52

#define THINKOS_CRITICAL_EXIT         53

#define THINKOS_DBGMON                54

#define THINKOS_TRACE                 55
#define THINKOS_TRACE_CTL             56

#define THINKOS_FLASH_MEM             57

/* THINKOS_CONSOLE options */
#define CONSOLE_WRITE                  0
#define CONSOLE_READ                   1
#define CONSOLE_TIMEDREAD              2
#define CONSOLE_OPEN                   3
#define CONSOLE_CLOSE                  4
#define CONSOLE_DRAIN                  5
#define CONSOLE_IOCTL                  6
#define CONSOLE_IS_CONNECTED           7
#define CONSOLE_IO_BREAK               8
#define CONSOLE_RAW_MODE_SET           9
#define CONSOLE_RD_NONBLOCK_SET        10
#define CONSOLE_WR_NONBLOCK_SET        11

#define CONSOLE_IO_WR                  (1 << 0)
#define CONSOLE_IO_RD                  (1 << 1)

/* THINKOS_COMM options */
#define COMM_SEND                      0
#define COMM_RECV                      1

/* THINKOS_CTL options */
#define THINKOS_CTL_ABORT              0
#define THINKOS_CTL_UDELAY_FACTOR      1
#define THINKOS_CTL_CLOCKS             2
#define THINKOS_CTL_SNAPSHOT           3
#define THINKOS_CTL_TRACE              4
#define THINKOS_CTL_THREAD_INF         5
#define THINKOS_CTL_THREAD_CYCCNT      6
#define THINKOS_CTL_REBOOT             7
#define THINKOS_CTL_CYCCNT             8

/* THINKOS_IRQ_CTL options */
#define THINKOS_IRQ_DISABLE            0
#define THINKOS_IRQ_ENABLE             1
#define THINKOS_IRQ_PRIORITY_SET       2
#define THINKOS_IRQ_SYSCALLS_SET       3
#define THINKOS_IRQ_REGISTER           4

/* THINKOS_TRACE options */
#define THINKOS_TRACE_CLOSE            0
#define THINKOS_TRACE_OPEN             1
#define THINKOS_TRACE_READ             2
#define THINKOS_TRACE_FLUSH            3
#define THINKOS_TRACE_GETFIRST         4
#define THINKOS_TRACE_GETNEXT          5

/* THINKOS_FLASH_MEM options */
#define THINKOS_FLASH_MEM_CLOSE        0
#define THINKOS_FLASH_MEM_OPEN         1
#define THINKOS_FLASH_MEM_READ         2
#define THINKOS_FLASH_MEM_WRITE        3
#define THINKOS_FLASH_MEM_SEEK         4
#define THINKOS_FLASH_MEM_ERASE        5
#define THINKOS_FLASH_MEM_LOCK         6
#define THINKOS_FLASH_MEM_UNLOCK       7

#ifndef __ASSEMBLER__

/* ------------------------------------------------------------------------- 
 * C service call macros 
 * ------------------------------------------------------------------------- */

#define __ARM_SVC_ASM(N) "svc " #N "\n" 
#define ARM_SVC(N) __ARM_SVC_ASM(N)

#define __SYSCALL_CALL(N) __extension__({ register int ret asm("r0"); \
asm volatile ("svc " #N "\n" : "=r"(ret) : : ); \
ret; })

#define __SYSCALL_CALL1(N, A1) __extension__({ \
register int ret asm("r0"); \
register int r0 asm("r0") = (int)A1; \
asm volatile ("svc " #N "\n" : "=r"(ret) : "0"(r0) : ); \
ret; } )

#define __SYSCALL_CALL2(N, A1, A2) __extension__( \
{ register int ret asm("r0"); \
register int r0 asm("r0") = (int)A1; \
register int r1 asm("r1") = (int)A2; \
asm volatile ("svc " #N "\n" : "=r"(ret) : \
"0"(r0), "r"(r1) : ); \
ret; })

#define __SYSCALL_CALL3(N, A1, A2, A3) __extension__({ \
register int ret asm("r0"); \
register int r0 asm("r0") = (int)A1; \
register int r1 asm("r1") = (int)A2; \
register int r2 asm("r2") = (int)A3; \
asm volatile ("svc " #N "\n" : "=r"(ret) : \
	"0"(r0), "r"(r1), "r"(r2) : ); \
	ret; })

#define __SYSCALL_CALL4(N, A1, A2, A3, A4) __extension__({\
register int ret asm("r0"); \
register int r0 asm("r0") = (int)A1; \
register int r1 asm("r1") = (int)A2; \
register int r2 asm("r2") = (int)A3; \
register int r3 asm("r3") = (int)A4; \
asm volatile ("svc " #N "\n" : "=r"(ret) : \
		"0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); \
		ret; })

#define __SYSCALL_CALL5(N, A1, A2, A3, A4, A5) __extension__({\
register int ret asm("r0"); \
register int r0 asm("r0") = (int)A1; \
register int r1 asm("r1") = (int)A2; \
register int r2 asm("r2") = (int)A3; \
register int r3 asm("r3") = (int)A4; \
register int r12 asm("r12") = (int)A5; \
asm volatile ("svc " #N "\n" : "=r"(ret) : \
			"0"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r12) : ); \
			ret; })

/* No arguments function */
#define THINKOS_SYSCALL0(N) __SYSCALL_CALL(N)

/* One argument function */
#define THINKOS_SYSCALL1(N, A1) __SYSCALL_CALL1(N, (A1))

/* Two arguments function */
#define THINKOS_SYSCALL2(N, A1, A2) __SYSCALL_CALL2(N, (A1), (A2))

/* Three arguments function */
#define THINKOS_SYSCALL3(N, A1, A2, A3) __SYSCALL_CALL3(N, (A1), (A2), (A3))

/* Four arguments function */
#define THINKOS_SYSCALL4(N, A1, A2, A3, A4) __SYSCALL_CALL4(N, (A1), (A2), \
                                             (A3), (A4))
/* Five arguments function */
#define THINKOS_SYSCALL5(N, A1, A2, A3, A4, A5) __SYSCALL_CALL5(N, (A1), \
                                                (A2), (A3), (A4), (A5))

#ifdef __cplusplus
extern "C" {
#endif

static inline int __attribute__((always_inline)) thinkos_thread_self(void) {
	return THINKOS_SYSCALL0(THINKOS_THREAD_SELF);
}

static inline int __attribute__((always_inline)) 
thinkos_thread_create(int (* task)(void *), 
					  void * arg, void * stack_ptr,
					  unsigned int opt) {
	return THINKOS_SYSCALL5(THINKOS_THREAD_CREATE, task, arg, 
							 stack_ptr, opt, 0);
}

static inline int __attribute__((always_inline)) 
thinkos_thread_create_inf(int (* task)(void *), void * arg, 
						  const struct thinkos_thread_inf * inf) {
	return THINKOS_SYSCALL5(THINKOS_THREAD_CREATE, task, arg, 
							 inf->stack_ptr, inf->opt, inf);
}

static inline int 
__attribute__((always_inline)) thinkos_cancel(unsigned int thread, int code) {
	return THINKOS_SYSCALL2(THINKOS_CANCEL, thread, code);
}

static inline int 
__attribute__((always_inline)) thinkos_exit(int code) {
	return THINKOS_SYSCALL1(THINKOS_EXIT, code);
}

static inline int 
__attribute__((always_inline)) thinkos_terminate(unsigned int thread, int code) {
	return THINKOS_SYSCALL2(THINKOS_TERMINATE, thread, code);
}

static inline int 
__attribute__((always_inline)) thinkos_thread_abort(unsigned int thread) {
	return THINKOS_SYSCALL2(THINKOS_TERMINATE, thread, THINKOS_THREAD_ABORTED);
}

static inline int 
__attribute__((always_inline)) thinkos_join(unsigned int thread) {
	return THINKOS_SYSCALL1(THINKOS_JOIN, thread);
}

static inline int 
__attribute__((always_inline)) thinkos_pause(unsigned int thread) {
	return THINKOS_SYSCALL1(THINKOS_PAUSE, thread);
}

static inline int 
__attribute__((always_inline)) thinkos_resume(unsigned int thread) {
	return THINKOS_SYSCALL1(THINKOS_RESUME, thread);
}

static inline int
__attribute__((always_inline)) thinkos_sleep(unsigned int ms) {
	return THINKOS_SYSCALL1(THINKOS_SLEEP, ms);
}

static inline int
__attribute__((always_inline)) thinkos_alarm(uint32_t clk) {
	return THINKOS_SYSCALL1(THINKOS_ALARM, clk);
}

static inline uint32_t __attribute__((always_inline)) thinkos_clock(void) {
return THINKOS_SYSCALL0(THINKOS_CLOCK);
}

static inline int __attribute__((always_inline)) thinkos_obj_alloc(int kind) {
return THINKOS_SYSCALL1(THINKOS_OBJ_ALLOC, kind);
}

static inline int __attribute__((always_inline)) thinkos_obj_free(int obj) {
	return THINKOS_SYSCALL1(THINKOS_OBJ_FREE, obj);
}

static inline int __attribute__((always_inline)) 
thinkos_mutex_lock(int mutex) {
	return THINKOS_SYSCALL1(THINKOS_MUTEX_LOCK, mutex);
}

static inline int __attribute__((always_inline)) 
thinkos_mutex_trylock(int mutex) {
	return THINKOS_SYSCALL1(THINKOS_MUTEX_TRYLOCK, mutex);
}

static inline int __attribute__((always_inline)) 
thinkos_mutex_timedlock(int mutex, unsigned int ms) {
	return THINKOS_SYSCALL2(THINKOS_MUTEX_TIMEDLOCK, mutex, ms);
}

static inline int __attribute__((always_inline)) 
thinkos_mutex_unlock(int mutex) {
	return THINKOS_SYSCALL1(THINKOS_MUTEX_UNLOCK, mutex);
}


static inline int __attribute__((always_inline)) 
thinkos_cond_wait(int cond, int mutex) {
	int ret;
	if ((ret = THINKOS_SYSCALL2(THINKOS_COND_WAIT, 
								 cond, mutex)) >= THINKOS_ETIMEDOUT) {
		THINKOS_SYSCALL1(THINKOS_MUTEX_LOCK, mutex);
	}
	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_cond_timedwait(int cond, int mutex, unsigned int ms) {
	int ret;
	if ((ret = THINKOS_SYSCALL3(THINKOS_COND_TIMEDWAIT, 
								 cond, mutex, ms)) >= THINKOS_ETIMEDOUT) { 
		THINKOS_SYSCALL1(THINKOS_MUTEX_LOCK, mutex);
	}
	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_cond_signal(int cond) {
	return THINKOS_SYSCALL1(THINKOS_COND_SIGNAL, cond);
}

static inline int __attribute__((always_inline)) 
thinkos_cond_broadcast(int cond) {
	return THINKOS_SYSCALL1(THINKOS_COND_BROADCAST, cond);
}


static inline int __attribute__((always_inline)) 
thinkos_sem_init(int sem, unsigned int value) {
	return THINKOS_SYSCALL2(THINKOS_SEM_INIT, sem, value);
}

static inline int __attribute__((always_inline)) thinkos_sem_wait(int sem) {
return THINKOS_SYSCALL1(THINKOS_SEM_WAIT, sem);
}

static inline int __attribute__((always_inline)) thinkos_sem_trywait(int sem) {
return THINKOS_SYSCALL1(THINKOS_SEM_TRYWAIT, sem);
}

static inline int __attribute__((always_inline))
thinkos_sem_timedwait(int sem, unsigned int ms) {
	return THINKOS_SYSCALL2(THINKOS_SEM_TIMEDWAIT, sem, ms);
}

static inline int __attribute__((always_inline)) thinkos_sem_post(int sem) {
return THINKOS_SYSCALL1(THINKOS_SEM_POST, sem);
}

static inline void  __attribute__((always_inline)) thinkos_sem_post_i(int sem) {
uintptr_t * except = (uintptr_t *)(0);
void (* sem_post_i)(int) = (void (*)(int))except[7];
sem_post_i(sem);
}

/* --------------------------------------------------------------------------
*  Event sets
* --------------------------------------------------------------------------*/

static inline int __attribute__((always_inline)) thinkos_ev_wait(int set) {
return THINKOS_SYSCALL1(THINKOS_EVENT_WAIT, set);
}

static inline int __attribute__((always_inline)) thinkos_ev_timedwait(
																  int set, unsigned int ms) {
return THINKOS_SYSCALL2(THINKOS_EVENT_TIMEDWAIT, set, ms);
}

static inline int __attribute__((always_inline)) thinkos_ev_raise(
															  int set, int ev) {
return THINKOS_SYSCALL2(THINKOS_EVENT_RAISE, set, ev);
}

static inline int __attribute__((always_inline)) thinkos_ev_mask(
															 int set, int ev, int val) {
return THINKOS_SYSCALL3(THINKOS_EVENT_MASK, set, ev, val);
}

static inline int __attribute__((always_inline)) thinkos_ev_clear(
															  int set, int ev) {
return THINKOS_SYSCALL2(THINKOS_EVENT_CLEAR, set, ev);
}

static inline void __attribute__((always_inline)) thinkos_ev_raise_i(
																 int set, int ev) {
uintptr_t * except = (uintptr_t *)(0);
void (* ev_raise_i)(int, int) = (void (*)(int, int))except[9];
ev_raise_i(set, ev);
}

/* ---------------------------------------------------------------------------
Flags
----------------------------------------------------------------------------*/

static inline int __attribute__((always_inline)) thinkos_flag_set(int flag) {
return THINKOS_SYSCALL1(THINKOS_FLAG_SET, flag);
}

static inline int __attribute__((always_inline)) thinkos_flag_clr(int flag) {
return THINKOS_SYSCALL1(THINKOS_FLAG_CLR, flag);
}

static inline int __attribute__((always_inline)) thinkos_flag_val(int flag) {
return THINKOS_SYSCALL1(THINKOS_FLAG_VAL, flag);
}

static inline int __attribute__((always_inline)) thinkos_flag_watch(int flag) {
return THINKOS_SYSCALL1(THINKOS_FLAG_WATCH, flag);
}

static inline int __attribute__((always_inline)) 
thinkos_flag_timedwatch(int flag, unsigned int ms) {
	return THINKOS_SYSCALL2(THINKOS_FLAG_TIMEDWATCH, flag, ms);
}

static inline int __attribute__((always_inline)) thinkos_flag_give(int flag) {
return THINKOS_SYSCALL1(THINKOS_FLAG_GIVE, flag);
}

static inline int __attribute__((always_inline)) thinkos_flag_take(int flag) {
return THINKOS_SYSCALL1(THINKOS_FLAG_TAKE, flag);
}

static inline int __attribute__((always_inline)) 
thinkos_flag_timedtake(int flag, unsigned int ms) {
	return THINKOS_SYSCALL2(THINKOS_FLAG_TIMEDTAKE, flag, ms);
}

static inline void __attribute__((always_inline)) 
thinkos_flag_give_i(int flag) {
	uintptr_t * except = (uintptr_t *)(0);
	void (* flag_give_i)(int) = (void (*)(int))except[10];
	flag_give_i(flag);
}

/* ---------------------------------------------------------------------------
Gates
---------------------------------------------------------------------------*/


static inline int __attribute__((always_inline)) 
thinkos_gate_open(int gate) {
	return THINKOS_SYSCALL1(THINKOS_GATE_OPEN, gate);
}

static inline int __attribute__((always_inline)) 
thinkos_gate_close(int gate) {
	return THINKOS_SYSCALL1(THINKOS_GATE_CLOSE, gate);
}

static inline int __attribute__((always_inline)) 
thinkos_gate_exit(int gate, unsigned int open) {
	return THINKOS_SYSCALL2(THINKOS_GATE_EXIT, gate, open);
}

static inline int __attribute__((always_inline)) 
thinkos_gate_wait(int gate) {
	return THINKOS_SYSCALL1(THINKOS_GATE_WAIT, gate);
}

static inline int __attribute__((always_inline)) 
thinkos_gate_timedwait(int gate, unsigned int ms) {
	return THINKOS_SYSCALL2(THINKOS_GATE_TIMEDWAIT, gate, ms);
}

static inline void __attribute__((always_inline)) 
thinkos_gate_open_i(int gate) {
	uintptr_t * except = (uintptr_t *)(0);
	void (* __gate_open_i)(uint32_t) = (void (*)(uint32_t))except[13];
	__gate_open_i(gate);
}

/* ---------------------------------------------------------------------------
IRQ
---------------------------------------------------------------------------*/

static inline int __attribute__((always_inline)) 
	thinkos_irq_timedwait(int irq, unsigned int ms) {

	register uint32_t err asm("r0");
	register uint32_t cyccnt asm("r1");

	asm volatile (ARM_SVC(THINKOS_IRQ_TIMEDWAIT) : 
				  "=r"(err), "=r"(cyccnt) :
				  "0"(irq), "1"(ms) );

	asm volatile (ARM_SVC(THINKOS_IRQ_TIMEDWAIT_CLEANUP) : 
				  "=r"(err) : "0"(irq) );

	return err;
}

static inline int __attribute__((always_inline)) thinkos_irq_wait(int irq) {
	register uint32_t err asm("r0");
	register uint32_t cyccnt asm("r1");

	asm volatile (ARM_SVC(THINKOS_IRQ_WAIT) : 
				  "=r"(err), "=r"(cyccnt) : "0"(irq) );

	return err;
}

static inline int __attribute__((always_inline)) 
	thinkos_irq_wait_cyccnt(int irq, uint32_t * pcyccnt) {

	register uint32_t err asm("r0");
	register uint32_t cyccnt asm("r1");

	asm volatile (ARM_SVC(THINKOS_IRQ_WAIT) : 
				  "=r"(err), "=r"(cyccnt) : "0"(irq) );

	*pcyccnt = cyccnt;

	return err;
}

static inline int __attribute__((always_inline)) 
	thinkos_irq_register(int irq, unsigned int pri, void (* isr)(void)) {
		return THINKOS_SYSCALL4(THINKOS_IRQ_CTL, THINKOS_IRQ_REGISTER, 
								irq, pri, isr);
	}

static inline int __attribute__((always_inline)) 
thinkos_irq_enable(int irq) {
	return THINKOS_SYSCALL2(THINKOS_IRQ_CTL, THINKOS_IRQ_ENABLE, irq);
}

static inline int __attribute__((always_inline)) 
thinkos_irq_disable(int irq) {
	return THINKOS_SYSCALL2(THINKOS_IRQ_CTL, THINKOS_IRQ_DISABLE, irq);
}

static inline int __attribute__((always_inline)) 
thinkos_irq_priority_set(int irq, unsigned int pri) {
	return THINKOS_SYSCALL3(THINKOS_IRQ_CTL, 
							 THINKOS_IRQ_PRIORITY_SET, irq, pri);
}

#include <arch/cortex-m3.h>



/* ---------------------------------------------------------------------------
   Console
   ---------------------------------------------------------------------------*/

static inline int __attribute__((always_inline)) 
thinkos_console_write(const void * buf, unsigned int len) {
	return THINKOS_SYSCALL3(THINKOS_CONSOLE, CONSOLE_WRITE, buf, len);
}

static inline int __attribute__((always_inline)) 
thinkos_console_ioctl(unsigned int ioctl, void * arg, unsigned int size) {
	return THINKOS_SYSCALL4(THINKOS_CONSOLE, CONSOLE_IOCTL, ioctl, arg, size);
}

static inline int __attribute__((always_inline)) 
thinkos_console_is_connected(void) {
	return THINKOS_SYSCALL1(THINKOS_CONSOLE, CONSOLE_IS_CONNECTED);
}

static inline int __attribute__((always_inline)) 
thinkos_console_read(void * buf, unsigned int len) {
	return THINKOS_SYSCALL3(THINKOS_CONSOLE, CONSOLE_READ, buf, len);
}

static inline int __attribute__((always_inline)) 
thinkos_console_timedread(void * buf, unsigned int len, unsigned int ms) {
	return THINKOS_SYSCALL4(THINKOS_CONSOLE, CONSOLE_TIMEDREAD, buf, len, ms);
}

static inline int __attribute__((always_inline)) 
thinkos_console_close(void) {
	return THINKOS_SYSCALL1(THINKOS_CONSOLE, CONSOLE_CLOSE);
}

static inline int __attribute__((always_inline)) 
thinkos_console_drain(void) {
	return THINKOS_SYSCALL1(THINKOS_CONSOLE, CONSOLE_DRAIN);
}

static inline int __attribute__((always_inline)) 
thinkos_console_io_break(unsigned int which) {
	return THINKOS_SYSCALL2(THINKOS_CONSOLE, CONSOLE_IO_BREAK, which);
}

static inline int __attribute__((always_inline)) 
thinkos_console_raw_mode(unsigned int enable) {
	return THINKOS_SYSCALL2(THINKOS_CONSOLE, CONSOLE_RAW_MODE_SET, enable);
}

static inline int __attribute__((always_inline)) 
thinkos_console_rd_nonblock(unsigned int enable) {
	return THINKOS_SYSCALL2(THINKOS_CONSOLE, CONSOLE_RD_NONBLOCK_SET, enable);
}

static inline int __attribute__((always_inline)) 
thinkos_console_wr_nonblock(unsigned int enable) {
	return THINKOS_SYSCALL2(THINKOS_CONSOLE, CONSOLE_WR_NONBLOCK_SET, enable);
}

/* ---------------------------------------------------------------------------
   OS Monitor and Control 
   ---------------------------------------------------------------------------*/

static inline int __attribute__((always_inline)) 
	thinkos_clocks(uint32_t * clk[]) {
		return THINKOS_SYSCALL2(THINKOS_CTL, THINKOS_CTL_CLOCKS, clk);
	}

static inline int __attribute__((always_inline)) 
	thinkos_udelay_factor(int32_t * factor) {
		return THINKOS_SYSCALL2(THINKOS_CTL, THINKOS_CTL_UDELAY_FACTOR, factor);
	}

static inline void __attribute__((always_inline, noreturn)) 
	thinkos_abort(void) {
		for (;;) {
		THINKOS_SYSCALL1(THINKOS_CTL, THINKOS_CTL_ABORT);
		}
	}

/* XXX: Deprecated 
static inline int __attribute__((always_inline))
	thinkos_trace(const char * msg) {
		return THINKOS_SYSCALL2(THINKOS_CTL, THINKOS_CTL_TRACE, msg);
	}
 */

static inline int __attribute__((always_inline))
	thinkos_reboot(uint32_t key) {
		return THINKOS_SYSCALL2(THINKOS_CTL, THINKOS_CTL_REBOOT, key);
	}

static inline int __attribute__((always_inline)) 
	thinkos_rt_snapshot(void * rt) {
		return THINKOS_SYSCALL2(THINKOS_CTL, THINKOS_CTL_SNAPSHOT, rt);
	}

static inline int __attribute__((always_inline)) 
	thinkos_thread_inf(const struct thinkos_thread_inf * inf[], 
					   unsigned int max) {
		return THINKOS_SYSCALL3(THINKOS_CTL, THINKOS_CTL_THREAD_INF, inf, max);
	}

static inline int __attribute__((always_inline)) 
	thinkos_thread_cyccnt(uint32_t cyccnt[], unsigned int max) {
		return THINKOS_SYSCALL3(THINKOS_CTL, THINKOS_CTL_THREAD_CYCCNT, cyccnt, max);
	}

static inline uint32_t __attribute__((always_inline)) thinkos_cyccnt(void) {
		return THINKOS_SYSCALL1(THINKOS_CTL, THINKOS_CTL_CYCCNT);
	}

struct dbgmon_comm;

static inline int __attribute__((always_inline)) 
	thinkos_dbgmon(void (* task)(const struct dbgmon_comm *, void *), 
				   const struct dbgmon_comm * comm, void * param) {
	return THINKOS_SYSCALL3(THINKOS_DBGMON, task, comm, param);
}

static inline int __attribute__((always_inline)) 
	thinkos_critical_enter(void) {
		return THINKOS_SYSCALL0(THINKOS_CRITICAL_ENTER);
	}

static inline int __attribute__((always_inline)) 
	thinkos_critical_exit(void) {
		return THINKOS_SYSCALL0(THINKOS_CRITICAL_EXIT);
	}

/* ---------------------------------------------------------------------------
   Communication channel
   ---------------------------------------------------------------------------*/

static inline int __attribute__((always_inline)) 
thinkos_comm_send(uint32_t hdr, const void * buf, unsigned int len) {
	return THINKOS_SYSCALL4(THINKOS_COMM, COMM_SEND, hdr, buf, len);
}

static inline int __attribute__((always_inline)) 
thinkos_comm_recv(uint32_t * hdr, void * buf, unsigned int len) {
	return THINKOS_SYSCALL4(THINKOS_COMM, COMM_RECV, hdr, buf, len);
}

/* ---------------------------------------------------------------------------
   In Kernel Trace Buffer
   ---------------------------------------------------------------------------*/

struct trace_ref;
struct trace_entry;

static inline int __attribute__((always_inline)) 
thinkos_trace(const struct trace_ref * ref) {
	return THINKOS_SYSCALL1(THINKOS_TRACE, ref);
}

static inline int __attribute__((always_inline)) 
thinkos_trace_open(void) {
	return THINKOS_SYSCALL1(THINKOS_TRACE_CTL, 
							 THINKOS_TRACE_OPEN);
}

static inline int __attribute__((always_inline)) 
thinkos_trace_close(int id) {
	return THINKOS_SYSCALL2(THINKOS_TRACE_CTL, 
							 THINKOS_TRACE_CLOSE, id);
}

static inline int __attribute__((always_inline)) 
thinkos_trace_read(int id, uint32_t * buf, unsigned int len) {
	return THINKOS_SYSCALL4(THINKOS_TRACE_CTL, 
							 THINKOS_TRACE_READ, id, buf, len);
}

static inline int __attribute__((always_inline)) 
thinkos_trace_flush(int id) {
	return THINKOS_SYSCALL2(THINKOS_TRACE_CTL, 
							 THINKOS_TRACE_FLUSH, id);
}

static inline int __attribute__((always_inline)) 
thinkos_trace_getfirst(int id, struct trace_entry * entry) {
	return THINKOS_SYSCALL3(THINKOS_TRACE_CTL, 
							 THINKOS_TRACE_GETFIRST, id, entry);
}

static inline int __attribute__((always_inline)) 
thinkos_trace_getnext(int id, struct trace_entry * entry) {
	return THINKOS_SYSCALL3(THINKOS_TRACE_CTL, 
							 THINKOS_TRACE_GETNEXT, id, entry);
}

/* ---------------------------------------------------------------------------
   Flash Memory
   ---------------------------------------------------------------------------*/

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_close(int mem) {
	return THINKOS_SYSCALL4(THINKOS_FLASH_MEM, THINKOS_FLASH_MEM_CLOSE, mem, 
							0, 0);
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_open(const char * tag) {
	return THINKOS_SYSCALL4(THINKOS_FLASH_MEM, THINKOS_FLASH_MEM_OPEN, 
							tag, 0, 0);
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_read(int mem, void * buf, size_t len) {
	return THINKOS_SYSCALL4(THINKOS_FLASH_MEM, THINKOS_FLASH_MEM_READ, 
							mem, buf, len);
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_write(int mem, const void * buf, size_t len) {
	return THINKOS_SYSCALL4(THINKOS_FLASH_MEM, THINKOS_FLASH_MEM_WRITE, 
							mem, buf, len);
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_seek(int mem, off_t offset) {
	return THINKOS_SYSCALL4(THINKOS_FLASH_MEM, THINKOS_FLASH_MEM_SEEK, 
							mem, offset, 0);
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_erase(int mem, off_t offset, size_t len) {
	return THINKOS_SYSCALL4(THINKOS_FLASH_MEM, THINKOS_FLASH_MEM_ERASE, mem, 
							offset, len);
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_lock(int mem, off_t offset, size_t len) {
	return THINKOS_SYSCALL4(THINKOS_FLASH_MEM, THINKOS_FLASH_MEM_LOCK, mem, 
							offset, len);
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_unlock(int mem, off_t offset, size_t len) {
	return THINKOS_SYSCALL4(THINKOS_FLASH_MEM, THINKOS_FLASH_MEM_UNLOCK, mem, 
							offset, len);
}

/* ---------------------------------------------------------------------------
   Other
   ---------------------------------------------------------------------------*/

static inline void thinkos_yield(void) {
	CM3_SCB->icsr = SCB_ICSR_PENDSVSET; /* PendSV rise */
	asm volatile ("dsb\n"); /* Data synchronization barrier */
}

static inline int thinkos_escalate(int (* call)(void *), void * arg) {
	return THINKOS_SYSCALL2(THINKOS_ESCALATE, call, arg);
}

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_SYSCALLS_H__ */

