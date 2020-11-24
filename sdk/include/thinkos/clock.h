#ifndef __THINKOS_CLOCK_H__
#define __THINKOS_CLOCK_H__

#ifndef __THINKOS_CLOCK__
#error "Never use <thinkos/clock.h> directly; include <thinkos/kernel.h> instead."
#endif 

#define __THINKOS_TIME__
#include <thinkos/time.h>

/* Convert from float point to clock interval */
#define FLOAT_CLK(X)     ((int64_t)((X) * 4294967296.))
/* Convert from clock interval to float point */
#define CLK_FLOAT(X)     ((float)(int64_t)(X) / 4294967296.)
/* Convert from clock interval to double float point */
#define CLK_DOUBLE(X)    ((double)(int64_t)(X) / 4294967296.)
/* convert from clock interval to microsseconds */
#define CLK_US(X)        (((int64_t)(X) * 1000000LL) / 4294967296LL)
/* convert from clock interval to milliseconds */
#define CLK_MS(X)        (((int64_t)(X) * 1000LL) / 4294967296LL)
/* convert from clock interval to seconds */
#define CLK_SEC(X)       ((int32_t)((int64_t)(X) / 4294967296LL))
/* Q31.32 multiplication */
#define CLK_MUL(X1, X2)   (((int64_t)(X1) * (int64_t)(X2)) >> 32)
/* Q31.32 division */
#define CLK_DIV(NUM, DEN) (((int64_t)(NUM) << 32) / (int32_t)(DEN))


/* Convert from fixed point Q1.31 to clock interval */
#define Q31_CLK(X)       ((int64_t)((X) * 2))
/* Convert from clock interval to fixed point Q1.31 */
#define CLK_Q31(X)       ((int32_t)(((X) + 1) / 2))
/* Convert from fixed point Q1.31 to float point */
#define Q31_FLOAT(X)     ((float)(X) / 2147483648.)
/* Convert from float point to fixed point Q1.31 */
#define FLOAT_Q31(X)     ((int32_t)((X) * 2147483648.))
/* Q1.31 multiplication */
#define Q31_MUL(X1, X2)   ((int32_t)(((int64_t)(X1) * (int64_t)(X2)) >> 31))
/* Q1.31 division */
#define Q31_DIV(NUM, DEN) ((int32_t)(((int64_t)(NUM) << 31) / (int32_t)(DEN)))


#ifndef __ASSEMBLER__

struct krn_clock {
	uint64_t timestamp;
	uint32_t increment;
	int64_t realtime_offs;
	uint32_t resolution;
};

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_CLOCK_H__ */

