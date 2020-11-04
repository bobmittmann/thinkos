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

#define THINKOS_IRQ_MAX                 31
#define THINKOS_ENABLE_IRQ_CTL          1

#define THINKOS_ENABLE_IDLE_WFI         1

#define THINKOS_ENABLE_TIMESHARE        0

#define THINKOS_THREADS_MAX             32
#define THINKOS_ENABLE_THREAD_ALLOC     1

#define THINKOS_ENABLE_JOIN             1
#define THINKOS_ENABLE_PAUSE            1
#define THINKOS_ENABLE_CANCEL           1
#define THINKOS_ENABLE_EXIT             1

#define THINKOS_MUTEX_MAX               32
#define THINKOS_ENABLE_MUTEX_ALLOC      1

#define THINKOS_COND_MAX                16
#define THINKOS_ENABLE_COND_ALLOC       1

#define THINKOS_SEMAPHORE_MAX           0
#define THINKOS_ENABLE_SEM_ALLOC        0

#define THINKOS_EVENT_MAX               0
#define THINKOS_ENABLE_EVENT_ALLOC      0

#define THINKOS_FLAG_MAX                0
#define THINKOS_ENABLE_FLAG_ALLOC       0
#define THINKOS_ENABLE_FLAG_WATCH       0

#define THINKOS_GATE_MAX                0
#define THINKOS_ENABLE_GATE_ALLOC       0

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
#define THINKOS_ENABLE_CRITICAL         1
#define THINKOS_ENABLE_PREEMPTION       1

#define THINKOS_ENABLE_OBJ_ALLOC        1
#define THINKOS_ENABLE_OBJ_FREE         1

#define THINKOS_ENABLE_PROFILING        1

#define THINKOS_ENABLE_MONITOR          0
#define THINKOS_ENABLE_MONITOR_CLOCK          0
#define THINKOS_ENABLE_MONITOR_THREADS  0

#define THINKOS_ENABLE_CONSOLE          0

#define THINKOS_EXCEPT_STACK_SIZE       320
#define THINKOS_ENABLE_EXCEPTIONS       1
#define THINKOS_ENABLE_HARDFAULT        1
#define THINKOS_ENABLE_BUSFAULT         1
#define THINKOS_ENABLE_USAGEFAULT       1
#define THINKOS_ENABLE_MEMFAULT         1
#define THINKOS_SYSRST_ONFAULT          0
#define THINKOS_ENABLE_DEBUG_BKPT       0
#define THINKOS_ENABLE_DEBUG_WPT        0
#define THINKOS_ENABLE_DEBUG_STEP       0
#define THINKOS_ENABLE_DEBUG_FAULT      0
#define THINKOS_ENABLE_ERROR_TRAP       0

#define THINKOS_ENABLE_MPU              0
#define THINKOS_ENABLE_ESCALATE         0
#define THINKOS_ENABLE_FPU              0

#define THINKOS_MONITOR_STACK_SIZE       256

#define THINKOS_ENABLE_STACK_INIT       1

#define THINKOS_CONSOLE_RX_FIFO_LEN     64
#define THINKOS_CONSOLE_TX_FIFO_LEN     128

#define THINKOS_ENABLE_IDLE_HOOKS       0
#define THINKOS_ENABLE_SCHED_DEBUG      0

/* -------------------------------------------------------------------------
 * delay
 * ------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------
   STM32
   -------------------------------------------------------------------------- */

#define STM32_HCLK_HZ       180000000 /* HBA bus frequency */
#define STM32_HSE_HZ        8000000 /* External oscillator frequency */
#define STM32_ENABLE_HSE    1 /* Use external oscillator */
#define STM32_ENABLE_PLL    1 /* Enable PLL */
#define STM32_PLL_CKIN_HSE  1 /* Use external oscillator for PLL */
#define STM32_ENABLE_PLLI2S 0 /* Enable SAI PLL */
#define STM32_ENABLE_PLLSAI 0 /* Disable SAI PLL */

#define THINKOS_ENABLE_IRQ_CYCCNT     0
#define THINKOS_ENABLE_IRQ_PRIORITY_0 0
#define THINKOS_ENABLE_IRQ_TIMEDWAIT  0
#define THINKOS_ENABLE_WQ_IRQ         0 

/* -------------------------------------------------------------------------
   USB 
   -------------------------------------------------------------------------- */

#define STM32_ENABLE_USB_FS          1

#define CDC_EP_OUT_MAX_PKT_SIZE      64
#define CDC_EP_IN_MAX_PKT_SIZE       64
#define CDC_EP_INT_MAX_PKT_SIZE      8

/* -------------------------------------------------------------------------
 * TCP/IP
 * ------------------------------------------------------------------------- */

#define MBUF_POOL_SIZE               64
#define PKTBUF_LEN                   (1500 + 16)

#define TCP_MAX_RCV_QUEUE            (1460 * 2)
#define TCP_MAX_SND_QUEUE            (1460 * 2)

#define TCP_DEFAULT_MSL_SEC          1
#define TCP_IDLE_DET_SEC             10
#define TCP_KEEP_ALIVE_PROBE_SEC     10
#define TCP_MAX_IDLE_SEC             15

#define TCP_ENABLE_TIMEWAIT          1
#define TCP_ENABLE_HEADER_PREDICTION 1
#define NET_TCP_PCB_ACTIVE_MAX       8
#define NET_UDP_PCB_ACTIVE_MAX       1

/* use hardware checksum */
#define  ENABLE_NET_TCP_CHECKSUM     0
#define  ENABLE_NET_UDP_CHECKSUM     0

/* -------------------------------------------------------------------------
   Bootloader 
   -------------------------------------------------------------------------- */
#define BOOT_MEM_RESERVED          0x1000
#define BOOT_ENABLE_GDB            0
#define BOOT_ENABLE_THIRD          0
#define BOOT_ENABLE_CUSTOM_COMM    1
#define BOOT_ENABLE_MONITOR        0


#define MONITOR_BOARDINFO_ENABLE   0
#define MONITOR_DUMPMEM_ENABLE     0
#define MONITOR_WATCHPOINT_ENABLE  0
#define MONITOR_THREAD_STEP_ENABLE 0
#define MONITOR_SELFTEST_ENABLE    0
#define MONITOR_CONFIGURE_ENABLE   0
#define MONITOR_UPGRADE_ENABLE     0
#define MONITOR_STACKUSAGE_ENABLE  0
#define MONITOR_THREADINFO_ENABLE  0
#define MONITOR_OSINFO_ENABLE      0
#define MONITOR_APPWIPE_ENABLE     0
#define MONITOR_APPTERM_ENABLE     0
#define MONITOR_APPRESTART_ENABLE  0
#define MONITOR_EXCEPTION_ENABLE   0
#define MONITOR_FAULT_ENABLE       0
#define MONITOR_OS_PAUSE           0
#define MONITOR_OS_RESUME          0
#define MONITOR_RESTART_MONITOR    0
#define MONITOR_PREBOOT_ENABLE     0
#define MONITOR_BREAKPOINT_ENABLE  0

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

#define ENABLE_TCP_SEND 1
#define ENABLE_TCP_RECV 1
#define ENABLE_USB      0
#define ENABLE_MONITOR  1
#define ENABLE_NETWORK  1
#define ENABLE_TFTP     1
#define ENABLE_NAND     1
#define ENABLE_COMM     1
#define ENABLE_GDB      1
#define ENABLE_VCOM     1
#define ENABLE_I2C      1
#define ENABLE_TELNET   1

/* -------------------------------------------------------------------------
 * Trace 
 * ------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------
 * LibC
 * ------------------------------------------------------------------------- */

#define PRINTF_ENABLE_FLOAT 1

#endif /* __CONFIG_H__ */


