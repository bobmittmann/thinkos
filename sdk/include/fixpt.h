
#ifndef __FIXPT_H__
#define __FIXPT_H__

#include <stdint.h>

#define Q15_MAX 32767
#define Q15_MIN -32768

/* Conversion form float to fixed point Q1.15 */
#define Q15(F) ((int32_t)((float)(F) * (float)32768.0))

/* Convert from fractional Q1.15 to float point */
#define Q15F(Q) ((float)((float)(Q) / (float)32768.0))

/* Q15 Multiply */
#define Q15_MUL(X1, X2) (((int32_t)(X1) * (int32_t)(X2) + (1 << 14)) >> 15)
//#define Q15_MUL(X1, X2) (((int32_t)(X1) * (int32_t)(X2)) >> 15)

#define Q15_UMUL(X1, X2) ((((uint32_t)(X1) * (uint32_t)(X2)) + (1 << 14)) >> 15)

/* Q15 Divide */
#define Q15_DIV(X, Y) (((X) * 32768) / (Y))

/* Q15 Saturation */
#define Q15_SAT(X) ((X) < Q15_MIN) ? Q15_MIN : (((X) > Q15_MAX) ? Q15_MAX: (X))



/* Conversion form float to fixed point Q1.15 */
#define Q24(F) ((int32_t)((F) * (1 << 24)))

/* Convert from fractional Q8.24 to float point */
#define Q24F(Q) ((float)(((float)(Q)) / (1.0 * (1 << 24))))

/* Q24 Multiply */
#define Q24_MUL(X1, X2) (((int64_t)(X1) * (int64_t)(X2) + (1 << 23)) >> 24)

/* Q24 Divide */
#define Q24_DIV(X, Y) (((int64_t)(X) << 24) / (Y))



/* Conversion form float to fixed point Q16.16 */
#define Q16(F) ((int32_t)((float)(F) * (float)65536.0))

/* Convert from fractional Q16.16 to float point */
#define Q16F(Q) ((float)(((float)(Q)) / (1.0 * (1 << 16))))

/* Q16.16 Multiply */
#define Q16_MUL(X1, X2) (((int64_t)(X1) * (int64_t)(X2) + (1 << 15)) >> 16)

/* Q16.16 Divide */
#define Q16_DIV(X, Y) (((int64_t)(X) << 16) / (Y))

/* Q16.16 Floor */
#define Q16_FLOOR(X) ((int32_t)((X) % (1 << 16)))

/* Q16.16 Int */
#define Q16_INT(X) ((int32_t)((X) >> 16))

#define Q31_MAX ((uint32_t)(1 << 31) - 1)
#define Q31_MIN ((int32_t)-1)

/* Q31 Saturation */
#define Q31_SAT(X) ((int64_t)(X) < (int64_t)Q31_MIN) ? Q31_MIN : \
	(((int64_t)(X) > (int64_t)Q31_MAX) ? Q31_MAX: (X))

/* Conversion form float to fixed point Q1.31 */
#define Q31(F) Q31_SAT((int64_t)((double)(F) * (double)(1LL << 31)))

//#define Q31(F) ((int64_t)(double)(F) * (double)(1LL << 31))

/* Convert from fractional Q1.31 to float point */
#define Q31F(Q) ((double)((double)(Q) * (1.0 / (double)(1LL << 31))))

/* Q31 Signed Multiply */
#define Q31_MUL(X1, X2) (((int64_t)(X1) * (int32_t)(X2) + (1 << 30)) >> 31)

/* Q31 Unsigned Multiply */
#define Q31_UMUL(X1, X2) ((((uint64_t)(X1) * (uint32_t)(X2)) + (1 << 30)) >> 31)

/* Q31 Divide */
#define Q31_DIV(X, Y) (((int64_t)(X) << 31) / (Y))

/* FLoor(log2(n)) for 32 bits 
   Use this macro only with constant values as the generated
   code can be very inneficient. The function ilog2() is better suited for 
   general usage.
 */
#ifndef ILOG2
  #define ILOG2(X) (((X)&(0xffff0000))?(((X)&(0xff000000))?\
  (((X)&(0xf0000000))?(((X)&(0xc0000000))?((X)&(0x80000000)?31:30):\
   ((X)&(0x20000000)?29:28)):(((X)&(0x0c000000))?((X)&(0x08000000)?27:26):\
   ((X)&(0x02000000)?25:24))):(((X)&(0x00f00000))?(((X)&(0x00c00000))?\
   ((X)&(0x00800000)?23:22):((X)&(0x00200000)?21:20)):\
  (((X)&(0x000c0000))?((X)&(0x00080000)?19:18):((X)&(0x00020000)?17:16)))):\
  (((X)&(0x0000ff00))?(((X)&(0x0000f000))?(((X)&(0x0000c000))?\
   ((X)&(0x00008000)?15:14):((X)&(0x00002000)?13:12)):\
  (((X)&(0x00000c00))?((X)&(0x00000800)?11:10):((X)&(0x00000200)?9:8))):\
  (((X)&(0x000000f0))?(((X)&(0x000000c0))?((X)&(0x00000080)?7:6):\
   ((X)&(0x00000020)?5:4)):(((X)&(0x0000000c))?((X)&(0x00000008)?3:2):\
   ((X)&(0x00000002)?1:0)))))
#endif

typedef struct {
	int16_t re;  
	int16_t im;  
} cplx16_t; 

#define Q15_ABS

int32_t ilog2(int32_t v);
int32_t isqrt(uint32_t x);

static inline uint16_t cplx16_abs(cplx16_t z) {
	uint32_t x;
	uint32_t y;

	x = z.re * z.re;
	y = z.im * z.im;

	return isqrt(x + y);
}

extern const uint16_t q15_db2amp_lut[];
extern const int8_t q15_db2amp_min;

extern const uint16_t q15_db2pwr_lut[];
extern const int8_t q15_db2pwr_min;

const uint16_t q15_db2amp(int amp);

const uint16_t q15_db2pwr(int pwr);

/* Normalized fixed point sine:
 x = Q31(0.0) .. (1.0) -> 0 .. pi */
int32_t q31sin(int32_t x);

#endif // __FIXPT_H__

