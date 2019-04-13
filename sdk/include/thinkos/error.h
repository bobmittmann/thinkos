#ifndef __THINKOS_ERROR_H__
#define __THINKOS_ERROR_H__

#ifndef __THINKOS_ERROR__
#error "Never use <thinkos/error.h> directly; include <thinkos/kernel.h> instead."
#endif 

/* -------------------------------------------------------------------------- 
 * Internal errors
 * --------------------------------------------------------------------------*/

enum thinkos_krn_err {
	THINKOS_NO_ERR                = 0,
	THINKOS_ERR_COND_INVALID      = 1,
	THINKOS_ERR_COND_ALLOC        = 2,
	THINKOS_ERR_MUTEX_INVALID     = 3,
	THINKOS_ERR_MUTEX_ALLOC       = 4,
	THINKOS_ERR_MUTEX_NOTMINE     = 5,
	THINKOS_ERR_MUTEX_LOCKED      = 6,
	THINKOS_ERR_SEM_INVALID       = 7,
	THINKOS_ERR_SEM_ALLOC         = 8,
	THINKOS_ERR_THREAD_INVALID    = 9,
	THINKOS_ERR_THREAD_ALLOC      = 10,
	THINKOS_ERR_THREAD_SMALLSTACK = 11,
	THINKOS_ERR_IRQ_INVALID       = 12,
	THINKOS_ERR_OBJECT_INVALID    = 13,
	THINKOS_ERR_OBJECT_ALLOC      = 14,
	THINKOS_ERR_GATE_INVALID      = 15,
	THINKOS_ERR_GATE_ALLOC        = 16,
	THINKOS_ERR_GATE_UNLOCKED     = 17,
	THINKOS_ERR_FLAG_INVALID      = 18,
	THINKOS_ERR_FLAG_ALLOC        = 19,
	THINKOS_ERR_EVSET_INVALID     = 20,
	THINKOS_ERR_EVSET_ALLOC       = 21,
	THINKOS_ERR_EVENT_OUTOFRANGE  = 22,
	THINKOS_ERR_CONSOLE_REQINV    = 23,
	THINKOS_ERR_CTL_REQINV        = 24,
	THINKOS_ERR_COMM_REQINV       = 25,
	THINKOS_ERR_SYSCALL_INVALID   = 26,
	THINKOS_ERR_CRITICAL_EXIT     = 27,
	THINKOS_ERR_INVALID_POINTER   = 28,
	THINKOS_ERR_CONSOLE_FAULT     = 29,
	THINKOS_ERR_INVALID_STACK     = 30,
	THINKOS_ERR_USER              = 31
};

#ifdef __cplusplus
extern "C" {
#endif

char const * thinkos_krn_strerr(enum thinkos_krn_err errno);

#ifdef __cplusplus
}
#endif

#endif /* __THINKOS_ERROR_H__ */

