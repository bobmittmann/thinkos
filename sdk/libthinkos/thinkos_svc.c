/* 
 * thikos_svc.c
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
#if THINKOS_ENABLE_OFAST
_Pragma ("GCC optimize (\"Ofast\")")
#endif
#include <thinkos.h>

void thinkos_thread_create_svc(int32_t * arg);

void thinkos_pause_svc(int32_t * arg, int self);

void thinkos_resume_svc(int32_t * arg);

void thinkos_join_svc(int32_t * arg, int self);

void thinkos_cancel_svc(int32_t * arg, int self);

void thinkos_exit_svc(int32_t * arg, int self);

void thinkos_terminate_svc(int32_t * arg, int self);


void thinkos_sleep_svc(int32_t * arg, int self);

void thinkos_alarm_svc(int32_t * arg, int self);


void thinkos_mutex_alloc_svc(int32_t * arg);

void thinkos_mutex_free_svc(int32_t * arg);

void thinkos_mutex_lock_svc(int32_t * arg, int self);

void thinkos_mutex_trylock_svc(int32_t * arg, int self);

void thinkos_mutex_timedlock_svc(int32_t * arg, int self);

void thinkos_mutex_unlock_svc(int32_t * arg, int self);


void thinkos_sem_alloc_svc(int32_t * arg);

void thinkos_sem_free_svc(int32_t * arg);

void thinkos_sem_init_svc(int32_t * arg);

void thinkos_sem_wait_svc(int32_t * arg, int self);

void thinkos_sem_trywait_svc(int32_t * arg);

void thinkos_sem_timedwait_svc(int32_t * arg, int self);

void thinkos_sem_post_svc(int32_t * arg);


void thinkos_cond_alloc_svc(int32_t * arg);

void thinkos_cond_free_svc(int32_t * arg);

void thinkos_cond_wait_svc(int32_t * arg, int self);

void thinkos_cond_timedwait_svc(int32_t * arg, int self);

void thinkos_cond_signal_svc(int32_t * arg);

void thinkos_cond_broadcast_svc(int32_t * arg);


void thinkos_ev_alloc_svc(int32_t * arg);

void thinkos_ev_free_svc(int32_t * arg);

void thinkos_ev_wait_svc(int32_t * arg, int self);

void thinkos_ev_timedwait_svc(int32_t * arg, int self);

void thinkos_ev_raise_svc(int32_t * arg);

void thinkos_ev_mask_svc(int32_t * arg);

void thinkos_ev_clear_svc(int32_t * arg);


void thinkos_flag_alloc_svc(int32_t * arg);

void thinkos_flag_free_svc(int32_t * arg);

void thinkos_flag_val_svc(int32_t * arg);

void thinkos_flag_set_svc(int32_t * arg);

void thinkos_flag_clr_svc(int32_t * arg);

void thinkos_flag_watch_svc(int32_t * arg, int self);

void thinkos_flag_timedwatch_svc(int32_t * arg, int self);

void thinkos_flag_give_svc(int32_t * arg);

void thinkos_flag_take_svc(int32_t * arg, int self);

void thinkos_flag_timedtake_svc(int32_t * arg, int self);


void thinkos_gate_alloc_svc(int32_t * arg);

void thinkos_gate_free_svc(int32_t * arg);

void thinkos_gate_open_svc(int32_t * arg);

void thinkos_gate_close_svc(int32_t * arg);

void thinkos_gate_exit_svc(int32_t * arg);

void thinkos_gate_wait_svc(int32_t * arg, int self);

void thinkos_gate_timedwait_svc(int32_t * arg, int self);


void thinkos_irq_wait_svc(int32_t * arg, int self);

void thinkos_irq_timedwait_svc(int32_t * arg, int self);

void thinkos_irq_timedwait_cleanup_svc(int32_t * arg, int self);

void thinkos_irq_ctl_svc(int32_t * arg);


void thinkos_console_svc(int32_t * arg, int self);

void thinkos_ctl_svc(int32_t * arg);

void thinkos_comm_svc(int32_t * arg, int self);

void thinkos_dbgmon_svc(int32_t * arg, int self);

void thinkos_idle_svc(int32_t * arg);

#if THINKOS_ENABLE_ESCALATE
/* Call a function in priviledged service mode. */
void thinkos_escalate_svc(int32_t * arg)
{
	int32_t ( * call)(int32_t, int32_t, int32_t) = (void *)arg[0];
	arg[0] = call(arg[1], arg[2], arg[3]);
}
#endif

static void thinkos_nosys(int32_t * arg)
{
	__THINKOS_ERROR(THINKOS_ERR_SYSCALL_INVALID);
	arg[0] = THINKOS_ENOSYS;
}

static void thinkos_clock_svc(int32_t * arg)
{
	arg[0] = thinkos_rt.ticks;
}

static void thinkos_thread_self_svc(int32_t * arg, int32_t self)
{
	/* Internal thread ids start form 0 whereas user
	   thread numbers start form one ... */
	arg[0] = self + 1;
}

#if THINKOS_ENABLE_CRITICAL
void thinkos_critical_enter_svc(int32_t * arg)
{
	struct cm3_scb * scb = CM3_SCB;
	if (++thinkos_rt.critical_cnt) {
		/* clear possibly pending service interrupt */
		scb->icsr = SCB_ICSR_PENDSVCLR;
	}
}

void thinkos_critical_exit_svc(int32_t * arg)
{
	if (thinkos_rt.critical_cnt == 0) {
		__THINKOS_ERROR(THINKOS_ERR_CRITICAL_EXIT);
		arg[0] = THINKOS_EFAULT;
	} else if ((--thinkos_rt.critical_cnt) == 0) {
		__thinkos_defer_sched();
	}
}
#endif

void thinkos_svc_isr(int32_t * arg, int32_t self, uint32_t svc)
{
	switch (svc) {
	case THINKOS_THREAD_SELF:
		thinkos_thread_self_svc(arg, self);
		break;

	case THINKOS_THREAD_CREATE:
		thinkos_thread_create_svc(arg);
		break;

/* ----------------------------------------------
 * Clock related
 * --------------------------------------------- */

	case THINKOS_CLOCK:
#if THINKOS_ENABLE_CLOCK
		thinkos_clock_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_ALARM:
#if THINKOS_ENABLE_ALARM
		thinkos_alarm_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;
		
	case THINKOS_SLEEP:
#if THINKOS_ENABLE_SLEEP
		thinkos_sleep_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

/* ----------------------------------------------
 * Mutex
 * --------------------------------------------- */

	case THINKOS_MUTEX_LOCK:
#if (THINKOS_MUTEX_MAX > 0)
		thinkos_mutex_lock_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_MUTEX_TRYLOCK:
#if (THINKOS_MUTEX_MAX > 0)
		thinkos_mutex_trylock_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_MUTEX_TIMEDLOCK:
#if (THINKOS_MUTEX_MAX > 0)
  #if THINKOS_ENABLE_TIMED_CALLS
		thinkos_mutex_timedlock_svc(arg, self);
  #else
		thinkos_mutex_lock_svc(arg, self);
  #endif
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_MUTEX_UNLOCK:
#if (THINKOS_MUTEX_MAX > 0)
		thinkos_mutex_unlock_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

/* ----------------------------------------------
 * Semaphores
 * --------------------------------------------- */

	case THINKOS_SEM_INIT:
#if (THINKOS_SEMAPHORE_MAX > 0)
		thinkos_sem_init_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_SEM_WAIT:
#if (THINKOS_SEMAPHORE_MAX > 0)
		thinkos_sem_wait_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_SEM_TRYWAIT:
#if (THINKOS_SEMAPHORE_MAX > 0)
		thinkos_sem_trywait_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_SEM_TIMEDWAIT:
#if (THINKOS_SEMAPHORE_MAX > 0)
  #if THINKOS_ENABLE_TIMED_CALLS
		thinkos_sem_timedwait_svc(arg, self);
  #else
		thinkos_sem_wait_svc(arg, self);
  #endif
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_SEM_POST:
#if (THINKOS_SEMAPHORE_MAX > 0)
		thinkos_sem_post_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;


/* ----------------------------------------------
 * Conditional Variables
 * --------------------------------------------- */

	case THINKOS_COND_WAIT:
#if (THINKOS_COND_MAX > 0)
		thinkos_cond_wait_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_COND_TIMEDWAIT:
#if (THINKOS_COND_MAX > 0)
  #if THINKOS_ENABLE_TIMED_CALLS
		thinkos_cond_timedwait_svc(arg, self);
  #else
		thinkos_cond_wait_svc(arg, self);
  #endif
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_COND_SIGNAL:
#if (THINKOS_COND_MAX > 0)
		thinkos_cond_signal_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_COND_BROADCAST:
#if (THINKOS_COND_MAX > 0)
		thinkos_cond_broadcast_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

/* ----------------------------------------------
 * Single (simple) flags
 * --------------------------------------------- */

	case THINKOS_FLAG_TAKE:
#if (THINKOS_FLAG_MAX > 0)
		thinkos_flag_take_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_FLAG_TIMEDTAKE:
#if (THINKOS_FLAG_MAX > 0)
  #if THINKOS_ENABLE_TIMED_CALLS
		thinkos_flag_timedtake_svc(arg, self);
  #else
		thinkos_flag_take_svc(arg, self);
  #endif
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_FLAG_GIVE:
#if (THINKOS_FLAG_MAX > 0)
		thinkos_flag_give_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_FLAG_VAL:
#if (THINKOS_FLAG_MAX > 0) && THINKOS_ENABLE_FLAG_WATCH
		thinkos_flag_val_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_FLAG_CLR:
#if (THINKOS_FLAG_MAX > 0) && THINKOS_ENABLE_FLAG_WATCH
		thinkos_flag_clr_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_FLAG_SET:
#if (THINKOS_FLAG_MAX > 0) && THINKOS_ENABLE_FLAG_WATCH
		thinkos_flag_set_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_FLAG_WATCH:
#if (THINKOS_FLAG_MAX > 0) && THINKOS_ENABLE_FLAG_WATCH
		thinkos_flag_watch_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_FLAG_TIMEDWATCH:
#if (THINKOS_FLAG_MAX > 0) && THINKOS_ENABLE_FLAG_WATCH
  #if THINKOS_ENABLE_TIMED_CALLS
		thinkos_flag_timedwatch_svc(arg, self);
  #else
		thinkos_flag_watch_svc(arg, self);
  #endif
#else
		thinkos_nosys(arg);
#endif
		break;

/* ----------------------------------------------
 * Gates (locked flags)
 * --------------------------------------------- */

	case THINKOS_GATE_WAIT:
#if (THINKOS_GATE_MAX > 0) 
		thinkos_gate_wait_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_GATE_TIMEDWAIT:
#if (THINKOS_GATE_MAX > 0)
  #if THINKOS_ENABLE_TIMED_CALLS
		thinkos_gate_timedwait_svc(arg, self);
  #else
		thinkos_gate_wait_svc(arg, self);
  #endif
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_GATE_EXIT:
#if (THINKOS_GATE_MAX > 0) 
		thinkos_gate_exit_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_GATE_OPEN:
#if (THINKOS_GATE_MAX > 0) 
		thinkos_gate_open_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_GATE_CLOSE:
#if (THINKOS_GATE_MAX > 0) 
		thinkos_gate_close_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;


/* ----------------------------------------------
 * Event sets (multiple flags)
 * --------------------------------------------- */

	case THINKOS_EVENT_WAIT:
#if (THINKOS_EVENT_MAX > 0)
		thinkos_ev_wait_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_EVENT_TIMEDWAIT:
#if (THINKOS_EVENT_MAX > 0)
  #if THINKOS_ENABLE_TIMED_CALLS
		thinkos_ev_timedwait_svc(arg, self);
  #else
		thinkos_ev_wait_svc(arg, self);
  #endif
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_EVENT_RAISE:
#if (THINKOS_EVENT_MAX > 0)
		thinkos_ev_raise_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_EVENT_MASK:
#if (THINKOS_EVENT_MAX > 0)
		thinkos_ev_mask_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_EVENT_CLEAR:
#if (THINKOS_EVENT_MAX > 0)
		thinkos_ev_clear_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

/* ----------------------------------------------
 * Console
 * --------------------------------------------- */

	case THINKOS_CONSOLE:
#if THINKOS_ENABLE_CONSOLE
		thinkos_console_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

/* ----------------------------------------------
 * Interrupts
 * --------------------------------------------- */

	case THINKOS_IRQ_WAIT:
#if THINKOS_IRQ_MAX > 0
		thinkos_irq_wait_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif /* THINKOS_IRQ_MAX > 0 */
		break;

	case THINKOS_IRQ_TIMEDWAIT:
#if THINKOS_ENABLE_IRQ_TIMEDWAIT 
		thinkos_irq_timedwait_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif /* THINKOS_ENABLE_IRQ_TIMEDWAIT  */
		break;

	case THINKOS_IRQ_TIMEDWAIT_CLEANUP:
#if THINKOS_ENABLE_IRQ_TIMEDWAIT 
		thinkos_irq_timedwait_cleanup_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif /* THINKOS_ENABLE_IRQ_TIMEDWAIT  */
		break;

	case THINKOS_IRQ_CTL:
#if THINKOS_ENABLE_IRQ_CTL
		thinkos_irq_ctl_svc(arg);
#else
		thinkos_nosys(arg);
#endif /* THINKOS_ENABLE_IRQ_CTL */
		break;


/* ----------------------------------------------
 * Dynamic Resource Allocation 
 * --------------------------------------------- */

#if THINKOS_MUTEX_MAX > 0
  #if THINKOS_ENABLE_MUTEX_ALLOC
	case THINKOS_MUTEX_ALLOC:
		thinkos_mutex_alloc_svc(arg);
		break;

	case THINKOS_MUTEX_FREE:
		thinkos_mutex_free_svc(arg);
		break;
  #endif /* THINKOS_MUTEX_ALLOC */
#endif /* THINKOS_MUTEX_MAX > 0 */

#if THINKOS_SEMAPHORE_MAX > 0
  #if THINKOS_ENABLE_SEM_ALLOC
	case THINKOS_SEM_ALLOC:
		thinkos_sem_alloc_svc(arg);
		break;

	case THINKOS_SEM_FREE:
		thinkos_sem_free_svc(arg);
		break;
  #endif /* THINKOS_ENABLE_SEM_ALLOC */
#endif /* THINKOS_SEMAPHORE_MAX > 0 */

#if THINKOS_COND_MAX > 0
  #if THINKOS_ENABLE_COND_ALLOC
	case THINKOS_COND_ALLOC:
		thinkos_cond_alloc_svc(arg);
		break;

	case THINKOS_COND_FREE:
		thinkos_cond_free_svc(arg);
		break;
  #endif /* THINKOS_COND_ALLOC */
#endif /* THINKOS_COND_MAX > 0 */

#if (THINKOS_FLAG_MAX > 0)
  #if THINKOS_ENABLE_FLAG_ALLOC
	case THINKOS_FLAG_ALLOC:
		thinkos_flag_alloc_svc(arg);
		break;

	case THINKOS_FLAG_FREE:
		thinkos_flag_free_svc(arg);
		break;
  #endif
#endif /* (THINKOS_FLAG_MAX > 0) */

#if (THINKOS_EVENT_MAX > 0)
  #if THINKOS_ENABLE_EVENT_ALLOC
	case THINKOS_EVENT_ALLOC:
		thinkos_ev_alloc_svc(arg);
		break;

	case THINKOS_EVENT_FREE:
		thinkos_ev_free_svc(arg);
		break;
  #endif
#endif /* (THINKOS_EVENT_MAX > 0) */

#if (THINKOS_GATE_MAX > 0)
  #if THINKOS_ENABLE_GATE_ALLOC
	case THINKOS_GATE_ALLOC:
		thinkos_gate_alloc_svc(arg);
		break;

	case THINKOS_GATE_FREE:
		thinkos_gate_free_svc(arg);
		break;
  #endif
#endif /* (THINKOS_GATE_MAX > 0) */

	case THINKOS_JOIN:
#if THINKOS_ENABLE_JOIN
		thinkos_join_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_PAUSE:
#if THINKOS_ENABLE_PAUSE
		thinkos_pause_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_RESUME:
#if THINKOS_ENABLE_PAUSE
		thinkos_resume_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_CANCEL:
#if THINKOS_ENABLE_CANCEL
		thinkos_cancel_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_EXIT:
#if THINKOS_ENABLE_EXIT
		thinkos_exit_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_TERMINATE:
#if THINKOS_ENABLE_TERMINATE
		thinkos_terminate_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_CTL:
#if THINKOS_ENABLE_CTL
		thinkos_ctl_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_ESCALATE:
#if THINKOS_ENABLE_ESCALATE
		thinkos_escalate_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_CRITICAL_ENTER:
#if THINKOS_ENABLE_CRITICAL
		thinkos_critical_enter_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_CRITICAL_EXIT:
#if THINKOS_ENABLE_CRITICAL
		thinkos_critical_exit_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;


/* ----------------------------------------------
 * Comm 
 * --------------------------------------------- */

	case THINKOS_COMM:
#if THINKOS_ENABLE_COMM
		thinkos_comm_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_ON_IDLE:
#if THINKOS_ENABLE_MONITOR || THINKOS_ENABLE_CRITICAL
		thinkos_idle_svc(arg);
#else
		thinkos_nosys(arg);
#endif
		break;

	case THINKOS_DBGMON:
#if THINKOS_ENABLE_MONITOR
		thinkos_dbgmon_svc(arg, self);
#else
		thinkos_nosys(arg);
#endif
		break;

	default:
		thinkos_nosys(arg);
		break;
	}
}

/* FIXME: this is a hack to force linking this file. 
 The linker then will override the weak alias for the cm3_svc_isr() */
const char thinkos_svc_nm[] = "SVC";

