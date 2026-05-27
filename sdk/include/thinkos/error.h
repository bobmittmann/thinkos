#ifndef __THINKOS_ERROR_H__
#define __THINKOS_ERROR_H__

#ifndef __THINKOS_ERROR__
#error "Never use <thinkos/error.h> directly; include <thinkos/kernel.h> instead."
#endif 

/* -------------------------------------------------------------------------- 
 * Internal errors
 * --------------------------------------------------------------------------*/

#define THINKOS_NO_ERROR               0
#define THINKOS_ERR_GENERAL            1
#define THINKOS_ERR_KRN_FAULT          2
#define THINKOS_ERR_HARD_FAULT         3
#define THINKOS_ERR_MEM_MANAGE         4
#define THINKOS_ERR_BUS_FAULT          5
#define THINKOS_ERR_USAGE_FAULT        6
#define THINKOS_ERR_COND_INVALID       7
#define THINKOS_ERR_COND_ALLOC         8
#define THINKOS_ERR_MUTEX_INVALID      9
#define THINKOS_ERR_MUTEX_ALLOC        10
#define THINKOS_ERR_MUTEX_NOTMINE      11
#define THINKOS_ERR_MUTEX_LOCKED       12
#define THINKOS_ERR_SEM_INVALID        13
#define THINKOS_ERR_SEM_ALLOC          14
#define THINKOS_ERR_THREAD_INVALID     15
#define THINKOS_ERR_THREAD_ALLOC       16
#define THINKOS_ERR_THREAD_SMALLSTACK  17
#define THINKOS_ERR_IRQ_INVALID        18
#define THINKOS_ERR_IRQ_TAKEN          19

#define THINKOS_ERR_OBJECT_INVALID     20
#define THINKOS_ERR_OBJECT_ALLOC       21
#define THINKOS_ERR_GATE_INVALID       22
#define THINKOS_ERR_GATE_ALLOC         23
#define THINKOS_ERR_GATE_UNLOCKED      24
#define THINKOS_ERR_FLAG_INVALID       25
#define THINKOS_ERR_FLAG_ALLOC         26
#define THINKOS_ERR_EVSET_INVALID      27
#define THINKOS_ERR_EVSET_ALLOC        28
#define THINKOS_ERR_EVENT_OUTOFRANGE   29
#define THINKOS_ERR_CONSOLE_REQINV     30
#define THINKOS_ERR_CTL_REQINV         31
#define THINKOS_ERR_COMM_REQINV        32
#define THINKOS_ERR_SYSCALL_INVALID    33
#define THINKOS_ERR_CRITICAL_EXIT      34
#define THINKOS_ERR_INVALID_POINTER    35
#define THINKOS_ERR_CONSOLE_FAULT      36
#define THINKOS_ERR_INVALID_STACK      37
#define THINKOS_ERR_DMA_INVALID        38
#define THINKOS_ERR_SP_INVALID         39

#define THINKOS_ERR_USER               40
#define THINKOS_ERR_THREAD_STACKADDR   41
#define THINKOS_ERR_FLASH_INVALID      42
#define THINKOS_ERR_STACK_LIMIT        43
#define THINKOS_ERR_IDLE_STACK         44
#define THINKOS_ERR_NOT_IMPLEMENTED    45
#define THINKOS_ERR_THREAD_STACKALIGN  46
#define THINKOS_ERR_THREAD_ENTRYADDR   47
#define THINKOS_ERR_THREAD_EXITADDR    48
#define THINKOS_ERR_THREAD_EXIST       49

#define THINKOS_ERR_APP_INVALID        50
#define THINKOS_ERR_APP_CRC_ERROR      51
#define THINKOS_ERR_APP_DATA_INVALID   52
#define THINKOS_ERR_APP_CODE_INVALID   53
#define THINKOS_ERR_APP_BSS_INVALID    54
#define THINKOS_ERR_APP_ABORT_REQ      55

#define THINKOS_ERR_KRN_RETMSP         56
#define THINKOS_ERR_KRN_RETHANDLER     57
#define THINKOS_ERR_KRN_IDLEFAULT      58
#define THINKOS_ERR_KRN_STACKOVF       59
#define THINKOS_ERR_KRN_UNSTACK        60
#define THINKOS_ERR_THREAD_CTX_INVALID 61

#define THINKOS_ERR_IDLE_ENTRY         62
#define THINKOS_ERR_IDLE_XCPT          63
#define THINKOS_ERR_IDLE_MSP           64

#define THINKOS_ERR_COMM_INVALID       65
#define THINKOS_ERR_MEMORY_INVALID     66


#define THINKOS_ERR_MAX                67

#ifndef __ASSEMBLER__


#ifdef __cplusplus
extern "C" {
#endif

char const * thinkos_krn_err_tag(unsigned int errno);
char const * thinkos_krn_strerr(unsigned int errno);


#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_ERROR_H__ */

