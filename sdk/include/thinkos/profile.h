/* 
 * thinkos/profile.h
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

#ifndef THINKOS_ENABLE_OBJ_ALLOC
#define THINKOS_ENABLE_OBJ_ALLOC        1
#endif

#ifndef THINKOS_THREADS_MAX 
#define THINKOS_THREADS_MAX             8
#endif

#ifndef THINKOS_NRT_THREADS_MAX
#define THINKOS_NRT_THREADS_MAX         0
#endif

#ifndef THINKOS_ENABLE_THREAD_ALLOC
#define THINKOS_ENABLE_THREAD_ALLOC     (THINKOS_ENABLE_OBJ_ALLOC) 
#endif

#ifndef THINKOS_ENABLE_THREAD_INFO
#define THINKOS_ENABLE_THREAD_INFO      0
#endif

/* THINKOS_ENABLE_THREAD_STAT - Add thread status property to cache 
   the current thread state. */
#ifndef THINKOS_ENABLE_THREAD_STAT
#define THINKOS_ENABLE_THREAD_STAT      0
#endif

/* THINKOS_ENABLE_JOIN - Used when its desirable to create and destroy threads 
   dynamically */
#ifndef THINKOS_ENABLE_JOIN
#define THINKOS_ENABLE_JOIN             0
#endif

/* THINKOS_ENABLE_PAUSE  - Allow for pausing and resuming a single thread 
   execution. This is needed for GDB and debug monitor. 
   Also adds thinkos_pause() and thinkos_resume() calls. */
#ifndef THINKOS_ENABLE_PAUSE
#define THINKOS_ENABLE_PAUSE            0
#endif

/* THINKOS_ENABLE_CANCEL - A thread can cancel another thread asynchronously. 
   Add thinkos_cancel() call. */
#ifndef THINKOS_ENABLE_CANCEL
#define THINKOS_ENABLE_CANCEL           0
#endif

/* THINKOS_ENABLE_EXIT - With this option it's possible to terminate a thread by calling thinkos_thread_exit(). Otherwise the thread terminates when the initial
   thread task exits. */
#ifndef THINKOS_ENABLE_EXIT
#define THINKOS_ENABLE_EXIT             0
#endif

#ifndef THINKOS_ENABLE_TERMINATE
#define THINKOS_ENABLE_TERMINATE        0
#endif

#ifndef THINKOS_ENABLE_BREAK
#define THINKOS_ENABLE_BREAK            0
#endif

#ifndef THINKOS_ENABLE_TIMESHARE
#define THINKOS_ENABLE_TIMESHARE        0
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
#define THINKOS_ENABLE_MUTEX_ALLOC      (THINKOS_ENABLE_OBJ_ALLOC)
#endif

#ifndef THINKOS_COND_MAX
#define THINKOS_COND_MAX                8
#endif

#ifndef THINKOS_ENABLE_COND_ALLOC
#define THINKOS_ENABLE_COND_ALLOC       (THINKOS_ENABLE_OBJ_ALLOC)
#endif

#ifndef THINKOS_SEMAPHORE_MAX
#define THINKOS_SEMAPHORE_MAX           8
#endif

#ifndef THINKOS_ENABLE_SEM_ALLOC
#define THINKOS_ENABLE_SEM_ALLOC        (THINKOS_ENABLE_OBJ_ALLOC)
#endif

#ifndef THINKOS_EVENT_MAX
#define THINKOS_EVENT_MAX               4
#endif

#ifndef THINKOS_ENABLE_EVENT_ALLOC
#define THINKOS_ENABLE_EVENT_ALLOC      (THINKOS_ENABLE_OBJ_ALLOC)
#endif

#ifndef THINKOS_FLAG_MAX
#define THINKOS_FLAG_MAX                8
#endif

#ifndef THINKOS_ENABLE_FLAG_ALLOC
#define THINKOS_ENABLE_FLAG_ALLOC       (THINKOS_ENABLE_OBJ_ALLOC)
#endif

#ifndef THINKOS_ENABLE_FLAG_WATCH
#define THINKOS_ENABLE_FLAG_WATCH       0
#endif

#ifndef THINKOS_GATE_MAX
#define THINKOS_GATE_MAX                8
#endif

#ifndef THINKOS_ENABLE_GATE_ALLOC
#define THINKOS_ENABLE_GATE_ALLOC       (THINKOS_ENABLE_OBJ_ALLOC)
#endif

#ifndef THINKOS_ENABLE_TIMED_CALLS
#define THINKOS_ENABLE_TIMED_CALLS      0
#endif

#ifndef THINKOS_ENABLE_ALARM
#define THINKOS_ENABLE_ALARM            0
#endif

#ifndef THINKOS_ENABLE_SLEEP
#define THINKOS_ENABLE_SLEEP            0
#endif

#ifndef THINKOS_ENABLE_CTL
#define THINKOS_ENABLE_CTL              0
#endif

#ifndef THINKOS_ENABLE_PREEMPTION
#define THINKOS_ENABLE_PREEMPTION       0
#endif

#ifndef THINKOS_ENABLE_CRITICAL
#define THINKOS_ENABLE_CRITICAL         0
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
#define THINKOS_ENABLE_CONSOLE_OPEN     0
#endif

/* Enable the console drain call.
 */
#ifndef THINKOS_ENABLE_CONSOLE_DRAIN
#define THINKOS_ENABLE_CONSOLE_DRAIN    0
#endif

/* Enable the console read call.
 */
#ifndef THINKOS_ENABLE_CONSOLE_READ
#define THINKOS_ENABLE_CONSOLE_READ     0
#endif

/* Enable the console miscelaneous calls Ex: is_connected.
 */
#ifndef THINKOS_ENABLE_CONSOLE_MISC
#define THINKOS_ENABLE_CONSOLE_MISC     0
#endif

#ifndef THINKOS_COMM_MAX
#define THINKOS_COMM_MAX                0
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
#define THINKOS_ENABLE_EXCEPTIONS       0
#endif

#ifndef THINKOS_ENABLE_HARDFAULT
#define THINKOS_ENABLE_HARDFAULT        0
#endif

#ifndef THINKOS_ENABLE_BUSFAULT
#define THINKOS_ENABLE_BUSFAULT         0
#endif

#ifndef THINKOS_ENABLE_MEMFAULT 
#define THINKOS_ENABLE_MEMFAULT         0
#endif

#ifndef THINKOS_ENABLE_USAGEFAULT 
#define THINKOS_ENABLE_USAGEFAULT       0
#endif

/* THINKOS_ENABLE_ERROR_TRAP - enable errors on syscalls to issue an exception.
   Depends on monitor. */
#ifndef THINKOS_ENABLE_ERROR_TRAP
#define THINKOS_ENABLE_ERROR_TRAP       0
#endif

#ifndef THINKOS_SYSRST_ONFAULT
#define THINKOS_SYSRST_ONFAULT          0
#endif

#ifndef THINKOS_EXCEPT_STACK_SIZE
#define THINKOS_EXCEPT_STACK_SIZE       320
#endif

#ifndef THINKOS_MONITOR_STACK_SIZE
#define THINKOS_MONITOR_STACK_SIZE      512
#endif

/* THINKOS_ENABLE_OFAST - enable the GCC compiler flag: -Ofast on
 critical kernel code. */
#ifndef THINKOS_ENABLE_OFAST
#define THINKOS_ENABLE_OFAST            1
#endif

/* Kernel Zeros all the memory blocks and data-structures on allocation
   or initialization */
#ifndef THINKOS_ENABLE_MEMORY_CLEAR
#define THINKOS_ENABLE_MEMORY_CLEAR     0
#endif


/* THINKOS_ENABLE_IDLE_WFI - enable the WFI instruction on the IDLE loop.
   In normal circumstances this flag should be enabled.
   Some platforms will go into a deep sleep state disabling the debug
   susbsystem. It may be desirable to disable this option when debugging
   the kernel on such cases. . */
#ifndef THINKOS_ENABLE_IDLE_WFI
#define THINKOS_ENABLE_IDLE_WFI         0
#endif

/* THINKOS_ENABLE_STACK_INIT: Fill the thread stacks with a test pattern 
   easily recognizeable: 0xDEADBEEF
   */
#ifndef THINKOS_ENABLE_STACK_INIT
#define THINKOS_ENABLE_STACK_INIT       0
#endif

/* THINKOS_ENABLE_STACK_ALIGN - enable 64bits stack alignment. */
#ifndef THINKOS_ENABLE_STACK_ALIGN
#define THINKOS_ENABLE_STACK_ALIGN      1
#endif

/* THINKOS_ENABLE_UNALIGN_TRAP - Enable unaligned memory access fault. 
 The GCC compiler flag: -mno-unaligned-access must
 be enabled to avoid generating unaligned code. */
#ifndef THINKOS_ENABLE_UNALIGN_TRAP
#define THINKOS_ENABLE_UNALIGN_TRAP     0
#endif

/* Enable division by zero trap */
#ifndef THINKOS_ENABLE_DIV0_TRAP
#define THINKOS_ENABLE_DIV0_TRAP        0
#endif

/* THINKOS_ENABLE_MONITOR: Enable the kernel monitor (MONITOR) framework.
   It's used to implement the bootloader and basic console, flash
   read/write and firmware upload. 
   Basic debuging facility like: start, stop, 
   resume, error report operations as well.
   Requires the kernel services (KRNSVC) to be enabled.
   */
#ifndef THINKOS_ENABLE_MONITOR
#define THINKOS_ENABLE_MONITOR               0
#endif

/* THINKOS_ENABLE_MONITOR_THREADS: Monitors the creation and termination of 
   threads. It's used by the bootloader to sequence the system initialization 
   also the monitor can use it to profile the user application. 
 
   Requires the kernel monitor to be enabled.
 */
#ifndef THINKOS_ENABLE_MONITOR_THREADS 
#define THINKOS_ENABLE_MONITOR_THREADS       0
#endif

/* THINKOS_ENABLE_MONITOR_CLOCK: Enable the kernel monitor clock.
   This is a kernel clock dedicated to the kernel monitor (MONITOR)
   susbsystem.

   Some modules or user defined monitor can use it to implement
   timers.

   Requires the clock module to be enabled.
   */
#ifndef THINKOS_ENABLE_MONITOR_CLOCK
#define THINKOS_ENABLE_MONITOR_CLOCK        0
#endif

/* THINKOS_ENABLE_MONITOR_SYSCALL: Enable user syscall to install
   a monitor. */
#ifndef THINKOS_ENABLE_MONITOR_SYSCALL
#define THINKOS_ENABLE_MONITOR_SYSCALL      0
#endif

/* THINKOS_ENABLE_MONITOR_SCHED: Enable the Monitor thread scheduler. */
#ifndef THINKOS_ENABLE_MONITOR_SCHED
#define THINKOS_ENABLE_MONITOR_SCHED        0
#endif

/* THINKOS_ENABLE_DEBUG_BASE - Enable the kernel debug base subsystem.
   All other DEBUG features depend on this flag
   */
#ifndef THINKOS_ENABLE_DEBUG_BASE
#define THINKOS_ENABLE_DEBUG_BASE           0
#endif

/* THINKOS_ENABLE_DEBUG - Enable the kernel debug subsystem.
   Requires the DebugMonitor fault to be present on the platform.
   */
#ifndef THINKOS_ENABLE_DEBUG
#define THINKOS_ENABLE_DEBUG                0
#endif

/* THINKOS_ENABLE_DEBUG_STEP - Step by step excecution
   Requires the the debug subsystem.
   */
#ifndef THINKOS_ENABLE_DEBUG_STEP 
#define THINKOS_ENABLE_DEBUG_STEP           0
#endif

/* THINKOS_ENABLE_DEBUG_BKPT - Support for breakpoints 
   Requires the the debug subsystem.
   */
#ifndef THINKOS_ENABLE_DEBUG_BKPT
#define THINKOS_ENABLE_DEBUG_BKPT           0
#endif

/* THINKOS_ENABLE_DEBUG_WPT - Support for watchpoints
   Requires the the debug subsystem.
   */
#ifndef THINKOS_ENABLE_DEBUG_WPT 
#define THINKOS_ENABLE_DEBUG_WPT            0
#endif

/* THINKOS_ENABLE_THREAD_FAULT - Per thread fault/error. With this option the 
   scheduler remains operational as long as a non-critical error ocurs.
   The falted thread is removed from the ready queue and marked appropriatelly.
   The monitor is signaled and the scheduler/kernel resumes operation with the 
   remaining of threads. 
 */
#ifndef THINKOS_ENABLE_THREAD_FAULT  
#define THINKOS_ENABLE_THREAD_FAULT         0 
#endif

/* THINKOS_ENABLE_SCHED_DEBUG - Enable scheduler debug trace */
#ifndef THINKOS_ENABLE_SCHED_DEBUG
#define THINKOS_ENABLE_SCHED_DEBUG          0
#endif

/* THINKOS_ENABLE_SCHED_ERROR - Enable stack check on scheduler */
#ifndef THINKOS_ENABLE_SCHED_ERROR
#define THINKOS_ENABLE_SCHED_ERROR          0
#endif

/* THINKOS_ENABLE_KRN_SCHED_BRK - Enable thread break */
#ifndef THINKOS_ENABLE_KRN_SCHED_BRK
#define THINKOS_ENABLE_KRN_SCHED_BRK        0
#endif

/* THINKOS_ENABLE_KRN_SCHED_SVC - This option is used to request the execution of
   a scheduler priority tasklet. */
#ifndef THINKOS_ENABLE_KRN_SCHED_SVC
#define THINKOS_ENABLE_KRN_SCHED_SVC        0
#endif

/* THINKOS_ENABLE_IDLE_HOOKS - This option is used to request the execution of
   a tasklet when the system is idle. The tasklet runs on the IDLE 
   thread context.
 */
#ifndef THINKOS_ENABLE_IDLE_HOOKS       
#define THINKOS_ENABLE_IDLE_HOOKS           0
#endif

/* THINKOS_ENABLE_I_CALLS - Enable building functions to be used in interrupt 
   handlers only. These functions are suffixed by _i.
   */
#ifndef THINKOS_ENABLE_I_CALLS
#define THINKOS_ENABLE_I_CALLS               0
#endif


/* -------------------------------------------------------------------------- 
 * Experimental configuration options
 *
 * Non implemented/Planned options, should not be used in 
 *   production code.
 */

/* THINKOS_ENABLE_KRNSVC - Enable kernel services. Provides a
   framework for high priority drivers and debug.
   It uses the undefined instruction UDF opcode. It will 
   cause a usage fault exception which handles the calls.
 */
#ifndef THINKOS_ENABLE_KRNSVC
#define THINKOS_ENABLE_KRNSVC               0
#endif

/* THINKOS_FLASH_MEM_MAX - enable the kernel to handle low-level
   flash memory erase read and write operations. */

#ifndef THINKOS_FLASH_MEM_MAX
#define THINKOS_FLASH_MEM_MAX                0
#endif 

#ifndef THINKOS_ENABLE_MEMORY_MAP
#define THINKOS_ENABLE_MEMORY_MAP            0
#endif

#ifndef THINKOS_DMA_MAX 
#define THINKOS_DMA_MAX                      0
#endif

#ifndef THINKOS_QUEUE_MAX 
#define THINKOS_QUEUE_MAX                    0
#endif

/* THINKOS_ENABLE_STACK_LIMIT - Enable thread stack limit check on
   scheduler. With this option enabled the kernel verify if the thread
   stack goes out of bounds and signal a fault condition to the monitor.
*/
#ifndef THINKOS_ENABLE_STACK_LIMIT
#define THINKOS_ENABLE_STACK_LIMIT          0     
#endif

/* THINKOS_ENABLE_IRQ_ASM_FAST - Enable generation of faster assembler 
   interrupt handlers stubs. This will generate one stub per IRQ as oposed 
 to a single stub shared by all. */
#ifndef THINKOS_ENABLE_IRQ_ASM_FAST
#define THINKOS_ENABLE_IRQ_ASM_FAST         0
#endif

/* THINKOS_ENABLE_PRIVILEGED_THREAD - Enable privileged thread */
#ifndef THINKOS_ENABLE_PRIVILEGED_THREAD
#define THINKOS_ENABLE_PRIVILEGED_THREAD    0
#endif

/* THINKOS_ENABLE_APP_FLAT          - Enable flat binary application */
#ifndef THINKOS_ENABLE_APP
#define THINKOS_ENABLE_APP                  0
#endif

/* THINKOS_ENABLE_APP_CRC          - Enable flat binary CRC ckeck for applications */
#ifndef THINKOS_ENABLE_APP_CRC
#define THINKOS_ENABLE_APP_CRC              0
#endif

/* THINKOS_ENABLE_UDELAY_CALIBRATE - Enable kernel to calibrate 
   the delay loop */
#ifndef THINKOS_ENABLE_UDELAY_CALIBRATE
#define THINKOS_ENABLE_UDELAY_CALIBRATE     1
#endif

/* THINKOS_ENABLE_READY_MASK - Enable runmask bitmap... */
#ifndef THINKOS_ENABLE_READY_MASK              
#define THINKOS_ENABLE_READY_MASK           0
#endif

/* THINKOS_ENABLE_DATE_AND_TIME - Enable wallclock date and time ... */
#ifndef THINKOS_ENABLE_DATE_AND_TIME 
#define THINKOS_ENABLE_DATE_AND_TIME        0
#endif

/* Enable kernel support for real time trace. The kernel hold the trace
   ring in a protected memory and mediate its access by the application 
   and debug monitor services. */
#ifndef THINKOS_ENABLE_TRACE    
#define THINKOS_ENABLE_TRACE                0 
#endif

#ifndef THINKOS_ENABLE_CTL_KRN_INFO
#define THINKOS_ENABLE_CTL_KRN_INFO         0
#endif

#ifndef THINKOS_THREAD_STACK_MAX
#define THINKOS_THREAD_STACK_MAX            65280
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


#if (THINKOS_ENABLE_IRQ_TIMEDWAIT) && !(THINKOS_ENABLE_TIMED_CALLS)
#error "THINKOS_ENABLE_IRQ_TIMEDWAIT depends on THINKOS_ENABLE_TIMED_CALLS"
#endif

//#if (THINKOS_ENABLE_IRQ_TIMEDWAIT) && !(THINKOS_ENABLE_WQ_IRQ)
//#error "THINKOS_ENABLE_IRQ_TIMEDWAIT depends on THINKOS_ENABLE_WQ_IRQ"
//#endif

//#if (THINKOS_ENABLE_IRQ_CYCCNT) && !(THINKOS_ENABLE_WQ_IRQ)
//#error "THINKOS_ENABLE_IRQ_CYCCNT depends on THINKOS_ENABLE_WQ_IRQ"
//#endif

#if (THINKOS_ENABLE_IRQ_CYCCNT) && !(THINKOS_ENABLE_IRQ_CTL)
#error "THINKOS_ENABLE_IRQ_CYCCNT depends on THINKOS_ENABLE_IRQ_CTL"
#endif

#if (THINKOS_ENABLE_HARDFAULT) && !(THINKOS_ENABLE_EXCEPTIONS)
#error "THINKOS_ENABLE_HARDFAULT depends on THINKOS_ENABLE_EXCEPTIONS"
#endif

#if (THINKOS_ENABLE_MEMFAULT) && !(THINKOS_ENABLE_EXCEPTIONS)
#error "THINKOS_ENABLE_MEMFAULT depends on THINKOS_ENABLE_EXCEPTIONS"
#endif

#if (THINKOS_ENABLE_BUSFAULT) && !(THINKOS_ENABLE_EXCEPTIONS)
#error "THINKOS_ENABLE_BUSFAULT depends on THINKOS_ENABLE_EXCEPTIONS"
#endif

#if (THINKOS_ENABLE_USAGEFAULT) && !(THINKOS_ENABLE_EXCEPTIONS)
#error "THINKOS_ENABLE_USAGEFAULT depends on THINKOS_ENABLE_EXCEPTIONS"
#endif

#if (THINKOS_ENABLE_KRNSVC) && !(THINKOS_ENABLE_USAGEFAULT)
#error "THINKOS_ENABLE_KRNSVC depends on THINKOS_ENABLE_USAGEFAULT"
#endif

#if (THINKOS_ENABLE_ERROR_TRAP) && !(THINKOS_ENABLE_DEBUG_BASE)
#error "THINKOS_ENABLE_ERROR_TRAP depends on THINKOS_ENABLE_DEBUG_BASE"
#endif

#if (THINKOS_ENABLE_ERROR_TRAP) && !(THINKOS_ENABLE_MONITOR)
#error "THINKOS_ENABLE_ERROR_TRAP depends on THINKOS_ENABLE_MONITOR"
#endif

/*
#if (THINKOS_ENABLE_I_CALLS) && !(THINKOS_ENABLE_KRNSVC)
#error "THINKOS_ENABLE_I_CALLS depends on THINKOS_ENABLE_KRNSVC"
#endif
*/

/* debug breakpoint depends on debug base */
#if (THINKOS_ENABLE_DEBUG_BKPT) && !(THINKOS_ENABLE_DEBUG_BASE)
#error "THINKOS_ENABLE_DEBUG_BKPT depends on THINKOS_ENABLE_DEBUG_BASE"
#endif

/* debug step depends on debug breakpoint */
#if (THINKOS_ENABLE_DEBUG_STEP) && !(THINKOS_ENABLE_DEBUG_BKPT)
#error "THINKOS_ENABLE_DEBUG_STEP depends on THINKOS_ENABLE_DEBUG_BKPT"
#endif

/* debug watchpoint depends on debug breakpoint */
#if (THINKOS_ENABLE_DEBUG_WPT) && !(THINKOS_ENABLE_DEBUG_BKPT)
#error "THINKOS_ENABLE_DEBUG_WPT depends on THINKOS_ENABLE_DEBUG_BKPT"
#endif

/* debug breakpoint depends on debug */
#if (THINKOS_ENABLE_DEBUG_BKPT) && !(THINKOS_ENABLE_DEBUG)
#error "THINKOS_ENABLE_DEBUG_BKPT depends on THINKOS_ENABLE_DEBUG"
#endif

/* monitor depends on idle hooks 
#if (THINKOS_ENABLE_MONITOR) && !(THINKOS_ENABLE_IDLE_HOOKS)
#error "THINKOS_ENABLE_MONITOR depends on THINKOS_ENABLE_IDLE_HOOKS"
#endif
*/

#if (THINKOS_ENABLE_MONITOR_CLOCK) && !(THINKOS_ENABLE_MONITOR)
#error "THINKOS_ENABLE_MONITOR_CLOCK depends on THINKOS_ENABLE_MONITOR"
#endif

/* user monitor request (syscall) depends on monitor */
#if (THINKOS_ENABLE_MONITOR_SYSCALL) && !(THINKOS_ENABLE_MONITOR)
#error "THINKOS_ENABLE_MONITOR_SYSCALL depends on THINKOS_ENABLE_MONITOR"
#endif

/* thread monitoring depends on monitor */
#if (THINKOS_ENABLE_MONITOR_THREADS) && !(THINKOS_ENABLE_MONITOR)
#error "THINKOS_ENABLE_MONITOR_THREADS depends on THINKOS_ENABLE_MONITOR"
#endif

/* monitor thread scheduler depends on monitor */
#if (THINKOS_ENABLE_MONITOR_SCHED) && !(THINKOS_ENABLE_MONITOR)
#error "THINKOS_ENABLE_MONITOR_SCHED depends on THINKOS_ENABLE_MONITOR"
#endif

#if (THINKOS_ENABLE_MONITOR) && ((THINKOS_MONITOR_STACK_SIZE) < (16 * 4))
#error "THINKOS_MONITOR_STACK_SIZE too small"
#endif

#if (THINKOS_ENABLE_FPU) && !(THINKOS_ENABLE_STACK_ALIGN)
#error "THINKOS_ENABLE_FPU depends on THINKOS_ENABLE_STACK_ALIGN"
#endif
 
#if (THINKOS_ENABLE_MEMFAULT) && !(THINKOS_ENABLE_MPU)
#error "THINKOS_ENABLE_MEMFAULT depends on THINKOS_ENABLE_MPU"
#endif

#if (THINKOS_ENABLE_CRITICAL) && !(THINKOS_ENABLE_PREEMPTION)
#error "THINKOS_ENABLE_CRITICAL depends on THINKOS_ENABLE_PREEMPTION"
#endif

#if (THINKOS_ENABLE_TIMESHARE) && !(THINKOS_ENABLE_PREEMPTION)
#error "THINKOS_ENABLE_TIMESHARE depends on THINKOS_ENABLE_PREEMPTION"
#endif

#if (THINKOS_ENABLE_EXIT) && !(THINKOS_ENABLE_TERMINATE)
#error "THINKOS_ENABLE_EXIT depends on THINKOS_ENABLE_TERMINATE"
#endif

#if (THINKOS_ENABLE_CANCEL) && !(THINKOS_ENABLE_TERMINATE)
#error "THINKOS_ENABLE_CANCEL depends on THINKOS_ENABLE_TERMINATE"
#endif

#if (THINKOS_ENABLE_JOIN) && !(THINKOS_ENABLE_TERMINATE)
#error "THINKOS_ENABLE_JOIN depends on THINKOS_ENABLE_TERMINATE"
#endif

/* timed calls, cancel, pause and debug step depend on thread status */
#if (THINKOS_ENABLE_TIMED_CALLS ) && !(THINKOS_ENABLE_THREAD_STAT)
#error "THINKOS_ENABLE_TIMED_CALLS  depends on THINKOS_ENABLE_THREAD_STAT"
#endif

#if (THINKOS_ENABLE_PAUSE ) && !(THINKOS_ENABLE_THREAD_STAT)
#error "THINKOS_ENABLE_PAUSE  depends on THINKOS_ENABLE_THREAD_STAT"
#endif

#if (THINKOS_ENABLE_CANCEL ) && !(THINKOS_ENABLE_THREAD_STAT)
#error "THINKOS_ENABLE_CANCEL  depends on THINKOS_ENABLE_THREAD_STAT"
#endif

#if (THINKOS_ENABLE_DEBUG_STEP ) && !(THINKOS_ENABLE_THREAD_STAT)
#error "THINKOS_ENABLE_DEBUG_STEP  depends on THINKOS_ENABLE_THREAD_STAT"
#endif

#if (THINKOS_ENABLE_FPU_LS) && !(THINKOS_ENABLE_FPU)
#error "THINKOS_ENABLE_FPU_LS depends on THINKOS_ENABLE_FPU"
#endif

#if (THINKOS_ENABLE_CONSOLE_BREAK) && !(THINKOS_ENABLE_CONSOLE)
#error "THINKOS_ENABLE_CONSOLE_BREAK depends on THINKOS_ENABLE_CONSOLE"
#endif

#if (THINKOS_ENABLE_CONSOLE_NONBLOCK) && !(THINKOS_ENABLE_CONSOLE)
#error "THINKOS_ENABLE_CONSOLE_NONBLOCK depends on THINKOS_ENABLE_CONSOLE"
#endif

#if (THINKOS_ENABLE_CONSOLE_MODE) && !(THINKOS_ENABLE_CONSOLE)
#error "THINKOS_ENABLE_CONSOLE_MODE depends on THINKOS_ENABLE_CONSOLE"
#endif

#if (THINKOS_ENABLE_CTL_KRN_INFO) && !(THINKOS_ENABLE_CTL)
#error "THINKOS_ENABLE_CTL_KRN_INFO depends on THINKOS_ENABLE_CTL"
#endif


/* -------------------------------------------------------------------------- 
 * Deprecated options
 * --------------------------------------------------------------------------*/

/* THINKOS_ENABLE_IDLE_MSP - IDLE thread and exceptions use the same
   stack (MSP) all the remaining threads (non privileged) make use of
   the PSP instead.
 */

#ifdef THINKOS_ENABLE_ESCALATE
#warning "THINKOS_ENABLE_ESCALATE is deprecated"
#endif

#ifdef THINKOS_ENABLE_IDLE_MSP
#error "THINKOS_ENABLE_IDLE_MSP is deprecated"
#endif

#ifdef THINKOS_ENABLE_THREAD_VOID 
#error "THINKOS_ENABLE_THREAD_VOID is deprecated"
#endif

#ifdef THINKOS_ASM_SCHEDULER
#error "THINKOS_ASM_SCHEDULER is deprecated"
#endif

#ifdef THINKOS_STDERR_FAULT_DUMP
#error "THINKOS_STDERR_FAULT_DUMP is deprecated"
#endif

#ifdef THINKOS_ENABLE_RT_DEBUG
#error "THINKOS_ENABLE_RT_DEBUG is deprecated"
#endif

#ifdef THINKOS_DBGMON_STACK_SIZE
#error "THINKOS_DBGMON_STACK_SIZE is deprecated"
#endif

#ifdef THINKOS_DBGMON_ENABLE_RST_VEC 
#error "THINKOS_DBGMON_ENABLE_RST_VEC is deprecated"
#endif

#ifdef THINKOS_ENABLE_DMCLOCK 
#error "THINKOS_ENABLE_DMCLOCK is deprecated"
#endif

#ifdef THINKOS_UNROLL_EXCEPTIONS
#error "THINKOS_UNROLL_EXCEPTIONS is deprecated"
#endif

#ifdef THINKOS_ENABLE_EXCEPT_CLEAR
#error "THINKOS_ENABLE_EXCEPT_CLEAR is deprecated"
#endif

#ifdef THINKOS_ENABLE_KRN_TRACE
#error "THINKOS_ENABLE_KRN_TRACE is deprecated"
#endif

#ifdef THINKOS_ENABLE_DEBUG_FAULT 
#error "THINKOS_ENABLE_DEBUG_FAULT is deprecated"
#endif

#ifdef THINKOS_ENABLE_RESET_RAM_VECTORS
#error "THINKOS_ENABLE_RESET_RAM_VECTORS is deprecated"
#endif

/* THINKOS_ENABLE_CLOCK: Enable the support for system timer */
#ifdef THINKOS_ENABLE_CLOCK
#error "THINKOS_ENABLE_CLOCK is deprecated"
#endif


/* -------------------------------------------------------------------------- 
 * Wait queues sizes
 * --------------------------------------------------------------------------*/

#define THINKOS_WQ_READY_CNT 1

#define THINKOS_WQ_THREAD_CNT (THINKOS_THREADS_MAX)

#define THINKOS_WQ_CLOCK_CNT 1

#if ((THINKOS_MUTEX_MAX) > 0)
  #define THINKOS_WQ_MUTEX_CNT (THINKOS_MUTEX_MAX)
#else
  #define THINKOS_WQ_MUTEX_CNT 0 
#endif

#if ((THINKOS_COND_MAX) > 0)
  #define THINKOS_WQ_COND_CNT (THINKOS_COND_MAX)
#else
  #define THINKOS_WQ_COND_CNT 0 
#endif

#if ((THINKOS_SEMAPHORE_MAX) > 0)
  #define THINKOS_WQ_SEMAPHORE_CNT (THINKOS_SEMAPHORE_MAX)
#else
  #define THINKOS_WQ_SEMAPHORE_CNT 0 
#endif

#if ((THINKOS_EVENT_MAX) > 0)
  #define THINKOS_WQ_EVENT_CNT (THINKOS_EVENT_MAX)
#else
  #define THINKOS_WQ_EVENT_CNT 0 
#endif

#if ((THINKOS_FLAG_MAX) > 0)
  #define THINKOS_WQ_FLAG_CNT (THINKOS_FLAG_MAX)
#else
  #define THINKOS_WQ_FLAG_CNT 0 
#endif

#if ((THINKOS_GATE_MAX) > 0)
  #define THINKOS_WQ_GATE_CNT (THINKOS_GATE_MAX)
#else
  #define THINKOS_WQ_GATE_CNT 0 
#endif

#if (THINKOS_ENABLE_CONSOLE)
  #define THINKOS_WQ_CONSOLE_CNT 2
#else
  #define THINKOS_WQ_CONSOLE_CNT 0 
#endif

#if (THINKOS_ENABLE_PAUSE)
  #define THINKOS_WQ_PAUSED_CNT 1
#else
  #define THINKOS_WQ_PAUSED_CNT 0 
#endif

#if (THINKOS_ENABLE_JOIN)
  #define THINKOS_WQ_CANCELED_CNT 1
#else
  #define THINKOS_WQ_CANCELED_CNT 0 
#endif

#if (THINKOS_ENABLE_TIMESHARE)
  #define THINKOS_WQ_TMSHARE_CNT 1
#else
  #define THINKOS_WQ_TMSHARE_CNT 0 
#endif

#if (THINKOS_COMM_MAX)
  #define THINKOS_WQ_COMM_RX_CNT (THINKOS_COMM_MAX)
  #define THINKOS_WQ_COMM_TX_CNT (THINKOS_COMM_MAX)
#else
  #define THINKOS_WQ_COMM_RX_CNT 0 
  #define THINKOS_WQ_COMM_TX_CNT 0 
#endif

#if (THINKOS_ENABLE_WQ_IRQ)
  #define THINKOS_WQ_IRQ_CNT 1
#else
  #define THINKOS_WQ_IRQ_CNT 0 
#endif

#if (THINKOS_ENABLE_WQ_DMA)
  #define THINKOS_WQ_DMA_CNT 1
#else
  #define THINKOS_WQ_DMA_CNT 0 
#endif

#if ((THINKOS_FLASH_MEM_MAX) > 0)
  #define THINKOS_WQ_FLASH_MEM_CNT (THINKOS_FLASH_MEM_MAX)
#else
  #define THINKOS_WQ_FLASH_MEM_CNT 0 
#endif

#if (THINKOS_ENABLE_THREAD_FAULT)
  #define THINKOS_WQ_FAULT_CNT 1
#else
  #define THINKOS_WQ_FAULT_CNT 0 
#endif

#define THINKOS_WQ_CNT (THINKOS_WQ_READY_CNT + \
  THINKOS_WQ_THREAD_CNT + \
  THINKOS_WQ_CLOCK_CNT + \
  THINKOS_WQ_MUTEX_CNT + \
  THINKOS_WQ_COND_CNT + \
  THINKOS_WQ_SEMAPHORE_CNT + \
  THINKOS_WQ_EVENT_CNT + \
  THINKOS_WQ_FLAG_CNT + \
  THINKOS_WQ_GATE_CNT + \
  THINKOS_WQ_CONSOLE_CNT + \
  THINKOS_WQ_PAUSED_CNT + \
  THINKOS_WQ_CANCELED_CNT + \
  THINKOS_WQ_TMSHARE_CNT + \
  THINKOS_WQ_COMM_RX_CNT + \
  THINKOS_WQ_COMM_TX_CNT + \
  THINKOS_WQ_IRQ_CNT + \
  THINKOS_WQ_DMA_CNT + \
  THINKOS_WQ_FLASH_MEM_CNT + \
  THINKOS_WQ_FAULT_CNT)


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
		uint32_t comm_max            :8;
		uint32_t irq_max             :8;
		uint32_t dma_max             :8;

		uint32_t flash_max           :8;

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
			uint32_t monitor         :1;
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
			uint32_t mpu                :1;
			uint32_t fpu                :1;
			uint32_t fpu_ls             :1;
			uint32_t profiling          :1;
			uint32_t memory_map         :1;
			uint32_t app                :1;
			uint32_t app_crc            :1;
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
			uint32_t stack_limit     :1;
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

	struct {
		union {
			uint16_t flags;
			struct {
				uint16_t enabled     :1;
				uint16_t clock       :1;
				uint16_t threads     :1;
				uint16_t sched       :1;
			};
		};
		uint16_t stack_size;
	} monitor;

	union {
		uint32_t flags;
		struct {
			uint32_t base      :1;
			uint32_t enabled   :1;
			uint32_t step      :1;
			uint32_t bkpt      :1;
			uint32_t wpt       :1;
			uint32_t fault     :1;
		};
	} debug;

	union {
		uint32_t misc_flags;
		struct {
			uint32_t idle_wfi        :1;
			uint32_t sched_debug     :1;
			uint32_t ofast           :1;
			uint32_t stack_align     :1;
			uint32_t unalign_trap    :1;
			uint32_t div0_trap       :1;
			uint32_t idle_hooks      :1;
		};
	} misc;

	union {
		uint32_t flags;
		struct {
			uint32_t asm_scheduler       :1;
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

