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
	[THINKOS_ERR_KRN_UNSTACK]       = "MSPUnstack",
	[THINKOS_ERR_IDLE_ENTRY]        = "IdleEntry",
	[THINKOS_ERR_IDLE_XCPT]         = "IdleExcept"
};

char const * thinkos_krn_err_tag(unsigned int errno)
{
	return (errno < THINKOS_ERR_MAX) ? thinkos_err_name_lut[errno] : "Undef";
}

void thinkos_krn_sched_brk_handler(struct thinkos_rt * krn, uint32_t stat)
{
	uint32_t act = SCHED_STAT_ACT(stat);
	uint32_t brk = SCHED_STAT_BRK(stat);
	uint32_t svc = SCHED_STAT_SVC(stat);
	uint32_t err = SCHED_STAT_ERR(stat);

	(void)act;
	(void)brk;
	(void)err;
	(void)svc;

	DCC_LOG4(LOG_INFO, VT_PSH VT_FGR " Service: act=%d brk=%d "
			 "err=%d, svc=%d " VT_POP, act, brk, err, svc);
}

#define THINKOS_ENABLE_KRN_SCHED_SVC 1

#if (THINKOS_ENABLE_KRN_SCHED_SVC) 
void thinkos_krn_sched_svc_reset(struct thinkos_rt * krn)
{
	DCC_LOG(LOG_WARNING, VT_PSH VT_FYW " !! DBG Reset  !! " VT_POP);

	__thinkos_krn_core_reset(krn);
}

/* Kernel defered services handler */
void thinkos_krn_sched_svc_handler(struct thinkos_rt * krn, uint32_t stat)
{
	uint32_t th_act =  SCHED_STAT_ACT(stat);
	uint32_t th_brk = SCHED_STAT_BRK(stat);
	uint32_t svc =  SCHED_STAT_SVC(stat);

	(void)th_act;
	(void)th_brk;
	(void)svc;

#if DEBUG
	DCC_LOG3(LOG_TRACE, VT_PSH VT_FBL VT_REV " Service: act=%d brk=%d svc=%d " 
			 VT_POP, th_act, th_brk, svc);
#endif

	switch (svc) {
	case 1:
		/* Clear the brake condition */
		__krn_sched_brk_clr(krn);
		break;

	case 2:
		if (th_act == 0)
			th_act = THINKOS_THREAD_IDLE;
		/* Set the brake condition */
		__krn_sched_brk_set(krn, th_act);
		break;
	}

	/* Notify the debug/monitor */
	monitor_signal(MONITOR_THREAD_BREAK);
}
#endif

/* Kernel error trap services handler */
void thinkos_krn_sched_err_handler(struct thinkos_rt * krn, uint32_t stat)
{
	uint32_t th_act =  SCHED_STAT_ACT(stat);
	uint32_t th_brk = SCHED_STAT_BRK(stat);
	uint32_t errno = SCHED_STAT_ERR(stat);

	(void)th_act;
	(void)th_brk;
	(void)errno;

#if DEBUG
	if (errno < THINKOS_ERR_MAX) {
		DCC_LOG4(LOG_WARNING, VT_PSH VT_FRD VT_REV 
				 " Error %d \"%s\" - act=%d brk=%d " VT_POP, 
				 errno, thinkos_err_name_lut[errno], th_act, th_brk); 
	} else {
		DCC_LOG3(LOG_WARNING, VT_PSH VT_FRD VT_REV 
				 " Error %d - act=%d brk=%d " 
				 VT_POP, errno, th_act, th_brk);
	}
	mdelay(500);
//	__tdump(krn);
#endif

#if (THINKOS_ENABLE_MONITOR) 

  #if (THINKOS_ENABLE_THREAD_FAULT)
	/* -----------------------------------------------------------------------
	 * Per thread error. Multiple faulty threads allowed ... 
	 * -----------------------------------------------------------------------
	 */
	__thread_stat_set(krn, th_act, THINKOS_WQ_FAULT, false);
	/* possibly remove from the time wait queue */
	__thread_clk_disable(krn, th_act);
	/* transfer error to thread */
	__thread_errno_set(krn, th_act, errno);
	/* clear scheduler error */
	__krn_sched_err_clr(krn);
  #else
	/* -----------------------------------------------------------------------
	 * Per thread error. Multiple faulty threads allowed ... 
	 * Global kernel/scheduler error. A single thread fault stops the kernel.
	 * -----------------------------------------------------------------------
	 */

	/* Disable all interrupts */
	__nvic_irq_disable_all();
  #endif

	/* Set the brake condition */
	__krn_sched_brk_set(krn, th_act);
	/* Notify the debug/monitor */
	monitor_signal_break(MONITOR_THREAD_FAULT);

#endif /* THINKOS_ENABLE_MONITOR */

}

