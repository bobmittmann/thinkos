/* parse-markup: reST */
  
/* 
 * thinkos.h
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/* 
 * @file thinkos.h
 * @brief ThinkOS API
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __THINKOS_H__
#define __THINKOS_H__

#define __THINKOS_VERSION__
#include <thinkos/version.h>

#define IRQ_PRIORITY_HIGHEST   (1 << 5)
#define IRQ_PRIORITY_VERY_HIGH (2 << 5)
#define IRQ_PRIORITY_HIGH      (3 << 5)
#define IRQ_PRIORITY_REGULAR   (4 << 5)
#define IRQ_PRIORITY_LOW       (5 << 5)
#define IRQ_PRIORITY_VERY_LOW  (6 << 5)

#define THINKOS_OPT_PRIORITY(VAL)   (((VAL) & 0xff) << 16)
#define THINKOS_OPT_ID(VAL)         (((VAL) & 0x03f) << 24)
#define THINKOS_OPT_PRIVILEGED      (1 << 30) /* execution privilege */
#define THINKOS_OPT_PAUSED          (1 << 31) /* don't run at startup */
#define THINKOS_OPT_STACK_SIZE(VAL) ((VAL) & 0xffff)

#ifndef __ASSEMBLER__

#include <stdint.h>

/** 
 * enum thinkos_err - System call and library error codes
 * @THINKOS_OK: No error 
 * @THINKOS_EPERM: Invalid permission 
 * @THINKOS_ENOSYS: Invalid or not implemented system call 
 * @THINKOS_EFAULT: General fault 
 * @THINKOS_ENOMEM: Resource pool exausted 
 * @THINKOS_ETIMEDOUT: System call timed out 
 * @THINKOS_EINTR: System call interrupted out 
 * @THINKOS_EINVAL: Invalid argument 
 * @THINKOS_EAGAIN: Non blocking call failed 
 * @THINKOS_EBADF: Closing console if not open
 * @THINKOS_EDEADLK: Deadlock condition detected 
 * @THINKOS_EBUSY: Device driver busy
 */
enum thinkos_err {
	THINKOS_OK        =  0, /**< No error */
	THINKOS_ETIMEDOUT = -1, /**< System call timed out */
	THINKOS_EINTR     = -2, /**< System call interrupted out */
	THINKOS_EINVAL    = -3, /**< Invalid argument */
	THINKOS_EAGAIN    = -4, /**< Non blocking call failed */
	THINKOS_EDEADLK   = -5, /**< Deadlock condition detected */
	THINKOS_EPERM     = -6,
	THINKOS_ENOSYS    = -7, /**< Invalid system call */
	THINKOS_EFAULT    = -8,        
	THINKOS_ENOMEM    = -9,  /**< Resource pool exausted */ 
	THINKOS_EBADF     = -10,  /**< Closing console if not open */ 
	THINKOS_EBUSY     = -11  /**< Closing console if not open */ 
};

/** 
 * enum thinkos_obj_kind - Kernel object class. 
 * @THINKOS_OBJ_READY: Ready queue
 * @THINKOS_OBJ_TMSHARE: time share waiting queue 
 * @THINKOS_OBJ_CLOCK: clock waiting queue 
 * @THINKOS_OBJ_MUTEX: mutex waiting queue 
 * @THINKOS_OBJ_COND: conditional variable waiting queue 
 * @THINKOS_OBJ_SEMAPHORE: semaphore waiting queue 
 * @THINKOS_OBJ_EVENT: event waiting queue 
 * @THINKOS_OBJ_FLAG: flag waiting queue 
 * @THINKOS_OBJ_GATE: gate waiting queue 
 * @THINKOS_OBJ_JOIN: thread join waiting queue 
 * @THINKOS_OBJ_CONREAD : console read waiting queue 
 * @THINKOS_OBJ_CONWRITE: console write waiting queue 
 * @THINKOS_OBJ_PAUSED: thread paused list 
 * @THINKOS_OBJ_CANCELED: thread canceled list 
 * @THINKOS_OBJ_COMMSEND  : comm channel send waiting queue 
 * @THINKOS_OBJ_COMMRECV: comm channel recv waiting queue 
 * @THINKOS_OBJ_IRQ: IRQ (Interrupt request) waiting queue 
 * @THINKOS_OBJ_DMA: IRQ (Direct Memory Access) waiting queue 
 * @THINKOS_OBJ_FLASH_MEM: Flesh Memory operation waiting queue 
 * @THINKOS_OBJ_FAULT: thread fault list 
 * @THINKOS_OBJ_INVALID: invalid object 
 */

enum thinkos_obj_kind {
	THINKOS_OBJ_READY     = 0,
	THINKOS_OBJ_TMSHARE   = 1,
	THINKOS_OBJ_CLOCK     = 2,
	THINKOS_OBJ_MUTEX     = 3,
	THINKOS_OBJ_COND      = 4,
	THINKOS_OBJ_SEMAPHORE = 5,
	THINKOS_OBJ_EVENT     = 6,
	THINKOS_OBJ_FLAG      = 7,
	THINKOS_OBJ_GATE      = 8,
	THINKOS_OBJ_JOIN      = 9,
	THINKOS_OBJ_CONREAD   = 10,
	THINKOS_OBJ_CONWRITE  = 11,
	THINKOS_OBJ_PAUSED    = 12,
	THINKOS_OBJ_CANCELED  = 13,
	THINKOS_OBJ_COMMSEND  = 14,
	THINKOS_OBJ_COMMRECV  = 15,
	THINKOS_OBJ_IRQ       = 16,
	THINKOS_OBJ_DMA       = 17,
	THINKOS_OBJ_FLASH_MEM = 18,
	THINKOS_OBJ_FAULT     = 19,
	THINKOS_OBJ_INVALID
};

/* -------------------------------------------------------------------------- 
 * Flattened thread state structure
 * --------------------------------------------------------------------------*/

struct cortex_m_context {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;

	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;

	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;

	uint32_t r12;
	uint32_t sp;
	uint32_t lr;
	uint32_t pc;

	uint32_t xpsr;
};

/* 
 * FIXME: this should be called struct thinkos_thread_inf... 
 */
struct thinkos_thread {
	uint32_t no: 6;
	uint32_t tmw: 1;
	uint32_t alloc: 1;
	uint16_t wq;
	uint8_t  sched_val;
	uint8_t  sched_pri;
	uint32_t clock;
	uint32_t cyccnt;
	const struct thinkos_thread_inf * inf;
};

struct thinkos_thread_inf {
	void * stack_ptr;
	uint32_t stack_size;
	uint8_t priority;
	uint8_t thread_id: 6;
	uint8_t privileged: 1;
	uint8_t paused: 1;
	char tag[10];
};

/** 
 * struct thinkos_thread_attr - Thread attributes. 
 * @stack_addr: Pointer to the base of the stack.
 * @stack_size: Size of the stak in bytes 
 * @priority: Round robin thread priority (only used if timeshared is enabled)
 * @thread_id: Requested thread number (may differ from the run-time thread 
 *             number if dynamic allocation is enabled)
 * @paused: Indicate the initial state of the thread.
 * @tag: thread name
 *
 */

/** 
 * typedef thinkos_task_t - Thread task function type.
 * Thread entry function template.
 *
 */

typedef int (* thinkos_task_t)(void * arg, unsigned int id);

/* ThinkOS Thread Task type cast macro */
#define C_TASK(__FUN) (int (*)(void *, unsigned int))(uintptr_t)(__FUN)
/* ThinkOS Thread Argument type cast macro */
#define C_ARG(__PTR) (void *)(uintptr_t)(__PTR)

/* ThinkOS Thread Stack Declaration cast macro */
#define THINKOS_THREAD_STACK(__SYM, __LEN, __SEC) \
	uint32_t __SYM[(((__LEN) + 7) /8)] \
	__attribute_ ((aligned(8), section(__SEC)))

/*
 * usage:
 *
 * Declare 1024 bytes of stack in the ccm region.
 * THINKOS_THREAD_STACK(my_stack, 1024, "ccm");
 *
 * thinkos_thread_create(TT_TASK(my_task), TT_ARG(my_arg), 
 *                       TT_STACK(my_stak), sizeof(my_stack));
 *    
 *
 */

/** 
 * struct thinkos_thread_init - Thread initializer. 
 * @task: Task startup function.
 * @arg: Parameter to the task
 * @attr: Static thread attributes.
 */

struct thinkos_thread_initializer;

#define __THINKOS_SYSCALLS__
#include <thinkos/syscalls.h>
#define __THINKOS_MEMORY__
#include <thinkos/memory.h>
#define __THINKOS_THREAD__
#include <thinkos/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 *  Kernel 
 * ---------------------------------------------------------------------------
 */

/** 
 * thinkos_krn_init() - Initializes the ThinkOS_ kernel.
 * @opt:  Optional arguments
 * @map:  Optional Memory Map
 * @lst:  Optional initial thread list
 * 
 * Return:
 * %THINKOS_EFAULT in case of a hardware initialization 
 * issue. Otherwise the thread id (number) is returned.
 *
 * On return the current program execution thread turns into the first 
 * thread of the system. 
 *
 */

int thinkos_krn_init(unsigned int opt, const struct thinkos_mem_map * map,
					 const struct thinkos_thread_initializer * lst[]);

/**
 * thinkos_krn_nrt_init() - Initializes the ThinkOS non-real-time extension.
 *
 * Starts the NRT scheduler and return it's thread id.
 * Return:
 * %THINKOS_OK
 */

/**
 * thinkos_krn_userland() - Switch processor to user mode.
 *
 */
void thinkos_krn_userland(void);

/**
 * thinkos_app_exec() - replace the current thread...
 * @addr: aplication location
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_app_exec(uint32_t addr);


/* ---------------------------------------------------------------------------
 *  Object Allocation
 * ---------------------------------------------------------------------------
 */

/**
 * thinkos_obj_alloc() - Allocate a kernel object.
 * @kind: specify the objcet type.
 *
 * Return:
 * %THINKOS_ENOSYS_ if call is not implemented. Otherwwise a positive
 * integer value corresponding to the object id (handler).
 */
int thinkos_obj_alloc(int kind);

/**
 * thinkos_obj_free() - Release a kernel object.
 * @obj: the object handler.
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_obj_free(int obj);


/* ---------------------------------------------------------------------------
 *  Threads
 * ---------------------------------------------------------------------------
 */

/**
 * thinkos_thread_create() - Create a new thread
 *
 * @task_ptr: thread's entry point function
 * @task_arg: argument to be passed to the entry function
 * @stack_ptr: pointer to a stack memory block
 * @opt: thread options 
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_thread_create(thinkos_task_t task_ptr, void * task_arg, 
						  void * stack_ptr, unsigned int opt);


/**
 * thinkos_thread_create_inf() - create a new thread with extend information.
 * @task_ptr: thread's entry point function
 * @task_arg: argument to be passed to the entry function
 * @inf: thread information block
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_thread_create_inf(thinkos_task_t task_ptr, void * task_arg,
							  const struct thinkos_thread_inf * inf);


/**
 * thinkos_thread_self() - obtaind a handler for the calling thread.
 *
 * Return:
 * This function always succeeds, returning the calling thread's ID.
 */
int thinkos_thread_self(void);

/**
 * thinkos_cancel() - request a thread to terminate.
 * @thread_id: thread handler
 * @code: the thread return code
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_cancel(unsigned int thread_id, int code);

/**
 * thinkos_exit() - cause the calling thread to terminate.
 * @code: the thread return code.
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_exit(int code);

/**
 * thinkos_abort() - abort all threads.
 * 
 * This call stops the user threads. It can be used in conjunction with 
 * the ThinkOS Monitor.
 *
 * Return:
 * This call does not return.
 */
void __attribute__((noreturn)) thinkos_abort(void);

/**
 * thinkos_terminate() - cause the thread to terminate.
 * @thread_id: thread handler
 * @code: the thread return code.
 *
 * If the thread_id is 0 then the current thread is terminated. In which
 * case the call will not return.
 * 
 * Return:
 * If thread_id is the current thread or 0 the call will not return.
 * %THINKOS_EINVAL if the thread does not exist.
 * %THINKOS_ENOSYS if call is not implemented. 
 * %THINKOS_OK otherwise.
 *  
 */
int thinkos_terminate(unsigned int thread, int code);

/**
 * thinkos_thread_abort() - cause the current thread to terminate 
 * imediatelly.
 * @code: the thread return code.
 *
 * Return:
 * This call does not return.
 */
void __attribute__((noreturn)) thinkos_thread_abort(int code);


/**
 * thinkos_join() - join with a terminated thread.
 * @thread_id: thread handler
 *
 * Description:
 * The thinkos_join() function waits for the thread specified by thread_id 
 * to terminate. If that thread has already terminated, then 
 * thinkos_join() returns immediately.
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_join(unsigned int thread_id);

/**
 * thinkos_pause() - pause the thread execution.
 *
 * @thread_id: thread handler
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_pause(unsigned int thread_id);

/**
 * thinkos_resume() - resume a thread previously paused.
 *
 * @thread_id: thread handler
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_resume(unsigned int thread_id);


/**
 * thinkos_yield() - causes the calling thread to relinquish the CPU.
 *
 * The thread is moved to the end of the queue for its static priority 
 * and a new thread gets to run.
 */
void thinkos_yield(void);


/** @defgroup time Time related calls
 *
 * @{
 */

/**
 * thinkos_sleep() - pause for a specified amount of time.
 * @ms: waiting time in milliseconds
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 *
 * Description:
 * The thinkos_sleep() function suspends execution of the calling thread 
 * for (at least) ms milliseconds. The sleep may be lengthened slightly by 
 * any system activity or by the time spent processing the call or by the 
 * granularity of system timer.
 *
 */
int thinkos_sleep(unsigned int ms);

/**
 * thinkos_clock() - retrieve the ThikOS clock time.
 *
 * Return:
 * the ThinkOS monotonic clock.
 *
 * Description:
 * The ThikOS clock is a tick counter with a resolution of 1 milisecond, 
 * which cannot be set and is allways running.
 * It represents monotonic time since the system power up.
 */
uint32_t thinkos_clock(void);

/**
 * thinkos_alarm() - wait for an absolute clock time.
 * @clock: time for the thread to be waked up.
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 *
 * Description:
 * This function should be used in conjucntion with thinkos_clock().
 *
 */
int thinkos_alarm(uint32_t clock);



/** 
 * DOC: Mutexes
 *
 *
 */

/**
 * thinkos_mutex_alloc() - alloc a mutex object.
 *
 * Return:
 * %THINKOS_ENOSYS_ if call is not implemented. Otherwwise a positive
 * integer value corresponding to the object id (handler).
 */
int thinkos_mutex_alloc(void);

/**
 * thinkos_mutex_free() - release a mutex object.
 * @mutex: mutual exclusion handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_mutex_free(int mutex);

/**
 * thinkos_mutex_lock() - lock a mutex, blocking if already locked.
 * @mutex: mutual exclusion handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_mutex_lock(int mutex);

/**
 * thinkos_mutex_trylock() - try to lock a mutex, non blocking.
 * @mutex: mutual exclusion handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_mutex_trylock(int mutex);

/**
 * thinkos_mutex_timedlock() - lock a mutex, blocking for limited time.
 * @mutex: mutual exclusion handler
 * @ms: waiting time in milliseconds
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_mutex_timedlock(int mutex, unsigned int ms);

/**
 * thinkos_mutex_unlock() - unlock the mutex.
 *
 * @mutex: mutual exclusion handler
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_mutex_unlock(int mutex);



/** 
 * DOC: Conditional Variables
 *
 */

/**
 * thinkos_cond_alloc() - alloc a conditional variable.
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_cond_alloc(void);

/**
 * thinkos_cond_free() - release a conditional variable.
 * @cond: conditional variable handler
 *
 * Return:
 * %THINKOS_ENOSYS_ if call is not implemented. Otherwwise a positive
 * integer value corresponding to the object id (handler).
 */
int thinkos_cond_free(int cond);

/**
 * thinkos_cond_wait() - wait for a conditional variable.
 * @cond: conditional variable handler
 * @mutex: mutex handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_cond_wait(int cond, int mutex);

/**
 * thinkos_cond_timedwait() - wait for a conditional variable or timeout.
 * @cond: conditional variable handler
 * @mutex: mutex handler
 * @ms: waiting time in milliseconds
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_cond_timedwait(int cond, int mutex, unsigned int ms);

/**
 * thinkos_cond_signal() - signal a conditional variable wake a single thread.
 * @cond: conditional variable handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_cond_signal(int cond);

/**
 * thinkos_cond_broadcast() - signal a conditional variable wake all 
 * waiting threads.
 * @cond: conditional variable handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_cond_broadcast(int cond);


/* ---------------------------------------------------------------------------
 * Semaphores
 * ---------------------------------------------------------------------------
 */

/**
 * thinkos_sem_alloc() - alloc a semaphore.
 * @val: initial semaphore value
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_sem_alloc(unsigned int val);

/**
 * thinkos_sem_free() - release a semaphore.
 * @sem: semaphore handler
 *
 * Return:
 * %THINKOS_ENOSYS_ if call is not implemented. Otherwwise a positive
 * integer value corresponding to the object id (handler).
 */
int thinkos_sem_free(int sem);

/**
 * thinkos_sem_init() - set the initial value of a semaphore.
 * @sem: semaphore handler
 * @val: initial semaphore value
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_sem_init(int sem, unsigned int val);

/**
 * thinkos_sem_wait() - wait for a semaphore to be signaled.
 * @sem: semaphore handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_sem_wait(int sem);

/**
 * thinkos_sem_timedwait() - wait for a semaphore to be signaled or timeout.
 * @sem: semaphore handler
 * @ms: time to wait in milliseconds
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_sem_timedwait(int sem, unsigned int ms);

/**
 * thinkos_sem_post() - signal a semaphore.
 * @sem: semaphore handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_sem_post(int sem);

/**
 * thinkos_sem_post_i() - signal a semaphore inside an interrupt handler.
 * @sem: semaphore handler
 */
void thinkos_sem_post_i(int sem);


/* ---------------------------------------------------------------------------
 * Event sets
 * ---------------------------------------------------------------------------
 */

/**
 * thinkos_ev_alloc() - alloc an event set.
 *
 * Return:
 * %THINKOS_ENOSYS_ if call is not implemented. Otherwwise a positive
 * integer value corresponding to the object id (handler).
 */
int thinkos_ev_alloc(void);

/**
 * thinkos_ev_free() - release an event set.
 * @set: event set handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_ev_free(int set);

/**
 * thinkos_ev_wait() - wait for an event.
 * @set: event set handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, an event otherwise. 
 */
int thinkos_ev_wait(int set);

/**
 * thinkos_ev_timedwait() - wait for an event or timeout.
 * @set: event set handler
 * @ms: time to wait in milliseconds
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, an event otherwise. 
 */
int thinkos_ev_timedwait(int set, unsigned int ms);

/**
 * thinkos_ev_raise() - signal an event.
 * @set: event set handler
 * @ev: event identifier
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_ev_raise(int set, int ev);

/**
 * thinkos_ev_raise_i() - signal an event from inside an interrupt handler.
 *
 * @set: event set handler
 * @ev: event identifier
 */
void thinkos_ev_raise_i(int set, int ev);

/**
 * thinkos_ev_mask() - mask/unmask an event.
 * @set: event set handler
 * @ev: event identifier
 * @val: mask value. 1 enable the event, 0 disable the event.
 * Return:
 *
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_ev_mask(int set, int ev, int val);

/**
 * thinkos_ev_clear() - clear an event.
 * @set: event set handler
 * @ev: event identifier
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_ev_clear(int set, int ev);



/** 
 * Flags
 *
 */

/**
 * thinkos_flag_alloc() - alloc a flag.
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_flag_alloc(void);

/**
 * thinkos_flag_free() - release a flag
 * @flag: flag object handler
 * Return:
 * %THINKOS_ENOSYS_ if call is not implemented. Otherwwise a positive
 * integer value corresponding to the object id (handler).
 */
int thinkos_flag_free(int flag);

/**
 * thinkos_flag_val() - return the flag value.
 * @flag: flag object handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_flag_val(int flag);

/**
 * thinkos_flag_set() - set a flag.
 * @flag: flag object handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_flag_set(int flag);

/**
 * thinkos_flag_clr() - clear a flag.
 * @flag: flag object handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_flag_clr(int flag);

/**
 * thinkos_flag_give() - signal a flag.
 * @flag: flag object handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_flag_give(int flag);

/**
 * thinkos_flag_give_i() - signal a flag form inside an interrupt handler.
 * @flag: flag object handler
 *
 */
void thinkos_flag_give_i(int flag);

/**
 * thinkos_flag_take() - wait for flag to be signaled.
 * @flag: flag object handler
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_flag_take(int flag);

/**
 * thinkos_flag_timedtake() - wait for flag or timeout.
 * @flag: flag object handler
 * @ms: timeout in milliseconds 
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_flag_timedtake(int flag, unsigned int ms);
/**@}*/


/** 
 * DOC: Gates
 *
 * Gates are syncronization objects which provide a convenient way of 
 * creating mutual exclusion acess to 
 * code blocks signaled by interrupt handlers...
 * 
 * A gate have a lock flag and a signal flag. A gate can be in one of 
 * the following states:
 * - @b CLOSED: no threads crossed the gate yet. 
 * - @b LOCKED: a thread entered the gate, closed and locked it.
 * - @b OPENED: no threads are waiting in the gate, the first thread to
 * call @c thinkos_gate_wait() will cross the gate.
 * - @b SIGNALED: a thread crossed the gate and locked it, but the gate
 * received a signal to open. When the thread exits the gate the gate will
 * stay open.
 * @{
 */

/**
 * thinkos_gate_alloc() - Alloc a gate synchronization object.
 *
 * Return:
 * return a handler for a new gate object, or a negative value if
 * an error ocurred.
 *
 * Errors:
 * - %THINKOS_ENOSYS if the system call is not enabled.
 * - %THINKOS_ENOMEM no gates left in the gate pool.
 */
int thinkos_gate_alloc(void);

/**
 * thinkos_gate_free() - Frees the gate synchronization object.
 * @gate: handler for a gate object which must have been returned by 
 * a previous call to @c thinkos_gate_alloc().
 *
 * Return:
 * returns %THINKOS_OK on sucess. On error a negative code value is returned.
 * an error ocurred.
 *
 * Errors:
 * - %THINKOS_EINVAL @p gate is not a valid gate handler.
 * - %THINKOS_ENOSYS not implemented.
 */
int thinkos_gate_free(int gate);

/**
 * thinkos_gate_wait() - Wait for a gate to open.
 * @gate: The gate descriptor.
 *
 * Return:
 * %THINKOS_EINVAL if @p gate is invalid, %THINKOS_OK otherwise. 
 *
 * Description:
 * If the gate is open this function return imediatelly, otherwise it will
 * block the calling thread.
 *
 */
int thinkos_gate_wait(int gate);

/**
 * thinkos_gate_timedwait() - Wait for a gate to open or a timeout.
 * @gate: The gate descriptor.
 * @ms: Timeout ins milliseconds.
 * Return:
 * - %THINKOS_OK is returned on sucess. On error a negative code value 
 * is returned.
 * - %THINKOS_EINVAL: @p gate is not a valid handler.
 * - %THINKOS_ETIMEDOUT: timer expired before the @p gate opens.
 * - %THINKOS_ENOSYS: syscall not implemented.
 *
 *
 * Description:
 * If the gate is open this function return imediatelly, otherwise it will
 * block the calling thread.
 *
 */
int thinkos_gate_timedwait(int gate, unsigned int ms);

/**
 * thinkos_gate_open() - Open or signal the gate.
 * @gate: The gate descriptor.
 *
 * Return:
 * %THINKOS_EINVAL if @p gate is invalid, %THINKOS_OK otherwise. 
 * 
 * Description:
 * The resulting gate's state will depend on the current gate state and 
 * whether there are threads waiting at the gate. There are four possible
 * scenarios ... :
 * -# the gate is open already, then this function does nothing.
 * -# the gate is closed and no threads are waiting it will open the gate,
 * allowing for the next thread calling @c gate_open() to enter the gate.
 * -# the gate is closed and at least one thread is waiting then it will allow 
 * the thread to cross the gate, in this case the gate will be locked.
 * -# a thread crossed the gate (gate state is @b LOCKED), then the gate
 * will be signaled to open when the gate is unlocked.
 */
int thinkos_gate_open(int gate);

/**
 * thinkos_gate_open_i() - Open or signal the gate from inside an 
 *                         interrupt handler.
 * @gate: The gate descriptor.
 *
 * Description:
 * This call is similar to the thinkos_gate_open() except that it
 * is safe to ba called from inside an interrupt handler.
 *
 * Warning: no argument validation is performed. 
 */
void thinkos_gate_open_i(int gate);


/**
 * thinkos_gate_close() - Close the gate if the gate is @b OPEN or 
 * remove pending signaling if the gate is @ LOCKED.
 * @gate: The gate descriptor.
 * Return:
 * %THINKOS_EINVAL if @p gate is invalid, %THINKOS_OK otherwise. 
 */
int thinkos_gate_close(int gate);

/**
 * thinkos_gate_exit() - Exit the gate, optionally leaving it open or closed.
 * @gate: The gate descriptor.
 * @open: Indicate the state of the gate on exit. 
 * - @p open > 0, the gate will be left open, allowing for another thread 
 * to enter the gate.
 * - @p open == 0, the gate will stay closed if not signaled, in wich case
 * it will open accordingly.
 *
 * Return:
 * %THINKOS_EINVAL if @p gate is invalid, %THINKOS_OK otherwise. 
 */
int thinkos_gate_exit(int gate, unsigned int open);

/**@}*/


/** @defgroup irq Interrupt Requests
 *
 * @{
 */

/**
 * thinkos_irq_wait() - wait for interrupt.
 * @irq: interrupt request number
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_irq_wait(int irq);

/**
 * thinkos_irq_timedwait() - wait for interrupt or timeout.
 * @irq: interrupt request number
 * @ms: timeout time in milliseconds.
 *
 * Return:
 * - %THINKOS_OK is returned on sucess. 
 * - %THINKOS_EINVAL: @p irq is not valid handler.
 * - %THINKOS_ETIMEDOUT: timer expired before the @p gate opens.
 * - %THINKOS_ENOSYS: syscall not implemented.
 */
int thinkos_irq_timedwait(int irq, unsigned int ms);

/**
 * thinkos_irq_register() - reqister an interrupt handler.
 * @irq: interrupt request number
 * @pri: pirority
 * @isr: interrupt service routine
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int	thinkos_irq_register(int irq, unsigned int pri, void (* isr)(void));
/**@}*/


/** @defgroup ctl OS Monitor and Control.
 *
 * These calls return information about the ThinkOS configuratiion
 * and operational parameters.
 *
 * @{
 */

/**
 * thinkos_clocks() - get hardware clocks frequencies. 
 * @clk:[] pointer to an array to receive the clocks frequency list.
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_clocks(uint32_t * clk[]);

/**
 * thinkos_udelay_factor() - get udelay calibration factor.
 * @factor: pointer to an integer
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_udelay_factor(int32_t * factor);

int thinkos_critical_enter(void);

int thinkos_critical_exit(void);

int thinkos_escalate(int (* call)(void *), void * arg);


/** @defgroup trace Real-time trace kernel support
 *
 * ...
 *
 * An user library -- libtrace -- is needed to take advantage of
 * the kernel trace ring.
 *
 * @{
 */

struct trace_entry;
struct trace_ref;

int thinkos_trace(const struct trace_ref * ref);

int thinkos_trace_open(void);

int thinkos_trace_close(int id);

int thinkos_trace_read(int id, uint32_t * buf, unsigned int len);

int thinkos_trace_flush(int id);

int thinkos_trace_getfirst(int id, struct trace_entry * entry);

int thinkos_trace_getnext(int id, struct trace_entry * entry);

/**@}*/

/**
 * thinkos_console_write() - write into to console driver.
 * @buf: pointer to an block of data to be transferred. 
 * @len: size of the data block in octets.
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_console_write(const void * buf, unsigned int len);

/**
 * thinkos_console_read() -  read from console driver.
 * @buf: pointer to a memory for data to be transferred. 
 * @len: size of the data block in octets.
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_console_read(void * buf, unsigned int len);

/**
 * thinkos_console_timedread() - read from console driver with timeout.
 * @buf: pointer to a memory for data to be transferred. 
 * @len: size of the data block in octets.
 * @ms: timeout time in milliseconds.
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_ETIMEDOUT if
 * it times out, OK %THINKOS_OK otherwise. 
 */
int thinkos_console_timedread(void * buf, unsigned int len, unsigned int ms);

int thinkos_console_is_connected(void);

int thinkos_console_close(void);

int thinkos_console_drain(void);

int thinkos_console_io_break(unsigned int which);

int thinkos_console_raw_mode(unsigned int enable);

int thinkos_console_rd_nonblock(unsigned int enable);

int thinkos_console_wr_nonblock(unsigned int enable);




/**
 * thinkos_flash_mem_close() - close the flash memory partition.
 * @key: memory partition key
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented, %THINKOS_OK otherwise. 
 */
int thinkos_flash_mem_close(int key);

/**
 * thinkos_flash_mem_open() - open the flash memory partition.
 * @tag: memory partition label
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented. 
 * %THINKOS_EINVAL if the partition does not exist.
 * A key (handler) for the partition is returned.
 */
int thinkos_flash_mem_open(const char * tag);

/**
 * thinkos_flash_mem_read() - reads from a flash memory partition.
 * @key: memory partition key
 * @offset: distance from the partition's start
 * @buf: buffer pointer 
 * @size: maximum number of bytes to be read from the partition  
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented. On success, the number 
 * of bytes read is returned.
 */
int thinkos_flash_mem_read(int key, off_t offset, void * buf, size_t size);

/**
 * thinkos_flash_mem_write() - writes to a flash memory partition.
 * @key: memory partition key
 * @offset: distance from the partition's start
 * @buf: buffer pointer
 * @size: number of bytes to transfer to the partition.
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented. On success, the number 
 * of bytes written is returned.
 */
int thinkos_flash_mem_write(int key, off_t offset, 
							const void * buf, size_t size);

/**
 * thinkos_flash_mem_erase() - erase flash blocks of a memory partition.
 * @key: memory partition key
 * @offset: position from the start of partition
 * @size: size of the erase area
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented. On success, the number 
 * of bytes written is returned.
 */
int thinkos_flash_mem_erase(int key, off_t offset, size_t size);

/**
 * thinkos_flash_mem_lock() - lock flash blocks of a memory partition.
 * @key: memory partition key
 * @offset: position from the start of partition
 * @size: size of the locking area
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented. On success, the number 
 * of bytes written is returned.
 */
int thinkos_flash_mem_lock(int key, off_t offset, size_t size);

/**
 * thinkos_flash_mem_unlock() - unlock flash blocks of a memory partition.
 * @key: memory partition key
 * @offset: position from the start of partition
 * @size: size of the locking area
 *
 * Return:
 * %THINKOS_ENOSYS if call is not implemented. On success, the number 
 * of bytes written is returned.
 */
int thinkos_flash_mem_unlock(int key, off_t offset, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_H__ */

