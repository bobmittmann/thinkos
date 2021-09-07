/* 
 * config.h
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

#ifndef __CONFIG_H__
#define __CONFIG_H__

#define THINKOS_EXCEPT_STACK_SIZE       320

#define THINKOS_IRQ_MAX                 80

#define THINKOS_ENABLE_IRQ_CTL          1

#define THINKOS_THREADS_MAX             32

#define THINKOS_ENABLE_THREAD_ALLOC     1

#define THINKOS_ENABLE_THREAD_INFO      1

#define THINKOS_ENABLE_THREAD_STAT      1

#define THINKOS_ENABLE_JOIN             1

#define THINKOS_ENABLE_PAUSE            1

#define THINKOS_ENABLE_CANCEL           1

#define THINKOS_ENABLE_EXIT             1

#define THINKOS_ENABLE_TERMINATE        1

#define THINKOS_ENABLE_TIMESHARE        1

#define THINKOS_SCHED_LIMIT_MAX         32

#define THINKOS_SCHED_LIMIT_MIN         1

#define THINKOS_MUTEX_MAX               64

#define THINKOS_ENABLE_MUTEX_ALLOC      1

#define THINKOS_COND_MAX                64

#define THINKOS_ENABLE_COND_ALLOC       1

#define THINKOS_SEMAPHORE_MAX           64

#define THINKOS_ENABLE_SEM_ALLOC        1

#define THINKOS_EVENT_MAX               64

#define THINKOS_ENABLE_EVENT_ALLOC      1

#define THINKOS_FLAG_MAX                64

#define THINKOS_ENABLE_FLAG_ALLOC       1

#define THINKOS_ENABLE_FLAG_WATCH       1

#define THINKOS_GATE_MAX                64

#define THINKOS_ENABLE_GATE_ALLOC       1

#define THINKOS_COMM_MAX                1

#define THINKOS_ENABLE_TIMED_CALLS      1

#define THINKOS_ENABLE_ALARM            1

#define THINKOS_ENABLE_SLEEP            1

#define THINKOS_ENABLE_BREAK            1

#define THINKOS_ENABLE_ARG_CHECK        1

#define THINKOS_ENABLE_DEADLOCK_CHECK   1

#define THINKOS_ENABLE_SANITY_CHECK     1

#define THINKOS_ENABLE_EXCEPTIONS       1

#define THINKOS_SYSRST_ONFAULT          1

#define THINKOS_ENABLE_BUSFAULT         1

#define THINKOS_ENABLE_USAGEFAULT       1

#define THINKOS_ENABLE_MEMFAULT         1

#define THINKOS_ENABLE_HARDFAULT        1

#define THINKOS_ENABLE_SCHED_DEBUG      1

#define THINKOS_ENABLE_PROFILING        1

#define THINKOS_ENABLE_MONITOR          1

#define THINKOS_ENABLE_MONITOR_CLOCK    1

#define THINKOS_ENABLE_DEBUG_STEP       1

#define THINKOS_ENABLE_THREAD_FAULT     1

#define THINKOS_ENABLE_MPU              1

#define THINKOS_ENABLE_FPU              1

#define THINKOS_ENABLE_CTL              1

#define THINKOS_ENABLE_STACK_INIT       1

#define THINKOS_ENABLE_THREAD_VOID      1

#define THINKOS_ENABLE_PREEMPTION       1

#define THINKOS_ENABLE_CRITICAL         1

#define THINKOS_ENABLE_ESCALATE         1

#define THINKOS_ENABLE_IDLE_WFI         1

#endif /* __CONFIG_H__ */

