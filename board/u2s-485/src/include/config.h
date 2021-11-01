/** 
 * 
 * This file is common to the application as well as the bootloader.
 * It holds configuration options for ThinkOS, Bootloader and other 
 * system wide parameters.
 * 
 */

/** 
 * @file config.h
 * @brief USB to Serial Converter configuration
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */


#ifndef __CONFIG_H__
#define __CONFIG_H__

/* -------------------------------------------------------------------------
   ThinkOS configuration
   -------------------------------------------------------------------------- */

#define THINKOS_ENABLE_TIMESHARE            0
#define THINKOS_ENABLE_THREAD_STAT          1
#define THINKOS_ENABLE_TIMED_CALLS          1

#define THINKOS_IRQ_MAX                     60
#define THINKOS_ENABLE_IRQ_CTL              1
#define THINKOS_ENABLE_IRQ_CYCCNT           0
#define THINKOS_ENABLE_IRQ_PRIORITY_0       0
#define THINKOS_ENABLE_IRQ_TIMEDWAIT        0
#define THINKOS_ENABLE_WQ_IRQ               0

#define THINKOS_ENABLE_ALARM                1
#define THINKOS_ENABLE_SLEEP                1
#define THINKOS_THREADS_MAX                 8

/* These settings are used when its desireable
   to create and destroy threads dinamically */
#define THINKOS_ENABLE_JOIN                 0
/* Allow for pausing and resuming a thread execution.
   This is needed for GDB and debug monitor. */
#define THINKOS_ENABLE_PAUSE                0
#define THINKOS_ENABLE_CANCEL               0
#define THINKOS_ENABLE_EXIT                 0

#define THINKOS_MUTEX_MAX                   4

#define THINKOS_COND_MAX                    0

#define THINKOS_SEMAPHORE_MAX               2

#define THINKOS_EVENT_MAX                   0

#define THINKOS_FLAG_MAX                    4
/* Enable Rendez vous synchronization. Wakes up all threads
 watching a flag in a given instant.*/
#define THINKOS_ENABLE_FLAG_WATCH           0
    
#define THINKOS_GATE_MAX                    0


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

/* Allow to interrupt a system call */
#define THINKOS_ENABLE_BREAK                0

/* Enable the per thread CPU usage counters */
#define THINKOS_ENABLE_PROFILING            0

#define THINKOS_ENABLE_KRNSVC               0
/* Enable the debug monitor for real-time debug */
#define THINKOS_ENABLE_MONITOR              1
#define THINKOS_ENABLE_MONITOR_THREADS      0
#define THINKOS_ENABLE_MONITOR_CLOCK        0
#define THINKOS_MONITOR_STACK_SIZE          320
#define THINKOS_MONITOR_ENABLE_COMM_BRK     0

#define THINKOS_ENABLE_CONSOLE              1
#define THINKOS_ENABLE_CONSOLE_MISC         0 
#define THINKOS_ENABLE_CONSOLE_BREAK        0
#define THINKOS_ENABLE_CONSOLE_OPEN         0
#define THINKOS_ENABLE_CONSOLE_NONBLOCK     0
#define THINKOS_ENABLE_CONSOLE_DRAIN        0
#define THINKOS_ENABLE_CONSOLE_MODE         0
#define THINKOS_CONSOLE_RX_FIFO_LEN         64
#define THINKOS_CONSOLE_TX_FIFO_LEN         64

#define THINKOS_EXCEPT_STACK_SIZE           320
#define THINKOS_ENABLE_ERROR_TRAP           0
#define THINKOS_ENABLE_EXCEPTIONS           0
#define THINKOS_ENABLE_HARDFAULT            0
#define THINKOS_ENABLE_BUSFAULT             0
#define THINKOS_ENABLE_USAGEFAULT           0
#define THINKOS_ENABLE_MEMFAULT             0
#define THINKOS_ENABLE_THREAD_FAULT         0
#if DEBUG
  #define THINKOS_ENABLE_HARDFAULT          0
  #define THINKOS_ENABLE_EXCEPTIONS         0
  #define THINKOS_ENABLE_SCHED_DEBUG        1
  #define THINKOS_SYSRST_ONFAULT            0
#else
  #define THINKOS_ENABLE_HARDFAULT          0
  #define THINKOS_ENABLE_EXCEPTIONS         0
  #define THINKOS_ENABLE_SCHED_DEBUG        0
  #define THINKOS_SYSRST_ONFAULT            1
#endif
#define THINKOS_ENABLE_DEBUG_BKPT           0
#define THINKOS_ENABLE_DEBUG_WPT            0
#define THINKOS_ENABLE_DEBUG_STEP           0

#define THINKOS_ENABLE_MPU                  0
#define THINKOS_ENABLE_ESCALATE             0
#define THINKOS_ENABLE_FPU                  0

#define THINKOS_ENABLE_STACK_INIT           0

#define THINKOS_ENABLE_CTL                  0
#define THINKOS_ENABLE_CRITICAL             0
#define THINKOS_ENABLE_PREEMPTION           1

#ifdef DEBUG
  #define THINKOS_ENABLE_IDLE_WFI           0
#else
  #define THINKOS_ENABLE_IDLE_WFI           0
#endif

#define THINKOS_ENABLE_IDLE_HOOKS           0

#define THINKOS_ENABLE_OBJ_ALLOC            0
#define THINKOS_ENABLE_OBJ_FREE             0

#define THINKOS_ENABLE_I_CALLS              0
#define THINKOS_ENABLE_THREAD_INFO          0

#define THINKOS_ENABLE_MEMORY_CLEAR         0
#define THINKOS_FLASH_MEM_MAX               0

#define THINKOS_ENABLE_OFAST                0

#define THINKOS_ENABLE_APP                  0
#define THINKOS_ENABLE_MEMORY_MAP           0

/* Communication channels */
#define THINKOS_COMM_MAX                    1

/* -------------------------------------------------------------------------
 * delay
 * ------------------------------------------------------------------------- */

#define THINKOS_ENABLE_UDELAY_CALIBRATE     1
//#define UDELAY_FACTOR           767

/* -------------------------------------------------------------------------
   RCC 
   -------------------------------------------------------------------------- */
#define STM32_HCLK_HZ           72000000 /* Target HBA bus frequency */
#define STM32_HSE_HZ            12000000 /* External oscillator frequency */
#define STM32_ENABLE_HSE        1 /* Use external oscillator */
#define STM32_ENABLE_PLL        1 /* Enable PLL */
#define STM32_ENABLE_HSI        0 /* Don't use internal oscillator */

/* -------------------------------------------------------------------------
   USB 
   -------------------------------------------------------------------------- */

#define STM32_ENABLE_USB_FS         1
#define STM32_USB_FS_VBUS_CTRL      1
#define STM32_USB_FS_EP_MAX         4
#define STM32_USB_FS_IO_INIT        0
#define STM32_USB_FS_IRQ_ENABLE     0
#define STM32_USB_FS_VBUS_ENABLE    1
#define STM32_USB_FS_SUSPEND        1

#define CDC_EP_OUT_MAX_PKT_SIZE 64
#define CDC_EP_IN_MAX_PKT_SIZE  64
#define CDC_EP_INT_MAX_PKT_SIZE 8

/* -------------------------------------------------------------------------
   GDB
   -------------------------------------------------------------------------- */

#define GDB_ENABLE_MEM_MAP         0

/* -------------------------------------------------------------------------
 * Serial
 * ------------------------------------------------------------------------- */

#define SERIAL_RX_FIFO_LEN  1024

/* -------------------------------------------------------------------------
 * LIBC 
 * ------------------------------------------------------------------------- */

#define PRINTF_ENABLE_FLOAT 0

#endif				/* __CONFIG_H__ */

