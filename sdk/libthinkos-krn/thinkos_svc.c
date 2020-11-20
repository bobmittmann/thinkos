/* 
 * thinkos_svc.c
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

#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#define __THINKOS_IRQ__
#include <thinkos/irq.h>
#define __THINKOS_MONITOR__
#include <thinkos/monitor.h>
#if (THINKOS_ENABLE_OFAST)
_Pragma ("GCC optimize (\"Ofast\")")
#endif
#include <thinkos.h>
#include <vt100.h>
#if (THINKOS_ENABLE_SCHED_DEBUG)
  #ifndef LOG_LEVEL
    #define LOG_LEVEL LOG_TRACE
  #elif LOG_LEVEL < LOG_TRACE
    #undef LOG_LEVEL 
    #define LOG_LEVEL LOG_TRACE
  #endif
#endif

#include <sys/dcclog.h>


void thinkos_thread_init_svc(int32_t * arg, int self);

void thinkos_pause_svc(int32_t * arg, int self);

void thinkos_resume_svc(int32_t * arg, int self);

void thinkos_join_svc(int32_t * arg, int self);

void thinkos_cancel_svc(int32_t * arg, int self);

void thinkos_exit_svc(int32_t * arg, int self);

void thinkos_terminate_svc(int32_t * arg, int self);


void thinkos_sleep_svc(int32_t * arg, int self);

void thinkos_alarm_svc(int32_t * arg, int self);


void thinkos_obj_alloc_svc(int32_t * arg, int self);

void thinkos_obj_free_svc(int32_t * arg, int self);

void thinkos_mutex_lock_svc(int32_t * arg, int self);

void thinkos_mutex_trylock_svc(int32_t * arg, int self);

void thinkos_mutex_timedlock_svc(int32_t * arg, int self);

void thinkos_mutex_unlock_svc(int32_t * arg, int self);


void thinkos_sem_init_svc(int32_t * arg, int self);

void thinkos_sem_wait_svc(int32_t * arg, int self);

void thinkos_sem_trywait_svc(int32_t * arg, int self);

void thinkos_sem_timedwait_svc(int32_t * arg, int self);

void thinkos_sem_post_svc(int32_t * arg, int self);


void thinkos_cond_wait_svc(int32_t * arg, int self);

void thinkos_cond_timedwait_svc(int32_t * arg, int self);

void thinkos_cond_signal_svc(int32_t * arg, int self);

void thinkos_cond_broadcast_svc(int32_t * arg, int self);


void thinkos_ev_wait_svc(int32_t * arg, int self);

void thinkos_ev_timedwait_svc(int32_t * arg, int self);

void thinkos_ev_raise_svc(int32_t * arg, int self);

void thinkos_ev_mask_svc(int32_t * arg, int self);

void thinkos_ev_clear_svc(int32_t * arg, int self);



void thinkos_flag_val_svc(int32_t * arg, int self);

void thinkos_flag_set_svc(int32_t * arg, int self);

void thinkos_flag_clr_svc(int32_t * arg, int self);

void thinkos_flag_watch_svc(int32_t * arg, int self);

void thinkos_flag_timedwatch_svc(int32_t * arg, int self);

void thinkos_flag_give_svc(int32_t * arg, int self);

void thinkos_flag_take_svc(int32_t * arg, int self);

void thinkos_flag_timedtake_svc(int32_t * arg, int self);


void thinkos_gate_open_svc(int32_t * arg, int self);

void thinkos_gate_close_svc(int32_t * arg, int self);

void thinkos_gate_exit_svc(int32_t * arg, int self);

void thinkos_gate_wait_svc(int32_t * arg, int self);

void thinkos_gate_timedwait_svc(int32_t * arg, int self);


void thinkos_irq_wait_svc(int32_t * arg, int self);

void thinkos_irq_timedwait_svc(int32_t * arg, int self);

void thinkos_irq_timedwait_cleanup_svc(int32_t * arg, int self);

void thinkos_irq_ctl_svc(int32_t * arg, int self);


void thinkos_console_svc(int32_t * arg, int self);

void thinkos_ctl_svc(int32_t * arg, int self);

void thinkos_comm_svc(int32_t * arg, int self);

void thinkos_monitor_svc(int32_t * arg, int self);

void thinkos_trace_svc(int32_t * arg, int self);

void thinkos_trace_ctl_svc(int32_t * arg, int self);

void thinkos_flash_mem_svc(int32_t * arg, int self);

void thinkos_app_exec_svc(int32_t * arg, int self);

#if (THINKOS_ENABLE_ESCALATE)
/* Call a function in priviledged service mode. */
void thinkos_escalate_svc(int32_t * arg, int self)
{
	int32_t ( * call)(int32_t, int32_t, int32_t);

	call = (int32_t (*)(int32_t, int32_t, int32_t))arg[0];
	arg[0] = call(arg[1], arg[2], arg[3]);
}
#endif

void thinkos_nosys_svc(int32_t * arg, int self)
{
	__THINKOS_ERROR(self, THINKOS_ERR_SYSCALL_INVALID);
	arg[0] = THINKOS_ENOSYS;
}

#if (THINKOS_ENABLE_CLOCK)
static void thinkos_clock_svc(int32_t * arg, int self)
{
	arg[0] = thinkos_rt.ticks;
}
#endif

static void thinkos_thread_self_svc(int32_t * arg, int32_t self)
{
	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	arg[0] = self + 1;
}

#if (THINKOS_ENABLE_CRITICAL)
void thinkos_critical_enter_svc(int32_t * arg, int self)
{
	if (++thinkos_rt.critical_cnt) {
		thinkos_krn_sched_off();
	}
}

void thinkos_critical_exit_svc(int32_t * arg, int self)
{
	if (thinkos_rt.critical_cnt == 0) {
		__THINKOS_ERROR(self, THINKOS_ERR_CRITICAL_EXIT);
		arg[0] = THINKOS_EFAULT;
	} else if ((--thinkos_rt.critical_cnt) == 0) {
		thinkos_krn_sched_on();
	}
}
#endif

typedef void (* thinkos_svc_t)(int32_t * arg, int self);

thinkos_svc_t const thinkos_svc_call_tab[] = {
	[THINKOS_THREAD_SELF] = thinkos_thread_self_svc,
	[THINKOS_THREAD_INIT] = thinkos_thread_init_svc,

#if (THINKOS_ENABLE_CLOCK)
	[THINKOS_CLOCK] = thinkos_clock_svc,
#else
	[THINKOS_CLOCK] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_ALARM)
	[THINKOS_ALARM] = thinkos_alarm_svc,
#else
	[THINKOS_ALARM] = thinkos_nosys_svc,
#endif
		
#if (THINKOS_ENABLE_SLEEP)
	[THINKOS_SLEEP] = thinkos_sleep_svc,
#else
	[THINKOS_SLEEP] = thinkos_nosys_svc,
#endif

/* ----------------------------------------------
 * Mutex
 * --------------------------------------------- */

#if (THINKOS_MUTEX_MAX) > 0
	[THINKOS_MUTEX_LOCK] = thinkos_mutex_lock_svc,
#else
	[THINKOS_MUTEX_LOCK] = thinkos_nosys_svc,
#endif

#if (THINKOS_MUTEX_MAX) > 0
	[THINKOS_MUTEX_TRYLOCK] = thinkos_mutex_trylock_svc,
#else
	[THINKOS_MUTEX_TRYLOCK] = thinkos_nosys_svc,
#endif

#if (THINKOS_MUTEX_MAX) > 0
  #if (THINKOS_ENABLE_TIMED_CALLS)
	[THINKOS_MUTEX_TIMEDLOCK] = thinkos_mutex_timedlock_svc,
  #else
	[THINKOS_MUTEX_TIMEDLOCK] = thinkos_mutex_lock_svc,
  #endif
#else
	[THINKOS_MUTEX_TIMEDLOCK] = thinkos_nosys_svc,
#endif

#if (THINKOS_MUTEX_MAX) > 0
	[THINKOS_MUTEX_UNLOCK] = thinkos_mutex_unlock_svc,
#else
	[THINKOS_MUTEX_UNLOCK] = thinkos_nosys_svc,
#endif

/* ----------------------------------------------
 * Semaphores
 * --------------------------------------------- */

#if (THINKOS_SEMAPHORE_MAX) > 0
	[THINKOS_SEM_INIT] = thinkos_sem_init_svc,
#else
	[THINKOS_SEM_INIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_SEMAPHORE_MAX) > 0
	[THINKOS_SEM_WAIT] = thinkos_sem_wait_svc,
#else
	[THINKOS_SEM_WAIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_SEMAPHORE_MAX) > 0
	[THINKOS_SEM_TRYWAIT] = thinkos_sem_trywait_svc,
#else
	[THINKOS_SEM_TRYWAIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_SEMAPHORE_MAX) > 0
  #if (THINKOS_ENABLE_TIMED_CALLS)
	[THINKOS_SEM_TIMEDWAIT] = thinkos_sem_timedwait_svc,
  #else
	[THINKOS_SEM_TIMEDWAIT] = thinkos_sem_wait_svc,
  #endif
#else
	[THINKOS_SEM_TIMEDWAIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_SEMAPHORE_MAX) > 0
	[THINKOS_SEM_POST] = thinkos_sem_post_svc,
#else
	[THINKOS_SEM_POST] = thinkos_nosys_svc,
#endif

/* ----------------------------------------------
 * Conditional Variables
 * --------------------------------------------- */

#if (THINKOS_COND_MAX) > 0
	[THINKOS_COND_WAIT] = thinkos_cond_wait_svc,
#else
	[THINKOS_COND_WAIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_COND_MAX) > 0
  #if (THINKOS_ENABLE_TIMED_CALLS)
	[THINKOS_COND_TIMEDWAIT] = thinkos_cond_timedwait_svc,
  #else
	[THINKOS_COND_TIMEDWAIT] = thinkos_cond_wait_svc,
  #endif
#else
	[THINKOS_COND_TIMEDWAIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_COND_MAX) > 0
	[THINKOS_COND_SIGNAL] = thinkos_cond_signal_svc,
#else
	[THINKOS_COND_SIGNAL] = thinkos_nosys_svc,
#endif

#if (THINKOS_COND_MAX) > 0
	[THINKOS_COND_BROADCAST] = thinkos_cond_broadcast_svc,
#else
	[THINKOS_COND_BROADCAST] = thinkos_nosys_svc,
#endif

/* ----------------------------------------------
 * Single (simple) flags
 * --------------------------------------------- */

#if (THINKOS_FLAG_MAX) > 0
	[THINKOS_FLAG_TAKE] = thinkos_flag_take_svc,
#else
	[THINKOS_FLAG_TAKE] = thinkos_nosys_svc,
#endif

#if (THINKOS_FLAG_MAX) > 0
  #if (THINKOS_ENABLE_TIMED_CALLS)
	[THINKOS_FLAG_TIMEDTAKE] = thinkos_flag_timedtake_svc,
  #else
	[THINKOS_FLAG_TIMEDTAKE] = thinkos_flag_take_svc,
  #endif
#else
	[THINKOS_FLAG_TIMEDTAKE] = thinkos_nosys_svc,
#endif

#if (THINKOS_FLAG_MAX) > 0
	[THINKOS_FLAG_GIVE] = thinkos_flag_give_svc,
#else
	[THINKOS_FLAG_GIVE] = thinkos_nosys_svc,
#endif

#if (THINKOS_FLAG_MAX) > 0 && THINKOS_ENABLE_FLAG_WATCH
	[THINKOS_FLAG_VAL] = thinkos_flag_val_svc,
#else
	[THINKOS_FLAG_VAL] = thinkos_nosys_svc,
#endif

#if (THINKOS_FLAG_MAX) > 0 && THINKOS_ENABLE_FLAG_WATCH
	[THINKOS_FLAG_CLR] = thinkos_flag_clr_svc,
#else
	[THINKOS_FLAG_CLR] = thinkos_nosys_svc,
#endif

#if (THINKOS_FLAG_MAX) > 0 && THINKOS_ENABLE_FLAG_WATCH
	[THINKOS_FLAG_SET] = thinkos_flag_set_svc,
#else
	[THINKOS_FLAG_SET] = thinkos_nosys_svc,
#endif

#if (THINKOS_FLAG_MAX) > 0 && THINKOS_ENABLE_FLAG_WATCH
	[THINKOS_FLAG_WATCH] = thinkos_flag_watch_svc,
#else
	[THINKOS_FLAG_WATCH] = thinkos_nosys_svc,
#endif

#if (THINKOS_FLAG_MAX) > 0 && THINKOS_ENABLE_FLAG_WATCH
  #if (THINKOS_ENABLE_TIMED_CALLS)
	[THINKOS_FLAG_TIMEDWATCH] = thinkos_flag_timedwatch_svc,
  #else
	[THINKOS_FLAG_TIMEDWATCH] = thinkos_flag_watch_svc,
  #endif
#else
	[THINKOS_FLAG_TIMEDWATCH] = thinkos_nosys_svc,
#endif

/* ----------------------------------------------
 * Gates (locked flags)
 * --------------------------------------------- */

#if (THINKOS_GATE_MAX) > 0 
	[THINKOS_GATE_WAIT] = thinkos_gate_wait_svc,
#else
	[THINKOS_GATE_WAIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_GATE_MAX) > 0
  #if (THINKOS_ENABLE_TIMED_CALLS)
	[THINKOS_GATE_TIMEDWAIT] = thinkos_gate_timedwait_svc,
  #else
	[THINKOS_GATE_TIMEDWAIT] = thinkos_gate_wait_svc,
  #endif
#else
	[THINKOS_GATE_TIMEDWAIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_GATE_MAX) > 0 
	[THINKOS_GATE_EXIT] = thinkos_gate_exit_svc,
#else
	[THINKOS_GATE_EXIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_GATE_MAX) > 0 
	[THINKOS_GATE_OPEN] = thinkos_gate_open_svc,
#else
	[THINKOS_GATE_OPEN] = thinkos_nosys_svc,
#endif

#if (THINKOS_GATE_MAX) > 0 
	[THINKOS_GATE_CLOSE] = thinkos_gate_close_svc,
#else
	[THINKOS_GATE_CLOSE] = thinkos_nosys_svc,
#endif

/* ----------------------------------------------
 * Event sets (multiple flags)
 * --------------------------------------------- */

#if (THINKOS_EVENT_MAX) > 0
	[THINKOS_EVENT_WAIT] = thinkos_ev_wait_svc,
#else
	[THINKOS_EVENT_WAIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_EVENT_MAX) > 0
  #if (THINKOS_ENABLE_TIMED_CALLS)
	[THINKOS_EVENT_TIMEDWAIT] = thinkos_ev_timedwait_svc,
  #else
	[THINKOS_EVENT_TIMEDWAIT] = thinkos_ev_wait_svc,
  #endif
#else
	[THINKOS_EVENT_TIMEDWAIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_EVENT_MAX) > 0
	[THINKOS_EVENT_RAISE] = thinkos_ev_raise_svc,
#else
	[THINKOS_EVENT_RAISE] = thinkos_nosys_svc,
#endif

#if (THINKOS_EVENT_MAX) > 0
	[THINKOS_EVENT_MASK] = thinkos_ev_mask_svc,
#else
	[THINKOS_EVENT_MASK] = thinkos_nosys_svc,
#endif

#if (THINKOS_EVENT_MAX) > 0
	[THINKOS_EVENT_CLEAR] = thinkos_ev_clear_svc,
#else
	[THINKOS_EVENT_CLEAR] = thinkos_nosys_svc,
#endif

/* ----------------------------------------------
 * Console
 * --------------------------------------------- */

#if (THINKOS_ENABLE_CONSOLE)
	[THINKOS_CONSOLE] = thinkos_console_svc,
#else
	[THINKOS_CONSOLE] = thinkos_nosys_svc,
#endif

/* ----------------------------------------------
 * Interrupts
 * --------------------------------------------- */

#if (THINKOS_IRQ_MAX) > 0
	[THINKOS_IRQ_WAIT] = thinkos_irq_wait_svc,
#else
	[THINKOS_IRQ_WAIT] = thinkos_nosys_svc,
#endif /* THINKOS_IRQ_MAX > 0 */

#if (THINKOS_ENABLE_IRQ_TIMEDWAIT) 
	[THINKOS_IRQ_TIMEDWAIT] = thinkos_irq_timedwait_svc,
#else
	[THINKOS_IRQ_TIMEDWAIT] = thinkos_nosys_svc,
#endif /* THINKOS_ENABLE_IRQ_TIMEDWAIT  */

#if (THINKOS_ENABLE_IRQ_TIMEDWAIT)
	[THINKOS_IRQ_TIMEDWAIT_CLEANUP] = thinkos_irq_timedwait_cleanup_svc,
#else
	[THINKOS_IRQ_TIMEDWAIT_CLEANUP] = thinkos_nosys_svc,
#endif /* THINKOS_ENABLE_IRQ_TIMEDWAIT  */

#if (THINKOS_ENABLE_IRQ_CTL)
	[THINKOS_IRQ_CTL] = thinkos_irq_ctl_svc,
#else
	[THINKOS_IRQ_CTL] = thinkos_nosys_svc,
#endif /* THINKOS_ENABLE_IRQ_CTL */

#if (THINKOS_ENABLE_OBJ_ALLOC)
	[THINKOS_OBJ_ALLOC] = thinkos_obj_alloc_svc,
#else
	[THINKOS_OBJ_ALLOC] = thinkos_nosys_svc,
#endif /* THINKOS_ENABLE_OBJ_ALLOC */

#if (THINKOS_ENABLE_OBJ_FREE)
	[THINKOS_OBJ_FREE] = thinkos_obj_free_svc,
#else
	[THINKOS_OBJ_FREE] = thinkos_nosys_svc,
#endif /* THINKOS_ENABLE_OBJ_FREE */

#if (THINKOS_ENABLE_JOIN)
	[THINKOS_JOIN] = thinkos_join_svc,
#else
	[THINKOS_JOIN] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_PAUSE)
	[THINKOS_PAUSE] = thinkos_pause_svc,
#else
	[THINKOS_PAUSE] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_PAUSE)
	[THINKOS_RESUME] = thinkos_resume_svc,
#else
	[THINKOS_RESUME] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_CANCEL)
	[THINKOS_CANCEL] = thinkos_cancel_svc,
#else
	[THINKOS_CANCEL] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_EXIT)
	[THINKOS_EXIT] = thinkos_exit_svc,
#else
	[THINKOS_EXIT] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_TERMINATE)
	[THINKOS_TERMINATE] = thinkos_terminate_svc,
#else
	[THINKOS_TERMINATE] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_CTL)
	[THINKOS_CTL] = thinkos_ctl_svc,
#else
	[THINKOS_CTL] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_ESCALATE)
	[THINKOS_ESCALATE] = thinkos_escalate_svc,
#else
	[THINKOS_ESCALATE] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_CRITICAL)
	[THINKOS_CRITICAL_ENTER] = thinkos_critical_enter_svc,
#else
	[THINKOS_CRITICAL_ENTER] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_CRITICAL)
	[THINKOS_CRITICAL_EXIT] = thinkos_critical_exit_svc,
#else
	[THINKOS_CRITICAL_EXIT] = thinkos_nosys_svc,
#endif


/* ----------------------------------------------
 * Comm 
 * --------------------------------------------- */

#if (THINKOS_ENABLE_COMM)
	[THINKOS_COMM] = thinkos_comm_svc,
#else
	[THINKOS_COMM] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_MONITOR_SYSCALL) 
	[THINKOS_MONITOR] = thinkos_monitor_svc,
#else
	[THINKOS_MONITOR] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_TRACE)
	[THINKOS_TRACE] = thinkos_trace_svc,
#else
	[THINKOS_TRACE] = thinkos_nosys_svc,
#endif

#if (THINKOS_ENABLE_TRACE)
	[THINKOS_TRACE_CTL] = thinkos_trace_ctl_svc,
#else
	[THINKOS_TRACE_CTL] = thinkos_nosys_svc,
#endif

/* ----------------------------------------------
 * Flash memory IO
 * --------------------------------------------- */

#if (THINKOS_FLASH_MEM_MAX) > 0
	[THINKOS_FLASH_MEM] = thinkos_flash_mem_svc,
#else
	[THINKOS_FLASH_MEM] = thinkos_nosys_svc,
#endif

/* ----------------------------------------------
 * Application
 * --------------------------------------------- */

#if (THINKOS_ENABLE_APP)
	[THINKOS_APP_EXEC] = thinkos_app_exec_svc,
#else
	[THINKOS_APP_EXEC] = thinkos_nosys_svc,
#endif

};

