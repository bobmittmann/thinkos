/* 
 * thinkos_svc.h
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

#ifndef __THINKOS_SYSCALLS__
#error "Never use <thinkos/syscalls.h> directly; include <thinkos.h> instead."
#endif 

/* -------------------------------------------------------------------------- 
 * Service numbers
 * --------------------------------------------------------------------------*/

#define THINKOS_THREAD_SELF            0
#define THINKOS_THREAD_INIT            1

#define THINKOS_CLOCK                  2
#define THINKOS_ALARM                  3
#define THINKOS_SLEEP                  4

#define THINKOS_CTL                    5

#define THINKOS_IRQ_WAIT               6
#define THINKOS_IRQ_TIMEDWAIT          7
#define THINKOS_IRQ_TIMEDWAIT_FIXUP    8
#define THINKOS_IRQ_CTL                9

#define THINKOS_OBJ_ALLOC             10
#define THINKOS_OBJ_FREE              11

#define THINKOS_MUTEX_LOCK            12
#define THINKOS_MUTEX_TRYLOCK         13
#define THINKOS_MUTEX_TIMEDLOCK       14
#define THINKOS_MUTEX_UNLOCK          15

#define THINKOS_SEM_INIT              16
#define THINKOS_SEM_WAIT              17
#define THINKOS_SEM_TRYWAIT           18
#define THINKOS_SEM_TIMEDWAIT         19
#define THINKOS_SEM_POST              20

#define THINKOS_COND_WAIT             21
#define THINKOS_COND_TIMEDWAIT        22
#define THINKOS_COND_SIGNAL           23
#define THINKOS_COND_BROADCAST        24

#define THINKOS_FLAG_TAKE             25
#define THINKOS_FLAG_TIMEDTAKE        26
#define THINKOS_FLAG_GIVE             27

#define THINKOS_FLAG_VAL              28
#define THINKOS_FLAG_CLR              29
#define THINKOS_FLAG_SET              30 
#define THINKOS_FLAG_WATCH            31
#define THINKOS_FLAG_TIMEDWATCH       32

#define THINKOS_GATE_WAIT             33
#define THINKOS_GATE_TIMEDWAIT        34
#define THINKOS_GATE_EXIT             35
#define THINKOS_GATE_OPEN             36
#define THINKOS_GATE_CLOSE            37

#define THINKOS_EVENT_WAIT            38
#define THINKOS_EVENT_TIMEDWAIT       39
#define THINKOS_EVENT_RAISE           40
#define THINKOS_EVENT_MASK            41
#define THINKOS_EVENT_CLEAR           42

#define THINKOS_CANCEL                43
#define THINKOS_TERMINATE             44
#define THINKOS_JOIN                  45
#define THINKOS_PAUSE                 46
#define THINKOS_RESUME                47
#define THINKOS_EXIT                  48

#define THINKOS_CONSOLE_CTL           49
#define THINKOS_CONSOLE_SEND          50
#define THINKOS_CONSOLE_RECV          51
#define THINKOS_CONSOLE_TIMED_FIXUP   52

#define THINKOS_COMM_CTL              53
#define THINKOS_COMM_SEND             54
#define THINKOS_COMM_RECV             55
#define THINKOS_COMM_TIMED_FIXUP      56

#define THINKOS_DATE_AND_TIME         57

#define THINKOS_CRITICAL_ENTER        58
#define THINKOS_CRITICAL_EXIT         59

#define THINKOS_MONITOR_CTL           60

#define THINKOS_TRACE                 61
#define THINKOS_TRACE_CTL             62

#define THINKOS_FLASH_MEM             63

#define THINKOS_APP_EXEC              64

#define THINKOS_SYSCALL_CNT           64

/* THINKOS_CONSOLE options */
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

/* THINKOS_CTL options */
#define THINKOS_CTL_ABORT              0
#define THINKOS_CTL_UDELAY_FACTOR      1
#define THINKOS_CTL_CLOCKS             2
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

/* THINKOS_TRACE operations */
#define THINKOS_TRACE_CLOSE            0
#define THINKOS_TRACE_OPEN             1
#define THINKOS_TRACE_READ             2
#define THINKOS_TRACE_FLUSH            3
#define THINKOS_TRACE_GETFIRST         4
#define THINKOS_TRACE_GETNEXT          5

/* THINKOS_FLASH_MEM operations */
#define THINKOS_FLASH_MEM_NOP          0
#define THINKOS_FLASH_MEM_OPEN         1
#define THINKOS_FLASH_MEM_CLOSE        2
#define THINKOS_FLASH_MEM_READ         3
#define THINKOS_FLASH_MEM_WRITE        4
#define THINKOS_FLASH_MEM_ERASE        5
#define THINKOS_FLASH_MEM_LOCK         6
#define THINKOS_FLASH_MEM_UNLOCK       7
#define THINKOS_FLASH_MEM_STAT         8

/* THINKOS_DATE_AND_TIME operations */
#define THINKOS_TIME_MONOTONIC_GET     0
#define THINKOS_TIME_REALTIME_GET      1
#define THINKOS_TIME_MONOTONIC_SET     2
#define THINKOS_TIME_REALTIME_SET      3
#define THINKOS_TIME_REALTIME_STEP     4
#define THINKOS_TIME_REALTIME_COMP     5

/* THINKOS_COMM_CTL operations */
#define THINKOS_COMM_OPEN              0
#define THINKOS_COMM_CLOSE             1


/* THINKOS_MONITOR_CTL operations */
#define MONITOR_CTL_TASK_INIT          0

#ifndef __ASSEMBLER__

#define __THINKOS_PROFILE__
#include <thinkos/profile.h>
#define __THINKOS_TIME__
#include <thinkos/time.h>

#include <stdint.h>
#include <sys/types.h>
#include <arch/cortex-m3.h>


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

static inline int 
thinkos_thread_init(unsigned int thread, 
                    const struct thinkos_thread_initializer * ini) {

	register uint32_t ret asm("r0");
	register void * ptr asm("r1") = (void *)ini;

	asm volatile (ARM_SVC(THINKOS_THREAD_INIT) : 
				  "=r"(ret) : "0"(thread), "r"(ptr) : "memory" );

	return ret;
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

static inline void __attribute__((always_inline, noreturn)) 
	thinkos_thread_abort(int code) {
	register uint32_t r0 asm("r0") = 0;
	register int32_t r1 asm("r1") = code;
	asm volatile (ARM_SVC(THINKOS_TERMINATE) : : "r"(r0), "r"(r1));
	for(;;);
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

static inline int __attribute__((always_inline)) thinkos_alarm(uint32_t clk) {
	return THINKOS_SYSCALL1(THINKOS_ALARM, clk);
}

static inline uint32_t  thinkos_clock(void) {
return THINKOS_SYSCALL0(THINKOS_CLOCK);
}

static inline int  thinkos_obj_alloc(int kind) {
return THINKOS_SYSCALL1(THINKOS_OBJ_ALLOC, kind);
}

static inline int thinkos_obj_thread_alloc(int hint) {
return THINKOS_SYSCALL2(THINKOS_OBJ_ALLOC, THINKOS_OBJ_THREAD, hint);
}

static inline int thinkos_obj_free(int obj) {
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

/* --------------------------------------------------------------------------
 * Flags
 * --------------------------------------------------------------------------*/

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

/* --------------------------------------------------------------------------
 * Gates
 * --------------------------------------------------------------------------
 */

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

/* --------------------------------------------------------------------------
 * IRQ
 * --------------------------------------------------------------------------
 */

static inline int __attribute__((always_inline)) thinkos_irq_wait(int irq) {
	register uint32_t ret asm("r0");
	register uint32_t cyccnt asm("r1");

	asm volatile (ARM_SVC(THINKOS_IRQ_WAIT) : 
				  "=r"(ret), "=r"(cyccnt) : "0"(irq) );

	return ret;
}

static inline int __attribute__((always_inline)) 
	thinkos_irq_wait_cyccnt(int irq, uint32_t * pcyccnt) {

	register uint32_t ret asm("r0");
	register uint32_t cyccnt asm("r1");

	asm volatile (ARM_SVC(THINKOS_IRQ_WAIT) : 
				  "=r"(ret), "=r"(cyccnt) : "0"(irq) );

	*pcyccnt = cyccnt;

	return ret;
}

static inline int __attribute__((always_inline)) 
	thinkos_irq_timedwait_cyccnt(int irq, unsigned int ms, uint32_t * pcyccnt) {

	register uint32_t ret asm("r0");
	register uint32_t cyccnt asm("r1");

	asm volatile (ARM_SVC(THINKOS_IRQ_TIMEDWAIT)
				  ARM_SVC(THINKOS_IRQ_TIMEDWAIT_FIXUP) : 
				  "=r"(ret), "=r"(cyccnt) :
				  "0"(irq), "1"(ms) );

	*pcyccnt = cyccnt;

	return ret;
}

static inline int __attribute__((always_inline)) 
	thinkos_irq_timedwait(int irq, unsigned int ms) {

	register uint32_t ret asm("r0");
	register uint32_t cyccnt asm("r1");

	asm volatile (ARM_SVC(THINKOS_IRQ_TIMEDWAIT)
				  ARM_SVC(THINKOS_IRQ_TIMEDWAIT_FIXUP) : 
				  "=r"(ret), "=r"(cyccnt) :
				  "0"(irq), "1"(ms) );

	return ret;
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

/* --------------------------------------------------------------------------
 * Console
 * --------------------------------------------------------------------------
 */

static inline ssize_t __attribute__((always_inline)) 
thinkos_console_write(const void * buf, size_t len) {
	register int32_t ret asm("r12");
	register uintptr_t r0 asm("r0") = (uintptr_t)buf;
	register uint32_t r1 asm("r1") = len;
	register uint32_t r2 asm("r2") = 0;
	asm volatile (ARM_SVC(THINKOS_CONSOLE_SEND) : 
				  "=r"(ret) : "r"(r0), "r"(r1), "r"(r2) : "memory" );
	return ret;
}

static inline ssize_t __attribute__((always_inline)) 
thinkos_console_read(void * buf, size_t len) {
	register int32_t ret asm("r12");
	register uintptr_t r0 asm("r0") = (uintptr_t)buf;
	register uint32_t r1 asm("r1") = len;
	register int32_t r2 asm("r2") = 0;
	asm volatile (ARM_SVC(THINKOS_CONSOLE_RECV) 
				  ARM_SVC(THINKOS_CONSOLE_TIMED_FIXUP) : 
				  "=r"(ret) : "r"(r0), "r"(r1), "r"(r2) : "memory" );
	return ret;
}

static inline ssize_t __attribute__((always_inline)) 
thinkos_console_timedread(void * buf, size_t len, int32_t ms) {
	register int32_t ret asm("r12");
	register uintptr_t r0 asm("r0") = (uintptr_t)buf;
	register uint32_t r1 asm("r1") = len;
	register uint32_t r2 asm("r2") = ms;
	asm volatile (ARM_SVC(THINKOS_CONSOLE_RECV) 
//				  ARM_SVC(THINKOS_CONSOLE_TIMED_FIXUP) 
				  : "=r"(ret) : "r"(r0), "r"(r1), "r"(r2) : "memory" );
	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_console_ioctl(unsigned int ioctl, void * arg, uint32_t size) {
	register int32_t ret asm("r12");
	register uint32_t r0 asm("r0") = ioctl;
	register uintptr_t r1 asm("r1") = (uintptr_t)arg;
	register uint32_t r2 asm("r2") = size;
	asm volatile (ARM_SVC(THINKOS_CONSOLE_CTL) 
				  : "=r"(ret) : "r"(r0), "r"(r1), "r"(r2));
	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_console_is_connected(void) {
	return THINKOS_SYSCALL1(THINKOS_CONSOLE_CTL, CONSOLE_IS_CONNECTED);
}

static inline int __attribute__((always_inline)) 
thinkos_console_close(void) {
	return THINKOS_SYSCALL1(THINKOS_CONSOLE_CTL, CONSOLE_CLOSE);
}

static inline int __attribute__((always_inline)) 
thinkos_console_drain(void) {
	return THINKOS_SYSCALL1(THINKOS_CONSOLE_CTL, CONSOLE_DRAIN);
}

static inline int __attribute__((always_inline)) 
thinkos_console_io_break(unsigned int which) {
	return THINKOS_SYSCALL2(THINKOS_CONSOLE_CTL, 
							CONSOLE_IO_BREAK, which);
}

static inline int __attribute__((always_inline)) 
thinkos_console_raw_mode(unsigned int enable) {
	return THINKOS_SYSCALL2(THINKOS_CONSOLE_CTL, 
							CONSOLE_RAW_MODE_SET, enable);
}

static inline int __attribute__((always_inline)) 
thinkos_console_rd_nonblock(unsigned int enable) {
	return THINKOS_SYSCALL2(THINKOS_CONSOLE_CTL, 
							CONSOLE_RD_NONBLOCK_SET, enable);
}

static inline int __attribute__((always_inline)) 
thinkos_console_wr_nonblock(unsigned int enable) {
	return THINKOS_SYSCALL2(THINKOS_CONSOLE_CTL, 
							CONSOLE_WR_NONBLOCK_SET, enable);
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

static inline int __attribute__((always_inline))
	thinkos_reboot(uint32_t key) {
		return THINKOS_SYSCALL2(THINKOS_CTL, THINKOS_CTL_REBOOT, key);
	}

static inline int __attribute__((always_inline)) 
	thinkos_thread_inf(const struct thinkos_thread_inf * inf[], 
					   unsigned int from, unsigned int cnt) {
		return THINKOS_SYSCALL3(THINKOS_CTL, THINKOS_CTL_THREAD_INF, 
								inf, ((from << 16) + cnt));
	}

static inline int __attribute__((always_inline)) 
	thinkos_thread_cyccnt(uint32_t cyccnt[], unsigned int from, unsigned int cnt) {
		return THINKOS_SYSCALL3(THINKOS_CTL, THINKOS_CTL_THREAD_CYCCNT, 
								cyccnt, ((from << 16) + cnt));
	}

static inline uint32_t __attribute__((always_inline)) thinkos_cyccnt(void) {
		return THINKOS_SYSCALL1(THINKOS_CTL, THINKOS_CTL_CYCCNT);
	}

struct monitor_comm;

static inline int __attribute__((always_inline)) 
	thinkos_monitor_ctl(unsigned int opc, uintptr_t arg1, uintptr_t arg2, 
						uintptr_t arg3) {
	return THINKOS_SYSCALL4(THINKOS_MONITOR_CTL, opc, arg1, arg2, arg3);
}

static inline int __attribute__((always_inline)) 
	thinkos_monitor(void (* task)(const struct monitor_comm *, void *), 
				   const struct monitor_comm * comm, void * env) {
	return THINKOS_SYSCALL4(THINKOS_MONITOR_CTL, MONITOR_CTL_TASK_INIT, 
							task, comm, env);
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

#define __COMM_OPEN_OPC(DEVNO) ((THINKOS_COMM_OPEN << 24) + DEVNO)

static inline int __attribute__((always_inline)) 
thinkos_comm_open(unsigned int devno) {
	register int32_t ret asm("r12");
	register int32_t opc asm("r0") = __COMM_OPEN_OPC(devno);
	asm volatile (ARM_SVC(THINKOS_COMM_CTL) : "=r"(ret) : "r"(opc) : );
	return ret;
}

static inline ssize_t __attribute__((always_inline)) 
thinkos_comm_send(unsigned int comm, const void * buf, size_t len) {
	register int32_t ret asm("r12");
	register uint32_t r0 asm("r0") = comm;
	register uint32_t r1 asm("r1") = (uintptr_t)buf;
	register uint32_t r2 asm("r2") = len;
	register uint32_t r3 asm("r3") = 0;
	asm volatile (ARM_SVC(THINKOS_COMM_SEND) : "=r"(ret) : 
				  "r"(r0), "r"(r1), "r"(r2), "r"(r3) : "memory" );
	return ret;
}

static inline ssize_t __attribute__((always_inline)) 
thinkos_comm_timedsend(unsigned int comm, const void * buf, size_t len,
					   uint32_t tmo) {
	register int32_t ret asm("r12");
	register uint32_t r0 asm("r0") = comm;
	register uint32_t r1 asm("r1") = (uintptr_t)buf;
	register uint32_t r2 asm("r2") = len;
	register uint32_t r3 asm("r3") = tmo;
	asm volatile (ARM_SVC(THINKOS_COMM_SEND) 
				  ARM_SVC(THINKOS_COMM_TIMED_FIXUP) : "=r"(ret) : 
				  "r"(r0), "r"(r1), "r"(r2), "r"(r3) : "memory" );
	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_comm_timedrecv(unsigned int comm, void * buf, unsigned int len,
					   unsigned int tmo) {
	register int32_t ret asm("r12");
	register uint32_t r0 asm("r0") = comm;
	register uint32_t r1 asm("r1") = (uintptr_t)buf;
	register uint32_t r2 asm("r2") = len;
	register uint32_t r3 asm("r3") = tmo;
	asm volatile (ARM_SVC(THINKOS_COMM_TIMEDRECV) 
				  ARM_SVC(THINKOS_COMM_TIMED_FIXUP) : "=r"(ret) : 
				  "r"(r0), "r"(r1), "r"(r2), "r"(r3) : "memory" );
	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_comm_recv(unsigned int comm, void * buf, unsigned int len) {
	register int32_t ret asm("r0");
	register uint32_t r1 asm("r1") = (uintptr_t)buf;
	register uint32_t r2 asm("r2") = len;
	asm volatile (ARM_SVC(THINKOS_COMM_RECV) : "=r"(ret) : 
				  "0"(comm), "r"(r1), "r"(r2) : "memory" );
	return ret;
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

#define __FLASH_OPC(OP, KEY) ((OP << 24) + KEY)

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_open(const char * tag) {
	uint32_t opc = __FLASH_OPC(THINKOS_FLASH_MEM_OPEN, 0);
	register int32_t ret asm("r0");
	register uint32_t r1 asm("r1") = (uintptr_t)tag;
	asm volatile (ARM_SVC(THINKOS_FLASH_MEM) : "=r"(ret) : 
				  "0"(opc), "r"(r1) : "memory" );
	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_close(int key) {
return THINKOS_SYSCALL1(THINKOS_FLASH_MEM, 
                        __FLASH_OPC(THINKOS_FLASH_MEM_CLOSE, key));
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_read(int key, off_t offset, void * buf, size_t size) {
	uint32_t opc = __FLASH_OPC(THINKOS_FLASH_MEM_READ, key);
	register int32_t ret asm("r0");
	register uint32_t r1 asm("r1") = (uint32_t)offset;
	register uint32_t r2 asm("r2") = size;
	register uint32_t r3 asm("r3") = (uintptr_t)buf;

	asm volatile (ARM_SVC(THINKOS_FLASH_MEM) : "=r"(ret) : 
				  "0"(opc), "r"(r1), "r"(r2) , "r"(r3) : "memory" );

	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_write(int key, off_t offset, const void * buf, size_t size) {
	uint32_t opc = __FLASH_OPC(THINKOS_FLASH_MEM_WRITE, key);
	register int32_t ret asm("r0");
	register uint32_t r1 asm("r1") = (uint32_t)offset;
	register uint32_t r2 asm("r2") = size;
	register uint32_t r3 asm("r3") = (uintptr_t)buf;

	asm volatile (ARM_SVC(THINKOS_FLASH_MEM) : "=r"(ret) : 
				  "0"(opc), "r"(r1), "r"(r2) , "r"(r3) : "memory" );

	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_erase(int key, off_t offset, size_t size) {
	return THINKOS_SYSCALL3(THINKOS_FLASH_MEM, 
							__FLASH_OPC(THINKOS_FLASH_MEM_ERASE, key),
							offset, size);
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_lock(int key, off_t offset, size_t size) {
	return THINKOS_SYSCALL3(THINKOS_FLASH_MEM,
							__FLASH_OPC(THINKOS_FLASH_MEM_LOCK, key),
							offset, size);
}

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_unlock(int key, off_t offset, size_t size) {
	return THINKOS_SYSCALL3(THINKOS_FLASH_MEM, 
							__FLASH_OPC(THINKOS_FLASH_MEM_UNLOCK, key),
							offset, size);
}

struct thinkos_mem_stat;

static inline int __attribute__((always_inline)) 
thinkos_flash_mem_stat(const char * tag, struct thinkos_mem_stat * stat) {
	uint32_t opc = __FLASH_OPC(THINKOS_FLASH_MEM_STAT, 0);
	register int32_t ret asm("r0");
	register uint32_t r1 asm("r1") = (uintptr_t)tag;
	register uint32_t r2 asm("r2") = (uintptr_t)stat;
	asm volatile (ARM_SVC(THINKOS_FLASH_MEM) : "=r"(ret) : 
				  "0"(opc), "r"(r1), "r"(r2) : "memory" );
	return ret;
}

/* ---------------------------------------------------------------------------
   Date and Time
   ---------------------------------------------------------------------------*/

static inline uint64_t __attribute__((always_inline)) 
thinkos_time_monotonic_get(void) {
	register uint32_t t_lo asm("r0");
	register uint32_t t_hi asm("r1");
	union krn_time tm;

	asm volatile (ARM_SVC(THINKOS_DATE_AND_TIME) : "=r"(t_lo), "=r"(t_hi) : 
				  "0"(THINKOS_TIME_MONOTONIC_GET));
	tm.frac = t_lo;
	tm.sec = t_hi;
	return tm.u64;
}

static inline uint64_t __attribute__((always_inline)) 
thinkos_time_realtime_get(void) {
	register uint32_t t_lo asm("r0");
	register uint32_t t_hi asm("r1");
	union krn_time tm;

	asm volatile (ARM_SVC(THINKOS_DATE_AND_TIME) : "=r"(t_lo), "=r"(t_hi) : 
				  "0"(THINKOS_TIME_REALTIME_GET));
	tm.frac = t_lo;
	tm.sec = t_hi;
	return tm.u64;
}

static inline int __attribute__((always_inline)) 
thinkos_time_realtime_set(uint64_t t) {
	register uint32_t ret asm("r0");
	register uint32_t r1 asm("r1") = t;
	register uint32_t r2 asm("r2") = t >> 32;

	asm volatile (ARM_SVC(THINKOS_DATE_AND_TIME) : "=r"(ret) : 
				  "0"(THINKOS_TIME_REALTIME_SET), "r"(r1), "r"(r2));
	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_time_realtime_step(int64_t dt) {
	register uint32_t ret asm("r0");
	register uint32_t r1 asm("r1") = dt;
	register int32_t r2 asm("r2") = dt >> 32;

	asm volatile (ARM_SVC(THINKOS_DATE_AND_TIME) : "=r"(ret) : 
				  "0"(THINKOS_TIME_REALTIME_STEP), "r"(r1), "r"(r2));
	return ret;
}

static inline int __attribute__((always_inline)) 
thinkos_time_realtime_comp(uint64_t dt) {
	register uint32_t ret asm("r0");
	register uint32_t r1 asm("r1") = dt;
	register uint32_t r2 asm("r2") = dt >> 32;

	asm volatile (ARM_SVC(THINKOS_DATE_AND_TIME) : "=r"(ret) : 
				  "0"(THINKOS_TIME_REALTIME_COMP), "r"(r1), "r"(r2));
	return ret;
}

/* ---------------------------------------------------------------------------
   Other
   ---------------------------------------------------------------------------*/

static inline void thinkos_bkpt(int no) {
	asm volatile ("bkpt %0" : : "I" (no) );
}

/* ---------------------------------------------------------------------------
   Application support
   ---------------------------------------------------------------------------*/

static inline int __attribute__((always_inline)) 
thinkos_app_exec(uintptr_t addr, uintptr_t arg0, uintptr_t arg1, 
				 uintptr_t arg2, uintptr_t arg3) {
	register int32_t ret asm("r12");
	register uintptr_t r0 asm("r0") = arg0;
	register uintptr_t r1 asm("r1") = arg1;
	register uintptr_t r2 asm("r2") = arg2;
	register uintptr_t r3 asm("r2") = arg3;
	asm volatile (ARM_SVC(THINKOS_APP_EXEC) : 
				  "=r"(ret) : "r"(r0), "r"(r1), "r"(r2), "r"(r3), "0"(addr) : );
	return ret;
}

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_SYSCALLS_H__ */

