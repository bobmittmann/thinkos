#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#include <thinkos.h>

static const char * const thinkos_krn_er_tab[] ={
	[THINKOS_ERR_COND_INVALID] = "Conditionall variable invalid",
	[THINKOS_ERR_COND_ALLOC] = "Conditional variable allocation",
	[THINKOS_ERR_MUTEX_INVALID] = "Mutex invalid",
	[THINKOS_ERR_MUTEX_ALLOC] = "Mutex allocation",
	[THINKOS_ERR_MUTEX_NOTMINE] = "Mutex locked by other thread",
	[THINKOS_ERR_MUTEX_LOCKED] = "Mutex already locked",
	[THINKOS_ERR_SEM_INVALID] = "Invalid Sempahore",
	[THINKOS_ERR_SEM_ALLOC] = "Semaphore allocation",
	[THINKOS_ERR_THREAD_INVALID] = "Thread invelid",
	[THINKOS_ERR_THREAD_ALLOC] = "Thread allocation",
	[THINKOS_ERR_THREAD_SMALLSTACK] = "Thread stack too small",
	[THINKOS_ERR_IRQ_INVALID] = "Invalid IRQ",
	[THINKOS_ERR_OBJECT_INVALID] = "Invalid kernel object",
	[THINKOS_ERR_OBJECT_ALLOC] = "Kernel object allocation",
	[THINKOS_ERR_GATE_INVALID] = "Gate invalid",
	[THINKOS_ERR_GATE_ALLOC] = "Gate allocation",
	[THINKOS_ERR_GATE_UNLOCKED] = "Gate unlocked",
	[THINKOS_ERR_FLAG_INVALID] = "Flag invalid",
	[THINKOS_ERR_FLAG_ALLOC] = "Flag allocation",
	[THINKOS_ERR_EVSET_INVALID] = "Event-set invalid",
	[THINKOS_ERR_EVSET_ALLOC] = "Event-set allocation ",
	[THINKOS_ERR_EVENT_OUTOFRANGE] = "Event out of range",
	[THINKOS_ERR_CONSOLE_REQINV] = "Console request invalid",
	[THINKOS_ERR_CTL_REQINV] = "Control request invalid",
	[THINKOS_ERR_COMM_REQINV] = "Comm request invalid",
	[THINKOS_ERR_SYSCALL_INVALID] = "System call invalid",
	[THINKOS_ERR_CRITICAL_EXIT] = "Critical exit",
	[THINKOS_ERR_INVALID_POINTER] = "Pointer invalid",
	[THINKOS_ERR_CONSOLE_FAULT] = "Console fault",
	[THINKOS_ERR_INVALID_STACK] = "Invelid stack",
	[THINKOS_ERR_USER] = "User defined error 0",
};

char const * thinkos_krn_strerr(enum thinkos_krn_err errno)
{
	if (errno > THINKOS_ERR_USER)
		return "";

	return thinkos_krn_er_tab[errno];
}

