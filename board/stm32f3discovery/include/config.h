/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the YARD-ICE.
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

/** 
 * @file config.h
 * @brief YARD-ICE
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* -------------------------------------------------------------------------
   ThinkOS configuration
   -------------------------------------------------------------------------- */

#define THINKOS_IRQ_MAX                 62
#define THINKOS_ENABLE_IRQ_CTL          1

#define THINKOS_ENABLE_IDLE_WFI         1
#define THINKOS_IDLE_STACK_CONST        0
#define THINKOS_IDLE_STACK_BSS          0
#define THINKOS_IDLE_STACK_ALLOC        1

#define THINKOS_ENABLE_TIMESHARE        1
#define THINKOS_SCHED_LIMIT_MAX         32
#define THINKOS_SCHED_LIMIT_MIN         1

#define THINKOS_THREADS_MAX             32
#define THINKOS_ENABLE_THREAD_ALLOC     1

#define THINKOS_ENABLE_JOIN             1
#define THINKOS_ENABLE_PAUSE            1
#define THINKOS_ENABLE_CANCEL           1
#define THINKOS_ENABLE_EXIT             1

#define THINKOS_MUTEX_MAX               16
#define THINKOS_ENABLE_MUTEX_ALLOC      1

#define THINKOS_COND_MAX                16
#define THINKOS_ENABLE_COND_ALLOC       1

#define THINKOS_SEMAPHORE_MAX           16
#define THINKOS_ENABLE_SEM_ALLOC        1

#define THINKOS_EVENT_MAX               8
#define THINKOS_ENABLE_EVENT_ALLOC      1

#define THINKOS_FLAG_MAX                16
#define THINKOS_ENABLE_FLAG_ALLOC       1
#define THINKOS_ENABLE_FLAG_WATCH       1

#define THINKOS_GATE_MAX                16
#define THINKOS_ENABLE_GATE_ALLOC       1

#define THINKOS_ENABLE_THREAD_STAT      1
#define THINKOS_ENABLE_TIMED_CALLS      1

#define THINKOS_ENABLE_CLOCK            1
#define THINKOS_ENABLE_ALARM            1
#define THINKOS_ENABLE_SLEEP            1
#define THINKOS_ENABLE_BREAK            1

#define THINKOS_ENABLE_ARG_CHECK        1
#define THINKOS_ENABLE_DEADLOCK_CHECK   1
#define THINKOS_ENABLE_SANITY_CHECK     1

#define THINKOS_ENABLE_CTL              1

#define THINKOS_ENABLE_SCHED_DEBUG      0
#define THINKOS_ENABLE_RT_DEBUG         0

#define THINKOS_ENABLE_PROFILING        1

#define THINKOS_ENABLE_MONITOR          1
#define THINKOS_ENABLE_DMCLOCK          1

#define THINKOS_ENABLE_CONSOLE          1

#define THINKOS_EXCEPT_STACK_SIZE       384
#define THINKOS_ENABLE_EXCEPTIONS       1
#define THINKOS_UNROLL_EXCEPTIONS       1
#define THINKOS_ENABLE_EXCEPT_CLEAR     1
#define THINKOS_ENABLE_HARDFAULT        0
#define THINKOS_ENABLE_BUSFAULT         1
#define THINKOS_ENABLE_USAGEFAULT       1
#define THINKOS_ENABLE_MEMFAULT         1
#define THINKOS_SYSRST_ONFAULT          0
#define THINKOS_STDERR_FAULT_DUMP       0
#define THINKOS_ENABLE_DEBUG_BKPT       1
#define THINKOS_ENABLE_DEBUG_WPT        1
#define THINKOS_ENABLE_DEBUG_STEP       1
#define THINKOS_ENABLE_DEBUG_FAULT      1
#define THINKOS_ENABLE_ERROR_TRAP       1

#define THINKOS_ENABLE_MPU              1
#define THINKOS_ENABLE_ESCALATE         1
#define THINKOS_ENABLE_FPU              1

#define THINKOS_ENABLE_THREAD_VOID      1

#define THINKOS_DMON_STACK_SIZE         (1024 + 128)

#define THINKOS_ENABLE_STACK_INIT       1

#define THINKOS_ASM_SCHEDULER           1
#define THINKOS_ENABLE_OFAST            1

#define THINKOS_ENABLE_RESET_RAM_VECTORS 1
#define THINKOS_DBGMON_ENABLE_FLOWCTL    1 
#define THINKOS_DBGMON_ENABLE_IRQ_MGMT   1 
#define THINKOS_CONSOLE_RX_FIFO_LEN     64
#define THINKOS_CONSOLE_TX_FIFO_LEN    128

/* -------------------------------------------------------------------------
   STM32
   -------------------------------------------------------------------------- */

#define STM32_HSE_HZ          8000000
#define STM32_HCLK_HZ        72000000

/* -------------------------------------------------------------------------
   USB 
   -------------------------------------------------------------------------- */

#define STM32_ENABLE_USB_FS     1
#define STM32_USB_FS_EP_MAX     4
#define STM32_USB_FS_VBUS_CTRL  0
#define STM32_USB_FS_SUSPEND    0
#define STM32_USB_FS_IRQ_ENABLE 0
#define STM32_USB_FS_IO_INIT    0

/* -------------------------------------------------------------------------
   GDB
   -------------------------------------------------------------------------- */

#define GDB_ENABLE_MEM_MAP       0

/* -------------------------------------------------------------------------
   Bootloader 
   -------------------------------------------------------------------------- */

#define BOOT_ENABLE_GDB             1
#define BOOT_MEM_RESERVED           0x1000
#define MONITOR_CONFIGURE_ENABLE    0
#define MONITOR_DUMPMEM_ENABLE      1
#define MONITOR_UPGRADE_ENABLE      0
#define MONITOR_STACKUSAGE_ENABLE   1
#define MONITOR_THREADINFO_ENABLE   1
#define MONITOR_APPWIPE_ENABLE      1
#define MONITOR_APPTERM_ENABLE      1
#define MONITOR_APPRESTART_ENABLE   1
#define MONITOR_SELFTEST_ENABLE     1

#endif /* __CONFIG_H__ */


