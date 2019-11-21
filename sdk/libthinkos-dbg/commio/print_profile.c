/* 
 * dbgmon_print_profile.c
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

#define __THINKOS_DBGMON__
#include <thinkos/dbgmon.h>
#define __THINKOS_PROFILE__
#include <thinkos/profile.h>

/* -------------------------------------------------------------------------- 
 * ThinkOS profile 
 * --------------------------------------------------------------------------*/
void dbgmon_print_profile(const struct dbgmon_comm * comm, 
                          const struct thinkos_profile * p)
{

	dbgmon_printf(comm, "%32s = %d.%d\r\n", "THINKOS_KERNEL_VERSION",          
			 p->kernel.version.major, p->kernel.version.minor);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_TIMESHARE", 
				  p->kernel.timeshare);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_PREEMPTION", 
				  p->kernel.preemption);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_SCHED_LIMIT_MAX", 
				  p->kernel.sched_limit_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_SCHED_LIMIT_MIN", 
				  p->kernel.sched_limit_min);

	/* Limit */
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_THREADS_MAX", 
				  p->limit.threads_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_NRT_THREADS_MAX", 
				  p->limit.nrt_threads_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_THREADS_MAX", 
				  p->limit.mutex_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_COND_MAX", 
				  p->limit.cond_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_SEMAPHORE_MAX", 
				  p->limit.semaphore_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_EVENT_MAX", 
				  p->limit.event_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_FLAG_MAX", p->limit.flag_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_GATE_MAX", p->limit.gate_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_QUEUE_MAX", 
				  p->limit.queue_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_IRQ_MAX", p->limit.irq_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_DMA_MAX", p->limit.dma_max);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_EXCEPT_STACK_SIZE", 
				  p->limit.except_stack_size);

	/* Syscall */

	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_JOIN", p->syscall.join);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CLOCK", p->syscall.clock);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_ALARM", p->syscall.alarm);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_SLEEP", p->syscall.sleep);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_KRN_CTL", p->syscall.ctl);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CRITICAL", p->syscall.critical);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_ESCALATE", p->syscall.escalate);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IRQ_CTL", p->syscall.irq_ctl);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_PAUSE", p->syscall.pause);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CANCEL", p->syscall.cancel);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_EXIT", p->syscall.exit);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_TERMINATE", p->syscall.terminate);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_BREAK", p->syscall.thread_break);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE", p->syscall.console);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_FLAG_WATCH", p->syscall.flag_watch);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_TIMED_CALLS", p->syscall.timed_calls);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IRQ_TIMEDWAIT", p->syscall.irq_timedwait);

	/* Alloc */
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_THREAD_ALLOC", p->alloc.thread_alloc);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MUTEX_ALLOC", p->alloc.mutex_alloc);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_COND_ALLOC", p->alloc.cond_alloc);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_SEM_ALLOC", p->alloc.sem_alloc);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_EVENT_ALLOC", p->alloc.event_alloc);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_GATE_ALLOC", p->alloc.gate_alloc);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_FLAG_ALLOC", p->alloc.flag_alloc);

	/* Feature */

	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_THREAD_INFO", p->feature.thread_info);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_THREAD_STAT", p->feature.thread_stat);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IRQ_CYCCNT", p->feature.irq_cyccnt);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IRQ_PRIORITY_0", p->feature.irq_priority_0);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_WQ_IRQ", p->feature.wq_irq);

	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_BREAK", p->feature.console_break);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_NONBLOCK", p->feature.console_nonblock);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_OPEN", p->feature.console_open);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_DRAIN", p->feature.console_drain);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_READ", p->feature.console_read);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_MISC", p->feature.console_misc);

	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_COMM", p->feature.comm);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MPU", p->feature.mpu);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_FPU", p->feature.fpu);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_FPU_LS", p->feature.fpu_ls);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_PROFILING", p->feature.profiling);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MEM_MAP", p->feature.mem_map);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_FLASH_MEM", p->feature.flash_mem);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_KRN_TRACE", p->feature.krn_trace);

	/* Security */

	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_ARG_CHECK", p->security.arg_check);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DEADLOCK_CHECK", p->security.deadlock_check);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_SANITY_CHECK", p->security.sanity_check);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_STACK_INIT", p->security.stack_init);

	/* Except */

	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_EXCEPTIONS", p->except.exceptions);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_BUSFAULT", p->except.busfault);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_USAGEFAULT", p->except.usagefault);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MEMFAULT", p->except.memfault);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_HARDFAULT", p->except.hardfault);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_EXCEPT_CLEAR", p->except.except_clear);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_ERROR_TRAP", p->except.error_trap);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_SYSRST_ONFAULT", p->except.sysrst_onfault);

	/* DbgMon */

	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MONITOR", p->dbgmon.monitor);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DMCLOCK", p->dbgmon.dmclock);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DEBUG_STEP", p->dbgmon.debug_step);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DEBUG_BKPT", p->dbgmon.debug_bkpt);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DEBUG_WPT", p->dbgmon.debug_wpt);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DEBUG_FAULT", p->dbgmon.debug_fault);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MONITOR_THREADS", p->dbgmon.monitor_threads);

	/* Misc */

	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_THREAD_VOID", p->misc.thread_void);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IDLE_WFI", p->misc.idle_wfi);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_SCHED_DEBUG", p->misc.sched_debug);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_OFAST", p->misc.ofast);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_ALIGN", p->misc.align);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IDLE_HOOKS", p->misc.idle_hooks);
	dbgmon_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IDLE_MSP", p->misc.idle_msp);

}

