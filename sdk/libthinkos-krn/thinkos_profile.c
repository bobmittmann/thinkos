/* 
 * thinkos_profile.c
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

#define __THINKOS_PROFILE__
#include <thinkos/profile.h>
#include <sys/dcclog.h>

/* -------------------------------------------------------------------------- 
 * ThinkOS profile block
 * --------------------------------------------------------------------------*/

const struct thinkos_profile thinkos_profile = {
	.header = {
		.size = sizeof(struct thinkos_profile),
		.version = THINKOS_PROFILE_VERSION,
		.reserved = 0
	},

	.kernel = {
		.version = {
			.major = THINKOS_KERNEL_VERSION_MAJOR,
			.minor = THINKOS_KERNEL_VERSION_MINOR,
			.reserved = 0
		},
		.timeshare  = THINKOS_ENABLE_TIMESHARE,
		.preemption = THINKOS_ENABLE_PREEMPTION,
		.sched_limit_max = THINKOS_SCHED_LIMIT_MAX,
		.sched_limit_min = THINKOS_SCHED_LIMIT_MIN
	},

	.app = {
		.raw = 1,
		.elf = 0
	},

	.limit = {
		.threads_max     = THINKOS_THREADS_MAX,
		.nrt_threads_max = THINKOS_NRT_THREADS_MAX,
		.mutex_max       = THINKOS_THREADS_MAX,
		.cond_max        = THINKOS_COND_MAX,

		.semaphore_max   = THINKOS_SEMAPHORE_MAX,
		.event_max       = THINKOS_EVENT_MAX,
		.flag_max        = THINKOS_FLAG_MAX,
		.gate_max        = THINKOS_GATE_MAX,

		.queue_max       =  THINKOS_QUEUE_MAX,
		.irq_max           =  THINKOS_IRQ_MAX,
		.dma_max           =  THINKOS_DMA_MAX,
		.except_stack_size =  THINKOS_EXCEPT_STACK_SIZE
	},

	.syscall = {
		.join         = THINKOS_ENABLE_JOIN,
		.clock        = THINKOS_ENABLE_CLOCK,
		.alarm        = THINKOS_ENABLE_ALARM,
		.sleep        = THINKOS_ENABLE_SLEEP,
		.ctl          = THINKOS_ENABLE_CTL,
		.critical     = THINKOS_ENABLE_CRITICAL,
		.escalate     = THINKOS_ENABLE_ESCALATE,
		.irq_ctl      = THINKOS_ENABLE_IRQ_CTL,
		.pause        = THINKOS_ENABLE_PAUSE,
		.cancel       = THINKOS_ENABLE_CANCEL,
		.exit         = THINKOS_ENABLE_EXIT,
		.terminate    = THINKOS_ENABLE_TERMINATE,
		.thread_break = THINKOS_ENABLE_BREAK,
		.console      = THINKOS_ENABLE_CONSOLE,
		.flag_watch   = THINKOS_ENABLE_FLAG_WATCH,
		.timed_calls  = THINKOS_ENABLE_TIMED_CALLS,
		.irq_timedwait = THINKOS_ENABLE_IRQ_TIMEDWAIT
	},

	.alloc = {
		.thread_alloc = THINKOS_ENABLE_THREAD_ALLOC, 
		.mutex_alloc  = THINKOS_ENABLE_MUTEX_ALLOC,
		.cond_alloc   = THINKOS_ENABLE_COND_ALLOC,
		.sem_alloc    = THINKOS_ENABLE_SEM_ALLOC,
		.event_alloc  = THINKOS_ENABLE_EVENT_ALLOC,
		.gate_alloc   = THINKOS_ENABLE_GATE_ALLOC,
		.flag_alloc   = THINKOS_ENABLE_FLAG_ALLOC
	},

	.feature = {
		.thread_info     = THINKOS_ENABLE_THREAD_INFO,
		.thread_stat     = THINKOS_ENABLE_THREAD_STAT,
		.irq_cyccnt      = THINKOS_ENABLE_IRQ_CYCCNT,
		.irq_priority_0  = THINKOS_ENABLE_IRQ_PRIORITY_0,
		.wq_irq          = THINKOS_ENABLE_WQ_IRQ,
		.console_break   = THINKOS_ENABLE_CONSOLE_BREAK,
		.console_mode    = THINKOS_ENABLE_CONSOLE_MODE,
		.console_nonblock = THINKOS_ENABLE_CONSOLE_NONBLOCK,
		.console_open    = THINKOS_ENABLE_CONSOLE_OPEN,
		.console_drain   = THINKOS_ENABLE_CONSOLE_DRAIN,
		.console_read    = THINKOS_ENABLE_CONSOLE_READ,
		.console_misc    = THINKOS_ENABLE_CONSOLE_MISC,
		.comm            = THINKOS_ENABLE_COMM,
		.mpu             = THINKOS_ENABLE_MPU,
		.fpu             = THINKOS_ENABLE_FPU,
		.fpu_ls          = THINKOS_ENABLE_FPU_LS,
		.profiling       = THINKOS_ENABLE_PROFILING,

	},

	.security = {
		.arg_check       = THINKOS_ENABLE_ARG_CHECK,
		.deadlock_check  = THINKOS_ENABLE_DEADLOCK_CHECK,
		.sanity_check    = THINKOS_ENABLE_SANITY_CHECK,
		.stack_init      = THINKOS_ENABLE_STACK_INIT,
	},

	.except = {
		.exceptions      = THINKOS_ENABLE_EXCEPTIONS,
		.busfault        = THINKOS_ENABLE_BUSFAULT,
		.usagefault      = THINKOS_ENABLE_USAGEFAULT,
		.memfault        = THINKOS_ENABLE_MEMFAULT,
		.hardfault       = THINKOS_ENABLE_HARDFAULT,
		.except_clear    = THINKOS_ENABLE_EXCEPT_CLEAR,
		.error_trap      = THINKOS_ENABLE_ERROR_TRAP,
		.sysrst_onfault  = THINKOS_SYSRST_ONFAULT
	},

	.dbgmon = {
		.monitor         = THINKOS_ENABLE_MONITOR,
		.dmclock         = THINKOS_ENABLE_DMCLOCK,
		.debug_step      = THINKOS_ENABLE_DEBUG_STEP,
		.debug_bkpt      = THINKOS_ENABLE_DEBUG_BKPT,
		.debug_wpt       = THINKOS_ENABLE_DEBUG_WPT,
		.debug_fault     = THINKOS_ENABLE_DEBUG_FAULT,
		.monitor_threads = THINKOS_ENABLE_MONITOR_THREADS
	},

	.misc = {
		.thread_void     = THINKOS_ENABLE_THREAD_VOID,
		.idle_wfi        = THINKOS_ENABLE_IDLE_WFI,
		.sched_debug     = THINKOS_ENABLE_SCHED_DEBUG,
		.ofast           = THINKOS_ENABLE_OFAST,
		.align           = THINKOS_ENABLE_ALIGN,
		.idle_hooks      = THINKOS_ENABLE_IDLE_HOOKS,
		.idle_msp        = THINKOS_ENABLE_IDLE_MSP,
	},

	.deprecated = {
		.asm_scheduler       = THINKOS_ASM_SCHEDULER,
		.rt_debug            = THINKOS_ENABLE_RT_DEBUG,
		.unroll_exceptions   = THINKOS_UNROLL_EXCEPTIONS,
		.stderr_fault_dump   = THINKOS_STDERR_FAULT_DUMP
	}
};

#if DEBUG
void __profile(void)
{
	const struct thinkos_profile * p = &thinkos_profile;

	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_JOIN            = %d", 
			 p->syscall.join);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_CLOCK           = %d", 
			 p->syscall.clock);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_ALARM           = %d", 
			 p->syscall.alarm);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_SLEEP           = %d", 
			 p->syscall.sleep);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_KRN_CTL         = %d", 
			 p->syscall.ctl);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_CRITICAL        = %d", 
			 p->syscall.critical);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_ESCALATE        = %d", 
			 p->syscall.escalate);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_IRQ_CTL         = %d", 
			 p->syscall.irq_ctl);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_PAUSE           = %d", 
			 p->syscall.pause);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_CANCEL          = %d", 
			 p->syscall.cancel);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_EXIT            = %d", 
			 p->syscall.exit);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_TERMINATE       = %d", 
			 p->syscall.terminate);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_BREAK           = %d", 
			 p->syscall.thread_break);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_CONSOLE         = %d", 
			 p->syscall.console);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_FLAG_WATCH      = %d", 
			 p->syscall.flag_watch);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_TIMED_CALLS     = %d", 
			 p->syscall.timed_calls);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_IRQ_TIMEDWAIT   = %d", 
			 p->syscall.irq_timedwait);

	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_THREAD_ALLOC    = %d", 
			 p->alloc.thread_alloc);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_MUTEX_ALLOC     = %d", 
			 p->alloc.mutex_alloc);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_COND_ALLOC      = %d", 
			 p->alloc.cond_alloc);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_SEM_ALLOC       = %d", 
			 p->alloc.sem_alloc);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_EVENT_ALLOC     = %d", 
			 p->alloc.event_alloc);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_GATE_ALLOC      = %d", 
			 p->alloc.gate_alloc);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_FLAG_ALLOC      = %d", 
			 p->alloc.flag_alloc);

	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_THREAD_INFO     = %d", 
			 p->feature.thread_info);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_THREAD_STAT     = %d", 
			 p->feature.thread_stat);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_IRQ_CYCCNT      = %d", 
			 p->feature.irq_cyccnt);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_IRQ_PRIORITY_0  = %d", 
			 p->feature.irq_priority_0);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_WQ_IRQ          = %d", 
			 p->feature.wq_irq);

	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_CONSOLE_BREAK   = %d", 
			 p->feature.console_break);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_CONSOLE_NONBLOCK = %d", 
			 p->feature.console_nonblock);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_CONSOLE_OPEN    = %d", 
			 p->feature.console_open);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_CONSOLE_DRAIN   = %d", 
			 p->feature.console_drain);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_CONSOLE_READ    = %d", 
			 p->feature.console_read);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_CONSOLE_MISC    = %d", 
			 p->feature.console_misc);

	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_COMM            = %d", 
			 p->feature.comm);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_MPU             = %d", 
			 p->feature.mpu);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_FPU             = %d", 
			 p->feature.fpu);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_FPU_LS          = %d", 
			 p->feature.fpu_ls);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_PROFILING       = %d", 
			 p->feature.profiling);

	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_ARG_CHECK       = %d", 
			 p->security.arg_check);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_DEADLOCK_CHECK  = %d", 
			 p->security.deadlock_check);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_SANITY_CHECK    = %d", 
			 p->security.sanity_check);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_STACK_INIT      = %d", 
			 p->security.stack_init);

	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_EXCEPTIONS      = %d", 
			 p->except.exceptions);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_BUSFAULT        = %d", 
			 p->except.busfault);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_USAGEFAULT      = %d", 
			 p->except.usagefault);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_MEMFAULT        = %d", 
			 p->except.memfault);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_HARDFAULT       = %d", 
			 p->except.hardfault);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_EXCEPT_CLEAR    = %d", 
			 p->except.except_clear);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_ERROR_TRAP      = %d", 
			 p->except.error_trap);
	DCC_LOG1(LOG_TRACE, "THINKOS_SYSRST_ONFAULT         = %d", 
			 p->except.sysrst_onfault);

	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_MONITOR         = %d", 
			 p->dbgmon.monitor);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_DMCLOCK         = %d", 
			 p->dbgmon.dmclock);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_DEBUG_STEP      = %d", 
			 p->dbgmon.debug_step);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_DEBUG_BKPT      = %d", 
			 p->dbgmon.debug_bkpt);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_DEBUG_WPT       = %d", 
			 p->dbgmon.debug_wpt);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_DEBUG_FAULT     = %d", 
			 p->dbgmon.debug_fault);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_MONITOR_THREADS = %d", 
			 p->dbgmon.monitor_threads);

	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_THREAD_VOID     = %d", 
			 p->misc.thread_void);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_IDLE_WFI        = %d", 
			 p->misc.idle_wfi);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_SCHED_DEBUG     = %d", 
			 p->misc.sched_debug);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_OFAST           = %d", 
			 p->misc.ofast);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_ALIGN           = %d", 
			 p->misc.align);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_IDLE_HOOKS      = %d", 
			 p->misc.idle_hooks);
	DCC_LOG1(LOG_TRACE, "THINKOS_ENABLE_IDLE_MSP        = %d", 
			 p->misc.idle_msp);

}
#endif

