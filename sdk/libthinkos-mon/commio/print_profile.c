/* 
 * monitor_print_profile.c
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

#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#define __THINKOS_PROFILE__
#include <thinkos/profile.h>

/* -------------------------------------------------------------------------- 
 * ThinkOS profile 
 * --------------------------------------------------------------------------*/
void monitor_print_profile(const struct monitor_comm * comm, 
                          const struct thinkos_profile * p)
{

	monitor_printf(comm, "%32s = %d.%d\r\n", "THINKOS_KERNEL_VERSION",          
			 p->kernel.version.major, p->kernel.version.minor);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_TIMESHARE", 
				  p->kernel.timeshare);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_PREEMPTION", 
				  p->kernel.preemption);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_SCHED_LIMIT_MAX", 
				  p->kernel.sched_limit_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_SCHED_LIMIT_MIN", 
				  p->kernel.sched_limit_min);

	/* Limit */
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_THREADS_MAX", 
				  p->limit.threads_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_NRT_THREADS_MAX", 
				  p->limit.nrt_threads_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_THREADS_MAX", 
				  p->limit.mutex_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_COND_MAX", 
				  p->limit.cond_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_SEMAPHORE_MAX", 
				  p->limit.semaphore_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_EVENT_MAX", 
				  p->limit.event_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_FLAG_MAX", p->limit.flag_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_GATE_MAX", p->limit.gate_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_QUEUE_MAX", 
				  p->limit.queue_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_IRQ_MAX", p->limit.irq_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_DMA_MAX", p->limit.dma_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_FLASH_MEM_MAX", 
				   p->limit.flash_max);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_EXCEPT_STACK_SIZE", 
				  p->limit.except_stack_size);

	/* Syscall */

	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_JOIN", p->syscall.join);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CLOCK", p->syscall.clock);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_ALARM", p->syscall.alarm);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_SLEEP", p->syscall.sleep);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_KRN_CTL", p->syscall.ctl);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CRITICAL", p->syscall.critical);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_ESCALATE", p->syscall.escalate);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IRQ_CTL", p->syscall.irq_ctl);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_PAUSE", p->syscall.pause);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CANCEL", p->syscall.cancel);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_EXIT", p->syscall.exit);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_TERMINATE", p->syscall.terminate);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_BREAK", p->syscall.thread_break);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE", p->syscall.console);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_FLAG_WATCH", p->syscall.flag_watch);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_TIMED_CALLS", p->syscall.timed_calls);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IRQ_TIMEDWAIT", p->syscall.irq_timedwait);

	/* Alloc */
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_THREAD_ALLOC", p->alloc.thread_alloc);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MUTEX_ALLOC", p->alloc.mutex_alloc);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_COND_ALLOC", p->alloc.cond_alloc);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_SEM_ALLOC", p->alloc.sem_alloc);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_EVENT_ALLOC", p->alloc.event_alloc);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_GATE_ALLOC", p->alloc.gate_alloc);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_FLAG_ALLOC", p->alloc.flag_alloc);

	/* Feature */

	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_THREAD_INFO", p->feature.thread_info);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_THREAD_STAT", p->feature.thread_stat);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IRQ_CYCCNT", p->feature.irq_cyccnt);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IRQ_PRIORITY_0", p->feature.irq_priority_0);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_WQ_IRQ", p->feature.wq_irq);

	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_BREAK", p->feature.console_break);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_NONBLOCK", p->feature.console_nonblock);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_OPEN", p->feature.console_open);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_DRAIN", p->feature.console_drain);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_READ", p->feature.console_read);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_CONSOLE_MISC", p->feature.console_misc);

	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_COMM", p->feature.comm);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MPU", p->feature.mpu);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_FPU", p->feature.fpu);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_FPU_LS", p->feature.fpu_ls);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_PROFILING", p->feature.profiling);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MEM_MAP", p->feature.mem_map);
	/* Security */

	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_ARG_CHECK", p->security.arg_check);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DEADLOCK_CHECK", p->security.deadlock_check);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_SANITY_CHECK", p->security.sanity_check);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_STACK_INIT", p->security.stack_init);

	/* Except */

	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_EXCEPTIONS", p->except.exceptions);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_BUSFAULT", p->except.busfault);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_USAGEFAULT", p->except.usagefault);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MEMFAULT", p->except.memfault);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_HARDFAULT", p->except.hardfault);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_ERROR_TRAP", p->except.error_trap);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_SYSRST_ONFAULT", p->except.sysrst_onfault);

	/* Monitor */

	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MONITOR", 
				   p->monitor.enabled);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MONITOR_CLOCK", 
				   p->monitor.clock);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_MONITOR_THREADS", 
				   p->monitor.threads);

	/* Debug */

	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DEBUG", 
				   p->debug.enabled);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DEBUG_STEP", 
				   p->debug.step);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DEBUG_BKPT", 
				   p->debug.bkpt);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DEBUG_WPT", 
				   p->debug.wpt);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_THREAD_FAULT", 
				   p->debug.fault);

	/* Misc */

	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IDLE_WFI", p->misc.idle_wfi);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_SCHED_DEBUG", p->misc.sched_debug);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_OFAST", 
				   p->misc.ofast);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_UNALIGN_TRAP", 
				   p->misc.unalign_trap);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_DIV0_TRAP", 
				   p->misc.div0_trap);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_STACK_ALIGN", 
				   p->misc.stack_align);
	monitor_printf(comm, "%32s = %d\r\n", "THINKOS_ENABLE_IDLE_HOOKS", p->misc.idle_hooks);

}

