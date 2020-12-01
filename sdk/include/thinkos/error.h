#ifndef __THINKOS_ERROR_H__
#define __THINKOS_ERROR_H__

#ifndef __THINKOS_ERROR__
#error "Never use <thinkos/error.h> directly; include <thinkos/kernel.h> instead."
#endif 

/* -------------------------------------------------------------------------- 
 * Internal errors
 * --------------------------------------------------------------------------*/

#define THINKOS_NO_ERROR               0
#define THINKOS_ERR_KRN_FAULT          1
#define THINKOS_ERR_HARD_FAULT         2
#define THINKOS_ERR_BUS_FAULT          3
#define THINKOS_ERR_USAGE_FAULT        4
#define THINKOS_ERR_MEM_MANAGE         5
#define THINKOS_ERR_COND_INVALID       6
#define THINKOS_ERR_COND_ALLOC         7
#define THINKOS_ERR_MUTEX_INVALID      8
#define THINKOS_ERR_MUTEX_ALLOC        9
#define THINKOS_ERR_MUTEX_NOTMINE      10
#define THINKOS_ERR_MUTEX_LOCKED       11
#define THINKOS_ERR_SEM_INVALID        12
#define THINKOS_ERR_SEM_ALLOC          13
#define THINKOS_ERR_THREAD_INVALID     14
#define THINKOS_ERR_THREAD_ALLOC       15
#define THINKOS_ERR_THREAD_SMALLSTACK  16
#define THINKOS_ERR_IRQ_INVALID        17
#define THINKOS_ERR_OBJECT_INVALID     18
#define THINKOS_ERR_OBJECT_ALLOC       19
#define THINKOS_ERR_GATE_INVALID       20
#define THINKOS_ERR_GATE_ALLOC         21
#define THINKOS_ERR_GATE_UNLOCKED      22
#define THINKOS_ERR_FLAG_INVALID       23
#define THINKOS_ERR_FLAG_ALLOC         24
#define THINKOS_ERR_EVSET_INVALID      25
#define THINKOS_ERR_EVSET_ALLOC        26
#define THINKOS_ERR_EVENT_OUTOFRANGE   27
#define THINKOS_ERR_CONSOLE_REQINV     28
#define THINKOS_ERR_CTL_REQINV         29
#define THINKOS_ERR_COMM_REQINV        30
#define THINKOS_ERR_SYSCALL_INVALID    31
#define THINKOS_ERR_CRITICAL_EXIT      32
#define THINKOS_ERR_INVALID_POINTER    33
#define THINKOS_ERR_CONSOLE_FAULT      34
#define THINKOS_ERR_INVALID_STACK      35
#define THINKOS_ERR_DMA_INVALID        36
#define THINKOS_ERR_SP_INVALID         37
#define THINKOS_ERR_USER               38
#define THINKOS_ERR_THREAD_STACKADDR   39
#define THINKOS_ERR_FLASH_INVALID      40
#define THINKOS_ERR_STACK_LIMIT        41
#define THINKOS_ERR_IDLE_STACK         42
#define THINKOS_ERR_NOT_IMPLEMENTED    43
#define THINKOS_ERR_THREAD_STACKALIGN  44
#define THINKOS_ERR_THREAD_ENTRYADDR   45
#define THINKOS_ERR_THREAD_EXITADDR    46
#define THINKOS_ERR_THREAD_EXIST       47
#define THINKOS_ERR_APP_INVALID        48
#define THINKOS_ERR_APP_CRC_ERROR      49
#define THINKOS_ERR_APP_DATA_INVALID   50
#define THINKOS_ERR_APP_CODE_INVALID   51
#define THINKOS_ERR_APP_BSS_INVALID    52

#define THINKOS_ERR_KRN_NORETTOBASE    53
#define THINKOS_ERR_KRN_RETMSP         54
#define THINKOS_ERR_KRN_IDLEFAULT      55
#define THINKOS_ERR_KRN_STACKOVF       56
#define THINKOS_ERR_KRN_UNSTACK        57

#define THINKOS_ERR_MAX                58

#ifndef __ASSEMBLER__

/* Mark for kernel breakpoint numbers. Breakpoints above this
   number are considered errors. */
#define THINKOS_BKPT_EXCEPT_OFF 128

#define THINKOS_ERROR_BKPT(_CODE_) ((THINKOS_BKPT_EXCEPT_OFF) + (_CODE_))

void thinkos_krn_syscall_err(unsigned int errno, unsigned int thread_idx);

#if (THINKOS_ENABLE_ERROR_TRAP)
  #define __THINKOS_ERROR(__TH, __CODE) thinkos_krn_syscall_err(__CODE, __TH)
#else
  #define __THINKOS_ERROR(__TH, __CODE)
#endif

extern const char thinkos_err_name_lut[THINKOS_ERR_MAX][12];

static inline char const * thinkos_krn_eertag(unsigned int errno)
{
	return (errno < THINKOS_ERR_MAX) ? thinkos_err_name_lut[errno] : "Undef";
}

#ifdef __cplusplus
extern "C" {
#endif

char const * thinkos_krn_strerr(unsigned int errno);


#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_ERROR_H__ */

