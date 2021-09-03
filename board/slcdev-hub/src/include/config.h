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

/* Maximum IRQ for this board */
#define THINKOS_IRQ_MAX                 80
/* Enable thinkos_irq_ctl system call */
#define THINKOS_ENABLE_IRQ_CTL          1
#define THINKOS_ENABLE_IRQ_CYCCNT       1
#define THINKOS_ENABLE_IRQ_PRIORITY_0   0
#define THINKOS_ENABLE_IRQ_TIMEDWAIT    1
#define THINKOS_ENABLE_WQ_IRQ           1

#define THINKOS_ENABLE_ALARM            1

#define THINKOS_ENABLE_TIMESHARE        0

#define THINKOS_THREADS_MAX             32

#define THINKOS_ENABLE_TERMINATE        1
/* These settings are used when its desireable
   to create and destroy threads dinamically */
#define THINKOS_ENABLE_JOIN             1
/* Allow for pausing and resuming a thread execution.
   This is needed for GDB and debug monitor. */
#define THINKOS_ENABLE_PAUSE            1
#define THINKOS_ENABLE_CANCEL           1
#define THINKOS_ENABLE_EXIT             1

#define THINKOS_MUTEX_MAX               32

#define THINKOS_COND_MAX                32

#define THINKOS_SEMAPHORE_MAX           32

#define THINKOS_EVENT_MAX               32

#define THINKOS_FLAG_MAX                32
/* Enable Rendez vous synchronization. Wakes up all threads
 watching a flag in a given instant.*/
#define THINKOS_ENABLE_FLAG_WATCH       1

#define THINKOS_GATE_MAX                16

#define THINKOS_ENABLE_THREAD_STAT      1
#define THINKOS_ENABLE_TIMED_CALLS      1

/* Check the validity of arguments to system calls */
#define THINKOS_ENABLE_ARG_CHECK        1
/* Check if a thread tries to lock a mutex which it
   has previously locked. */
#define THINKOS_ENABLE_DEADLOCK_CHECK   1
/* Check if athread tries to unlock a mutex which is
   not his own. Also check for minimum stack size on 
 thread creation. */
#define THINKOS_ENABLE_SANITY_CHECK     1

/* Enable scheduler trace calls */
#if DEBUG
#define THINKOS_ENABLE_SCHED_DEBUG      1
#else
#define THINKOS_ENABLE_SCHED_DEBUG      0
#endif

/* Enable scheduler stack error detection */
#if DEBUG
#define THINKOS_ENABLE_SCHED_ERROR      1
#else
#define THINKOS_ENABLE_SCHED_ERROR      0
#endif

/* Allow to interrupt a system call */
#define THINKOS_ENABLE_BREAK            1

/* Enable the per thread CPU usage counters */
#define THINKOS_ENABLE_PROFILING        1

/* Enable the debug monitor for real-time debug */
#define THINKOS_ENABLE_MONITOR          1
#define THINKOS_ENABLE_MONITOR_THREADS  1
#define THINKOS_ENABLE_MONITOR_CLOCK    1
#define THINKOS_MONITOR_STACK_SIZE      888

#define THINKOS_ENABLE_CONSOLE          1
#define THINKOS_ENABLE_CONSOLE_MISC     1 
#define THINKOS_ENABLE_CONSOLE_BREAK    1
#define THINKOS_ENABLE_CONSOLE_OPEN     1
#define THINKOS_ENABLE_CONSOLE_NONBLOCK 1
#define THINKOS_ENABLE_CONSOLE_DRAIN    1
#define THINKOS_ENABLE_CONSOLE_MODE     1
#define THINKOS_ENABLE_CONSOLE_READ     1
#define THINKOS_CONSOLE_RX_FIFO_LEN     64
#define THINKOS_CONSOLE_TX_FIFO_LEN     128

#define THINKOS_EXCEPT_STACK_SIZE       384
#define THINKOS_ENABLE_KRNSVC           1
#define THINKOS_ENABLE_ERROR_TRAP       1
#define THINKOS_ENABLE_EXCEPTIONS       1
#define THINKOS_ENABLE_HARDFAULT        1
#define THINKOS_ENABLE_BUSFAULT         1
#define THINKOS_ENABLE_USAGEFAULT       1
#define THINKOS_ENABLE_MEMFAULT         1

#define THINKOS_ENABLE_THREAD_FAULT     1
#if DEBUG
  #define THINKOS_SYSRST_ONFAULT        0
#else
  #define THINKOS_SYSRST_ONFAULT        1
#endif

#define THINKOS_ENABLE_DEBUG_BASE       1
#define THINKOS_ENABLE_DEBUG            1
#define THINKOS_ENABLE_DEBUG_BKPT       1
#define THINKOS_ENABLE_DEBUG_WPT        1
#define THINKOS_ENABLE_DEBUG_STEP       1

#define THINKOS_ENABLE_MPU              1
#define THINKOS_ENABLE_ESCALATE         0
#define THINKOS_ENABLE_FPU              1

#define THINKOS_ENABLE_STACK_INIT       1

#define THINKOS_ENABLE_CTL              1
#define THINKOS_ENABLE_CRITICAL         1
#define THINKOS_ENABLE_PREEMPTION       1

#ifdef DEBUG
  #define THINKOS_ENABLE_IDLE_WFI       0
#else
  #define THINKOS_ENABLE_IDLE_WFI       1
#endif

#define THINKOS_ENABLE_IDLE_HOOKS       1

#define THINKOS_ENABLE_OBJ_ALLOC        1
#define THINKOS_ENABLE_OBJ_FREE         1

#define THINKOS_ENABLE_I_CALLS          1
#define THINKOS_ENABLE_THREAD_INFO      1

#define THINKOS_ENABLE_MEMORY_CLEAR     1
#define THINKOS_FLASH_MEM_MAX           2

#define THINKOS_ENABLE_OFAST            1

#define THINKOS_ENABLE_STACK_LIMIT      1 

#define THINKOS_ENABLE_APP              1

/* -------------------------------------------------------------------------
   RCC 
   -------------------------------------------------------------------------- */

#define STM32_HCLK_HZ       168000000 /* HBA bus frequency */
#define STM32_HSE_HZ        12000000 /* External oscillator frequency */
#define STM32_ENABLE_HSE    1 /* Use external oscillator */
#define STM32_ENABLE_PLL    1 /* Enable PLL */
#define STM32_PLL_CKIN_HSE  1 /* Use external oscillator for PLL */
#define STM32_ENABLE_PLLI2S 1 /* Enable SAI PLL */
#define STM32_ENABLE_PLLSAI 0 /* Disable SAI PLL */


/* -------------------------------------------------------------------------
   USB 
   -------------------------------------------------------------------------- */

#define STM32_ENABLE_OTG_FS          1
#define STM32_OTG_FS_IO_INIT         0
#define STM32_OTG_FS_IRQ_ENABLE      0
#define STM32_OTG_FS_EP_MAX          4
#define STM32_OTG_FS_VBUS_ENABLE     1

#define CDC_EP_OUT_MAX_PKT_SIZE      64
#define CDC_EP_IN_MAX_PKT_SIZE       64
#define CDC_EP_INT_MAX_PKT_SIZE      8

/* -------------------------------------------------------------------------
   Bootloader 
   -------------------------------------------------------------------------- */
#define BOOT_ENABLE_GDB            0
#define BOOT_ENABLE_THIRD          0
#define BOOT_CUSTOM_COMM           1

#define MONITOR_APPUPLOAD_ENABLE   1
#define MONITOR_BOARDINFO_ENABLE   1
#define MONITOR_DUMPMEM_ENABLE     1
#define MONITOR_WATCHPOINT_ENABLE  0
#define MONITOR_THREAD_STEP_ENABLE 0
#define MONITOR_SELFTEST_ENABLE    1
#define MONITOR_CONFIGURE_ENABLE   0
#define MONITOR_UPGRADE_ENABLE     0
#define MONITOR_STACKUSAGE_ENABLE  1
#define MONITOR_THREADINFO_ENABLE  1
#define MONITOR_OSINFO_ENABLE      1
#define MONITOR_APPWIPE_ENABLE     1
#define MONITOR_APPTERM_ENABLE     1
#define MONITOR_APPRESTART_ENABLE  1
#define MONITOR_EXCEPTION_ENABLE   1
#define MONITOR_FAULT_ENABLE       1
#define MONITOR_OS_PAUSE           1
#define MONITOR_OS_RESUME          1
#define MONITOR_RESTART_MONITOR    1
#define MONITOR_PREBOOT_ENABLE     1

/* -------------------------------------------------------------------------
 * JTAG
 * ------------------------------------------------------------------------- */

#define JTAG_VEC_BITS_MAX 256
#define JTAG_TAP_MAX      8
#define ENABLE_ICE_TEST   0

/* -------------------------------------------------------------------------
 * Shell
 * ------------------------------------------------------------------------- */

#define SHELL_LINE_MAX    72
#define SHELL_ARG_MAX     16
#define SHELL_HISTORY_MAX 32

/* -------------------------------------------------------------------------
 * Services
 * ------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------
 * Trace 
 * ------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------
 * LibC
 * ------------------------------------------------------------------------- */

#define PRINTF_ENABLE_FLOAT 1

/* -------------------------------------------------------------------------
 * TCP/IP
 * ------------------------------------------------------------------------- */

#define ENABLE_NETWORK 1
#define ENABLE_NET_TCP 1
#define ENABLE_NET_UDP 1
#define ENABLE_NET_RAW 1

#define ENABLE_TCPDUMP 0

#define IFNET_INTERFACES_MAX         2

#define MBUF_POOL_SIZE               128

#define TCP_FAST_TMR_MS              100
#define TCP_ENABLE_TIMEWAIT          1
#define TCP_ENABLE_HEADER_PREDICTION 1
#define NET_TCP_PCB_ACTIVE_MAX       64
#define NET_UDP_PCB_ACTIVE_MAX       1

#define TCP_DEFAULT_MSL_SEC          1
#define TCP_IDLE_DET_SEC             10
#define TCP_KEEP_ALIVE_PROBE_SEC     20
#define TCP_MAX_IDLE_SEC             60
#define TCP_MAX_SND_QUEUE            (1460 * 4)

/* use hardware checksum */
#define ENABLE_NET_TCP_CHECKSUM 0
#define ENABLE_NET_UDP_CHECKSUM 0

/* -------------------------------------------------------------------------
 * Ethernet Driver
 * ------------------------------------------------------------------------- */

#define ETH_PHY_RST_GPIO         STM32_GPIOE, 5

/* Number of reception DMA descriptors for the Ethernet driver */
#define STM32F_ETH_RX_NDESC      4
/* Number of transmision DMA descriptors for the Ethernet driver */
#define STM32F_ETH_TX_NDESC      4
/* Number of packet in the buffer pool */
#define PKTBUF_POOL_SIZE        10 

/* -------------------------------------------------------------------------
 * MS/TP
 * ------------------------------------------------------------------------- */

#define MSTP_LNK_MAX_MASTERS    8

#endif /* __CONFIG_H__ */

