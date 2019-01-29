
#ifndef __FIXPT_H__
#define __FIXPT_H__

#include <stdint.h>

#define Q15_MAX 32767
#define Q15_MIN -32768

/* Conversion form float to fixed point Q1.15 */
#define Q15(F) ((int32_t)((float)(F) * (float)32768.0))

/* Convert from fractional Q1.15 to float point with round */
#define Q15F(Q) ((float)(((float)(Q) + (float)(0.5/32768.0)) / (float)32768.0))

/* Q15 Signed Multiply with rounding. Ties are rouded up. */
#define Q15_MUL(X1, X2) (((int32_t)(X1) * (int32_t)(X2) + (1 << 14)) >> 15)
/* Q15 Signed Multiply non rounding (faster) */
#define Q15_MUL_FLOOR(X1, X2) (((int32_t)(X1) * (int32_t)(X2)) >> 15)
/* Q15 Unsigned Multiply with rounding */
#define Q15_UMUL(X1, X2) ((((uint32_t)(X1) * (uint32_t)(X2)) + (1 << 14)) >> 15)
/* Q15 Extended Signed Multiply with rounding. Ties are rouded up. */
#define Q15X_MUL(X1, X2) (((int64_t)(X1) * (int32_t)(X2) + (1 << 14)) >> 15)

/* Q15 Divide */
#define Q15_DIV(X, Y) (((X) * 32768) / (Y))

/* Q15 Saturation */
#define Q15_SAT(X) ((X) < Q15_MIN) ? Q15_MIN : (((X) > Q15_MAX) ? Q15_MAX: (X))

/* Conversion form float to fixed point Q1.15 with saturation */
#define Q15S(F) Q15_SAT(Q15(F))

/* Conversion form float to fixed point Q7.24 */
#define Q24(F) ((int32_t)((F) * 16777216))

/* Convert from fractional Q8.24 to float point */
#define Q24F(Q) ((float)(((float)(Q)) / (float)16777216.0))

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

/* -------------------------------------------------------------------------
 Q8.23 format: -256.0 ... 255.9999999
 */
 
#define Q23_MAX ((int32_t)2147483647)
#define Q23_MIN ((int32_t)-2147483648)


#define Q23_ONE ((int32_t)(1 << 23))

/* Q23 Saturation */
#define Q23_SAT(X) (((int64_t)(X) < (int64_t)Q23_MIN) ? Q23_MIN : \
	(((int64_t)(X) > (int64_t)Q23_MAX) ? Q23_MAX: (X)))

/* Saturate to -1..0.9999 instead of the min..max (-256..255.99999) */
#define Q23_SAT_ONE(X) \\
	(((int32_t)(X) < -(int32_t)(1 << 23)) ? -(int32_t)(1 << 23): \
	(((int32_t)(X) > ((int32_t)(1 << 23) - 1)) ? ((int32_t)(1 << 23) - 1): (X)))

/* Conversion form float to fixed point Q9.23 */
#define Q23(F)           ((int64_t)((float)(F) * (float)8388608.0))

/* Convert from fractional Q9.23 to float point */
#define Q23F(Q) ((float)((float)(Q) * (float)(1.0/8388608.0)))

/* Convert from fixed point Q9.23 to float point */
#define Q23_FLOAT(X)     ((float)(X) / (1 << 23))

/* Conversion form float to fixed point Q9.23 with saturation */
#define Q23S(F) Q23_SAT(Q23(F))

/* Q23 Signed Multiply */
#define Q23_MUL(X1, X2)  ((int64_t)((int64_t)(X1) * (int32_t)(X2) + \
									(1 << 22)) >> 23)

/* Q23 Unsigned Multiply */
#define Q23_UMUL(X1, X2) ((uint64_t)(((uint64_t)(X1) * (uint32_t)(X2)) \
									 + (1 << 22)) >> 23)

/* Q23 Divide */
#define Q23_DIV(X, Y)    (((int64_t)(X) << 23) / (int32_t)(Y))
/* Q23 Unsigned Divide */
#define Q23_UDIV(X, Y)   (((uint64_t)(X) << 23) / (uint32_t)(Y))

/* Q23 to Q15 conversion */
#define Q23to15(X)       (X>>8)

/* Q23 to Q15 conversion */
#define Q15to23(X)       (X<<8)

/* -------------------------------------------------------------------------
 Q2.30 format: -2.0 ... 1.9999999
 */
 
#define Q30_MAX ((int32_t)2147483647)
#define Q30_MIN ((int32_t)-2147483648)

/* Q30 Saturation */
#define Q30_SAT(X) (((int64_t)(X) < (int64_t)Q30_MIN) ? Q30_MIN : \
	(((int64_t)(X) > (int64_t)Q30_MAX) ? Q30_MAX: (X)))

/* Saturate to -1..0.9999 instead of the min..max (-2..1.99999) */
#define Q30_SAT_ONE(X) (((int32_t)(X) < -1073741824) ? -1073741824: \
	(((int32_t)(X) > 1073741823) ? 1073741823: (X)))

/* Conversion form float to fixed point Q2.30 */
#define Q30(F)           ((int64_t)((double)(F) * (double)(1073741824.)))
/* Convert from float point to fixed point Q2.30 */
#define FLOAT_Q30(X)     ((int32_t)((X) * 1073741824.))

/* Convert from fractional Q2.30 to float point */
#define Q30F(Q) ((double)((double)(Q) * (1.0 / (double)(1073741824.))))
/* Convert from fixed point Q2.30 to float point */
#define Q30_FLOAT(X)     ((float)(X) / 1073741824.)

/* Q30 Signed Multiply */
#define Q30_MUL(X1, X2)  (((int64_t)(X1) * (int32_t)(X2) + (1 << 29)) >> 30)

/* Q30 Unsigned Multiply */
#define Q30_UMUL(X1, X2) ((((uint64_t)(X1) * (uint32_t)(X2)) + (1 << 29)) >> 30)

/* Q30 Divide */
#define Q30_DIV(X, Y)    (((int64_t)(X) << 30) / (int32_t)(Y))
/* Q30 Unsigned Divide */
#define Q30_UDIV(X, Y)   (((uint64_t)(X) << 30) / (uint32_t)(Y))


/* -------------------------------------------------------------------------
 Q1.31 format: -1.0 ... .9999999
 */
 
#define Q31_MAX ((int32_t)2147483647)
#define Q31_MIN ((int32_t)-2147483648)

/* Q31 Saturation */
#define Q31_SAT(X) ((int64_t)(X) < (int64_t)Q31_MIN) ? Q31_MIN : \
	(((int64_t)(X) > (int64_t)Q31_MAX) ? Q31_MAX: (X))

/* Conversion form float to fixed point Q1.31 */
#define Q31(F)           ((int64_t)((double)(F) * (double)(2147483648.)))
/* Convert from float point to fixed point Q1.31 */
#define FLOAT_Q31(X)     ((int32_t)((X) * 2147483648.))

/* Convert from fractional Q1.31 to float point */
#define Q31F(Q) ((double)((double)(Q) * (1.0 / (double)(2147483648.))))
/* Convert from fixed point Q1.31 to float point */
#define Q31_FLOAT(X)     ((float)(X) / 2147483648.)

/* Q31 Signed Multiply */
#define Q31_MUL(X1, X2) (((int64_t)(X1) * (int32_t)(X2)) >> 31)
//#define Q31_MUL(X1, X2) ((((int64_t)(X1) * (int32_t)(X2)) + (1 << 30)) >> 31)

/* Q31 Unsigned Multiply */
#define Q31_UMUL(X1, X2) ((((uint64_t)(X1) * (uint32_t)(X2)) + (1 << 30)) >> 31)

/* Q31 Divide */
#define Q31_DIV(X, Y) (((int64_t)(X) << 31) / (int32_t)(Y))
/* Q31 Unsigned Divide */
#define Q31_UDIV(X, Y) (((uint64_t)(X) << 31) / (uint32_t)(Y))


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
uint32_t u32sqrt(uint32_t x);
uint32_t u64sqrt(uint64_t x);

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

/* Normalized fixed point cosine:
 x = Q31(0.0) .. (1.0) -> 0 .. pi */
int32_t q31cos(int32_t x);

#endif // __FIXPT_H__

