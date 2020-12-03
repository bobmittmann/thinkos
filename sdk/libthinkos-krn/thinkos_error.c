#include "thinkos_krn-i.h"

#if (DEBUG)
  #ifndef LOG_LEVEL
    #define LOG_LEVEL LOG_TRACE
  #elif LOG_LEVEL < LOG_TRACE
    #undef LOG_LEVEL 
    #define LOG_LEVEL LOG_TRACE
  #endif
#endif

#include <sys/dcclog.h>


const char thinkos_err_name_lut[THINKOS_ERR_MAX][12] = {
	[THINKOS_NO_ERROR]              = "Ok",
	[THINKOS_ERR_KRN_FAULT]         = "KrnFault",
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
	[THINKOS_ERR_DMA_INVALID]       = "DmaInvalid",
	[THINKOS_ERR_SP_INVALID]        = "SpInvalid",
	[THINKOS_ERR_USER]              = "User",
	[THINKOS_ERR_THREAD_STACKADDR]  = "StackAddr",
	[THINKOS_ERR_FLASH_INVALID]     = "FlshInvalid",
	[THINKOS_ERR_STACK_LIMIT]       = "StackLimit",
	[THINKOS_ERR_IDLE_STACK]        = "IdleStack",
	[THINKOS_ERR_NOT_IMPLEMENTED]   = "IdleStack",
	[THINKOS_ERR_THREAD_STACKALIGN] = "StackAlign",
	[THINKOS_ERR_THREAD_ENTRYADDR]  = "EntryAddr",
	[THINKOS_ERR_THREAD_EXITADDR]   = "ExitAddr",
	[THINKOS_ERR_THREAD_EXIST]      = "ThreadExist",
	[THINKOS_ERR_APP_INVALID]       = "AppInvalid",
	[THINKOS_ERR_APP_CRC_ERROR]     = "AppCRC",
	[THINKOS_ERR_APP_DATA_INVALID]  = "AppData",
	[THINKOS_ERR_APP_CODE_INVALID]  = "AppCode",
	[THINKOS_ERR_APP_BSS_INVALID]   = "AppBss",
	[THINKOS_ERR_KRN_NORETTOBASE]   = "NoRettobase",
	[THINKOS_ERR_KRN_RETMSP]        = "RetToMSP",
	[THINKOS_ERR_KRN_IDLEFAULT]     = "FaultOnIdle",
	[THINKOS_ERR_KRN_STACKOVF]      = "MSPStackOvf",
	[THINKOS_ERR_KRN_UNSTACK]       = "MSPUnstack"
};

/* Raise a fault */
void __thread_fault_raise(struct thinkos_rt * krn, unsigned int th, 
						  int errno) {
#if (THINKOS_ENABLE_THREAD_FAULT)
	__thread_fault_set(krn, th);
	__thread_stat_set(krn, th, THINKOS_WQ_FAULT, false);
	/* possibly remove from the time wait queue */
	__thread_clk_disable(krn, th);
	__thread_errno_set(krn, th, errno);
#endif

#if (THINKOS_ENABLE_READY_MASK)
	__thread_disble_all(krn);
#else
	__krn_suspend_all(krn);
#endif

	/* request brake */
	__krn_sched_brk_set(krn, th, errno);

	__krn_defer_sched(krn);
}

void __thread_discard(struct thinkos_rt * krn, unsigned int idx) 
{
	/* Suspend the thread */
	__krn_thread_suspend(krn, idx);

	__krn_defer_sched(krn);
}

void thinkos_krn_syscall_err(unsigned int errno, unsigned int thread)
{
	struct thinkos_rt * krn = &thinkos_rt;

	DCC_LOG2(LOG_WARNING, VT_PSH VT_FMG VT_REV "/!\\ <%2d> Error %d /!\\"
			 VT_POP, thread, errno);

#if DEBUG
	mdelay(500);
#endif

	DCC_LOG1(LOG_WARNING, VT_PSH VT_FMG VT_REV "    %8s    " VT_POP, 
			__thread_tag_get(krn, thread));

	__nvic_irq_disable_all();

#if (THINKOS_ENABLE_MONITOR) 
	__thread_fault_raise(krn, thread, errno);

	__tdump(krn);

	monitor_signal_break(MONITOR_THREAD_FAULT);
#else
	__krn_suspend_all(krn);
#endif
}

void thinkos_krn_sched_on_break(uint32_t sp, uint32_t thread)
{
//	struct thinkos_rt * krn = &thinkos_rt;

	DCC_LOG(LOG_WARNING, VT_PSH VT_FMG "Scheduler: On Break" VT_POP);

//	__krn_sched_brk_clr(krn);

	/* Notify the debug/monitor */
	thinkos_monitor_wakeup(); 
}

void thinkos_krn_sched_stack_err(uint32_t sp, uint32_t thread)
{
	struct thinkos_rt * krn = &thinkos_rt;

	DCC_LOG1(LOG_ERROR, VT_PSH VT_REV VT_FGR
			 " Scheduler stack limit, thread %d " VT_POP, 
			 thread);

#if DEBUG
	mdelay(1000);
#endif

	__tdump(krn);

	__krn_suspend_all(krn);


	for(;;);
}

uintptr_t thinkos_krn_sched_nullptr_err(uint32_t sp, uint32_t thread)
{
	struct thinkos_rt * krn = &thinkos_rt;

	DCC_LOG(LOG_ERROR, VT_PSH VT_REV VT_FRD
			 " /!\\ Scheduler NULL pointer! /!\\ " VT_POP);

	__tdump(krn);
	__krn_suspend_all(krn);
#if DEBUG
	mdelay(1000);
#endif

	return (uintptr_t)thinkos_krn_idle_reset();
}

uintptr_t thinkos_krn_sched_idle_err(uint32_t sp, unsigned int thread)
{
	struct thinkos_rt * krn = &thinkos_rt;

	DCC_LOG(LOG_ERROR, VT_PSH VT_REV VT_FGR
			 " /!\\ Scheduler idle error/!\\ " VT_POP);

	__tdump(krn);
	__krn_suspend_all(krn);

#if DEBUG
	mdelay(1000);
#endif

	return (uintptr_t)thinkos_krn_idle_reset();
}


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

