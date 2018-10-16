/* 
 * thikos/config.h
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
#define THINKOS_ENABLE_MUTEX_ALLOC      1
#endif

#ifndef THINKOS_COND_MAX
#define THINKOS_COND_MAX                8
#endif

#ifndef THINKOS_ENABLE_COND_ALLOC
#define THINKOS_ENABLE_COND_ALLOC       1
#endif

#ifndef THINKOS_SEMAPHORE_MAX
#define THINKOS_SEMAPHORE_MAX           8
#endif

#ifndef THINKOS_ENABLE_SEM_ALLOC
#define THINKOS_ENABLE_SEM_ALLOC        1
#endif

#ifndef THINKOS_EVENT_MAX
#define THINKOS_EVENT_MAX               4
#endif

#ifndef THINKOS_ENABLE_EVENT_ALLOC
#define THINKOS_ENABLE_EVENT_ALLOC      1
#endif

#ifndef THINKOS_FLAG_MAX
#define THINKOS_FLAG_MAX                8
#endif

#ifndef THINKOS_ENABLE_FLAG_ALLOC
#define THINKOS_ENABLE_FLAG_ALLOC       1
#endif

#ifndef THINKOS_ENABLE_FLAG_WATCH
#define THINKOS_ENABLE_FLAG_WATCH       1
#endif

#ifndef THINKOS_GATE_MAX
#define THINKOS_GATE_MAX                8
#endif

#ifndef THINKOS_ENABLE_GATE_ALLOC
#define THINKOS_ENABLE_GATE_ALLOC       1
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

#ifndef THINKOS_ENABLE_CONSOLE
#define THINKOS_ENABLE_CONSOLE          0
#endif

/* Enable the thinkos_console_break() syscall. This can be used to 
   interrupt any blocking conole's system calls */
#ifndef THINKOS_ENABLE_CONSOLE_BREAK
#define THINKOS_ENABLE_CONSOLE_BREAK    0 
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
#define THINKOS_ENABLE_ARG_CHECK        1
#endif

#ifndef THINKOS_ENABLE_DEADLOCK_CHECK
#define THINKOS_ENABLE_DEADLOCK_CHECK   1
#endif

#ifndef THINKOS_ENABLE_SANITY_CHECK
#define THINKOS_ENABLE_SANITY_CHECK     1
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

#ifndef THINKOS_STDERR_FAULT_DUMP
#define THINKOS_STDERR_FAULT_DUMP       0
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

#ifndef THINKOS_ENABLE_THREAD_VOID 
#define THINKOS_ENABLE_THREAD_VOID      1
#endif

#ifndef THINKOS_ENABLE_IDLE_WFI
#define THINKOS_ENABLE_IDLE_WFI         1
#endif

#ifndef THINKOS_ASM_SCHEDULER
#define THINKOS_ASM_SCHEDULER           1
#endif

#ifndef THINKOS_ENABLE_OFAST
#define THINKOS_ENABLE_OFAST            1
#endif

#ifndef THINKOS_ENABLE_SCHED_DEBUG
#define THINKOS_ENABLE_SCHED_DEBUG      0
#endif

#ifndef THINKOS_ENABLE_RT_DEBUG
#define THINKOS_ENABLE_RT_DEBUG         0
#endif

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
   stack (MSP) all the remainig threads (non priviledged) makd use of
   the PSP instead.
 */
#ifndef THINKOS_ENABLE_IDLE_MSP
#define THINKOS_ENABLE_IDLE_MSP          1
#endif



/* -------------------------------------------------------------------------- 
 * Dependency check
 * --------------------------------------------------------------------------*/

#if THINKOS_ENABLE_THREAD_ALLOC && (THINKOS_THREADS_MAX > 32)
#undef THINKOS_THREADS_MAX 
#define THINKOS_THREADS_MAX 32
#warn "THINKOS_THREADS_MAX set to 32"
#endif

#if (THINKOS_ENABLE_COND_ALLOC) & !(THINKOS_COND_MAX)
#undef THINKOS_ENABLE_COND_ALLOC
#define THINKOS_ENABLE_COND_ALLOC 0
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
#define THINKOS_ENABLE_IRQ_CTL 1
#define THINKOS_ENABLE_WQ_IRQ 1
#endif

#if THINKOS_ENABLE_IRQ_TIMEDWAIT 
/* IRQ timedwait depends on THINKOS_ENABLE_WQ_IRQ & 
   THINKOS_ENABLE_TIMED_CALLS */
#undef THINKOS_ENABLE_TIMED_CALLS
#undef THINKOS_ENABLE_WQ_IRQ
#define THINKOS_ENABLE_TIMED_CALLS 1
#define THINKOS_ENABLE_WQ_IRQ 1
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

/* dbug step depends on debug breakpoint */
#if (THINKOS_ENABLE_DEBUG_STEP) && !(THINKOS_ENABLE_DEBUG_BKPT)
#undef THINKOS_ENABLE_DEBUG_BKPT
#define THINKOS_ENABLE_DEBUG_BKPT 1
#endif

/* dbug watchpoint depends on debug breakpoint */
#if (THINKOS_ENABLE_DEBUG_WPT) && !(THINKOS_ENABLE_DEBUG_BKPT)
#undef THINKOS_ENABLE_DEBUG_BKPT
#define THINKOS_ENABLE_DEBUG_BKPT 1
#endif

/* dbug breakpoint depends on monitor */
#if (THINKOS_ENABLE_DEBUG_BKPT) && !(THINKOS_ENABLE_MONITOR)
#undef THINKOS_ENABLE_MONITOR
#define THINKOS_ENABLE_MONITOR 1
#endif

/* dbug monitir depend on idle hooks */
#if (THINKOS_ENABLE_MONITOR) && !(THINKOS_ENABLE_IDLE_HOOKS)
#undef THINKOS_ENABLE_IDLE_HOOKS
#define THINKOS_ENABLE_IDLE_HOOKS 1
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

#if THINKOS_ENABLE_TIMESHARE
#undef THINKOS_ENABLE_PREEMPTION
#define THINKOS_ENABLE_PREEMPTION 1
#undef THINKOS_ENABLE_CLOCK
#define THINKOS_ENABLE_CLOCK 1
#endif

#if !THINKOS_ENABLE_PREEMPTION
#undef THINKOS_ENABLE_CRITICAL
#define THINKOS_ENABLE_CRITICAL 0
#endif

#if THINKOS_ENABLE_DEBUG_FAULT
#undef THINKOS_ENABLE_MEMFAULT
#define THINKOS_ENABLE_MEMFAULT   1
#undef THINKOS_ENABLE_BUSFAULT
#define THINKOS_ENABLE_BUSFAULT   1
#undef THINKOS_ENABLE_USAGEFAULT 
#define THINKOS_ENABLE_USAGEFAULT 1
#undef THINKOS_UNROLL_EXCEPTIONS 
#define THINKOS_UNROLL_EXCEPTIONS 1
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
		uint32_t mutex_max           :8;
		uint32_t cond_max            :8;
		uint32_t semaphore_max       :8;

		uint32_t event_max           :8;
		uint32_t flag_max            :8;
		uint32_t gate_max            :8;
		uint32_t queue_max           :8;

		uint32_t irq_max             :8;
		uint32_t except_stack_size   :16;
	} limit;

	union {
		uint32_t syscall_flags;
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
		};
	};

	union {
		uint32_t alloc_flags;
		struct {
			uint32_t thread_alloc    :1;
			uint32_t mutex_alloc     :1;
			uint32_t cond_alloc      :1;
			uint32_t sem_alloc       :1;
			uint32_t event_alloc     :1;
			uint32_t gate_alloc      :1;
			uint32_t flag_alloc      :1;
		};
	};

	union {
		uint32_t feature_flags;
		struct {
			uint32_t thread_info     :1;
			uint32_t thread_stat     :1;
			uint32_t irq_cyccnt      :1;
			uint32_t irq_priority_0  :1;
			uint32_t wq_irq          :1;
			uint32_t console_break   :1;
			uint32_t comm            :1;
			uint32_t mpu             :1;
			uint32_t fpu             :1;
			uint32_t fpu_ls          :1;
			uint32_t profiling       :1;
		};
	};

	union {
		uint32_t security_flags;
		struct {
			uint32_t arg_check       :1;
			uint32_t deadlock_check  :1;
			uint32_t sanity_check    :1;
		};
	};

	union {
		uint32_t xcpt_flags;
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
	};

	union {
		uint32_t dbgmon_flags;
		struct {
			uint32_t monitor         :1;
			uint32_t dmclock         :1;
			uint32_t debug_step      :1;
			uint32_t debug_bkpt      :1;
			uint32_t debug_wpt       :1;
			uint32_t debug_fault     :1;
		};
	};

	union {
		uint32_t misc_flags;
		struct {
			uint32_t stack_init      :1;
			uint32_t thread_void     :1;
			uint32_t idle_wfi        :1;
			uint32_t sched_debug     :1;
			uint32_t ofast           :1;
			uint32_t align           :1;
			uint32_t idle_hooks      :1;
			uint32_t idle_msp        :1;
		};
	};


	union {
		uint32_t obsolete_flags;
		struct {
			uint32_t asm_scheduler       :1;
			uint32_t rt_debug            :1;
			uint32_t unroll_exceptions   :1;
			uint32_t stderr_fault_dump   :1;
		};
	};
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

