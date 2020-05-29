/* 
 * thinkos/config.h
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

#ifndef __THINKOS_PROFILE__
#error "Never use <thinkos/profile.h> directly; include <thinkos/kernel.h> instead."
#endif 

#ifndef __THINKOS_PROFILE_H__
#define __THINKOS_PROFILE_H__

#define __THINKOS_VERSION__
#include <thinkos/version.h>

/* -------------------------------------------------------------------------- 
 * Load local configuration
 * --------------------------------------------------------------------------*/
#ifdef CONFIG_H
#include "config.h"
#endif

/* -------------------------------------------------------------------------- 
 * Default configuration options
 * --------------------------------------------------------------------------*/

#ifndef THINKOS_THREADS_MAX 
#define THINKOS_THREADS_MAX             8
#endif

#ifndef THINKOS_NRT_THREADS_MAX
#define THINKOS_NRT_THREADS_MAX         0
#endif

#ifndef THINKOS_ENABLE_THREAD_ALLOC
#define THINKOS_ENABLE_THREAD_ALLOC     1
#endif

#ifndef THINKOS_ENABLE_THREAD_INFO
#define THINKOS_ENABLE_THREAD_INFO      1
#endif

#ifndef THINKOS_ENABLE_THREAD_STAT
#define THINKOS_ENABLE_THREAD_STAT      1
#endif

#ifndef THINKOS_ENABLE_JOIN
#define THINKOS_ENABLE_JOIN             1
#endif

#ifndef THINKOS_ENABLE_PAUSE
#define THINKOS_ENABLE_PAUSE            1
#endif

#ifndef THINKOS_ENABLE_CANCEL
#define THINKOS_ENABLE_CANCEL           1
#endif

#ifndef THINKOS_ENABLE_EXIT
#define THINKOS_ENABLE_EXIT             1
#endif

#ifndef THINKOS_ENABLE_TERMINATE
#define THINKOS_ENABLE_TERMINATE        1
#endif

#ifndef THINKOS_ENABLE_BREAK
#define THINKOS_ENABLE_BREAK            1
#endif

#ifndef THINKOS_ENABLE_TIMESHARE
#define THINKOS_ENABLE_TIMESHARE        1
#endif

#ifndef THINKOS_SCHED_LIMIT_MAX
#define THINKOS_SCHED_LIMIT_MAX         32
#endif

#ifndef THINKOS_SCHED_LIMIT_MIN
#define THINKOS_SCHED_LIMIT_MIN         1
#endif

#ifndef THINKOS_MUTEX_MAX
#define THINKOS_MUTEX_MAX               8
#endif

#ifndef THINKOS_ENABLE_MUTEX_ALLOC
#define THINKOS_ENABLE_MUTEX_ALLOC      0
#endif

#ifndef THINKOS_COND_MAX
#define THINKOS_COND_MAX                8
#endif

#ifndef THINKOS_ENABLE_COND_ALLOC
#define THINKOS_ENABLE_COND_ALLOC       0
#endif

#ifndef THINKOS_SEMAPHORE_MAX
#define THINKOS_SEMAPHORE_MAX           8
#endif

#ifndef THINKOS_ENABLE_SEM_ALLOC
#define THINKOS_ENABLE_SEM_ALLOC        0
#endif

#ifndef THINKOS_EVENT_MAX
#define THINKOS_EVENT_MAX               4
#endif

#ifndef THINKOS_ENABLE_EVENT_ALLOC
#define THINKOS_ENABLE_EVENT_ALLOC      0
#endif

#ifndef THINKOS_FLAG_MAX
#define THINKOS_FLAG_MAX                8
#endif

#ifndef THINKOS_ENABLE_FLAG_ALLOC
#define THINKOS_ENABLE_FLAG_ALLOC       0
#endif

#ifndef THINKOS_ENABLE_FLAG_WATCH
#define THINKOS_ENABLE_FLAG_WATCH       1
#endif

#ifndef THINKOS_GATE_MAX
#define THINKOS_GATE_MAX                8
#endif

#ifndef THINKOS_ENABLE_GATE_ALLOC
#define THINKOS_ENABLE_GATE_ALLOC       0
#endif

#ifndef THINKOS_ENABLE_CLOCK
#define THINKOS_ENABLE_CLOCK            1
#endif

#ifndef THINKOS_ENABLE_TIMED_CALLS
#define THINKOS_ENABLE_TIMED_CALLS      (THINKOS_ENABLE_CLOCK)
#endif

#ifndef THINKOS_ENABLE_ALARM
#define THINKOS_ENABLE_ALARM            (THINKOS_ENABLE_CLOCK)
#endif

#ifndef THINKOS_ENABLE_SLEEP
#define THINKOS_ENABLE_SLEEP            1
#endif

#ifndef THINKOS_ENABLE_CTL
#define THINKOS_ENABLE_CTL              1
#endif

#ifndef THINKOS_ENABLE_PREEMPTION
#define THINKOS_ENABLE_PREEMPTION       1
#endif

#ifndef THINKOS_ENABLE_CRITICAL
#define THINKOS_ENABLE_CRITICAL         1
#endif

#ifndef THINKOS_ENABLE_ESCALATE
#define THINKOS_ENABLE_ESCALATE         0
#endif

#ifndef THINKOS_IRQ_MAX 
#define THINKOS_IRQ_MAX                 80
#endif

#ifndef THINKOS_ENABLE_IRQ_CTL
#define THINKOS_ENABLE_IRQ_CTL          0
#endif

/* This option cause thinkos_irq_wait() to return the value
   of the CPU cycle count at the moment the interrupt was
   detected. */
#ifndef THINKOS_ENABLE_IRQ_CYCCNT
#define THINKOS_ENABLE_IRQ_CYCCNT       0
#endif

/* Allow IRQs with priority 0 for low latency.
   Be carefull if real ISRs are used as this option can impair 
   the debug monitor operation */
#ifndef THINKOS_ENABLE_IRQ_PRIORITY_0
#define THINKOS_ENABLE_IRQ_PRIORITY_0   0
#endif

#ifndef THINKOS_ENABLE_WQ_IRQ
#define THINKOS_ENABLE_WQ_IRQ           0
#endif

#ifndef THINKOS_ENABLE_IRQ_TIMEDWAIT 
#define THINKOS_ENABLE_IRQ_TIMEDWAIT    0
#endif

#ifndef THINKOS_ENABLE_CONSOLE
#define THINKOS_ENABLE_CONSOLE          0
#endif

#ifndef THINKOS_ENABLE_OBJ_ALLOC
#define THINKOS_ENABLE_OBJ_ALLOC        1
#endif

#ifndef THINKOS_ENABLE_OBJ_FREE
#define THINKOS_ENABLE_OBJ_FREE         0
#endif

/* Enable the thinkos_console_break() syscall. This can be used to 
   interrupt any blocking console's system calls */
#ifndef THINKOS_ENABLE_CONSOLE_BREAK
#define THINKOS_ENABLE_CONSOLE_BREAK    0 
#endif

/* Enable the console nonblocking calls. */
#ifndef THINKOS_ENABLE_CONSOLE_NONBLOCK
#define THINKOS_ENABLE_CONSOLE_NONBLOCK 0
#endif

/* Enable the console mode set calls. Ex:. raw_mode 
 */
#ifndef THINKOS_ENABLE_CONSOLE_MODE
#define THINKOS_ENABLE_CONSOLE_MODE     0
#endif

/* Enable the console open/close calls.
 */
#ifndef THINKOS_ENABLE_CONSOLE_OPEN
#define THINKOS_ENABLE_CONSOLE_OPEN     THINKOS_ENABLE_CONSOLE
#endif

/* Enable the console drain call.
 */
#ifndef THINKOS_ENABLE_CONSOLE_DRAIN
#define THINKOS_ENABLE_CONSOLE_DRAIN    THINKOS_ENABLE_CONSOLE
#endif

/* Enable the console read call.
 */
#ifndef THINKOS_ENABLE_CONSOLE_READ
#define THINKOS_ENABLE_CONSOLE_READ     THINKOS_ENABLE_CONSOLE
#endif

/* Enable the console miscelaneous calls Ex: is_connected.
 */
#ifndef THINKOS_ENABLE_CONSOLE_MISC
#define THINKOS_ENABLE_CONSOLE_MISC     THINKOS_ENABLE_CONSOLE
#endif

#ifndef THINKOS_ENABLE_COMM
#define THINKOS_ENABLE_COMM             0
#endif

#ifndef THINKOS_ENABLE_MPU 
#define THINKOS_ENABLE_MPU              0
#endif

/* Enable FPU (Floating Point Unit) */
#ifndef THINKOS_ENABLE_FPU 
#define THINKOS_ENABLE_FPU              0
#endif

/* Enable FP lazy context save */
#ifndef THINKOS_ENABLE_FPU_LS 
#define THINKOS_ENABLE_FPU_LS           0
#endif

#ifndef THINKOS_ENABLE_PROFILING
#define THINKOS_ENABLE_PROFILING        0
#endif

#ifndef THINKOS_ENABLE_ARG_CHECK
#define THINKOS_ENABLE_ARG_CHECK        0
#endif

#ifndef THINKOS_ENABLE_DEADLOCK_CHECK
#define THINKOS_ENABLE_DEADLOCK_CHECK   0
#endif

#ifndef THINKOS_ENABLE_SANITY_CHECK
#define THINKOS_ENABLE_SANITY_CHECK     0
#endif

#ifndef THINKOS_ENABLE_EXCEPTIONS
#define THINKOS_ENABLE_EXCEPTIONS       1
#endif

#ifndef THINKOS_UNROLL_EXCEPTIONS 
#define THINKOS_UNROLL_EXCEPTIONS       0
#endif

#ifndef THINKOS_ENABLE_EXCEPT_CLEAR
#define THINKOS_ENABLE_EXCEPT_CLEAR     1
#endif

#ifndef THINKOS_ENABLE_BUSFAULT
#define THINKOS_ENABLE_BUSFAULT         0
#endif

#ifndef THINKOS_ENABLE_USAGEFAULT 
#define THINKOS_ENABLE_USAGEFAULT       0
#endif

#ifndef THINKOS_ENABLE_MEMFAULT 
#define THINKOS_ENABLE_MEMFAULT         0
#endif

#ifndef THINKOS_ENABLE_HARDFAULT
#define THINKOS_ENABLE_HARDFAULT        0
#endif

#ifndef THINKOS_ENABLE_ERROR_TRAP
#define THINKOS_ENABLE_ERROR_TRAP       0
#endif

#ifndef THINKOS_SYSRST_ONFAULT
#define THINKOS_SYSRST_ONFAULT          0
#endif

#ifndef THINKOS_EXCEPT_STACK_SIZE
#define THINKOS_EXCEPT_STACK_SIZE       320
#endif

#ifndef THINKOS_ENABLE_MONITOR
#define THINKOS_ENABLE_MONITOR          0
#endif

#ifndef THINKOS_ENABLE_DMCLOCK
#define THINKOS_ENABLE_DMCLOCK          (THINKOS_ENABLE_CLOCK)
#endif

#ifndef THINKOS_ENABLE_MONITOR_THREADS 
#define THINKOS_ENABLE_MONITOR_THREADS  0
#endif

#ifndef THINKOS_ENABLE_DEBUG_STEP 
#define THINKOS_ENABLE_DEBUG_STEP       0
#endif

#ifndef THINKOS_ENABLE_DEBUG_BKPT
#define THINKOS_ENABLE_DEBUG_BKPT       0
#endif

#ifndef THINKOS_ENABLE_DEBUG_WPT 
#define THINKOS_ENABLE_DEBUG_WPT        0
#endif

#ifndef THINKOS_ENABLE_DEBUG_FAULT
#define THINKOS_ENABLE_DEBUG_FAULT      0
#endif

#ifndef THINKOS_ENABLE_STACK_INIT
#define THINKOS_ENABLE_STACK_INIT       1
#endif

/* Kernel Zeros all the memory blocks and datastructures on alocation
   or initialization */
#ifndef THINKOS_ENABLE_MEMORY_CLEAR
#define THINKOS_ENABLE_MEMORY_CLEAR     1
#endif

#ifndef THINKOS_ENABLE_THREAD_VOID 
#define THINKOS_ENABLE_THREAD_VOID      1
#endif

#ifndef THINKOS_ENABLE_IDLE_WFI
#define THINKOS_ENABLE_IDLE_WFI         1
#endif

#ifndef THINKOS_ENABLE_SCHED_DEBUG
#define THINKOS_ENABLE_SCHED_DEBUG      0
#endif

/* Enable stack check on scheduler */
#ifndef THINKOS_ENABLE_SCHED_ERROR
#define THINKOS_ENABLE_SCHED_ERROR      0
#endif

/* Compiler flag: 

   -Ofast  */
#ifndef THINKOS_ENABLE_OFAST
#define THINKOS_ENABLE_OFAST            1
#endif

/* Compiler flag:

   -mno-unaligned-access */
#ifndef THINKOS_ENABLE_ALIGN
#define THINKOS_ENABLE_ALIGN            1
#endif

/* THINKOS_ENABLE_IDLE_HOOKS - This option is used to request the execution of
   a tasklet when the system is idle. The tasklet runs on the IDLE 
   thread context.
 */
#ifndef THINKOS_ENABLE_IDLE_HOOKS       
#define THINKOS_ENABLE_IDLE_HOOKS        0
#endif

/* THINKOS_ENABLE_IDLE_MSP - IDLE thread and exceptions use the same
   stack (MSP) all the remaining threads (non privileged) make use of
   the PSP instead.
 */
#ifndef THINKOS_ENABLE_IDLE_MSP
#define THINKOS_ENABLE_IDLE_MSP          0
#endif

/* -------------------------------------------------------------------------- 
 * Default configuration options
 * Non implemented/Planned options, should not be used in 
 *   production code.
 */

/* THINKOS_ENABLE_FLASH_MEM - enable the kernel to handle low-level
   flash memory erase read and write operations.
 */

#ifndef THINKOS_ENABLE_FLASH_MEM
#define THINKOS_ENABLE_FLASH_MEM         0
#endif

#ifndef THINKOS_ENABLE_MEM_MAP
#define THINKOS_ENABLE_MEM_MAP           0
#endif

#ifndef THINKOS_DMA_MAX 
#define THINKOS_DMA_MAX                  0
#endif

#ifndef THINKOS_QUEUE_MAX 
#define THINKOS_QUEUE_MAX                0
#endif

#ifndef THINKOS_ENABLE_KRN_TRACE
#define THINKOS_ENABLE_KRN_TRACE         0
#endif

/* THINKOS_ENABLE_I_CALLS - Enable building functions to be used in interrupt 
   handlers only. These functions are suffixed by _i.
   */
#ifndef THINKOS_ENABLE_I_CALLS
#define THINKOS_ENABLE_I_CALLS           1
#endif

/* THINKOS_ENABLE_IRQ_ASM_FAST - Enable generation of faster assembler 
   interrupt handlers stubs. This will generate one stub per IRQ as oposed 
 to a single stub shared by all. */
#ifndef THINKOS_ENABLE_IRQ_ASM_FAST
#define THINKOS_ENABLE_IRQ_ASM_FAST     0
#endif


/* Deprecated options, to be removed in the future 
 */

#ifndef THINKOS_ASM_SCHEDULER
#define THINKOS_ASM_SCHEDULER           1
#endif

#ifndef THINKOS_ENABLE_RT_DEBUG
#define THINKOS_ENABLE_RT_DEBUG         0
#endif

#ifndef THINKOS_STDERR_FAULT_DUMP
#define THINKOS_STDERR_FAULT_DUMP       0
#endif

/* -------------------------------------------------------------------------- 
 * Dependency check
 * --------------------------------------------------------------------------*/

#if THINKOS_ENABLE_THREAD_ALLOC && (THINKOS_THREADS_MAX > 32)
#undef THINKOS_THREADS_MAX 
#define THINKOS_THREADS_MAX 32
#warn "THINKOS_THREADS_MAX set to 32"
#endif

#if (THINKOS_ENABLE_MUTEX_ALLOC) & !(THINKOS_MUTEX_MAX)
#undef THINKOS_ENABLE_MUTEX_ALLOC
#define THINKOS_ENABLE_MUTEX_ALLOC 0
#endif

#if (THINKOS_ENABLE_COND_ALLOC) & !(THINKOS_COND_MAX)
#undef THINKOS_ENABLE_COND_ALLOC
#define THINKOS_ENABLE_COND_ALLOC 0
#endif

#if (THINKOS_ENABLE_SEM_ALLOC) & !(THINKOS_SEMAPHORE_MAX)
#undef THINKOS_ENABLE_SEM_ALLOC
#define THINKOS_ENABLE_SEM_ALLOC 0
#endif

#if (THINKOS_ENABLE_EVENT_ALLOC) & !(THINKOS_EVENT_MAX)
#undef THINKOS_ENABLE_EVENT_ALLOC
#define THINKOS_ENABLE_EVENT_ALLOC 0
#endif

#if (THINKOS_ENABLE_FLAG_ALLOC) & !(THINKOS_FLAG_MAX)
#undef THINKOS_ENABLE_FLAG_ALLOC
#define THINKOS_ENABLE_FLAG_ALLOC 0
#endif

#if (THINKOS_ENABLE_GATE_ALLOC) & !(THINKOS_GATE_MAX)
#undef THINKOS_ENABLE_GATE_ALLOC
#define THINKOS_ENABLE_GATE_ALLOC 0
#endif


#if THINKOS_ENABLE_IRQ_CYCCNT
/* IRQ return cyclecnt depends on THINKOS_ENABLE_IRQ_CTL */
#undef THINKOS_ENABLE_IRQ_CTL
#undef THINKOS_ENABLE_WQ_IRQ
#define THINKOS_ENABLE_IRQ_CTL      1
#define THINKOS_ENABLE_WQ_IRQ       1
#endif

#if THINKOS_ENABLE_IRQ_TIMEDWAIT 
/* IRQ timedwait depends on THINKOS_ENABLE_WQ_IRQ & 
   THINKOS_ENABLE_TIMED_CALLS */
#undef THINKOS_ENABLE_TIMED_CALLS
#undef THINKOS_ENABLE_WQ_IRQ
#define THINKOS_ENABLE_TIMED_CALLS  1
#define THINKOS_ENABLE_WQ_IRQ       1
#endif

/* timed calls depends on clock */
#if (THINKOS_ENABLE_TIMED_CALLS) && !(THINKOS_ENABLE_CLOCK)
#error "THINKOS_ENABLE_ALARM depends on THINKOS_ENABLE_CLOCK"
#endif

#if (THINKOS_ENABLE_ALARM) && !(THINKOS_ENABLE_CLOCK)
#error "THINKOS_ENABLE_ALARM depends on THINKOS_ENABLE_CLOCK"
#endif

#if (THINKOS_ENABLE_DMCLOCK) && !(THINKOS_ENABLE_CLOCK)
#error "THINKOS_ENABLE_DMCLOCK depends on THINKOS_ENABLE_CLOCK"
#endif

/* debug step depends on debug breakpoint */
#if (THINKOS_ENABLE_DEBUG_STEP) && !(THINKOS_ENABLE_DEBUG_BKPT)
#undef THINKOS_ENABLE_DEBUG_BKPT
#define THINKOS_ENABLE_DEBUG_BKPT 1
#endif

/* debug watchpoint depends on debug breakpoint */
#if (THINKOS_ENABLE_DEBUG_WPT) && !(THINKOS_ENABLE_DEBUG_BKPT)
#undef THINKOS_ENABLE_DEBUG_BKPT
#define THINKOS_ENABLE_DEBUG_BKPT 1
#endif

/* debug breakpoint depends on monitor */
#if (THINKOS_ENABLE_DEBUG_BKPT) && !(THINKOS_ENABLE_MONITOR)
#undef THINKOS_ENABLE_MONITOR
#define THINKOS_ENABLE_MONITOR 1
#endif

/* debug monitor depend on idle hooks */
#if (THINKOS_ENABLE_MONITOR) && !(THINKOS_ENABLE_IDLE_HOOKS)
//#error "THINKOS_ENABLE_MONITOR depends on THINKOS_ENABLE_IDLE_HOOKS"
#endif

#if (THINKOS_ENABLE_MONITOR_THREADS) && !(THINKOS_ENABLE_MONITOR)
#error "THINKOS_ENABLE_MONITOR_THREADS depends on THINKOS_ENABLE_MONITOR"
#endif

#if (THINKOS_ENABLE_MONITOR_THREADS) && !(THINKOS_ENABLE_MONITOR)
#error "THINKOS_ENABLE_MONITOR_THREADS depends on THINKOS_ENABLE_MONITOR"
#endif

/* timed calls, cancel, pause and debug step depend on thread status */
#if THINKOS_ENABLE_TIMED_CALLS || THINKOS_ENABLE_PAUSE || \
	THINKOS_ENABLE_CANCEL || THINKOS_ENABLE_DEBUG_STEP 
#undef THINKOS_ENABLE_THREAD_STAT
#define THINKOS_ENABLE_THREAD_STAT 1
#endif

#if THINKOS_ENABLE_EXIT  || THINKOS_ENABLE_CANCEL || THINKOS_ENABLE_JOIN
#undef THINKOS_ENABLE_THREAD_VOID
#define THINKOS_ENABLE_THREAD_VOID 1
#undef THINKOS_ENABLE_TERMINATE
#define THINKOS_ENABLE_TERMINATE 1
#endif

#if THINKOS_ENABLE_MEMFAULT && !THINKOS_ENABLE_MPU 
#undef THINKOS_ENABLE_MEMFAULT 
#define THINKOS_ENABLE_MEMFAULT 0
#endif

#if (THINKOS_ENABLE_TIMESHARE)
#undef THINKOS_ENABLE_PREEMPTION
#define THINKOS_ENABLE_PREEMPTION 1
#undef THINKOS_ENABLE_CLOCK
#define THINKOS_ENABLE_CLOCK 1
#endif

#if !(THINKOS_ENABLE_PREEMPTION)
#undef THINKOS_ENABLE_CRITICAL
#define THINKOS_ENABLE_CRITICAL 0
#endif

#if (THINKOS_ENABLE_DEBUG_FAULT)
  #undef THINKOS_ENABLE_MEMFAULT
  #undef THINKOS_ENABLE_BUSFAULT
  #undef THINKOS_ENABLE_USAGEFAULT 
  #define THINKOS_ENABLE_USAGEFAULT 1
  #define THINKOS_ENABLE_MEMFAULT   1
  #define THINKOS_ENABLE_BUSFAULT   1
#endif

#if !(THINKOS_ENABLE_CONSOLE)
  #undef THINKOS_ENABLE_CONSOLE_BREAK
  #undef THINKOS_ENABLE_CONSOLE_NONBLOCK
  #undef THINKOS_ENABLE_CONSOLE_MODE
  #define THINKOS_ENABLE_CONSOLE_BREAK    0 
  #define THINKOS_ENABLE_CONSOLE_NONBLOCK 0
  #define THINKOS_ENABLE_CONSOLE_MODE     0
#endif

#if (THINKOS_ENABLE_MUTEX_ALLOC | THINKOS_ENABLE_COND_ALLOC | \
  THINKOS_ENABLE_SEM_ALLOC | THINKOS_ENABLE_EVENT_ALLOC | \
  THINKOS_ENABLE_FLAG_ALLOC |  THINKOS_ENABLE_GATE_ALLOC)
  #undef THINKOS_ENABLE_OBJ_FREE
  #undef THINKOS_ENABLE_OBJ_ALLOC
  #define THINKOS_ENABLE_OBJ_ALLOC        1
  #define THINKOS_ENABLE_OBJ_FREE         1
#endif

#ifndef THINKOS_ENABLE_OBJ_FREE
#define THINKOS_ENABLE_OBJ_FREE         0
#endif

#if THINKOS_ENABLE_FPU_LS 
#error "THINKOS_ENABLE_FPU_LS depends on THINKOS_ENABLE_FPU"
#endif

/* -------------------------------------------------------------------------- 
 * FIXME: ??? Not sure what is the intent here ????
 * --------------------------------------------------------------------------*/
#ifdef THINKOS_DEBUG
  #ifndef DEBUG
  #define DEBUG
  #endif
#endif

#ifndef __ASSEMBLER__

#include <stdint.h>

/* -------------------------------------------------------------------------- 
 * Profile block
 * --------------------------------------------------------------------------*/

struct thinkos_profile {
	struct {
		uint16_t size; 
		uint8_t version; /* profile version */
		uint8_t reserved; /* unused */
	} header;

	struct {
		union {
			uint32_t u32;
			struct {
				uint8_t    major;
				uint8_t    minor;
				uint16_t   reserved;
			};
		} version;
		union {
			uint32_t flags;
			struct {
				uint32_t timeshare   :1;
				uint32_t preemption  :1;
			};
		};
		uint32_t sched_limit_max     :8;
		uint32_t sched_limit_min     :8;
	} kernel;
	
	struct {
		union {
			uint32_t flags;
			struct {
				uint32_t raw :1;
				uint32_t elf :1;
			};
		};
	} app;

	struct {
		uint32_t threads_max         :8;
		uint32_t nrt_threads_max     :8;
		uint32_t mutex_max           :8;
		uint32_t cond_max            :8;

		uint32_t semaphore_max       :8;
		uint32_t event_max           :8;
		uint32_t flag_max            :8;
		uint32_t gate_max            :8;

		uint32_t queue_max           :8;
		uint32_t irq_max             :8;
		uint32_t dma_max             :8;
		uint32_t except_stack_size   :16;
	} limit;

	union {
		uint32_t flags;
		struct {
			uint32_t join            :1;
			uint32_t clock           :1;
			uint32_t alarm           :1;
			uint32_t sleep           :1;
			uint32_t ctl             :1;
			uint32_t critical        :1;
			uint32_t escalate        :1;
			uint32_t irq_ctl         :1;
			uint32_t pause           :1;
			uint32_t cancel          :1;
			uint32_t exit            :1;
			uint32_t terminate       :1;
			uint32_t thread_break    :1;
			uint32_t console         :1;
			uint32_t flag_watch      :1;
			uint32_t timed_calls     :1;
			uint32_t irq_timedwait   :1;
		};
	} syscall;

	union {
		uint32_t flags;
		struct {
			uint32_t thread_alloc    :1;
			uint32_t mutex_alloc     :1;
			uint32_t cond_alloc      :1;
			uint32_t sem_alloc       :1;
			uint32_t event_alloc     :1;
			uint32_t gate_alloc      :1;
			uint32_t flag_alloc      :1;
		};
	} alloc;

	union {
		uint32_t flags;
		struct {
			uint32_t thread_info        :1;
			uint32_t thread_stat        :1;
			uint32_t irq_cyccnt         :1;
			uint32_t irq_priority_0     :1;
			uint32_t wq_irq             :1;
			uint32_t console_break      :1;
			uint32_t console_mode       :1;
			uint32_t console_nonblock   :1;
			uint32_t console_open       :1;
			uint32_t console_drain      :1;
			uint32_t console_read       :1;
			uint32_t console_misc       :1;
			uint32_t comm               :1;
			uint32_t mpu                :1;
			uint32_t fpu                :1;
			uint32_t fpu_ls             :1;
			uint32_t profiling          :1;
			uint32_t mem_map            :1;
			uint32_t flash_mem          :1;
			uint32_t krn_trace          :1;
		};
	} feature;

	union {
		uint32_t flags;
		struct {
			uint32_t arg_check       :1;
			uint32_t deadlock_check  :1;
			uint32_t sanity_check    :1;
			uint32_t stack_init      :1;
			uint32_t memory_clear    :1;
		};
	} security;

	union {
		uint32_t flags;
		struct {
			uint32_t exceptions      :1;
			uint32_t busfault        :1;
			uint32_t usagefault      :1;
			uint32_t memfault        :1;
			uint32_t hardfault       :1;
			uint32_t except_clear    :1;
			uint32_t error_trap      :1;
			uint32_t sysrst_onfault  :1;
		};
	} except;

	union {
		uint32_t flags;
		struct {
			uint32_t monitor         :1;
			uint32_t dmclock         :1;
			uint32_t debug_step      :1;
			uint32_t debug_bkpt      :1;
			uint32_t debug_wpt       :1;
			uint32_t debug_fault     :1;
			uint32_t monitor_threads :1;
		};
	} dbgmon;

	union {
		uint32_t misc_flags;
		struct {
			uint32_t thread_void     :1;
			uint32_t idle_wfi        :1;
			uint32_t sched_debug     :1;
			uint32_t ofast           :1;
			uint32_t align           :1;
			uint32_t idle_hooks      :1;
			uint32_t idle_msp        :1;
		};
	} misc;

	union {
		uint32_t flags;
		struct {
			uint32_t asm_scheduler       :1;
			uint32_t rt_debug            :1;
			uint32_t unroll_exceptions   :1;
			uint32_t stderr_fault_dump   :1;
		};
	} deprecated;
};

extern const struct thinkos_profile thinkos_profile;

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_PROFILE_H__ */

