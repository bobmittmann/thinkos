# 1 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
# 1 "C:\\devel\\yard-ice\\thinkos\\board\\stm32f072bdiscovery\\boot//"
# 1 "<built-in>"
# 1 "<command-line>"
# 1 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
# 23 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
# 1 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 1
# 30 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
# 1 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/profile.h" 1
# 30 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/profile.h"
# 1 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/version.h" 1
# 31 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/profile.h" 2





# 1 "C:/devel/yard-ice/thinkos/board/stm32f072bdiscovery/include/config.h" 1
# 37 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/profile.h" 2
# 653 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/profile.h"
# 1 "C:/devel/yard-ice/thinkos/sdk/include/stdint.h" 1
# 29 "C:/devel/yard-ice/thinkos/sdk/include/stdint.h"
typedef signed char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;

typedef int intptr_t;
typedef unsigned int uintptr_t;
# 654 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/profile.h" 2





struct thinkos_profile {
 struct {
  uint16_t size;
  uint8_t version;
  uint8_t reserved;
 } header;

 struct {
  union {
   uint32_t u32;
   struct {
    uint8_t major;
    uint8_t minor;
    uint16_t reserved;
   };
  } version;
  union {
   uint32_t flags;
   struct {
    uint32_t timeshare :1;
    uint32_t preemption :1;
   };
  };
  uint32_t sched_limit_max :8;
  uint32_t sched_limit_min :8;
 } kernel;

 struct {
  union {
   uint32_t flags;
   struct {
    uint32_t raw :1;
    uint32_t elf :1;
   };
  };
 } app;

 struct {
  uint32_t threads_max :8;
  uint32_t nrt_threads_max :8;
  uint32_t mutex_max :8;
  uint32_t cond_max :8;

  uint32_t semaphore_max :8;
  uint32_t event_max :8;
  uint32_t flag_max :8;
  uint32_t gate_max :8;

  uint32_t queue_max :8;
  uint32_t irq_max :8;
  uint32_t dma_max :8;
  uint32_t except_stack_size :16;
 } limit;

 union {
  uint32_t flags;
  struct {
   uint32_t join :1;
   uint32_t clock :1;
   uint32_t alarm :1;
   uint32_t sleep :1;
   uint32_t ctl :1;
   uint32_t critical :1;
   uint32_t escalate :1;
   uint32_t irq_ctl :1;
   uint32_t pause :1;
   uint32_t cancel :1;
   uint32_t exit :1;
   uint32_t terminate :1;
   uint32_t thread_break :1;
   uint32_t console :1;
   uint32_t flag_watch :1;
   uint32_t timed_calls :1;
   uint32_t irq_timedwait :1;
  };
 } syscall;

 union {
  uint32_t flags;
  struct {
   uint32_t thread_alloc :1;
   uint32_t mutex_alloc :1;
   uint32_t cond_alloc :1;
   uint32_t sem_alloc :1;
   uint32_t event_alloc :1;
   uint32_t gate_alloc :1;
   uint32_t flag_alloc :1;
  };
 } alloc;

 union {
  uint32_t flags;
  struct {
   uint32_t thread_info :1;
   uint32_t thread_stat :1;
   uint32_t irq_cyccnt :1;
   uint32_t irq_priority_0 :1;
   uint32_t wq_irq :1;
   uint32_t console_break :1;
   uint32_t console_mode :1;
   uint32_t console_nonblock :1;
   uint32_t console_open :1;
   uint32_t console_drain :1;
   uint32_t console_read :1;
   uint32_t console_misc :1;
   uint32_t comm :1;
   uint32_t mpu :1;
   uint32_t fpu :1;
   uint32_t fpu_ls :1;
   uint32_t profiling :1;
   uint32_t mem_map :1;
   uint32_t flash_mem :1;
   uint32_t krn_trace :1;
  };
 } feature;

 union {
  uint32_t flags;
  struct {
   uint32_t arg_check :1;
   uint32_t deadlock_check :1;
   uint32_t sanity_check :1;
   uint32_t stack_init :1;
   uint32_t memory_clear :1;
  };
 } security;

 union {
  uint32_t flags;
  struct {
   uint32_t exceptions :1;
   uint32_t busfault :1;
   uint32_t usagefault :1;
   uint32_t memfault :1;
   uint32_t hardfault :1;
   uint32_t except_clear :1;
   uint32_t error_trap :1;
   uint32_t sysrst_onfault :1;
  };
 } except;

 union {
  uint32_t flags;
  struct {
   uint32_t monitor :1;
   uint32_t dmclock :1;
   uint32_t debug_step :1;
   uint32_t debug_bkpt :1;
   uint32_t debug_wpt :1;
   uint32_t debug_fault :1;
   uint32_t monitor_threads :1;
  };
 } dbgmon;

 union {
  uint32_t misc_flags;
  struct {
   uint32_t thread_void :1;
   uint32_t idle_wfi :1;
   uint32_t sched_debug :1;
   uint32_t ofast :1;
   uint32_t align :1;
   uint32_t idle_hooks :1;
   uint32_t idle_msp :1;
  };
 } misc;

 union {
  uint32_t flags;
  struct {
   uint32_t asm_scheduler :1;
   uint32_t rt_debug :1;
   uint32_t unroll_exceptions :1;
   uint32_t stderr_fault_dump :1;
  };
 } deprecated;
};

extern const struct thinkos_profile thinkos_profile;
# 31 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 2


# 1 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/error.h" 1
# 56 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/error.h"
extern const char thinkos_err_name_lut[38][12];





char const * thinkos_krn_strerr(unsigned int errno);
# 34 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 2
# 293 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
# 1 "C:/devel/yard-ice/thinkos/sdk/include/arch/cortex-m3.h" 1
# 684 "C:/devel/yard-ice/thinkos/sdk/include/arch/cortex-m3.h"
struct cm3_context {
 uint32_t r[13];
 uint32_t sp;
 uint32_t lr;
 uint32_t pc;
 uint32_t xpsr;
};

struct cm3_except_context {
 uint32_t r0;
 uint32_t r1;
 uint32_t r2;
 uint32_t r3;
 uint32_t r12;
 uint32_t lr;
 uint32_t pc;
 uint32_t xpsr;
};

struct v7m_basic_frame {
 uint32_t r0;
 uint32_t r1;
 uint32_t r2;
 uint32_t r3;
 uint32_t r12;
 uint32_t lr;
 uint32_t pc;
 uint32_t xpsr;
};

struct v7m_extended_frame {
 uint32_t r0;
 uint32_t r1;
 uint32_t r2;
 uint32_t r3;
 uint32_t r12;
 uint32_t lr;
 uint32_t pc;
 uint32_t xpsr;
 float s[16];
 uint32_t fpscr;
 uint32_t res;
};
# 748 "C:/devel/yard-ice/thinkos/sdk/include/arch/cortex-m3.h"
struct cm3_scb {
 volatile uint32_t cpuid;
 volatile uint32_t icsr;
 volatile uint32_t vtor;
 volatile uint32_t aircr;
 volatile uint32_t scr;
 volatile uint32_t ccr;
 volatile uint8_t shp[12];
 volatile uint32_t shcsr;
 volatile uint32_t cfsr;
 volatile uint32_t hfsr;
 volatile uint32_t dfsr;
 volatile uint32_t mmfar;
 volatile uint32_t bfar;
 volatile uint32_t afsr;
 volatile uint32_t pfr[2];
 volatile uint32_t dfr;
 volatile uint32_t adr;
 volatile uint32_t mmfr[4];
 volatile uint32_t isar[5];
 volatile uint32_t reserved0[5];
 volatile uint32_t cpacr;
 volatile uint32_t reserved1[0x5d];
 volatile uint32_t reserved2[0x34 / 4];
 volatile uint32_t fpccr;
 volatile uint32_t fpcar;
 volatile uint32_t fpdscr;
 volatile uint32_t mvfr0;
 volatile uint32_t mvfr1;
 volatile uint32_t mvfr2;
};


struct cm3_sys {
 volatile uint32_t mcr;
 uint32_t ictr;
 volatile uint32_t actlr;
 volatile uint32_t stir;
};
# 796 "C:/devel/yard-ice/thinkos/sdk/include/arch/cortex-m3.h"
struct cm3_dcb {
 volatile uint32_t dhcsr;
 volatile uint32_t dcrsr;
 volatile uint32_t dcrdr;
 volatile uint32_t demcr;
};






struct cm3_nvic {
 volatile uint32_t iser[8];
 uint32_t reserved0[24];
 volatile uint32_t icer[8];
 uint32_t rserved1[24];
 volatile uint32_t ispr[8];
 uint32_t reserved2[24];
 volatile uint32_t icpr[8];
 uint32_t reserved3[24];
 volatile uint32_t iabr[8];
 uint32_t reserved4[56];
 volatile uint8_t ip[240];
 uint32_t reserved5[644];
 volatile uint32_t stir;
};





struct cm3_systick {
 volatile uint32_t csr;
 volatile uint32_t rvr;
 volatile uint32_t cvr;
 volatile uint32_t calib;
};




struct cm3_mpu {
 volatile uint32_t type;
 volatile uint32_t ctrl;
 volatile uint32_t rnr;
 volatile uint32_t rbar;
 volatile uint32_t rasr;
 volatile uint32_t rbar_a1;
 volatile uint32_t rasr_a1;
 volatile uint32_t rbar_a2;
 volatile uint32_t rasr_a2;
 volatile uint32_t rbar_a3;
 volatile uint32_t rasr_a3;
};





struct cm3_itm {
 union {
  volatile uint8_t u8;
  volatile uint16_t u16;
  volatile uint32_t u32;
 } port[32];
 uint32_t reserved0[864];
 volatile uint32_t ter;
 uint32_t reserved1[15];
 volatile uint32_t tpr;
 uint32_t reserved2[15];
 volatile uint32_t tcr;
 uint32_t reserved3[29];
 volatile uint32_t iwr;
 volatile uint32_t irr;
 volatile uint32_t imcr;
 uint32_t reserved4[43];
 volatile uint32_t lar;
 volatile uint32_t lsr;
 uint32_t reserved5[6];
 volatile uint32_t pid4;
 volatile uint32_t pid5;
 volatile uint32_t pid6;
 volatile uint32_t pid7;
 volatile uint32_t pid0;
 volatile uint32_t pid1;
 volatile uint32_t pid2;
 volatile uint32_t pid3;
 volatile uint32_t cid0;
 volatile uint32_t cid1;
 volatile uint32_t cid2;
 volatile uint32_t cid3;
};





struct cm3_dwt {
 volatile uint32_t ctrl;
 volatile uint32_t cyccnt;
 volatile uint32_t cpicnt;
 volatile uint32_t exccnt;
 volatile uint32_t sleepcnt;
 volatile uint32_t lsucnt;
 volatile uint32_t foldcnt;
 volatile uint32_t pcsr;

 struct {
  volatile uint32_t comp;
  volatile uint32_t mask;
  volatile uint32_t function;
  uint32_t reserved0;
 } wp[4];
# 923 "C:/devel/yard-ice/thinkos/sdk/include/arch/cortex-m3.h"
 uint32_t reserved3[(0xfd0 - 0x05c) / 4];
 volatile uint32_t pid4;
 volatile uint32_t pid5;
 volatile uint32_t pid6;
 volatile uint32_t pid7;
 volatile uint32_t pid0;
 volatile uint32_t pid1;
 volatile uint32_t pid2;
 volatile uint32_t pid3;
 volatile uint32_t cid0;
 volatile uint32_t cid1;
 volatile uint32_t cid2;
 volatile uint32_t cid3;
};
# 997 "C:/devel/yard-ice/thinkos/sdk/include/arch/cortex-m3.h"
struct cm3_fpb {
 uint32_t ctrl;
 uint32_t remap;
 uint32_t comp[8];
};
# 1013 "C:/devel/yard-ice/thinkos/sdk/include/arch/cortex-m3.h"
extern const uint32_t cm3_systick_load_1ms;

struct cm4_fpu {
 uint32_t fpccr;
 uint32_t fpcar;
 uint32_t fpdscr;
};
# 1028 "C:/devel/yard-ice/thinkos/sdk/include/arch/cortex-m3.h"
static inline void __attribute__((always_inline)) cm3_irq_enable(int irq) {
 ((struct cm3_nvic *) ((0xe000e000) + 0x0100))->iser[irq >> 5] = (1 << (irq & 0x1f));
}


static inline void __attribute__((always_inline)) cm3_irq_disable(int irq) {
 ((struct cm3_nvic *) ((0xe000e000) + 0x0100))->icer[irq >> 5] = (1 << (irq & 0x1f));
}

static inline int cm3_irq_is_enabled(int irq) {
 return (((struct cm3_nvic *) ((0xe000e000) + 0x0100))->icer[irq >> 5] & (1 << (irq & 0x1f))) ? 1 : 0;
}


static inline int cm3_irq_pend_get(int irq) {
 return (((struct cm3_nvic *) ((0xe000e000) + 0x0100))->ispr[irq >> 5] & (1 << (irq & 0x1f))) ? 1 : 0;
}


static inline void __attribute__((always_inline)) cm3_irq_pend_set(int irq) {
 ((struct cm3_nvic *) ((0xe000e000) + 0x0100))->ispr[irq >> 5] = (1 << (irq & 0x1f));
}


static inline void __attribute__((always_inline)) cm3_irq_pend_clr(int irq) {
 ((struct cm3_nvic *) ((0xe000e000) + 0x0100))->icpr[irq >> 5] = (1 << (irq & 0x1f));
}


static inline int __attribute__((always_inline)) cm3_irq_act_get(int irq) {
 return (((struct cm3_nvic *) ((0xe000e000) + 0x0100))->iabr[irq >> 5] & (1 << (irq & 0x1f))) ? 1 : 0;
}


static inline void __attribute__((always_inline)) cm3_irq_pri_set(int irq, int pri) {
 ((struct cm3_nvic *) ((0xe000e000) + 0x0100))->ip[irq] = pri;
}


static inline int __attribute__((always_inline)) cm3_irq_pri_get(int irq) {
 return ((struct cm3_nvic *) ((0xe000e000) + 0x0100))->ip[irq];
}


static inline void __attribute__((always_inline)) cm3_except_pri_set(int xcpt, int pri) {
 ((struct cm3_scb *) ((0xe000e000) + 0x0d00))->shp[xcpt - 4] = pri;
}


static inline int __attribute__((always_inline)) cm3_except_pri_get(int xcpt) {
 return ((struct cm3_scb *) ((0xe000e000) + 0x0d00))->shp[xcpt - 4];
}


static inline uint32_t __attribute__((always_inline)) __bit_mem_rd(void * ptr, int32_t bit) {

 uint32_t * bit_addr = ((uint32_t *) (((((uint32_t)(ptr)) - 0x20000000 + 0x01100000) * 32) + (4 * ((0)))));
 return bit_addr[bit];
}


static inline void __attribute__((always_inline)) __bit_mem_wr(void * ptr, int32_t bit, uint32_t val) {

 uint32_t * bit_addr = ((uint32_t *) (((((uint32_t)(ptr)) - 0x20000000 + 0x01100000) * 32) + (4 * ((0)))));
 bit_addr[bit] = val;
}
# 1105 "C:/devel/yard-ice/thinkos/sdk/include/arch/cortex-m3.h"
static inline void __attribute__((always_inline)) cm3_control_set(uint32_t val) {
 asm volatile ("msr CONTROL, %0\n"
      "isb\n" : : "r" (val));
}

static inline uint32_t __attribute__((always_inline)) cm3_control_get(void) {
 uint32_t val;
 asm volatile ("mrs %0, CONTROL\n" : "=r" (val));
 return val;
}

static inline void __attribute__((always_inline))
cm3_basepri_set(uint32_t val) {
 asm volatile ("msr BASEPRI, %0\n" : : "r" (val));
}

static inline uint32_t __attribute__((always_inline))
cm3_basepri_get(void) {
 uint32_t val;
 asm volatile ("mrs %0, BASEPRI\n" : "=r" (val));
 return val;
}

static inline uint32_t __attribute__((always_inline)) cm3_ipsr_get(void) {
 uint32_t ipsr;
 asm volatile ("mrs %0, IPSR\n" : "=r" (ipsr));
 return ipsr;
}


static inline void __attribute__((always_inline)) cm3_cpsid_i(void) {
 asm volatile ("cpsid i\n");
}


static inline void __attribute__((always_inline)) cm3_cpsie_i(void) {
 asm volatile ("cpsie i\n");
}


static inline void __attribute__((always_inline)) cm3_cpsid_f(void) {
 asm volatile ("cpsid f\n");
}


static inline void __attribute__((always_inline)) cm3_cpsie_f(void) {
 asm volatile ("cpsie f\n");
}

static inline void __attribute__((always_inline)) cm3_primask_set(uint32_t val) {
 asm volatile ("msr PRIMASK, %0\n" : : "r" (val));
}

static inline uint32_t __attribute__((always_inline)) cm3_primask_get(void) {
 uint32_t val;
 asm volatile ("mrs %0, PRIMASK\n" : "=r" (val));
 return val;
}

static inline void __attribute__((always_inline)) cm3_faultmask_set(uint32_t val) {
 asm volatile ("msr FAULTMASK, %0\n" : : "r" (val));
}

static inline uint32_t __attribute__((always_inline)) cm3_faultmask_get(void) {
 uint32_t val;
 asm volatile ("mrs %0, FAULTMASK\n" : "=r" (val));
 return val;
}

static inline void __attribute__((always_inline)) cm3_msp_set(uint32_t val) {
 asm volatile ("msr MSP, %0\n" : : "r" (val));
}

static inline uint32_t __attribute__((always_inline)) cm3_msp_get(void) {
 uint32_t msp;
 asm volatile ("mrs %0, MSP\n" : "=r" (msp));
 return msp;
}

static inline void __attribute__((always_inline)) cm3_psp_set(uint32_t val) {
 asm volatile ("msr PSP, %0\n" : : "r" (val));
}

static inline uint32_t __attribute__((always_inline)) cm3_psp_get(void) {
 uint32_t psp;
 asm volatile ("mrs %0, PSP\n" : "=r" (psp));
 return psp;
}

static inline uint32_t __attribute__((always_inline)) cm3_sp_get(void) {
 register uint32_t sp;
 asm volatile ("mov %0, sp\n" : "=r" (sp));
 return sp;
}

static inline void __attribute__((always_inline)) cm3_sp_set(uint32_t sp) {
 asm volatile ("mov sp, %0\n" : : "r" (sp));
}

static inline uint32_t __attribute__((always_inline)) cm3_lr_get(void) {
 register uint32_t lr;
 asm volatile ("mov %0, lr\n" : "=r" (lr));
 return lr;
}

static inline void __attribute__((always_inline)) cm3_lr_set(uint32_t lr) {
 asm volatile ("mov lr, %0\n" : : "r" (lr));
}





static inline uint32_t __attribute__((always_inline)) __clz(uint32_t val) {
 register uint32_t ret;
 asm volatile ("clz %0, %1\n" : "=r" (ret) : "r" (val));
 return ret;
}


static inline uint32_t __attribute__((always_inline)) __rbit(uint32_t val) {
 register uint32_t ret;
 asm volatile ("rbit %0, %1\n" : "=r" (ret) : "r" (val));
 return ret;
}

static inline uint8_t __attribute__((always_inline)) __ldrexb(uint8_t * addr) {
 register uint8_t ret;
 asm volatile ("ldrexb %0, [%1]" : "=r" (ret) : "r" (addr));
 return ret;
}

static inline uint32_t __attribute__((always_inline)) __ldrexh(uint16_t * addr) {
 register uint16_t ret;
 asm volatile ("ldrexh %0, [%1]" : "=r" (ret) : "r" (addr));
 return ret;
}

static inline uint32_t __attribute__((always_inline)) __ldrex(uint32_t * addr) {
 register uint32_t ret;
 asm volatile ("ldrex %0, [%1]" : "=r" (ret) : "r" (addr));
 return ret;
}

static inline uint32_t __attribute__((always_inline)) __strexb(uint8_t * addr,
                  uint8_t val) {
 register uint32_t ret;
 asm volatile ("strexb %0, %2, [%1]" : "=r" (ret) : "r" (addr), "r" (val));
 return ret;
}

static inline uint32_t __attribute__((always_inline)) __strexh(uint16_t * addr,
                  uint16_t val) {
 register uint32_t ret;
 asm volatile ("strexh %0, %2, [%1]" : "=r" (ret) : "r" (addr), "r" (val));
 return ret;
}

static inline uint32_t __attribute__((always_inline)) __strex(uint32_t * addr,
                 uint32_t val) {
 register uint32_t ret;
 asm volatile ("strex %0, %2, [%1]" : "=r" (ret) : "r" (addr), "r" (val));
 return ret;
}

static inline void __attribute__((always_inline)) __clrex(void) {
 asm volatile ("clrex" : );
}

static inline void __attribute__((always_inline)) __dsb(void) {
 asm volatile ("dsb" : );
}

static inline void __attribute__((always_inline)) __isb(void) {
 asm volatile ("isb" : );
}

static inline void __attribute__((always_inline)) __nop(void) {
 asm volatile ("nop" : );
}

static inline void __attribute__((always_inline)) __bkpt(int no) {
 asm volatile ("bkpt %0" : : "I" (no) );
}

__attribute__( ( always_inline ) )
static inline int64_t __smlal(int64_t acc, int32_t op1, int32_t op2)
{
 union {
  int32_t w32[2];
  int64_t w64;
 } llr;

 llr.w64 = acc;


 asm volatile ("smlal %0, %1, %2, %3" :
      "=r" (llr.w32[0]), "=r" (llr.w32[1]) :
      "r" (op1), "r" (op2),
      "0" (llr.w32[0]), "1" (llr.w32[1]));






 return(llr.w64);
}

__attribute__( ( always_inline ) )
static inline int64_t __smull(int32_t op1, int32_t op2)
{
 union {
  int32_t w32[2];
  int64_t w64;
 } llr;


 asm volatile ("smull %0, %1, %2, %3" :
      "=r" (llr.w32[0]), "=r" (llr.w32[1]) :
      "r" (op1), "r" (op2));






 return(llr.w64);
}
# 1342 "C:/devel/yard-ice/thinkos/sdk/include/arch/cortex-m3.h"
void cm3_udelay_calibrate(void);

void __attribute__((noreturn)) cm3_sysrst(void);

void cm3_nvic_irq_disable_all(void);
# 294 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 2

# 1 "C:/devel/yard-ice/thinkos/sdk/include/stdbool.h" 1
# 296 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 2
# 1 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stddef.h" 1 3 4
# 149 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stddef.h" 3 4

# 149 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stddef.h" 3 4
typedef int ptrdiff_t;
# 216 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stddef.h" 3 4
typedef unsigned int size_t;
# 328 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stddef.h" 3 4
typedef unsigned int wchar_t;
# 426 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stddef.h" 3 4
typedef struct {
  long long __max_align_ll __attribute__((__aligned__(__alignof__(long long))));
  long double __max_align_ld __attribute__((__aligned__(__alignof__(long double))));
# 437 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stddef.h" 3 4
} max_align_t;
# 297 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 2






# 302 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
struct thinkos_context {

 uint32_t r4;
 uint32_t r5;
 uint32_t r6;
 uint32_t r7;

 uint32_t r8;
 uint32_t r9;
 uint32_t r10;
 uint32_t r11;


 uint32_t sp;
 uint32_t ret;



 uint32_t r0;
 uint32_t r1;
 uint32_t r2;
 uint32_t r3;

 uint32_t r12;
 uint32_t lr;
 uint32_t pc;
 uint32_t xpsr;

};
# 351 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
struct thinkos_rt {




 struct thinkos_context * ctx[((32) + 2)];
# 366 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
 uint32_t cyccnt[(32) + (0) + 2];






 uint32_t critical_cnt;




 uint16_t xcpt_ipsr;
 int8_t step_id;
 int8_t res;

 uint32_t step_svc;
 uint32_t step_req;





 uint32_t cycref;


 uint32_t active;

 union {
  uint32_t wq_lst[(1 + 0 + 1 + (32) + (16) + 0 + 0 + 0 + 0 + (32) + 2 + 1 + 1 + 0 + 0 + 0 + 0 + 1)];
  struct {
   uint32_t wq_ready;






   uint32_t wq_clock;



   uint32_t wq_mutex[32];



   uint32_t wq_cond[16];
# 432 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
   uint32_t wq_join[32];



   uint32_t wq_console_wr;
   uint32_t wq_console_rd;



   uint32_t wq_paused;



   uint32_t wq_canceled;
# 466 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
   uint32_t wq_fault;

  };
 };


 uint16_t th_stat[32];
# 487 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
 struct {
  uint32_t ticks;

  uint32_t clock[32];




 };
# 521 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
 int8_t lock[32];



 int8_t irq_th[31];



 uint32_t th_alloc[1];



 uint32_t mutex_alloc[(32 + 31) / 32];



 uint32_t cond_alloc[(16 + 31) / 32];
# 559 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
 const struct thinkos_thread_inf * th_inf[(32) +
  (0) + 2];





};
# 674 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
struct thinkos_thread_opt {
 uint16_t stack_size;
 uint8_t priority;
 uint8_t id: 7;
 uint8_t paused: 1;
};


struct thinkos_thread_create_args {
 int (* task)(void *);
 void * arg;
 void * stack_ptr;
 struct thinkos_thread_opt opt;
 struct thinkos_thread_inf * inf;
};



# 1 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/memory.h" 1
# 87 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/memory.h"
struct thinkos_mem_blk {
 char tag[8];
 uint32_t off;
 uint8_t opt;
 uint8_t siz;
 uint16_t cnt;
};


struct thinkos_mem_desc {
 char tag[8];
 uint32_t base;
 uint8_t cnt;
 struct thinkos_mem_blk blk[];
};

struct thinkos_memory_map {
 const struct thinkos_mem_sct * flash;
 const struct thinkos_mem_sct * ram;
 const struct thinkos_mem_sct * periph;
 const struct thinkos_mem_sct * stack;
};
# 693 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 2







extern struct thinkos_rt thinkos_rt;

extern uint32_t * const thinkos_obj_alloc_lut[];

extern const uint16_t thinkos_wq_base_lut[];

extern const char thinkos_type_name_lut[][6];

extern const char thinkos_type_prefix_lut[];

extern const char __xcpt_name_lut[16][12];


extern const struct thinkos_thread_inf thinkos_main_inf;


extern uint32_t * const thinkos_main_stack;





void __attribute__((noreturn)) __thinkos_thread_terminate_stub(int code);

void __attribute__((noreturn)) __thinkos_thread_exit_stub(int code);

void __thinkos_thread_abort(unsigned int thread_id);
# 741 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
static inline void __attribute__((always_inline))
thinkos_bit_set(void * bmp, unsigned int bit)
{
 __bit_mem_wr(bmp, bit, 1);
}


static inline void __attribute__((always_inline))
thinkos_bit_clr(void * bmp, unsigned int bit)
{
 __bit_mem_wr(bmp, bit, 0);
}


static inline void __attribute__((always_inline)) __thinkos_defer_sched(void) {
 struct cm3_scb * scb = ((struct cm3_scb *) ((0xe000e000) + 0x0d00));

 scb->icsr = (1 << 28);
 asm volatile ("dsb\n");
}


static inline void __attribute__((always_inline)) __thinkos_preempt(void) {


 if (thinkos_rt.critical_cnt == 0)

  __thinkos_defer_sched();

}


static inline void __attribute__((always_inline)) __thinkos_defer_svc(void) {
 struct cm3_scb * scb = ((struct cm3_scb *) ((0xe000e000) + 0x0d00));

 scb->icsr = (1 << 26);
 asm volatile ("dsb\n");
}

static inline void __attribute__((always_inline)) __thinkos_ready_clr(void) {
 thinkos_rt.wq_ready = 0;



}

static inline void __attribute__((always_inline)) __thinkos_suspend(int thread) {


 __bit_mem_wr(&thinkos_rt.wq_ready, thread, 0);
# 813 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
}

static inline int __attribute__((always_inline)) __wq_idx(uint32_t * ptr) {
 return ptr - thinkos_rt.wq_lst;
}

static inline int __attribute__((always_inline))
__thinkos_wq_head(unsigned int wq) {

 return __clz(__rbit(thinkos_rt.wq_lst[wq]));
}

static inline void __attribute__((always_inline))
__thinkos_wq_insert(unsigned int wq, unsigned int th) {

 __bit_mem_wr(&thinkos_rt.wq_lst[wq], th, 1);

 thinkos_rt.th_stat[th] = wq << 1;

}


static inline void __attribute__((always_inline))
__thinkos_tmdwq_insert(unsigned int wq, unsigned int th, unsigned int ms) {

 thinkos_rt.clock[th] = thinkos_rt.ticks + ms;

 __bit_mem_wr(&thinkos_rt.wq_clock, th, 1);

 __bit_mem_wr(&thinkos_rt.wq_lst[wq], th, 1);


 thinkos_rt.th_stat[th] = (wq << 1) + 1;

}



static inline void __attribute__((always_inline))
 __thinkos_wq_clock_insert(unsigned int th, unsigned int ms) {

 thinkos_rt.clock[th] = thinkos_rt.ticks + ms;

 __bit_mem_wr(&thinkos_rt.wq_clock, th, 1);


 thinkos_rt.th_stat[th] = 1;

 }


static inline void __attribute__((always_inline))
__thinkos_wq_remove(unsigned int wq, unsigned int th) {

 __bit_mem_wr(&thinkos_rt.wq_lst[wq], th, 0);


 __bit_mem_wr(&thinkos_rt.wq_clock, th, 0);



 thinkos_rt.th_stat[th] = 0;

}

static inline void __attribute__((always_inline))
__thinkos_wakeup(unsigned int wq, unsigned int th) {

 __bit_mem_wr(&thinkos_rt.wq_ready, th, 1);

 __bit_mem_wr(&thinkos_rt.wq_lst[wq], th, 0);


 __bit_mem_wr(&thinkos_rt.wq_clock, th, 0);

 thinkos_rt.ctx[th]->r0 = 0;



 thinkos_rt.th_stat[th] = 0;

}

static inline void __attribute__((always_inline))
__thinkos_wakeup_return(unsigned int wq, unsigned int th, int ret) {

 __bit_mem_wr(&thinkos_rt.wq_ready, th, 1);

 __bit_mem_wr(&thinkos_rt.wq_lst[wq], th, 0);


 __bit_mem_wr(&thinkos_rt.wq_clock, th, 0);



 thinkos_rt.th_stat[th] = 0;


 thinkos_rt.ctx[th]->r0 = ret;
}


static inline uint32_t __attribute__((always_inline)) __thinkos_ticks(void) {
 return thinkos_rt.ticks;
}




static inline void __thinkos_thread_fault_set(unsigned int th) {
 __bit_mem_wr(&thinkos_rt.wq_fault, th, 1);

 thinkos_rt.th_stat[th] = ((
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 3 4
                         __builtin_offsetof (
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
                         struct thinkos_rt
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 3 4
                         , 
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
                         wq_fault
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 3 4
                         ) 
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
                         - 
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 3 4
                         __builtin_offsetof (
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
                         struct thinkos_rt
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 3 4
                         , 
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
                         wq_lst
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h" 3 4
                         )
# 925 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/kernel.h"
                         ) / sizeof(uint32_t)) << 1;



 __bit_mem_wr(&thinkos_rt.wq_clock, th, 0);

}


static inline void __thinkos_thread_fault_clr(unsigned int th) {
 __bit_mem_wr(&thinkos_rt.wq_fault, th, 0);
}




static inline void __thinkos_thread_pause_set(unsigned int th) {
 __bit_mem_wr(&thinkos_rt.wq_paused, th, 1);
}


static inline void __thinkos_thread_pause_clr(unsigned int th) {
 __bit_mem_wr(&thinkos_rt.wq_paused, th, 0);
}


static inline void __thinkos_thread_ctx_set(unsigned int th,
           struct thinkos_context * __ctx) {
 thinkos_rt.ctx[th] = __ctx;
}

static inline struct thinkos_context * __thinkos_thread_ctx_get(unsigned int th) {
 return thinkos_rt.ctx[th];
}


static inline const struct thinkos_thread_inf * __thinkos_thread_inf_get(unsigned int th) {
 return thinkos_rt.th_inf[th];
}


void thinkos_trace_rt(struct thinkos_rt * rt);

int thinkos_obj_type_get(unsigned int oid);

void __thinkos_bmp_init(uint32_t bmp[], int bits);

int __thinkos_bmp_alloc(uint32_t bmp[], int bits);

int __thinkos_thread_alloc(int target_id);

struct thinkos_context * __thinkos_thread_init(unsigned int thread_id,
                                               uint32_t sp,
                                               int (* task)(void *),
                                               void * arg);

_Bool __thinkos_thread_resume(unsigned int thread_id);

_Bool __thinkos_thread_pause(unsigned int thread_id);

_Bool __thinkos_thread_isalive(unsigned int thread_id);

_Bool __thinkos_thread_ispaused(unsigned int thread_id);

_Bool __thinkos_thread_isfaulty(unsigned int thread_id);

void __thinkos_thread_inf_set(unsigned int thread_id,
         const struct thinkos_thread_inf * inf);

void __thinkos_irq_reset_all(void);

void __thinkos_kill_all(void);

void __thinkos_pause_all(void);

void __thinkos_resume_all(void);

int __thinkos_thread_getnext(int th);

_Bool __thinkos_active(void);

_Bool __thinkos_suspended(void);

void __thinkos_memcpy(void * __dst, const void * __src,
       unsigned int __len);

void __thinkos_memcpy32(void * __dst, const void * __src,
      unsigned int __len);

void __thinkos_memset32(void * __dst, uint32_t __val, unsigned int __len);

unsigned int __thinkos_strlen(const char * __s, unsigned int __max);

void __thinkos_core_reset(void);

void __thinkos_system_reset(void);

void __thinkos_sched_stop(void);







void thinkos_console_init(void);

void thinkos_console_connect_set(_Bool val);

void thinkos_console_raw_mode_set(_Bool val);


int thinkos_console_rx_pipe_ptr(uint8_t ** ptr);

void thinkos_console_rx_pipe_commit(int cnt);

int thinkos_console_tx_pipe_ptr(uint8_t ** ptr);
void thinkos_console_tx_pipe_commit(int cnt);

_Bool thinkos_console_is_raw_mode(void);


void __thinkos_sem_post(uint32_t wq);

void __thinkos_ev_raise(uint32_t wq, int ev);

void __thinkos_gate_open(uint32_t wq);

void __thinkos_ev_info(unsigned int wq);


_Bool thinkos_sched_active(void);

_Bool thinkos_syscall_active(void);

_Bool thinkos_clock_active(void);

_Bool thinkos_dbgmon_active(void);

_Bool thinkos_kernel_active(void);





void __context(struct thinkos_context * __ctx, uint32_t __thread_id);
void __trace(struct thinkos_context * __ctx, uint32_t __thread_id);
void __thinkos(struct thinkos_rt * rt);
void __profile(void);





void __thinkos_exec(int thread_id, void (* func)(void *),
     void * arg, _Bool paused);

_Bool __thinkos_mem_usr_rw_chk(uint32_t addr, uint32_t size);
# 24 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 2


# 25 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
#pragma GCC optimize ("Ofast")
# 25 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"


# 1 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h" 1
# 62 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
enum thinkos_err {
 THINKOS_OK = 0,
 THINKOS_ETIMEDOUT = -1,
 THINKOS_EINTR = -2,
 THINKOS_EINVAL = -3,
 THINKOS_EAGAIN = -4,
 THINKOS_EDEADLK = -5,
 THINKOS_EPERM = -6,
 THINKOS_ENOSYS = -7,
 THINKOS_EFAULT = -8,
 THINKOS_ENOMEM = -9,
 THINKOS_EBADF = -10
};
# 102 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
enum thinkos_obj_kind {
 THINKOS_OBJ_READY = 0,
 THINKOS_OBJ_TMSHARE = 1,
 THINKOS_OBJ_CLOCK = 2,
 THINKOS_OBJ_MUTEX = 3,
 THINKOS_OBJ_COND = 4,
 THINKOS_OBJ_SEMAPHORE = 5,
 THINKOS_OBJ_EVENT = 6,
 THINKOS_OBJ_FLAG = 7,
 THINKOS_OBJ_GATE = 8,
 THINKOS_OBJ_JOIN = 9,
 THINKOS_OBJ_CONREAD = 10,
 THINKOS_OBJ_CONWRITE = 11,
 THINKOS_OBJ_PAUSED = 12,
 THINKOS_OBJ_CANCELED = 13,
 THINKOS_OBJ_COMMSEND = 14,
 THINKOS_OBJ_COMMRECV = 15,
 THINKOS_OBJ_IRQ = 16,
 THINKOS_OBJ_DMA = 17,
 THINKOS_OBJ_FLASH_MEM = 18,
 THINKOS_OBJ_FAULT = 19,
 THINKOS_OBJ_INVALID
};

enum thinkos_thread_status {
 THINKOS_THREAD_CANCELED = 0x7ffffffe,
 THINKOS_THREAD_ABORTED = 0x7fffffff
};





struct cortex_m_context {
 uint32_t r0;
 uint32_t r1;
 uint32_t r2;
 uint32_t r3;

 uint32_t r4;
 uint32_t r5;
 uint32_t r6;
 uint32_t r7;

 uint32_t r8;
 uint32_t r9;
 uint32_t r10;
 uint32_t r11;

 uint32_t r12;
 uint32_t sp;
 uint32_t lr;
 uint32_t pc;

 uint32_t xpsr;
};




struct thinkos_thread {
 uint32_t no: 6;
 uint32_t tmw: 1;
 uint32_t alloc: 1;
 uint16_t wq;
 uint8_t sched_val;
 uint8_t sched_pri;
 uint32_t clock;
 uint32_t cyccnt;
 const struct thinkos_thread_inf * inf;
};

struct thinkos_thread_inf {
 void * stack_ptr;
 union {
  uint32_t opt;
  struct {
   uint16_t stack_size;
   uint8_t priority;
   uint8_t thread_id: 7;
   uint8_t paused: 1;
  };
 };

 char tag[8];
};
# 201 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
struct thinkos_thread_attr {
 void * stack_addr;
 uint32_t stack_size: 16;
 uint32_t priority: 8;
 uint32_t thread_id: 7;
 uint32_t paused: 1;
 char tag[8];
};

typedef int (* thinkos_task_t)(void * arg, unsigned int id);
# 227 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
# 1 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/syscalls.h" 1
# 27 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/syscalls.h"
# 1 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/profile.h" 1
# 28 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/syscalls.h" 2

# 1 "C:/devel/yard-ice/thinkos/sdk/include/sys/types.h" 1
# 30 "C:/devel/yard-ice/thinkos/sdk/include/sys/types.h"
# 1 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stddef.h" 1 3 4
# 31 "C:/devel/yard-ice/thinkos/sdk/include/sys/types.h" 2

typedef long ssize_t;
typedef long clock_t;
typedef long time_t;

typedef int clockid_t;

typedef unsigned int off_t;

typedef unsigned long useconds_t;
typedef long suseconds_t;

typedef unsigned long dev_t;
typedef unsigned long ino_t;
typedef unsigned short mode_t;
typedef unsigned char nlink_t;
typedef unsigned char uid_t;
typedef unsigned char gid_t;
typedef unsigned int pid_t;
# 30 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/syscalls.h" 2
# 253 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/syscalls.h"
static inline int __attribute__((always_inline)) thinkos_thread_self(void) {
 return __extension__({ register int ret asm("r0"); asm volatile ("svc " "0" "\n" : "=r"(ret) : : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_thread_create(int (* task)(void *),
       void * arg, void * stack_ptr,
       unsigned int opt) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(task); register int r1 asm("r1") = (int)(arg); register int r2 asm("r2") = (int)(stack_ptr); register int r3 asm("r3") = (int)(opt); register int r12 asm("r12") = (int)(0); asm volatile ("svc " "1" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r12) : ); ret; })
                          ;
}

static inline int __attribute__((always_inline))
thinkos_thread_create_inf(int (* task)(void *), void * arg,
        const struct thinkos_thread_inf * inf) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(task); register int r1 asm("r1") = (int)(arg); register int r2 asm("r2") = (int)(inf->stack_ptr); register int r3 asm("r3") = (int)(inf->opt); register int r12 asm("r12") = (int)(inf); asm volatile ("svc " "1" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r12) : ); ret; })
                                      ;
}

static inline int
__attribute__((always_inline)) thinkos_cancel(unsigned int thread, int code) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(thread); register int r1 asm("r1") = (int)(code); asm volatile ("svc " "45" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int
__attribute__((always_inline)) thinkos_exit(int code) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(code); asm volatile ("svc " "46" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int
__attribute__((always_inline)) thinkos_terminate(unsigned int thread, int code) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(thread); register int r1 asm("r1") = (int)(code); asm volatile ("svc " "47" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int
__attribute__((always_inline)) thinkos_thread_abort(unsigned int thread) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(thread); register int r1 asm("r1") = (int)(THINKOS_THREAD_ABORTED); asm volatile ("svc " "47" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int
__attribute__((always_inline)) thinkos_join(unsigned int thread) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(thread); asm volatile ("svc " "42" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int
__attribute__((always_inline)) thinkos_pause(unsigned int thread) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(thread); asm volatile ("svc " "43" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int
__attribute__((always_inline)) thinkos_resume(unsigned int thread) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(thread); asm volatile ("svc " "44" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int
__attribute__((always_inline)) thinkos_sleep(unsigned int ms) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(ms); asm volatile ("svc " "4" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int
__attribute__((always_inline)) thinkos_alarm(uint32_t clk) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(clk); asm volatile ("svc " "3" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline uint32_t __attribute__((always_inline)) thinkos_clock(void) {
return __extension__({ register int ret asm("r0"); asm volatile ("svc " "2" "\n" : "=r"(ret) : : ); ret; });
}

static inline int __attribute__((always_inline)) thinkos_obj_alloc(int kind) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(kind); asm volatile ("svc " "40" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline)) thinkos_obj_free(int obj) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(obj); asm volatile ("svc " "41" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_mutex_lock(int mutex) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(mutex); asm volatile ("svc " "5" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_mutex_trylock(int mutex) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(mutex); asm volatile ("svc " "6" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_mutex_timedlock(int mutex, unsigned int ms) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(mutex); register int r1 asm("r1") = (int)(ms); asm volatile ("svc " "7" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_mutex_unlock(int mutex) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(mutex); asm volatile ("svc " "8" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}


static inline int __attribute__((always_inline))
thinkos_cond_wait(int cond, int mutex) {
 int ret;
 if ((ret = __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(cond); register int r1 asm("r1") = (int)(mutex); asm volatile ("svc " "14" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; })
                     ) >= THINKOS_ETIMEDOUT) {
  __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(mutex); asm volatile ("svc " "5" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
 }
 return ret;
}

static inline int __attribute__((always_inline))
thinkos_cond_timedwait(int cond, int mutex, unsigned int ms) {
 int ret;
 if ((ret = __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(cond); register int r1 asm("r1") = (int)(mutex); register int r2 asm("r2") = (int)(ms); asm volatile ("svc " "15" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2) : ); ret; })
                         ) >= THINKOS_ETIMEDOUT) {
  __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(mutex); asm volatile ("svc " "5" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
 }
 return ret;
}

static inline int __attribute__((always_inline))
thinkos_cond_signal(int cond) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(cond); asm volatile ("svc " "16" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_cond_broadcast(int cond) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(cond); asm volatile ("svc " "17" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}


static inline int __attribute__((always_inline))
thinkos_sem_init(int sem, unsigned int value) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(sem); register int r1 asm("r1") = (int)(value); asm volatile ("svc " "9" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline)) thinkos_sem_wait(int sem) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(sem); asm volatile ("svc " "10" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline)) thinkos_sem_trywait(int sem) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(sem); asm volatile ("svc " "11" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_sem_timedwait(int sem, unsigned int ms) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(sem); register int r1 asm("r1") = (int)(ms); asm volatile ("svc " "12" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline)) thinkos_sem_post(int sem) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(sem); asm volatile ("svc " "13" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline void __attribute__((always_inline)) thinkos_sem_post_i(int sem) {
uintptr_t * except = (uintptr_t *)(0);
void (* sem_post_i)(int) = (void (*)(int))except[7];
sem_post_i(sem);
}





static inline int __attribute__((always_inline)) thinkos_ev_wait(int set) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(set); asm volatile ("svc " "31" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline)) thinkos_ev_timedwait(
                  int set, unsigned int ms) {
return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(set); register int r1 asm("r1") = (int)(ms); asm volatile ("svc " "32" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline)) thinkos_ev_raise(
                 int set, int ev) {
return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(set); register int r1 asm("r1") = (int)(ev); asm volatile ("svc " "33" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline)) thinkos_ev_mask(
                int set, int ev, int val) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(set); register int r1 asm("r1") = (int)(ev); register int r2 asm("r2") = (int)(val); asm volatile ("svc " "34" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2) : ); ret; });
}

static inline int __attribute__((always_inline)) thinkos_ev_clear(
                 int set, int ev) {
return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(set); register int r1 asm("r1") = (int)(ev); asm volatile ("svc " "35" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline void __attribute__((always_inline)) thinkos_ev_raise_i(
                 int set, int ev) {
uintptr_t * except = (uintptr_t *)(0);
void (* ev_raise_i)(int, int) = (void (*)(int, int))except[9];
ev_raise_i(set, ev);
}





static inline int __attribute__((always_inline)) thinkos_flag_set(int flag) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(flag); asm volatile ("svc " "23" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline)) thinkos_flag_clr(int flag) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(flag); asm volatile ("svc " "22" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline)) thinkos_flag_val(int flag) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(flag); asm volatile ("svc " "21" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline)) thinkos_flag_watch(int flag) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(flag); asm volatile ("svc " "24" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_flag_timedwatch(int flag, unsigned int ms) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(flag); register int r1 asm("r1") = (int)(ms); asm volatile ("svc " "25" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline)) thinkos_flag_give(int flag) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(flag); asm volatile ("svc " "20" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline)) thinkos_flag_take(int flag) {
return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(flag); asm volatile ("svc " "18" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_flag_timedtake(int flag, unsigned int ms) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(flag); register int r1 asm("r1") = (int)(ms); asm volatile ("svc " "19" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline void __attribute__((always_inline))
thinkos_flag_give_i(int flag) {
 uintptr_t * except = (uintptr_t *)(0);
 void (* flag_give_i)(int) = (void (*)(int))except[10];
 flag_give_i(flag);
}






static inline int __attribute__((always_inline))
thinkos_gate_open(int gate) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(gate); asm volatile ("svc " "29" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_gate_close(int gate) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(gate); asm volatile ("svc " "30" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_gate_exit(int gate, unsigned int open) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(gate); register int r1 asm("r1") = (int)(open); asm volatile ("svc " "28" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_gate_wait(int gate) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(gate); asm volatile ("svc " "26" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_gate_timedwait(int gate, unsigned int ms) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(gate); register int r1 asm("r1") = (int)(ms); asm volatile ("svc " "27" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline void __attribute__((always_inline))
thinkos_gate_open_i(int gate) {
 uintptr_t * except = (uintptr_t *)(0);
 void (* __gate_open_i)(uint32_t) = (void (*)(uint32_t))except[13];
 __gate_open_i(gate);
}





static inline int __attribute__((always_inline))
 thinkos_irq_timedwait(int irq, unsigned int ms) {

 register uint32_t err asm("r0");
 register uint32_t cyccnt asm("r1");

 asm volatile ("svc " "38" "\n" :
      "=r"(err), "=r"(cyccnt) :
      "0"(irq), "1"(ms) );

 asm volatile ("svc " "50" "\n" :
      "=r"(err) : "0"(irq) );

 return err;
}

static inline int __attribute__((always_inline)) thinkos_irq_wait(int irq) {
 register uint32_t err asm("r0");
 register uint32_t cyccnt asm("r1");

 asm volatile ("svc " "37" "\n" :
      "=r"(err), "=r"(cyccnt) : "0"(irq) );

 return err;
}

static inline int __attribute__((always_inline))
 thinkos_irq_wait_cyccnt(int irq, uint32_t * pcyccnt) {

 register uint32_t err asm("r0");
 register uint32_t cyccnt asm("r1");

 asm volatile ("svc " "37" "\n" :
      "=r"(err), "=r"(cyccnt) : "0"(irq) );

 *pcyccnt = cyccnt;

 return err;
}

static inline int __attribute__((always_inline))
 thinkos_irq_register(int irq, unsigned int pri, void (* isr)(void)) {
  return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(4); register int r1 asm("r1") = (int)(irq); register int r2 asm("r2") = (int)(pri); register int r3 asm("r3") = (int)(isr); asm volatile ("svc " "39" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; })
                      ;
 }

static inline int __attribute__((always_inline))
thinkos_irq_enable(int irq) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(1); register int r1 asm("r1") = (int)(irq); asm volatile ("svc " "39" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_irq_disable(int irq) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(0); register int r1 asm("r1") = (int)(irq); asm volatile ("svc " "39" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_irq_priority_set(int irq, unsigned int pri) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(2); register int r1 asm("r1") = (int)(irq); register int r2 asm("r2") = (int)(pri); asm volatile ("svc " "39" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2) : ); ret; })
                                           ;
}
# 600 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/syscalls.h"
static inline int __attribute__((always_inline))
thinkos_console_write(const void * buf, unsigned int len) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(0); register int r1 asm("r1") = (int)(buf); register int r2 asm("r2") = (int)(len); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_console_ioctl(unsigned int ioctl, void * arg, unsigned int size) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(6); register int r1 asm("r1") = (int)(ioctl); register int r2 asm("r2") = (int)(arg); register int r3 asm("r3") = (int)(size); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_console_is_connected(void) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(7); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_console_read(void * buf, unsigned int len) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(1); register int r1 asm("r1") = (int)(buf); register int r2 asm("r2") = (int)(len); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_console_timedread(void * buf, unsigned int len, unsigned int ms) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(2); register int r1 asm("r1") = (int)(buf); register int r2 asm("r2") = (int)(len); register int r3 asm("r3") = (int)(ms); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_console_close(void) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(4); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_console_drain(void) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(5); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_console_io_break(unsigned int which) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(8); register int r1 asm("r1") = (int)(which); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_console_raw_mode(unsigned int enable) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(9); register int r1 asm("r1") = (int)(enable); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_console_rd_nonblock(unsigned int enable) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(10); register int r1 asm("r1") = (int)(enable); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_console_wr_nonblock(unsigned int enable) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(11); register int r1 asm("r1") = (int)(enable); asm volatile ("svc " "36" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}





static inline int __attribute__((always_inline))
 thinkos_clocks(uint32_t * clk[]) {
  return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(2); register int r1 asm("r1") = (int)(clk); asm volatile ("svc " "48" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
 }

static inline int __attribute__((always_inline))
 thinkos_udelay_factor(int32_t * factor) {
  return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(1); register int r1 asm("r1") = (int)(factor); asm volatile ("svc " "48" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
 }

static inline void __attribute__((always_inline, noreturn))
 thinkos_abort(void) {
  for (;;) {
  __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(0); asm volatile ("svc " "48" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
  }
 }
# 683 "C:/devel/yard-ice/thinkos/sdk/include/thinkos/syscalls.h"
static inline int __attribute__((always_inline))
 thinkos_reboot(uint32_t key) {
  return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(7); register int r1 asm("r1") = (int)(key); asm volatile ("svc " "48" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
 }

static inline int __attribute__((always_inline))
 thinkos_rt_snapshot(void * rt) {
  return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(3); register int r1 asm("r1") = (int)(rt); asm volatile ("svc " "48" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
 }

static inline int __attribute__((always_inline))
 thinkos_thread_inf(const struct thinkos_thread_inf * inf[],
        unsigned int max) {
  return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(5); register int r1 asm("r1") = (int)(inf); register int r2 asm("r2") = (int)(max); asm volatile ("svc " "48" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2) : ); ret; });
 }

static inline int __attribute__((always_inline))
 thinkos_thread_cyccnt(uint32_t cyccnt[], unsigned int max) {
  return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(6); register int r1 asm("r1") = (int)(cyccnt); register int r2 asm("r2") = (int)(max); asm volatile ("svc " "48" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2) : ); ret; });
 }

static inline uint32_t __attribute__((always_inline)) thinkos_cyccnt(void) {
  return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(8); asm volatile ("svc " "48" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
 }

struct dbgmon_comm;

static inline int __attribute__((always_inline))
 thinkos_dbgmon(void (* task)(const struct dbgmon_comm *, void *),
       const struct dbgmon_comm * comm, void * param) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(task); register int r1 asm("r1") = (int)(comm); register int r2 asm("r2") = (int)(param); asm volatile ("svc " "54" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2) : ); ret; });
}

static inline int __attribute__((always_inline))
 thinkos_critical_enter(void) {
  return __extension__({ register int ret asm("r0"); asm volatile ("svc " "52" "\n" : "=r"(ret) : : ); ret; });
 }

static inline int __attribute__((always_inline))
 thinkos_critical_exit(void) {
  return __extension__({ register int ret asm("r0"); asm volatile ("svc " "53" "\n" : "=r"(ret) : : ); ret; });
 }





static inline int __attribute__((always_inline))
thinkos_comm_send(uint32_t hdr, const void * buf, unsigned int len) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(0); register int r1 asm("r1") = (int)(hdr); register int r2 asm("r2") = (int)(buf); register int r3 asm("r3") = (int)(len); asm volatile ("svc " "49" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; });
}

static inline int __attribute__((always_inline))
thinkos_comm_recv(uint32_t * hdr, void * buf, unsigned int len) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(1); register int r1 asm("r1") = (int)(hdr); register int r2 asm("r2") = (int)(buf); register int r3 asm("r3") = (int)(len); asm volatile ("svc " "49" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; });
}





struct trace_ref;
struct trace_entry;

static inline int __attribute__((always_inline))
thinkos_trace(const struct trace_ref * ref) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(ref); asm volatile ("svc " "55" "\n" : "=r"(ret) : "0"(r0) : ); ret; } );
}

static inline int __attribute__((always_inline))
thinkos_trace_open(void) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(1); asm volatile ("svc " "56" "\n" : "=r"(ret) : "0"(r0) : ); ret; } )
                           ;
}

static inline int __attribute__((always_inline))
thinkos_trace_close(int id) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(0); register int r1 asm("r1") = (int)(id); asm volatile ("svc " "56" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; })
                                ;
}

static inline int __attribute__((always_inline))
thinkos_trace_read(int id, uint32_t * buf, unsigned int len) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(2); register int r1 asm("r1") = (int)(id); register int r2 asm("r2") = (int)(buf); register int r3 asm("r3") = (int)(len); asm volatile ("svc " "56" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; })
                                         ;
}

static inline int __attribute__((always_inline))
thinkos_trace_flush(int id) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(3); register int r1 asm("r1") = (int)(id); asm volatile ("svc " "56" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; })
                                ;
}

static inline int __attribute__((always_inline))
thinkos_trace_getfirst(int id, struct trace_entry * entry) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(4); register int r1 asm("r1") = (int)(id); register int r2 asm("r2") = (int)(entry); asm volatile ("svc " "56" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2) : ); ret; })
                                          ;
}

static inline int __attribute__((always_inline))
thinkos_trace_getnext(int id, struct trace_entry * entry) {
 return __extension__({ register int ret asm("r0"); register int r0 asm("r0") = (int)(5); register int r1 asm("r1") = (int)(id); register int r2 asm("r2") = (int)(entry); asm volatile ("svc " "56" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2) : ); ret; })
                                         ;
}





static inline int __attribute__((always_inline))
thinkos_flash_mem_close(int mem) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(0); register int r1 asm("r1") = (int)(mem); register int r2 asm("r2") = (int)(0); register int r3 asm("r3") = (int)(0); asm volatile ("svc " "57" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; })
            ;
}

static inline int __attribute__((always_inline))
thinkos_flash_mem_open(const char * tag) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(1); register int r1 asm("r1") = (int)(tag); register int r2 asm("r2") = (int)(0); register int r3 asm("r3") = (int)(0); asm volatile ("svc " "57" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; })
                 ;
}

static inline int __attribute__((always_inline))
thinkos_flash_mem_read(int mem, void * buf, size_t len) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(2); register int r1 asm("r1") = (int)(mem); register int r2 asm("r2") = (int)(buf); register int r3 asm("r3") = (int)(len); asm volatile ("svc " "57" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; })
                     ;
}

static inline int __attribute__((always_inline))
thinkos_flash_mem_write(int mem, const void * buf, size_t len) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(3); register int r1 asm("r1") = (int)(mem); register int r2 asm("r2") = (int)(buf); register int r3 asm("r3") = (int)(len); asm volatile ("svc " "57" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; })
                     ;
}

static inline int __attribute__((always_inline))
thinkos_flash_mem_seek(int mem, off_t offset) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(4); register int r1 asm("r1") = (int)(mem); register int r2 asm("r2") = (int)(offset); register int r3 asm("r3") = (int)(0); asm volatile ("svc " "57" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; })
                      ;
}

static inline int __attribute__((always_inline))
thinkos_flash_mem_erase(int mem, off_t offset, size_t len) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(5); register int r1 asm("r1") = (int)(mem); register int r2 asm("r2") = (int)(offset); register int r3 asm("r3") = (int)(len); asm volatile ("svc " "57" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; })
                   ;
}

static inline int __attribute__((always_inline))
thinkos_flash_mem_lock(int mem, off_t offset, size_t len) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(6); register int r1 asm("r1") = (int)(mem); register int r2 asm("r2") = (int)(offset); register int r3 asm("r3") = (int)(len); asm volatile ("svc " "57" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; })
                   ;
}

static inline int __attribute__((always_inline))
thinkos_flash_mem_unlock(int mem, off_t offset, size_t len) {
 return __extension__({register int ret asm("r0"); register int r0 asm("r0") = (int)(7); register int r1 asm("r1") = (int)(mem); register int r2 asm("r2") = (int)(offset); register int r3 asm("r3") = (int)(len); asm volatile ("svc " "57" "\n" : "=r"(ret) : "0"(r0), "r"(r1), "r"(r2), "r"(r3) : ); ret; })
                   ;
}





static inline void thinkos_yield(void) {
 ((struct cm3_scb *) ((0xe000e000) + 0x0d00))->icsr = (1 << 28);
 asm volatile ("dsb\n");
}

static inline void thinkos_bkpt(int no) {
 asm volatile ("bkpt %0" : : "I" (no) );
}

static inline int thinkos_escalate(int (* call)(void *), void * arg) {
 return __extension__( { register int ret asm("r0"); register int r0 asm("r0") = (int)(call); register int r1 asm("r1") = (int)(arg); asm volatile ("svc " "51" "\n" : "=r"(ret) : "0"(r0), "r"(r1) : ); ret; });
}
# 228 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h" 2
# 247 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_krn_init(unsigned int opt, const struct thinkos_memory_map * map,
      const struct thinkos_thread_attr * lst[]);






int thinkos_nrt_init(void);







void thinkos_krn_mpu_init(uint32_t krn_offs, unsigned int krn_size);




void thinkos_userland(void);






int thinkos_obj_alloc(int kind);




int thinkos_obj_free(int obj);
# 295 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_thread_create(int (* task_ptr)(void *),
        void * task_arg, void * stack_ptr,
        unsigned int opt);
# 306 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_thread_create_inf(int (* task_ptr)(void *), void * task_arg,
         const struct thinkos_thread_inf * inf);





int thinkos_thread_self(void);







int thinkos_cancel(unsigned int thread_id, int code);






int thinkos_exit(int code);
# 339 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_join(unsigned int thread_id);






int thinkos_pause(unsigned int thread_id);






int thinkos_resume(unsigned int thread_id);






void thinkos_yield(void);
# 381 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_sleep(unsigned int ms);
# 391 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
uint32_t thinkos_clock(void);
# 401 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_alarm(uint32_t clock);
# 415 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_mutex_alloc(void);






int thinkos_mutex_free(int mutex);






int thinkos_mutex_lock(int mutex);






int thinkos_mutex_trylock(int mutex);







int thinkos_mutex_timedlock(int mutex, unsigned int ms);






int thinkos_mutex_unlock(int mutex);
# 465 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_cond_alloc(void);






int thinkos_cond_free(int cond);







int thinkos_cond_wait(int cond, int mutex);
# 489 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_cond_timedwait(int cond, int mutex, unsigned int ms);






int thinkos_cond_signal(int cond);






int thinkos_cond_broadcast(int cond);
# 518 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_sem_alloc(unsigned int val);






int thinkos_sem_free(int sem);







int thinkos_sem_init(int sem, unsigned int val);






int thinkos_sem_wait(int sem);







int thinkos_sem_timedwait(int sem, unsigned int ms);






int thinkos_sem_post(int sem);





void thinkos_sem_post_i(int sem);
# 575 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_ev_alloc(void);






int thinkos_ev_free(int set);






int thinkos_ev_wait(int set);







int thinkos_ev_timedwait(int set, unsigned int ms);







int thinkos_ev_raise(int set, int ev);






void thinkos_ev_raise_i(int set, int ev);
# 621 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_ev_mask(int set, int ev, int val);







int thinkos_ev_clear(int set, int ev);
# 643 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_flag_alloc(void);






int thinkos_flag_free(int flag);






int thinkos_flag_val(int flag);






int thinkos_flag_set(int flag);






int thinkos_flag_clr(int flag);






int thinkos_flag_give(int flag);





void thinkos_flag_give_i(int flag);





int thinkos_flag_take(int flag);







int thinkos_flag_timedtake(int flag, unsigned int ms);
# 728 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_gate_alloc(void);
# 741 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_gate_free(int gate);
# 751 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_gate_wait(int gate);
# 767 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_gate_timedwait(int gate, unsigned int ms);
# 785 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_gate_open(int gate);
# 796 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
void thinkos_gate_open_i(int gate);
# 805 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_gate_close(int gate);
# 817 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_gate_exit(int gate, unsigned int open);
# 832 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_irq_wait(int irq);
# 843 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_irq_timedwait(int irq, unsigned int ms);
# 852 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_irq_register(int irq, unsigned int pri, void (* isr)(void));
# 869 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_clocks(uint32_t * clk[]);






int thinkos_udelay_factor(int32_t * factor);





void __attribute__((noreturn)) thinkos_abort(void);

int thinkos_critical_enter(void);

int thinkos_critical_exit(void);

int thinkos_escalate(int (* call)(void *), void * arg);

int thinkos_thread_abort(unsigned int thread);
# 904 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
struct trace_entry;
struct trace_ref;

int thinkos_trace(const struct trace_ref * ref);

int thinkos_trace_open(void);

int thinkos_trace_close(int id);

int thinkos_trace_read(int id, uint32_t * buf, unsigned int len);

int thinkos_trace_flush(int id);

int thinkos_trace_getfirst(int id, struct trace_entry * entry);

int thinkos_trace_getnext(int id, struct trace_entry * entry);
# 929 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_console_write(const void * buf, unsigned int len);







int thinkos_console_read(void * buf, unsigned int len);
# 947 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_console_timedread(void * buf, unsigned int len, unsigned int ms);

int thinkos_console_is_connected(void);

int thinkos_console_close(void);

int thinkos_console_drain(void);

int thinkos_console_io_break(unsigned int which);

int thinkos_console_raw_mode(unsigned int enable);

int thinkos_console_rd_nonblock(unsigned int enable);

int thinkos_console_wr_nonblock(unsigned int enable);
# 971 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_flash_mem_close(int mem);






int thinkos_flash_mem_open(const char * tag);
# 988 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_flash_mem_read(int mem, void * buf, size_t len);
# 998 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_flash_mem_write(int mem, const void * buf, size_t len);
# 1007 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_flash_mem_seek(int mem, off_t offset);
# 1017 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_flash_mem_erase(int mem, off_t offset, size_t len);
# 1027 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_flash_mem_lock(int mem, off_t offset, size_t len);
# 1037 "C:/devel/yard-ice/thinkos/sdk/include/thinkos.h"
int thinkos_flash_mem_unlock(int mem, off_t offset, size_t len);
# 28 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 2
# 1 "C:/devel/yard-ice/thinkos/sdk/include/sys/dcclog.h" 1
# 29 "C:/devel/yard-ice/thinkos/sdk/include/sys/dcclog.h"
# 1 "C:/devel/yard-ice/thinkos/sdk/include/stdio.h" 1
# 30 "C:/devel/yard-ice/thinkos/sdk/include/stdio.h"
# 1 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stddef.h" 1 3 4
# 31 "C:/devel/yard-ice/thinkos/sdk/include/stdio.h" 2
# 1 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stdarg.h" 1 3 4
# 40 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stdarg.h" 3 4

# 40 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 99 "c:\\program files (x86)\\gnu tools arm embedded\\8 2018-q4-major\\lib\\gcc\\arm-none-eabi\\8.2.1\\include\\stdarg.h" 3 4
typedef __gnuc_va_list va_list;
# 32 "C:/devel/yard-ice/thinkos/sdk/include/stdio.h" 2


# 33 "C:/devel/yard-ice/thinkos/sdk/include/stdio.h"
struct file;

typedef struct file FILE;

extern FILE * stdin;
extern FILE * stdout;
extern FILE * stderr;
# 54 "C:/devel/yard-ice/thinkos/sdk/include/stdio.h"
int fputc(int c, FILE * f);

int putchar(int c);

int getchar(void);

int puts(const char * s);

int vfprintf(FILE * f, const char * fmt, va_list ap);

int fprintf(FILE * f, const char * fmt, ...);

int printf(const char * fmt, ...);

int vsnprintf(char * str, size_t size, const char * fmt, va_list ap);

int snprintf(char * str, size_t size, const char *fmt, ...);

int sprintf(char * str, const char * fmt, ...);

FILE * fopen(const char * path, const char * mode);

int fflush(FILE * f);

int feof(FILE * f);

int fclose(FILE * f);

size_t fwrite(const void * ptr, size_t size, size_t nmemb, FILE * f);

size_t fread(void * ptr, size_t size, size_t nmemb, FILE * f);

char * fgets(char * s, int size, FILE * f);

int fgetc(FILE * f);

int fputs(const char * s, FILE * f);

int scanf(const char * fmt, ...);

int fscanf(FILE * f, const char * fmt, ...);

int sscanf(const char * str, const char * fmt, ...);

int vscanf(const char * fmt, va_list ap);

int vsscanf(const char * str, const char * fmt, va_list ap);

int vfscanf(FILE * f, const char * fmt, va_list ap);




int ftmgetc(FILE * f, unsigned int tmo);
# 30 "C:/devel/yard-ice/thinkos/sdk/include/sys/dcclog.h" 2
# 70 "C:/devel/yard-ice/thinkos/sdk/include/sys/dcclog.h"
struct dcc_trace_entry {
 const char * file;
 unsigned short line;
 unsigned char level;
 unsigned char opt;
 const char * function;
 const char * msg;
};

enum log_level {
 LOG_NONE = 0,
 LOG_PANIC = 1,
 LOG_EXCEPT = 2,
 LOG_ERROR = 3,
 LOG_WARNING = 4,
 LOG_TRACE = 5,
 LOG_INFO = 6,
 LOG_MSG = 7,
 LOG_YAP = 8,
 LOG_JABBER = 9
};

enum {
 LOG_OPT_NONE = 0,
 LOG_OPT_STR = 1,
 LOG_OPT_XXD = 2
};
# 293 "C:/devel/yard-ice/thinkos/sdk/include/sys/dcclog.h"
void ice_trace_init(void);

void ice_comm_connect();

FILE * ice_comm_fopen(void);

void ice_trace0(const struct dcc_trace_entry * __entry);

void ice_trace1(const struct dcc_trace_entry * __entry, int __a);

void ice_trace2(const struct dcc_trace_entry * __entry, int __a, int __b);

void ice_trace3(const struct dcc_trace_entry * __entry, int __a, int __b, int __c);

void ice_trace4(const struct dcc_trace_entry * __entry, int __a, int __b,
    int __c, int __d);

void ice_trace5(const struct dcc_trace_entry * __entry, int __a, int __b,
    int __c, int __d, int e);

void ice_trace6(const struct dcc_trace_entry * __entry, int __a, int __b,
    int __c, int __d, int __e, int __f);

void ice_trace7(const struct dcc_trace_entry * __entry, int __a, int __b,
    int __c, int __d, int __e, int __f, int __g);

void ice_trace8(const struct dcc_trace_entry * __entry, int __a, int __b,
    int __c, int __d, int __e, int __f, int __g, int __h);

void ice_trace9(const struct dcc_trace_entry * __entry, int __a, int __b,
    int __c, int __d, int __e, int __f, int __g, int __h,
    int __i);

void ice_trace10(const struct dcc_trace_entry * __entry, int __a, int __b,
    int __c, int __d, int __e, int __f, int __g, int __h,
    int __i, int __j);

void ice_tracestr(const struct dcc_trace_entry * __entry, const char * __s);

void ice_tracebin(const struct dcc_trace_entry * __entry,
      const void * __ptr, unsigned int __len);
# 29 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 2



void thinkos_mutex_lock_svc(int32_t * arg, int self)
{
 unsigned int wq = arg[0];
 unsigned int mutex = wq - ((
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          __builtin_offsetof (
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          struct thinkos_rt
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          , 
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          wq_mutex
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          ) 
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          - 
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          __builtin_offsetof (
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          struct thinkos_rt
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          , 
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          wq_lst
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          )
# 35 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          ) / sizeof(uint32_t));


 if (mutex >= 32) {
  ;
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (8))));
  arg[0] = THINKOS_EINVAL;
  return;
 }

 if (__bit_mem_rd(thinkos_rt.mutex_alloc, mutex) == 0) {
  ;
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (9))));
  arg[0] = THINKOS_EINVAL;
  return;
 }



 if (thinkos_rt.lock[mutex] == -1) {
  thinkos_rt.lock[mutex] = self;
  ;
  arg[0] = 0;
  return;
 }



 if (thinkos_rt.lock[mutex] == self) {
  ;
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (11))));
  arg[0] = THINKOS_EDEADLK;
  return;
 }


 __thinkos_wq_insert(wq, self);
 ;

 arg[0] = 0;


 __thinkos_suspend(self);

 __thinkos_defer_sched();
}

void thinkos_mutex_trylock_svc(int32_t * arg, int self)
{
 unsigned int wq = arg[0];
 unsigned int mutex = wq - ((
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          __builtin_offsetof (
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          struct thinkos_rt
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          , 
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          wq_mutex
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          ) 
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          - 
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          __builtin_offsetof (
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          struct thinkos_rt
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          , 
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          wq_lst
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          )
# 85 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          ) / sizeof(uint32_t));


 if (mutex >= 32) {
  ;
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (8))));
  arg[0] = THINKOS_EINVAL;
  return;
 }

 if (__bit_mem_rd(thinkos_rt.mutex_alloc, mutex) == 0) {
  ;
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (9))));
  arg[0] = THINKOS_EINVAL;
  return;
 }



 if (thinkos_rt.lock[mutex] == -1) {
  ;
  thinkos_rt.lock[mutex] = self;
  arg[0] = 0;
 } else {

  if (thinkos_rt.lock[mutex] == self) {
   ;
   asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (11))));
   arg[0] = THINKOS_EDEADLK;
  } else

  {
   ;
   arg[0] = THINKOS_EAGAIN;
  }
 }
}


void thinkos_mutex_timedlock_svc(int32_t * arg, int self)
{
 unsigned int wq = arg[0];
 uint32_t ms = (uint32_t)arg[1];
 unsigned int mutex = wq - ((
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          __builtin_offsetof (
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          struct thinkos_rt
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          , 
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          wq_mutex
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          ) 
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          - 
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          __builtin_offsetof (
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          struct thinkos_rt
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          , 
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          wq_lst
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          )
# 128 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          ) / sizeof(uint32_t));


 if (mutex >= 32) {
  ;
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (8))));
  arg[0] = THINKOS_EINVAL;
  return;
 }

 if (__bit_mem_rd(&thinkos_rt.mutex_alloc, mutex) == 0) {
  ;
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (9))));
  arg[0] = THINKOS_EINVAL;
  return;
 }



 if (thinkos_rt.lock[mutex] == -1) {
  thinkos_rt.lock[mutex] = self;
  ;
  arg[0] = 0;
  return;
 }



 if (thinkos_rt.lock[mutex] == self) {
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (11))));
  arg[0] = THINKOS_EDEADLK;
  return;
 }



 __thinkos_tmdwq_insert(wq, self, ms);
 ;



 arg[0] = THINKOS_ETIMEDOUT;


 __thinkos_suspend(self);

 __thinkos_defer_sched();

}


void thinkos_mutex_unlock_svc(int32_t * arg, int self)
{
 unsigned int wq = arg[0];
 unsigned int mutex = wq - ((
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          __builtin_offsetof (
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          struct thinkos_rt
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          , 
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          wq_mutex
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          ) 
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          - 
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          __builtin_offsetof (
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          struct thinkos_rt
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          , 
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          wq_lst
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c" 3 4
                          )
# 182 "c:/devel/yard-ice/thinkos/sdk/libthinkos-krn/thinkos_mutex.c"
                          ) / sizeof(uint32_t));
 int th;


 if (mutex >= 32) {
  ;
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (8))));
  arg[0] = THINKOS_EINVAL;
  return;
 }

 if (__bit_mem_rd(thinkos_rt.mutex_alloc, mutex) == 0) {
  ;
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (9))));
  arg[0] = THINKOS_EINVAL;
  return;
 }






 if (thinkos_rt.lock[mutex] != self) {
 
                                                   ;
  asm volatile ("nop\n" "bkpt %0\n" : : "I" (((128) + (10))));
  arg[0] = THINKOS_EPERM;
  return;
 }


 arg[0] = 0;

 ;

 if ((th = __thinkos_wq_head(wq)) == (32)) {


  ;
  thinkos_rt.lock[mutex] = -1;
 } else {

  thinkos_rt.lock[mutex] = th;
  ;

  __thinkos_wakeup(wq, th);

  __thinkos_defer_sched();
 }
}
