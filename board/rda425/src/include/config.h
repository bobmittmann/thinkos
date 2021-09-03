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

#define THINKOS_IRQ_MAX                     98
#define THINKOS_ENABLE_IRQ_CTL              1
#define THINKOS_ENABLE_IRQ_CYCCNT           1
#define THINKOS_ENABLE_IRQ_PRIORITY_0       1
#define THINKOS_ENABLE_IRQ_TIMEDWAIT        0
#define THINKOS_ENABLE_WQ_IRQ               1

#define THINKOS_ENABLE_ALARM                1
#define THINKOS_ENABLE_SLEEP                1

#define THINKOS_ENABLE_TIMESHARE            0

#define THINKOS_THREADS_MAX                 32

/* These settings are used when its desirable
   to create and destroy threads dynamically */
#define THINKOS_ENABLE_JOIN                 1
/* Allow for pausing and resuming a thread execution.
   This is needed for debug monitor. */
#define THINKOS_ENABLE_PAUSE                1
#define THINKOS_ENABLE_CANCEL               1
#define THINKOS_ENABLE_EXIT                 1

#define THINKOS_MUTEX_MAX                   16

#define THINKOS_COND_MAX                    8

#define THINKOS_SEMAPHORE_MAX               8

#define THINKOS_EVENT_MAX                   4

#define THINKOS_FLAG_MAX                    8
/* Enable Rendez vous synchronization. Wakes up all threads
 watching a flag in a given instant.*/
#define THINKOS_ENABLE_FLAG_WATCH           0

#define THINKOS_GATE_MAX                    8

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

/* Enable scheduler trace calls */
#if DEBUG
#define THINKOS_ENABLE_SCHED_DEBUG          1
#else
#define THINKOS_ENABLE_SCHED_DEBUG          0
#endif

/* Enable scheduler stack error detection */
#if DEBUG
#define THINKOS_ENABLE_SCHED_ERROR          1
#else
#define THINKOS_ENABLE_SCHED_ERROR          0
#endif

#if DEBUG
  #define THINKOS_SYSRST_ONFAULT            0
#else
  #define THINKOS_SYSRST_ONFAULT            0
#endif

/* Allow to interrupt a system call */
#define THINKOS_ENABLE_BREAK                0

/* Enable the per thread CPU usage counters */
#define THINKOS_ENABLE_PROFILING            1
#define THINKOS_ENABLE_STACK_LIMIT          1

#define THINKOS_ENABLE_TERMINATE            1

#define THINKOS_ENABLE_APP                  1
#define THINKOS_ENABLE_APP_CRC              1
#define THINKOS_ENABLE_UDELAY_CALIBRATE     1

/* Enable monitor */
#define THINKOS_ENABLE_MONITOR              1
#define THINKOS_ENABLE_MONITOR_CLOCK        0
#define THINKOS_ENABLE_MONITOR_THREADS      1
#define THINKOS_MONITOR_STACK_SIZE          1024

#define THINKOS_EXCEPT_STACK_SIZE           512
#define THINKOS_ENABLE_DEBUG_BASE           1
#define THINKOS_ENABLE_ERROR_TRAP           1
#define THINKOS_ENABLE_EXCEPTIONS           1
#define THINKOS_ENABLE_HARDFAULT            1
#define THINKOS_ENABLE_BUSFAULT             1
#define THINKOS_ENABLE_USAGEFAULT           1
#define THINKOS_ENABLE_MEMFAULT             1
#define THINKOS_ENABLE_THREAD_FAULT         1
#define THINKOS_ENABLE_DEBUG_BKPT           0
#define THINKOS_ENABLE_DEBUG_WPT            0
#define THINKOS_ENABLE_DEBUG_STEP           0

#define THINKOS_ENABLE_MPU                  1
#define THINKOS_ENABLE_ESCALATE             0
#define THINKOS_ENABLE_FPU                  1


#define THINKOS_ENABLE_STACK_INIT           1

#define THINKOS_ENABLE_CTL                  1
#define THINKOS_ENABLE_I_CALLS              0
#define THINKOS_ENABLE_CRITICAL             1
#define THINKOS_ENABLE_PREEMPTION           1
#define THINKOS_ENABLE_PRIVILEGED_THREAD    1

#ifdef DEBUG
  #define THINKOS_ENABLE_IDLE_WFI           0
#else
  #define THINKOS_ENABLE_IDLE_WFI           1
#endif

#define THINKOS_ENABLE_IDLE_HOOKS           1

#define THINKOS_ENABLE_OBJ_ALLOC            1
#define THINKOS_ENABLE_OBJ_FREE             1

#define THINKOS_ENABLE_THREAD_INFO          1

#define THINKOS_ENABLE_MEMORY_CLEAR         0
#define THINKOS_FLASH_MEM_MAX               1

#define THINKOS_ENABLE_OFAST                1

#define THINKOS_ENABLE_CONSOLE              1
#define THINKOS_ENABLE_CONSOLE_MISC         1
#define THINKOS_ENABLE_CONSOLE_BREAK        1
#define THINKOS_ENABLE_CONSOLE_OPEN         1
#define THINKOS_ENABLE_CONSOLE_NONBLOCK     1
#define THINKOS_ENABLE_CONSOLE_DRAIN        1
#define THINKOS_ENABLE_CONSOLE_MODE         1
#define THINKOS_CONSOLE_RX_FIFO_LEN         64
#define THINKOS_CONSOLE_TX_FIFO_LEN         512

#define THINKOS_ENABLE_TRACE                0
#define THINKOS_ENABLE_ALIGN                1

#define THINKOS_ENABLE_KRNSVC               1

#define THINKOS_ENABLE_STACK_ALIGN          1
#define THINKOS_ENABLE_UNALIGN_TRAP         1
#define THINKOS_ENABLE_DIV0_TRAP            1

#define THINKOS_ENABLE_APP_CRC              1
#define THINKOS_CUSTOM_APP_TAG              "RDA8COM"

/* -------------------------------------------------------------------------
   STM32 Init
   -------------------------------------------------------------------------- */

#if 0
#define STM32_HCLK_HZ           168000000	/* Target HBA bus frequency */
#define STM32_HSE_HZ             12000000	/* External oscillator frequency */
#endif
#define STM32_HCLK_HZ           180633600	/* Target HBA bus frequency */
#define STM32_HSE_HZ             11289600	/* External oscillator frequency */
#define STM32_ENABLE_HSE         1	/* Use external oscillator */
#define STM32_ENABLE_PLL         1	/* Enable PLL */
#define STM32_ENABLE_PLLSAI      1
#define STM32_ENABLE_PLLI2S      1
#define STM32_ENABLE_PREFETCH    0	/* Disable prefetch for better 
									   ADC accuracy */

/* -------------------------------------------------------------------------
   USB 
   -------------------------------------------------------------------------- */

#define STM32_ENABLE_OTG_FS       1
#define STM32_OTG_FS_INEP_MAX     3
#define STM32_OTG_FS_OUTEP_MAX    2
#define STM32_OTG_FS_IO_INIT      0
#define STM32_OTG_FS_VBUS_ENABLE  0
#define STM32_VBUS_SENS_ENABLED   0
#define STM32_OTG_FS_RX_FIFO_SIZE 192
#define STM32_OTG_FS_BULK_PKTS    6

#define CDC_EP_IN_MAX_PKT_SIZE    64

/* -------------------------------------------------------------------------
   GDB
   -------------------------------------------------------------------------- */


/* -------------------------------------------------------------------------
   Bootloader 
   -------------------------------------------------------------------------- */

#define BOOT_ENABLE_GDB            0
#define MONITOR_DUMPMEM_ENABLE     1
#define MONITOR_WATCHPOINT_ENABLE  0
#define MONITOR_BREAKPOINT_ENABLE  0
#define MONITOR_THREAD_STEP_ENABLE 0
#define MONITOR_SELFTEST_ENABLE    1
#define MONITOR_CONFIGURE_ENABLE   0
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
 * Serial
 * ------------------------------------------------------------------------- */

#define SERIAL_RX_FIFO_LEN  1024

/* -------------------------------------------------------------------------
 * LibC
 * ------------------------------------------------------------------------- */

#define PRINTF_ENABLE_FLOAT 1

/* -------------------------------------------------------------------------
 * Shell 
 * ------------------------------------------------------------------------- */

#define SHELL_LINE_MAX    80
#define SHELL_HISTORY_MAX 16
#define SHELL_ARG_MAX     16

#define TTY_DEV_MAX 1

/* -------------------------------------------------------------------------
 * Trace
 * ------------------------------------------------------------------------- */

#define TRACE_TIMER STM32F_TIM5
#define TRACE_RING_SIZE 1024
#define TRACE_STRING_MAX 64

#endif				/* __CONFIG_H__ */
