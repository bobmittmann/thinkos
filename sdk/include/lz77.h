/* lz77.h - compressor
 * ------
 *
 *   ************************************************************************
 *   **            Company Confidential - For Internal Use Only            **
 *   **          Mircom Technologies Ltd. & Affiliates ("Mircom")          **
 *   **                                                                    **
 *   **   This information is confidential and the exclusive property of   **
 *   ** Mircom.  It is intended for internal use and only for the purposes **
 *   **   provided,  and may not be disclosed to any third party without   **
 *   **                prior written permission from Mircom.               **
 *   **                                                                    **
 *   **                        Copyright 2017-2018                         **
 *   ************************************************************************
 *
 */

/** 
 * @file lz77.h
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 * @brief Low memory footprint compression 
 * 
 * 
 */

#ifndef __LZ77_H__
#define __LZ77_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <endian.h>

#define ALGO_LZ77  9
#define BCL1_MAGIC 0x42434c31

static inline uint32_t ntohl(uint32_t __netlong)
{
#if __BYTE_ORDER == __BIG_ENDIAN
	return __netlong;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	return ((__netlong & 0xff) << 24) | ((__netlong & 0xff00) << 8) |
	    ((__netlong & 0xff0000) >> 8) | ((__netlong & 0xff000000) >> 24);
#endif
}

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif				/* __LZ77_H__ */
