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
	[THINKOS_ERR_GENERAL]           = "Error",
	[THINKOS_ERR_KRN_FAULT]         = "KrnFault",
	[THINKOS_ERR_HARD_FAULT]        = "HardFault",
	[THINKOS_ERR_MEM_MANAGE]        = "MemManage",
	[THINKOS_ERR_USAGE_FAULT]       = "UsageFault",
	[THINKOS_ERR_BUS_FAULT]         = "BusFault",
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
	[THINKOS_ERR_IRQ_TAKEN]         = "IrqTaken",
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
	[THINKOS_ERR_NOT_IMPLEMENTED]   = "NotImplmnt",
	[THINKOS_ERR_THREAD_STACKALIGN] = "StackAlign",
	[THINKOS_ERR_THREAD_ENTRYADDR]  = "EntryAddr",
	[THINKOS_ERR_THREAD_EXITADDR]   = "ExitAddr",
	[THINKOS_ERR_THREAD_EXIST]      = "ThreadExist",
	[THINKOS_ERR_APP_INVALID]       = "AppInvalid",
	[THINKOS_ERR_APP_CRC_ERROR]     = "AppCRC",
	[THINKOS_ERR_APP_DATA_INVALID]  = "AppData",
	[THINKOS_ERR_APP_CODE_INVALID]  = "AppCode",
	[THINKOS_ERR_APP_BSS_INVALID]   = "AppBss",
	[THINKOS_ERR_APP_ABORT_REQ]     = "AbortReq",
	[THINKOS_ERR_IDLE_MSP]          = "IdleMSP",
	[THINKOS_ERR_KRN_RETMSP]        = "RetToMSP",
	[THINKOS_ERR_KRN_IDLEFAULT]     = "FaultOnIdle",
	[THINKOS_ERR_KRN_STACKOVF]      = "MSPStackOvf",
	[THINKOS_ERR_KRN_UNSTACK]       = "MSPUnstack",
	[THINKOS_ERR_IDLE_ENTRY]        = "IdleEntry",
	[THINKOS_ERR_IDLE_XCPT]         = "IdleExcept",
	[THINKOS_ERR_COMM_INVALID]      = "CommInvalid",
	[THINKOS_ERR_MEMORY_INVALID]    = "MemInvalid" 
};

char const * thinkos_krn_err_tag(unsigned int errno)
{
	return (errno < THINKOS_ERR_MAX) ? thinkos_err_name_lut[errno] : "Undef";
}

/* Kernel error trap services handler */
#if (THINKOS_ENABLE_ERROR_TRAP)
void thinkos_krn_sched_err_handler(struct thinkos_rt * krn, uint32_t ctrl)
{
#if (DEBUG)
	uint32_t thread = SCHED_CTRL_ACT(ctrl);
	uint32_t errno = SCHED_CTRL_ERR(ctrl);
	uint32_t xcpno = SCHED_CTRL_XCP(ctrl);
	(void)thread;
	(void)errno;

	if (errno > 0) {
		if (errno < THINKOS_ERR_MAX) {
			DCC_LOG3(LOG_WARNING, VT_PSH VT_FYW VT_REV 
					 " Error %d \"%s\" - thread=%d" VT_POP, 
					 errno, thinkos_err_name_lut[errno], thread); 
		} else {
			DCC_LOG2(LOG_WARNING, VT_PSH VT_FRD VT_REV 
					 " Error %d - thread=%d" VT_POP, 
					 errno, thread);
		}
	}

	if (xcpno > 0) {
		DCC_LOG3(LOG_WARNING, VT_PSH VT_FYW VT_REV 
				 " Fault %d \"%s\" - thread=%d" VT_POP, 
				 xcpno, thinkos_err_name_lut[xcpno], thread); 
	}
	
	__xdump(krn, &thinkos_fault_rt);
#endif

#if (THINKOS_ENABLE_MONITOR) 
	/* Signal monitor */
	monitor_signal_break(MONITOR_THREAD_FAULT);
#else
#if (THINKOS_SYSRST_ONFAULT)
	thinkos_krn_sysrst();
#else
	__krn_ctrl_err_set(0);
#endif
#endif
}
#endif /* THINKOS_ENABLE_ERROR_TRAP */

#if (THINKOS_ENABLE_SCHED_ERROR) 
/*
 * This is called when an inconsistency is detected on 
 * the scheduler. No associated context is provided.
 */
void thinkos_krn_fatal_err_handler(struct thinkos_rt * krn)
{
#if (DEBUG)
	uint32_t errno;

	errno = __krn_sched_err_get(krn);

	DCC_LOG2(LOG_PANIC, VT_PSH VT_FRD VT_REV 
			 " Krn error %d \"%s\"" VT_POP, 
			 errno, thinkos_err_name_lut[errno]); 
	__kdump(krn);

#endif

#if (THINKOS_SYSRST_ONFAULT)
	thinkos_krn_sysrst();
#else
	thinkos_krn_halt();
#endif

}
#endif /* THINKOS_ENABLE_SCHED_ERROR */

 
