/* 
 * thinkos_krnsvc.h
 *
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the ThinkOS library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */


#ifndef __THINKOS_KRNSVC_H__
#define __THINKOS_KRNSVC_H__

#define __THINKOS_PROFILE__
#include <thinkos/profile.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- 
 * Kernel Service numbers
 * --------------------------------------------------------------------------*/

#define THINKOS_KRNSVC_SIGNAL          0
#define THINKOS_KRNSVC_ERROR           1

#define THINKOS_KRNSVC_THREAD_START    64
#define THINKOS_KRNSVC_THREAD_STOP     65


#ifndef __ASSEMBLER__

/* ------------------------------------------------------------------------- 
 * C kernel services call macros 
 * ------------------------------------------------------------------------- */

#define __KRNSVC0(N) __extension__({ \
register int32_t ret asm("r0"); \
asm volatile ("udf %0 \n" : "=r"(ret) : "I" (OPC), "r"(r0): ); \
ret; })

#define __KRNSVC1(OPC, A1) __extension__({ \
register int32_t ret asm("r0"); \
register int32_t r0 asm("r0") = (int32_t)A1; \
asm volatile ("udf %0 \n" : "=r"(ret) : "I" (OPC), "r"(r0): ); \
ret; } )

#define __KRNSVC2(OPC, A1, A2) __extension__({ \
register int32_t ret asm("r0"); \
register int32_t r0 asm("r0") = (int32_t)A1; \
register int32_t r1 asm("r1") = (int32_t)A2; \
asm volatile ("udf %0 \n" : "=r"(ret) : "I" (OPC), \
			  "r"(r0), "r"(r1): ); \
ret; })

#define __KRNSVC3(OPC, A1, A2, A3) __extension__({ \
register int32_t ret asm("r0"); \
register int32_t r0 asm("r0") = (int32_t)A1; \
register int32_t r1 asm("r1") = (int32_t)A2; \
register int32_t r2 asm("r2") = (int32_t)A3; \
asm volatile ("udf %0 \n" : "=r"(ret) : "I" (OPC), \
			  "r"(r0), "r"(r1), "r"(r2): ); \
ret; })

#define __KRNSVC4(OPC, A1, A2, A3, A4) __extension__({ \
register int32_t ret asm("r0"); \
register int32_t r0 asm("r0") = (int32_t)A1; \
register int32_t r1 asm("r1") = (int32_t)A2; \
register int32_t r2 asm("r2") = (int32_t)A3; \
register int32_t r2 asm("r3") = (int32_t)A4; \
asm volatile ("udf %0 \n" : "=r"(ret) : "I" (OPC), \
			  "r"(r0), "r"(r1), "r"(r2), "r"(r3): ); \
ret; })


/* No arguments kernel service call */
#define THINKOS_KRNSVC0(N) __KRNSVC0(N)

/* One argument kernel service call */
#define THINKOS_KRNSVC1(N, A1) __KRNSVC1(N, (A1))

/* Two arguments kernel service call */
#define THINKOS_KRNSVC2(N, A1, A2) __KRNSVC2(N, (A1), (A2))

/* Three arguments kernel service call */
#define THINKOS_KRNSVC3(N, A1, A2, A3) __KRNSVC3(N, (A1), (A2), (A3))

/* Four arguments kernel service call */
#define THINKOS_KRNSVC4(N, A1, A2, A3, A4) __KRNSVC4(N, (A1), (A2), \
                                             (A3), (A4))
/* Five arguments kernel service call */
#define THINKOS_KRNSVC5(N, A1, A2, A3, A4, A5) __KRNSVC5(N, (A1), \
                                                (A2), (A3), (A4), (A5))

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLER__ */

#endif /* __THINKOS_KRNSVC_H__ */

