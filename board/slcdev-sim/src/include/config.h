/* config.h - system configuration
 * --------
 *
 *   ************************************************************************
 *   **            Company Confidential - For Internal Use Only            **
 *   **          Mircom Technologies Ltd. & Affiliates ("Mircom")          **
 *   **                                                                    **
 *   **   This information is confidential and the exclusive property of   **
 *   ** Mircom.  It is intended for internal use and only for the purposes **
 *   **   provided,  and may not be disclosed to any third party without   **
 *   **                prior written permission from Mircom.               **
 *   **                                                                    **
 *   **                        Copyright 2017-2018                         **
 *   ************************************************************************
 *
 */

/** 
 * @file config.h
 * @author Robinson Mittmann <bmittmann@mircomgroup.com>
 * @brief System wide configuration options.
 * 
 * This file is common to the application as well as the bootloader.
 * It holds configuration options for ThinkOS, Bootloader and other 
 * system wide parameters.
 * 
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/* -------------------------------------------------------------------------
   ThinkOS configuration
   -------------------------------------------------------------------------- */

#define THINKOS_IRQ_MAX                 44
#define THINKOS_ENABLE_IRQ_CTL          1
#define THINKOS_ENABLE_IRQ_RESTORE      0
#define THINKOS_ENABLE_IRQ_CYCCNT       0
#define THINKOS_ENABLE_IRQ_PRIORITY_0   0
#define THINKOS_ENABLE_IRQ_TIMEDWAIT    0
#define THINKOS_ENABLE_WQ_IRQ           0

#define THINKOS_ENABLE_CLOCK            1
#define THINKOS_ENABLE_ALARM            1
#define THINKOS_ENABLE_SLEEP            1

#define THINKOS_ENABLE_TIMESHARE        0
#define THINKOS_SCHED_LIMIT_MAX         32
#define THINKOS_SCHED_LIMIT_MIN         1

#define THINKOS_THREADS_MAX             8

/* These settings are used when its desireable
   to create and destroy threads dinamically */
#define THINKOS_ENABLE_JOIN             0
/* Allow for pausing and resuming a thread execution.
   This is needed for GDB and debug monitor. */
#define THINKOS_ENABLE_PAUSE            0
#define THINKOS_ENABLE_CANCEL           0
#define THINKOS_ENABLE_EXIT             0

#define THINKOS_MUTEX_MAX               8

#define THINKOS_COND_MAX                0

#define THINKOS_SEMAPHORE_MAX           8

#define THINKOS_EVENT_MAX               0

#define THINKOS_FLAG_MAX                0
/* Enable Rendez vous synchronization. Wakes up all threads
 watching a flag in a given instant.*/
#define THINKOS_ENABLE_FLAG_WATCH       0

#define THINKOS_GATE_MAX                0

#define THINKOS_ENABLE_THREAD_STAT      1
#define THINKOS_ENABLE_TIMED_CALLS      1

/* Check the validity of arguments to system calls */
#define THINKOS_ENABLE_ARG_CHECK        0
/* Check if a thread tries to lock a mutex which it
   has previously locked. */
#define THINKOS_ENABLE_DEADLOCK_CHECK   0
/* Check if athread tries to unlock a mutex which is
   not his own. Also check for minimum stack size on 
 thread creation. */
#define THINKOS_ENABLE_SANITY_CHECK     0

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
#define THINKOS_ENABLE_BREAK            0

/* Enable the per thread CPU usage counters */
#define THINKOS_ENABLE_PROFILING        0

/* Enable the debug monitor for real-time debug */
#define THINKOS_ENABLE_MONITOR          1
#define THINKOS_ENABLE_MONITOR_THREADS  1
#define THINKOS_ENABLE_MONITOR_CLOCK          0

#define THINKOS_ENABLE_CONSOLE          1
#define THINKOS_ENABLE_CONSOLE_MISC     0 
#define THINKOS_ENABLE_CONSOLE_BREAK    0
#define THINKOS_ENABLE_CONSOLE_OPEN     0
#define THINKOS_ENABLE_CONSOLE_NONBLOCK 0
#define THINKOS_ENABLE_CONSOLE_DRAIN    0
#define THINKOS_ENABLE_CONSOLE_MODE     0
#define THINKOS_CONSOLE_RX_FIFO_LEN     64
#define THINKOS_CONSOLE_TX_FIFO_LEN     256

#define THINKOS_EXCEPT_STACK_SIZE       (384 + 128)
#define THINKOS_ENABLE_ERROR_TRAP       1
#define THINKOS_ENABLE_EXCEPTIONS       1
#define THINKOS_ENABLE_EXCEPT_CLEAR     1
#define THINKOS_ENABLE_HARDFAULT        1
#define THINKOS_ENABLE_BUSFAULT         1
#define THINKOS_ENABLE_USAGEFAULT       1
#define THINKOS_ENABLE_MEMFAULT         1
#define THINKOS_ENABLE_DEBUG_FAULT      0
#if DEBUG
  #define THINKOS_SYSRST_ONFAULT        0
#else
  #define THINKOS_SYSRST_ONFAULT        0
#endif
#define THINKOS_ENABLE_DEBUG_BKPT       0
#define THINKOS_ENABLE_DEBUG_WPT        0
#define THINKOS_ENABLE_DEBUG_STEP       0

#define THINKOS_ENABLE_MPU              1
#define THINKOS_ENABLE_ESCALATE         0
#define THINKOS_ENABLE_FPU              0

#define THINKOS_ENABLE_THREAD_VOID      1

#define THINKOS_DBGMON_STACK_SIZE       (1024 + 384)
#define THINKOS_ENABLE_STACK_INIT       1

#define THINKOS_ENABLE_CTL              0
#define THINKOS_ENABLE_CRITICAL         0
#define THINKOS_ENABLE_PREEMPTION       0

#ifdef DEBUG
  #define THINKOS_ENABLE_IDLE_WFI       0
#else
  #define THINKOS_ENABLE_IDLE_WFI       0
#endif

#define THINKOS_ENABLE_IDLE_HOOKS       1
#define THINKOS_ENABLE_IDLE_MSP         0

#define THINKOS_ENABLE_KRN_TRACE        0
#define THINKOS_ENABLE_OBJ_ALLOC        0
#define THINKOS_ENABLE_OBJ_FREE         0

#define THINKOS_ENABLE_I_CALLS          0
#define THINKOS_ENABLE_THREAD_INFO      0

#define THINKOS_ENABLE_MEMORY_CLEAR     1
#define THINKOS_ENABLE_FLASH_MEM        0

/* Enable the rt_snapshot() service to get
   the state of the kernel */
#define THINKOS_ENABLE_RT_DEBUG         0

#define THINKOS_STDERR_FAULT_DUMP       0
#define THINKOS_ASM_SCHEDULER           1
#define THINKOS_ENABLE_OFAST            1

#define THINKOS_ENABLE_RESET_RAM_VECTORS 0

#define THINKOS_ENABLE_USAGEFAULT_MONITOR 1

#define THINKOS_UNROLL_EXCEPTIONS       0
/* -------------------------------------------------------------------------
   RCC 
   -------------------------------------------------------------------------- */
#define STM32_HCLK_HZ           72000000 /* Target HBA bus frequency */
#define STM32_HSE_HZ            12000000 /* External oscillator frequency */
#define STM32_ENABLE_HSE        1 /* Use external oscillator */
#define STM32_ENABLE_PLL        1 /* Enable PLL */
#define STM32_ENABLE_HSI        0 /* Don't use internal oscillator */

/* -------------------------------------------------------------------------
   GDB
   -------------------------------------------------------------------------- */

#define GDB_ENABLE_MEM_MAP         0

/* -------------------------------------------------------------------------
   Bootloader 
   -------------------------------------------------------------------------- */

#define BOOT_MEM_RESERVED          0x0800
#define BOOT_ENABLE_THIRD          0
  
#define BOOT_ENABLE_GDB            0
#define MONITOR_DUMPMEM_ENABLE     0
#define MONITOR_WATCHPOINT_ENABLE  0
#define MONITOR_BREAKPOINT_ENABLE  0
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
#define MONITOR_BOARDINFO_ENABLE   0
#define MONITOR_VT100_ENABLE       0

/* -------------------------------------------------------------------------
 * Serial
 * ------------------------------------------------------------------------- */

#define SERIAL_RX_FIFO_LEN  1024

/* -------------------------------------------------------------------------
 * LIBC 
 * ------------------------------------------------------------------------- */

#define PRINTF_ENABLE_FLOAT 0

#endif				/* __CONFIG_H__ */
