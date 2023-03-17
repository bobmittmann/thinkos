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
#define THINKOS_ERR_IRQ_TAKEN          18

#define THINKOS_ERR_OBJECT_INVALID     19
#define THINKOS_ERR_OBJECT_ALLOC       20
#define THINKOS_ERR_GATE_INVALID       21
#define THINKOS_ERR_GATE_ALLOC         22
#define THINKOS_ERR_GATE_UNLOCKED      23
#define THINKOS_ERR_FLAG_INVALID       24
#define THINKOS_ERR_FLAG_ALLOC         25
#define THINKOS_ERR_EVSET_INVALID      26
#define THINKOS_ERR_EVSET_ALLOC        27
#define THINKOS_ERR_EVENT_OUTOFRANGE   28
#define THINKOS_ERR_CONSOLE_REQINV     29
#define THINKOS_ERR_CTL_REQINV         30
#define THINKOS_ERR_COMM_REQINV        31
#define THINKOS_ERR_SYSCALL_INVALID    32
#define THINKOS_ERR_CRITICAL_EXIT      33
#define THINKOS_ERR_INVALID_POINTER    34
#define THINKOS_ERR_CONSOLE_FAULT      35
#define THINKOS_ERR_INVALID_STACK      36
#define THINKOS_ERR_DMA_INVALID        37
#define THINKOS_ERR_SP_INVALID         38
#define THINKOS_ERR_USER               39
#define THINKOS_ERR_THREAD_STACKADDR   40
#define THINKOS_ERR_FLASH_INVALID      41
#define THINKOS_ERR_STACK_LIMIT        42
#define THINKOS_ERR_IDLE_STACK         43
#define THINKOS_ERR_NOT_IMPLEMENTED    44
#define THINKOS_ERR_THREAD_STACKALIGN  45
#define THINKOS_ERR_THREAD_ENTRYADDR   46
#define THINKOS_ERR_THREAD_EXITADDR    47
#define THINKOS_ERR_THREAD_EXIST       48

#define THINKOS_ERR_APP_INVALID        49
#define THINKOS_ERR_APP_CRC_ERROR      50
#define THINKOS_ERR_APP_DATA_INVALID   51
#define THINKOS_ERR_APP_CODE_INVALID   52
#define THINKOS_ERR_APP_BSS_INVALID    53
#define THINKOS_ERR_APP_ABORT_REQ      54

#define THINKOS_ERR_KRN_RETMSP         55
#define THINKOS_ERR_KRN_RETHANDLER     56
#define THINKOS_ERR_KRN_IDLEFAULT      57
#define THINKOS_ERR_KRN_STACKOVF       58
#define THINKOS_ERR_KRN_UNSTACK        59
#define THINKOS_ERR_THREAD_CTX_INVALID 60

#define THINKOS_ERR_IDLE_ENTRY         61
#define THINKOS_ERR_IDLE_XCPT          62
#define THINKOS_ERR_IDLE_MSP           63

#define THINKOS_ERR_COMM_INVALID       64
#define THINKOS_ERR_MEMORY_INVALID     65


#define THINKOS_ERR_MAX                66

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

