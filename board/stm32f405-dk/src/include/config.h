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
#define THINKOS_IRQ_MAX                    90

/* Enable thinkos_irq_ctl system call */
#define THINKOS_ENABLE_IRQ_CTL              1
#define THINKOS_ENABLE_IRQ_CYCCNT           1
#define THINKOS_ENABLE_IRQ_PRIORITY_0       1
#define THINKOS_ENABLE_IRQ_TIMEDWAIT        1
#define THINKOS_ENABLE_WQ_IRQ               1

#define THINKOS_ENABLE_ALARM                1
#define THINKOS_ENABLE_SLEEP                1

#define THINKOS_THREADS_MAX                32

/* These settings are used when its desireable
   to create and destroy threads dinamically */
#define THINKOS_ENABLE_JOIN                 1
/* Allow for pausing and resuming a thread execution.
   This is needed for GDB and debug monitor. */
#define THINKOS_ENABLE_PAUSE                1
#define THINKOS_ENABLE_CANCEL               1
#define THINKOS_ENABLE_EXIT                 1
#define THINKOS_ENABLE_TERMINATE            1

#define THINKOS_MUTEX_MAX                  32

#define THINKOS_COND_MAX                   32

#define THINKOS_SEMAPHORE_MAX              32

#define THINKOS_EVENT_MAX                  32

#define THINKOS_FLAG_MAX                   32

/* Enable Rendez vous synchronization. Wakes up all threads
 watching a flag in a given instant.*/
#define THINKOS_ENABLE_FLAG_WATCH           1

#define THINKOS_GATE_MAX                   32

#define THINKOS_ENABLE_THREAD_STAT          1
#define THINKOS_ENABLE_TIMED_CALLS          1

/* Check the validity of arguments to system calls */
#define THINKOS_ENABLE_ARG_CHECK            1
/* Check if a thread tries to lock a mutex which it
   has previously locked. */
#define THINKOS_ENABLE_DEADLOCK_CHECK       1
/* Check if athread tries to unlock a mutex which is
   not his own. Also check for minimum stack size on 
 thread creation. */
#define THINKOS_ENABLE_SANITY_CHECK         1

/* Allow to interrupt a system call */
#define THINKOS_ENABLE_BREAK                1

/* Enable the per thread CPU usage counters */
#define THINKOS_ENABLE_PROFILING            1

/* Enable the debug monitor for real-time debug */
#define THINKOS_ENABLE_MONITOR              1
#define THINKOS_ENABLE_MONITOR_THREADS      1
#define THINKOS_ENABLE_MONITOR_CLOCK        1
#define THINKOS_ENABLE_MONITOR_SCHED        0
#define THINKOS_ENABLE_MONITOR_SYSCALL      1
#define THINKOS_MONITOR_STACK_SIZE          (1024 + 512)

#define THINKOS_ENABLE_CONSOLE              1
#define THINKOS_ENABLE_CONSOLE_READ         1 
#define THINKOS_ENABLE_CONSOLE_MISC         1 
#define THINKOS_ENABLE_CONSOLE_BREAK        1
#define THINKOS_ENABLE_CONSOLE_OPEN         1
#define THINKOS_ENABLE_CONSOLE_NONBLOCK     1
#define THINKOS_ENABLE_CONSOLE_DRAIN        1
#define THINKOS_ENABLE_CONSOLE_MODE         1
#define THINKOS_CONSOLE_RX_FIFO_LEN       512
#define THINKOS_CONSOLE_TX_FIFO_LEN      2048

#define THINKOS_ENABLE_STACK_ALIGN          1
#define THINKOS_ENABLE_UNALIGN_TRAP         1
#define THINKOS_EXCEPT_STACK_SIZE         512
#define THINKOS_ENABLE_STACK_LIMIT          1
#define THINKOS_ENABLE_DEBUG_BASE           1
#define THINKOS_ENABLE_ERROR_TRAP           1
#define THINKOS_ENABLE_EXCEPTIONS           1
#define THINKOS_ENABLE_HARDFAULT            1
#define THINKOS_ENABLE_BUSFAULT             1
#define THINKOS_ENABLE_USAGEFAULT           1
#define THINKOS_ENABLE_MEMFAULT             1
#define THINKOS_ENABLE_THREAD_FAULT         1
#if DEBUG
  #define THINKOS_SYSRST_ONFAULT            0
#else
  #define THINKOS_SYSRST_ONFAULT            1
#endif
#if DEBUG
/* Enable scheduler trace calls */
  #define THINKOS_ENABLE_SCHED_DEBUG        0
/* Enable scheduler stack error detection */
#define THINKOS_ENABLE_SCHED_ERROR          0
#else
  #define THINKOS_ENABLE_SCHED_DEBUG        0
/* Enable scheduler stack error detection */
#define THINKOS_ENABLE_SCHED_ERROR          0
#endif

#define THINKOS_ENABLE_DEBUG                0
#define THINKOS_ENABLE_DEBUG_BKPT           0
#define THINKOS_ENABLE_DEBUG_WPT            0
#define THINKOS_ENABLE_DEBUG_STEP           0

#define THINKOS_ENABLE_MPU                  1
#define THINKOS_ENABLE_FPU                  1
#define THINKOS_ENABLE_PRIVILEGED_THREAD    1

#define THINKOS_ENABLE_CTL                  1
#define THINKOS_ENABLE_CRITICAL             1
#define THINKOS_ENABLE_PREEMPTION           1

#ifdef DEBUG
  #define THINKOS_ENABLE_IDLE_WFI           0
#else
  #define THINKOS_ENABLE_IDLE_WFI           1
#endif

#define THINKOS_ENABLE_IDLE_HOOKS           1

#define THINKOS_ENABLE_OBJ_ALLOC            1
#define THINKOS_ENABLE_OBJ_FREE             1
 
#define THINKOS_ENABLE_THREAD_INFO          1

#define THINKOS_ENABLE_STACK_INIT           1
#define THINKOS_ENABLE_MEMORY_CLEAR         1
#define THINKOS_FLASH_MEM_MAX               1
#define THINKOS_ENABLE_UDELAY_CALIBRATE     1

#define THINKOS_ENABLE_OFAST                1

#define THINKOS_ENABLE_APP                  1
#define THINKOS_ENABLE_APP_FLAT             1
#define THINKOS_ENABLE_APP_ELF              0
#define THINKOS_ENABLE_APP_CRC              1
#define THINKOS_MONITOR_ENABLE_COMM_BRK     1

#define THINKOS_ENABLE_FRACTIONAL_CLOCK     1
#define THINKOS_ENABLE_DATE_AND_TIME        1
#define THINKOS_CUSTOM_APP_TAG              "S32F405"

#define THINKOS_ENABLE_TIMESHARE            0
#define THINKOS_ENABLE_I_CALLS              0
#define THINKOS_ENABLE_READY_MASK           0
#define THINKOS_ENABLE_KRNSVC               0
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
#define STM32_OTG_FS_INEP_MAX        3
#define STM32_OTG_FS_OUTEP_MAX       2
#define STM32_OTG_FS_IO_INIT         0
#define STM32_OTG_FS_VBUS_SENS       1
#define STM32_OTG_FS_VBUS_CONNECT    0
#define STM32_OTG_FS_RX_FIFO_SIZE    192
#define STM32_OTG_FS_BULK_PKTS       6

//#define STM32_OTG_FS_VBUS            STM32_GPIOA, 9

#define CDC_EP_IN_MAX_PKT_SIZE       64

/* -------------------------------------------------------------------------
   Bootloader 
   -------------------------------------------------------------------------- */
#define BOOT_ENABLE_GDB            0
#define BOOT_ENABLE_THIRD          0
#define BOOT_CUSTOM_COMM           0

#define MONITOR_DUMPMEM_ENABLE     1
#define MONITOR_WATCHPOINT_ENABLE  0
#define MONITOR_THREAD_STEP_ENABLE 0
#define MONITOR_SELFTEST_ENABLE    1
#define MONITOR_UPGRADE_ENABLE     1
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


