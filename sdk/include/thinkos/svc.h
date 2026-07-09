
/** 
 * @file thinkos/svc.h
 * @brief ThinkOS svc calls
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __THINKOS_SVC_H__
#define __THINKOS_SVC_H__

#ifndef __THINKOS_SVC__
#error "Only use this file on privileged code"
#endif 

#include <stdint.h>

/* Argument to receive the return value in the call stack 
 * The number 4 corresponds to r12 */
#define SVC_ARG_R0      0
#define SVC_ARG_R1      1
#define SVC_ARG_R2      2
#define SVC_ARG_R3      3
#define SVC_ARG_R12     4
#define SVC_ARG_LR      5
#define SVC_ARG_PC      6
#define SVC_ARG_XPSR    7
#define SVC_RETURN      (SVC_ARG_R12)

typedef void (* thinkos_svc_t)(int32_t arg[], int self, struct thinkos_krn * krn);

#ifdef __cplusplus
extern "C" {
#endif

void thinkos_thread_init_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_pause_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_resume_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_join_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_cancel_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_exit_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_terminate_svc(int32_t arg[], int self, struct thinkos_krn * krn);


void thinkos_sleep_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_alarm_svc(int32_t arg[], int self, struct thinkos_krn * krn);


void thinkos_obj_alloc_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_obj_free_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_mutex_lock_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_mutex_trylock_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_mutex_timedlock_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_mutex_unlock_svc(int32_t arg[], int self, struct thinkos_krn * krn);


void thinkos_sem_init_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_sem_wait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_sem_trywait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_sem_timedwait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_sem_post_svc(int32_t arg[], int self, struct thinkos_krn * krn);


void thinkos_cond_wait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_cond_timedwait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_cond_signal_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_cond_broadcast_svc(int32_t arg[], int self, struct thinkos_krn * krn);


void thinkos_ev_wait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_ev_timedwait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_ev_raise_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_ev_mask_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_ev_clear_svc(int32_t arg[], int self, struct thinkos_krn * krn);



void thinkos_flag_val_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_flag_set_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_flag_clr_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_flag_watch_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_flag_timedwatch_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_flag_give_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_flag_take_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_flag_timedtake_svc(int32_t arg[], int self, struct thinkos_krn * krn);


void thinkos_gate_open_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_gate_close_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_gate_exit_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_gate_wait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_gate_timedwait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_irq_dbg_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_irq_wait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_irq_timedwait_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_irq_timedwait_fixup_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_irq_ctl_svc(int32_t arg[], int self, struct thinkos_krn * krn);


void thinkos_console_ctl_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_console_write_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_console_read_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_console_timedread_svc(int32_t arg[], int self, 
								  struct thinkos_krn * krn);

void thinkos_ctl_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_comm_ctl_svc(int32_t arg[], int self, struct thinkos_krn * krn);
void thinkos_comm_send_svc(int32_t arg[], int self, struct thinkos_krn * krn);
void thinkos_comm_recv_svc(int32_t arg[], int self, struct thinkos_krn * krn);
void thinkos_comm_timedsend_svc(int32_t arg[], int self, 
								struct thinkos_krn * krn);
void thinkos_comm_timedrecv_svc(int32_t arg[], int self, 
								struct thinkos_krn * krn);
void thinkos_comm_timed_fixup_svc(int32_t arg[], int self, 
								  struct thinkos_krn * krn);


void thinkos_monitor_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_trace_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_trace_ctl_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_flash_mem_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_app_exec_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_core_reset_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_time_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_nosys_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_clock_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_thread_self_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_yield_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_critical_enter_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_critical_exit_svc(int32_t arg[], int self, struct thinkos_krn * krn);

void thinkos_nrt_thread_alloc_svc(int32_t * arg, int self, struct thinkos_krn * krn);

#ifdef __cplusplus
}
#endif	

#endif /* __THINKOS_SVC_H__ */

