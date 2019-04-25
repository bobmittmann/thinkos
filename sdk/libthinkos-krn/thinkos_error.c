#define __THINKOS_KERNEL__
#include <thinkos/kernel.h>
#include <thinkos.h>

const char thinkos_err_name_lut[THINKOS_ERR_MAX][12] = {
	[THINKOS_NO_ERROR]              = "Ok",
	[THINKOS_ERR_KERNEL_PANIC]      = "KrnPanic",
	[THINKOS_ERR_HARD_FAULT]        = "HardFault",
	[THINKOS_ERR_BUS_FAULT]         = "BusFault",
	[THINKOS_ERR_USAGE_FAULT]       = "UsageFault",
	[THINKOS_ERR_MEM_MANAGE]        = "MemManage",
	[THINKOS_ERR_COND_INVALID]      = "CondInvlid",
	[THINKOS_ERR_COND_ALLOC]        = "CondAlloc",
	[THINKOS_ERR_MUTEX_INVALID]     = "MtxInvalid",
	[THINKOS_ERR_MUTEX_ALLOC]       = "MtxAlloc",
	[THINKOS_ERR_MUTEX_NOTMINE]     = "MtxLock",
	[THINKOS_ERR_MUTEX_LOCKED]      = "MtxRelock",
	[THINKOS_ERR_SEM_INVALID]       = "SemInvalid",
	[THINKOS_ERR_SEM_ALLOC]         = "SemAlloc",
	[THINKOS_ERR_THREAD_INVALID]    = "ThrdInavlid",
	[THINKOS_ERR_THREAD_ALLOC]      = "ThrdAlloc",
	[THINKOS_ERR_THREAD_SMALLSTACK] = "SmallStack",
	[THINKOS_ERR_IRQ_INVALID]       = "IrqInvalid",
	[THINKOS_ERR_OBJECT_INVALID]    = "ObjInvalid",
	[THINKOS_ERR_OBJECT_ALLOC]      = "ObjAlloc",
	[THINKOS_ERR_GATE_INVALID]      = "GateInvalid",
	[THINKOS_ERR_GATE_ALLOC]        = "GateAlloc",
	[THINKOS_ERR_GATE_UNLOCKED]     = "GateUnlockd",
	[THINKOS_ERR_FLAG_INVALID]      = "FlagInvalid",
	[THINKOS_ERR_FLAG_ALLOC]        = "FlagAlloc",
	[THINKOS_ERR_EVSET_INVALID]     = "EvStInvalid",
	[THINKOS_ERR_EVSET_ALLOC]       = "EvStAlloc",
	[THINKOS_ERR_EVENT_OUTOFRANGE]  = "EvStRange",
	[THINKOS_ERR_CONSOLE_REQINV]    = "ConInvalid",
	[THINKOS_ERR_CTL_REQINV]        = "CtlInvalid",
	[THINKOS_ERR_COMM_REQINV]       = "CommInvalid",
	[THINKOS_ERR_SYSCALL_INVALID]   = "SysInvalid",
	[THINKOS_ERR_CRITICAL_EXIT]     = "CritExit",
	[THINKOS_ERR_INVALID_POINTER]   = "PtrInvalid",
	[THINKOS_ERR_CONSOLE_FAULT]     = "ConFault",
	[THINKOS_ERR_INVALID_STACK]     = "StkInvalid",
	[THINKOS_ERR_SP_INVALID]        = "SpInvalid"
};

#if 0
static const char * const thinkos_krn_er_tab[] ={
	[THINKOS_NO_ERROR]              = "No Error",
	[THINKOS_ERR_KERNEL_PANIC]      = "Kernel Panic",
	[THINKOS_ERR_HARD_FAULT] = "Hard Fault",
	[THINKOS_ERR_BUS_FAULT] = "Bus Fault",
	[THINKOS_ERR_USAGE_FAULT] = "Usage Fault",
	[THINKOS_ERR_MEM_MANAGE] = "Memory Management",
	[THINKOS_ERR_COND_INVALID] = "Conditional variable invalid",
	[THINKOS_ERR_COND_ALLOC] = "Conditional variable allocation",
	[THINKOS_ERR_MUTEX_INVALID] = "Mutex invalid",
	[THINKOS_ERR_MUTEX_ALLOC] = "Mutex allocation",
	[THINKOS_ERR_MUTEX_NOTMINE] = "Mutex locked by other thread",
	[THINKOS_ERR_MUTEX_LOCKED] = "Mutex already locked",
	[THINKOS_ERR_SEM_INVALID] = "Invalid Sempahore",
	[THINKOS_ERR_SEM_ALLOC] = "Semaphore allocation",
	[THINKOS_ERR_THREAD_INVALID] = "Thread invalid",
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
	[THINKOS_ERR_INVALID_STACK] = "Invalid stack",
	[THINKOS_ERR_SP_INVALID] = "Invalid stack pointer",
	[THINKOS_ERR_USER] = "User defined error 0",
};

char const * thinkos_krn_strerr(unsigned int errno)
{
	if (errno > THINKOS_ERR_USER)
		return "";

	return thinkos_krn_er_tab[errno];
}
#endif
