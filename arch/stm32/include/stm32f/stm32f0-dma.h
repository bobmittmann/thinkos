/* 
 * Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
 * 
 * This file is part of the libstm32f.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3.0 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

/** 
 * @file stm32f-dma.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __STM32F0_DMA_H__
#define __STM32F0_DMA_H__

/*-------------------------------------------------------------------------
  Direct Memory Access Controller (DMA)
  -------------------------------------------------------------------------*/

#if defined(STM32F0X)

/* DMA interrupt status register */
#define STM32F_DMA_ISR 0x000

/* Bits 31:28 Reserved, must be kept at reset value.
Bits 27, 23, 19, 15,
11, 7, 3
TEIFx: Channel x transfer error flag (x = 1 ..7)
This bit is set by hardware. It is cleared by software writing 1 to the corresponding bit in the
DMA_IFCR register.
0: No transfer error (TE) on channel x
1: A transfer error (TE) occurred on channel x
Bits 26, 22, 18, 14,
10, 6, 2
HTIFx: Channel x half transfer flag (x = 1 ..7)
This bit is set by hardware. It is cleared by software writing 1 to the corresponding bit in the
DMA_IFCR register.
0: No half transfer (HT) event on channel x
1: A half transfer (HT) event occurred on channel x
Bits 25, 21, 17, 13,
9, 5, 1
TCIFx: Channel x transfer complete flag (x = 1 ..7)
This bit is set by hardware. It is cleared by software writing 1 to the corresponding bit in the
DMA_IFCR register.
0: No transfer complete (TC) event on channel x
1: A transfer complete (TC) event occurred on channel x
Bits 24, 20, 16, 12,
8, 4, 0
GIFx: Channel x global interrupt flag (x = 1 ..7)
This bit is set by hardware. It is cleared by software writing 1 to the corresponding bit in the
DMA_IFCR register.
0: No TE, HT or TC event on channel x
1: A TE, HT or TC event occurred on channel x */

#define DMA_TEIF7 (1 << 27)
#define DMA_HTIF7 (1 << 26)
#define DMA_TCIF7 (1 << 25)
#define DMA_GIF7  (1 << 24)

#define DMA_TEIF6 (1 << 23)
#define DMA_HTIF6 (1 << 22)
#define DMA_TCIF6 (1 << 21)
#define DMA_GIF6  (1 << 20)

#define DMA_TEIF5 (1 << 19)
#define DMA_HTIF5 (1 << 18)
#define DMA_TCIF5 (1 << 17)
#define DMA_GIF5  (1 << 16)

#define DMA_TEIF4 (1 << 15)
#define DMA_HTIF4 (1 << 14)
#define DMA_TCIF4 (1 << 13)
#define DMA_GIF4  (1 << 12)

#define DMA_TEIF3 (1 << 11)
#define DMA_HTIF3 (1 << 10)
#define DMA_TCIF3 (1 << 9)
#define DMA_GIF3  (1 << 8)

#define DMA_TEIF2 (1 << 7)
#define DMA_HTIF2 (1 << 6)
#define DMA_TCIF2 (1 << 5)
#define DMA_GIF2  (1 << 4)

#define DMA_TEIF1 (1 << 3)
#define DMA_HTIF1 (1 << 2)
#define DMA_TCIF1 (1 << 1)
#define DMA_GIF1  (1 << 0)

/* DMA interrupt flag clear register */
#define STM32F_DMA_IFCR 0x004

/*
Bits 31:28 Reserved, must be kept at reset value.
Bits 27, 23, 19, 15,
11, 7, 3
CTEIFx: Channel x transfer error clear (x = 1 ..7)
This bit is set and cleared by software.
0: No effect
1: Clears the corresponding TEIF flag in the DMA_ISR register
Bits 26, 22, 18, 14,
10, 6, 2
CHTIFx: Channel x half transfer clear (x = 1 ..7)
This bit is set and cleared by software.
0: No effect
1: Clears the corresponding HTIF flag in the DMA_ISR register
Bits 25, 21, 17, 13,
9, 5, 1
CTCIFx: Channel x transfer complete clear (x = 1 ..7)
This bit is set and cleared by software.
0: No effect
1: Clears the corresponding TCIF flag in the DMA_ISR register
Bits 24, 20, 16, 12,
8, 4, 0
CGIFx: Channel x global interrupt clear (x = 1 ..7)
This bit is set and cleared by software.
0: No effect
1: Clears the GIF, TEIF, HTIF and TCIF flags in the DMA_ISR register
*/

#define DMA_CTEIF7 (1 << 27)
#define DMA_CHTIF7 (1 << 26)
#define DMA_CTCIF7 (1 << 25)
#define DMA_CGIF7  (1 << 24)

#define DMA_CTEIF6 (1 << 23)
#define DMA_CHTIF6 (1 << 22)
#define DMA_CTCIF6 (1 << 21)
#define DMA_CGIF6  (1 << 20)

#define DMA_CTEIF5 (1 << 19)
#define DMA_CHTIF5 (1 << 18)
#define DMA_CTCIF5 (1 << 17)
#define DMA_CGIF5  (1 << 16)

#define DMA_CTEIF4 (1 << 15)
#define DMA_CHTIF4 (1 << 14)
#define DMA_CTCIF4 (1 << 13)
#define DMA_CGIF4  (1 << 12)

#define DMA_CTEIF3 (1 << 11)
#define DMA_CHTIF3 (1 << 10)
#define DMA_CTCIF3 (1 << 9)
#define DMA_CGIF3  (1 << 8)

#define DMA_CTEIF2 (1 << 7)
#define DMA_CHTIF2 (1 << 6)
#define DMA_CTCIF2 (1 << 5)
#define DMA_CGIF2  (1 << 4)

#define DMA_CTEIF1 (1 << 3)
#define DMA_CHTIF1 (1 << 2)
#define DMA_CTCIF1 (1 << 1)
#define DMA_CGIF1  (1 << 0)

/*	DMA channel x configuration register */
#define STM32F_DMA_CCR1 (0x08 + 20 * 0)
#define STM32F_DMA_CCR2 (0x08 + 20 * 1)
#define STM32F_DMA_CCR3 (0x08 + 20 * 2)
#define STM32F_DMA_CCR4 (0x08 + 20 * 3)
#define STM32F_DMA_CCR5 (0x08 + 20 * 4)
#define STM32F_DMA_CCR6 (0x08 + 20 * 5)
#define STM32F_DMA_CCR7 (0x08 + 20 * 6)

/* Bits 31:15 Reserved, must be kept at reset value. */

/*Bit 14 MEM2MEM: Memory to memory mode */
#define DMA_MEM2MEM (1 << 14)
/* This bit is set and cleared by software.
0: Memory to memory mode disabled
1: Memory to memory mode enabled */

/* Bits 13:12 PL[1:0]: Channel priority level */
#define DMA_PL_LOW       (0 << 12)
#define DMA_PL_MEDIUM    (1 << 12)
#define DMA_PL_HIGH      (2 << 12)
#define DMA_PL_VERY_HIGH (2 << 12)
/*These bits are set and cleared by software.
00: Low
01: Medium
10: High
11: Very high */

/* Bits 11:10 MSIZE[1:0]: Memory size */
#define DMA_MSIZE_8  (0 << 10)
#define DMA_MSIZE_16 (1 << 10)
#define DMA_MSIZE_32 (2 << 10)
/* These bits are set and cleared by software.
00: 8-bits
01: 16-bits
10: 32-bits
11: Reserved */

/* Bits 9:8 PSIZE[1:0]: Peripheral size */
#define DMA_PSIZE_8  (0 << 8)
#define DMA_PSIZE_16 (1 << 8)
#define DMA_PSIZE_32 (2 << 8)
/* These bits are set and cleared by software.
00: 8-bits
01: 16-bits
10: 32-bits
11: Reserved */

/* Bit 7 MINC: Memory increment mode */
#define DMA_MINC (1 << 7)
/* This bit is set and cleared by software.
0: Memory increment mode disabled
1: Memory increment mode enabled */

/* Bit 6 PINC: Peripheral increment mode */
#define DMA_PINC (1 << 6)
/* This bit is set and cleared by software.
0: Peripheral increment mode disabled
1: Peripheral increment mode enabled */

/* Bit 5 CIRC: Circular mode */
#define DMA_CIRC (1 << 5)
/* This bit is set and cleared by software.
0: Circular mode disabled
1: Circular mode enabled */

/* Bit 4 DIR: Data transfer direction */
#define DMA_DIR (1 << 4)
#define DMA_DIR_PTM (0 << 4)
#define DMA_DIR_MTP (1 << 4)
/* This bit is set and cleared by software.
0: Read from peripheral
1: Read from memory */

/* Bit 3 TEIE: Transfer error interrupt enable */
#define DMA_TEIE (1 << 3)
/* This bit is set and cleared by software.
0: TE interrupt disabled
1: TE interrupt enabled */

/* Bit 2 HTIE: Half transfer interrupt enable */
#define DMA_HTIE (1 << 2)
/* This bit is set and cleared by software.
0: HT interrupt disabled
1: HT interrupt enabled */

/* Bit 1 TCIE: Transfer complete interrupt enable */
#define DMA_TCIE (1 << 1)
/* This bit is set and cleared by software.
0: TC interrupt disabled
1: TC interrupt enabled */

/* Bit 0 EN: Channel enable */
#define DMA_EN (1 << 0)
/* This bit is set and cleared by software.
0: Channel disabled
1: Channel enabled*/

/*	DMA channel x number of data register */
#define STM32F_DMA_CNDTR1 (0x0C + 20 * 0)
#define STM32F_DMA_CNDTR2 (0x0C + 20 * 1)
#define STM32F_DMA_CNDTR3 (0x0C + 20 * 2)
#define STM32F_DMA_CNDTR4 (0x0C + 20 * 3)
#define STM32F_DMA_CNDTR5 (0x0C + 20 * 4)
#define STM32F_DMA_CNDTR6 (0x0C + 20 * 5)
#define STM32F_DMA_CNDTR7 (0x0C + 20 * 6)

/*	DMA channel x peripheral address register */
#define STM32F_DMA_CPAR1 (0x10 + 20 * 0)
#define STM32F_DMA_CPAR2 (0x10 + 20 * 1)
#define STM32F_DMA_CPAR3 (0x10 + 20 * 2)
#define STM32F_DMA_CPAR4 (0x10 + 20 * 3)
#define STM32F_DMA_CPAR5 (0x10 + 20 * 4)
#define STM32F_DMA_CPAR6 (0x10 + 20 * 5)
#define STM32F_DMA_CPAR7 (0x10 + 20 * 6)

/*	DMA channel x memory address register */
#define STM32F_DMA_CMAR1 (0x14 + 20 * 0)
#define STM32F_DMA_CMAR2 (0x14 + 20 * 1)
#define STM32F_DMA_CMAR3 (0x14 + 20 * 2)
#define STM32F_DMA_CMAR4 (0x14 + 20 * 3)
#define STM32F_DMA_CMAR5 (0x14 + 20 * 4)
#define STM32F_DMA_CMAR6 (0x14 + 20 * 5)
#define STM32F_DMA_CMAR7 (0x14 + 20 * 6)

#define STM32_DMA_CHANNEL1 0
#define STM32_DMA_CHANNEL2 1
#define STM32_DMA_CHANNEL3 2
#define STM32_DMA_CHANNEL4 3
#define STM32_DMA_CHANNEL5 4
#define STM32_DMA_CHANNEL6 5
#define STM32_DMA_CHANNEL7 6

#endif

#ifndef __ASSEMBLER__

#include <stdint.h>

/* Fake Channel selection */
#define DMA_CHSEL_SET(VAL) 0

struct stm32f_dma_channel {
	volatile uint32_t ccr;
	volatile uint32_t cndtr;
	volatile void * cpar;
	volatile void * cmar;
	uint32_t res1;
};

/*
XXX: t seems that the 1xx and 3xx devices implements a reduced version
of the the DMA stream found on 2xx and 4xx parts... */
struct stm32f_dma_stream {
	volatile uint32_t cr;
	volatile uint32_t ndtr;
	volatile void * par;
	volatile void * m0ar;
};

struct stm32f_dma {
	volatile uint32_t isr;
	volatile uint32_t ifcr;
	union {
		volatile struct stm32f_dma_channel ch[8];
		volatile struct stm32f_dma_stream s[8];
	};
	volatile uint32_t cselr; /* 0xa8 */
};

#endif /* __ASSEMBLER__ */

#endif /* __STM32F0_DMA_H__ */

