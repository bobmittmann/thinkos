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
 * @file stm32l4-syscfg.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

/* ------------------------------------------------------------------------- */
/* SYSCFG - System configuration controller */
/* ------------------------------------------------------------------------- */

#ifndef __STM32_SYSCFG_H__
#define __STM32_SYSCFG_H__

/* ------------------------------------------------------------------------- */
/* SYSCFG configuration register - CFGR */
#define STM32_SYSCFG_MEMRMP 0x0000

/* Bits [2..0] - Memory mapping selection */
#define SYSCFG_MEM_MODE (0x7 << 0)
#define SYSCFG_MEM_MODE_MFLASH (0 << 0)
#define SYSCFG_MEM_MODE_SFLASH (1 << 0)
#define SYSCFG_MEM_MODE_SRAM1 (3 << 0)
#define SYSCFG_MEM_MODE_QUADSPI (6 << 0)
/* These bits control the memory internal mapping at address 0x0000 0000. These bits are
used to select the physical remap by software and so, bypass the BOOT mode setting. After
reset these bits take the value selected by BOOT0 (pin or option bit depending on
nSWBOOT0 option bit) and BOOT1 option bit.
000: Main Flash memory mapped at 0x00000000.
001: System Flash memory mapped at 0x00000000.
010: Reserved
011: SRAM1 mapped at 0x00000000.
100: Reserved
101: Reserved
110: QUADSPI memory mapped at 0x00000000.
111: Reserved
*/

/* ------------------------------------------------------------------------- */
/* SYSCFG configuration register - CFGR */
#define STM32_SYSCFG_CFGR1 0x0004
/*
Bits 31:26 FPU_IE[5..0]: Floating Point Unit interrupts enable bits
FPU_IE[5]: Inexact interrupt enable
FPU_IE[4]: Input denormal interrupt enable
FPU_IE[3]: Overflow interrupt enable
FPU_IE[2]: underflow interrupt enable
FPU_IE[1]: Divide-by-zero interrupt enable
FPU_IE[0]: Invalid operation interrupt enable
Bits 25:24 Reserved, must be kept at reset value.
Bit 23 I2C4_FMP: Fast-mode Plus driving capability activation
This bit enables the Fm+ driving mode on I2C4 pins selected through AF selection bits.
0: Fm+ mode is not enabled on I2C4 pins selected through AF selection bits
1: Fm+ mode is enabled on I2C4 pins selected through AF selection bits.
Bit 22 I2C3_FMP: I2C3 Fast-mode Plus driving capability activation
This bit enables the Fm+ driving mode on I2C3 pins selected through AF selection bits.
0: Fm+ mode is not enabled on I2C3 pins selected through AF selection bits
1: Fm+ mode is enabled on I2C3 pins selected through AF selection bits.
Bit 21 I2C2_FMP: I2C2 Fast-mode Plus driving capability activation
This bit enables the Fm+ driving mode on I2C2 pins selected through AF selection bits.
0: Fm+ mode is not enabled on I2C2 pins selected through AF selection bits
1: Fm+ mode is enabled on I2C2 pins selected through AF selection bits.
Bit 20 I2C1_FMP: I2C1 Fast-mode Plus driving capability activation
This bit enables the Fm+ driving mode on I2C1 pins selected through AF selection bits.
0: Fm+ mode is not enabled on I2C1 pins selected through AF selection bits
1: Fm+ mode is enabled on I2C1 pins selected through AF selection bits.
Bit 19 I2C_PB9_FMP: Fast-mode Plus (Fm+) driving capability activation on PB9
This bit enables the Fm+ driving mode for PB9.
0: PB9 pin operates in standard mode.
1: Fm+ mode enabled on PB9 pin, and the Speed control is bypassed. */


/* Bit 18 I2C_PB8_FMP: Fast-mode Plus (Fm+) driving capability activation on PB8 */
/* This bit enables the Fm+ driving mode for PB8.
0: PB8 pin operates in standard mode.
1: Fm+ mode enabled on PB8 pin, and the Speed control is bypassed. */
/* Bit 17 I2C_PB7_FMP: Fast-mode Plus (Fm+) driving capability activation on PB7 */
/* This bit enables the Fm+ driving mode for PB7.
0: PB7 pin operates in standard mode.
1: Fm+ mode enabled on PB7 pin, and the Speed control is bypassed. */
/* Bit 16 I2C_PB6_FMP: Fast-mode Plus (Fm+) driving capability activation on PB6 */
/* This bit enables the Fm+ driving mode for PB6.
0: PB6 pin operates in standard mode.
1: Fm+ mode enabled on PB6 pin, and the Speed control is bypassed. */
/* Bits 15:9 Reserved, must be kept at reset value. */
/* Bit 8 BOOSTEN: I/O analog switch voltage booster enable */
/* 0: I/O analog switches are supplied by VDDA voltage. This is the recommended configuration
when using the ADC in high VDDA voltage operation.
1: I/O analog switches are supplied by a dedicated voltage booster (supplied by VDD). This is
the recommended configuration when using the ADC in low VDDA voltage operation. */
/* Bits 7:1 Reserved, must be kept at reset value. */

/* Bit 0 FWDIS: Firewall disable */

/* This bit is cleared by software to protect the access to the memory segments according to
the Firewall configuration. Once enabled, the firewall cannot be disabled by software. Only a
system reset set the bit.
0 : Firewall protection enabled
1 : Firewall protection disabled */



/* SYSCFG external interrupt configuration register 1 */
#define STM32F_SYSCFG_EXTICR1 0x08

/* [31..16] Reserved */

/* Bits [15..0] - EXTI x configuration (x = 0 to 3) */
#define SYSCFG_EXTIX_MSK (((1 << (3 + 1)) - 1) << 0)
#define SYSCFG_EXTIX_SET(VAL) (((VAL) << 0) & SYSCFG_EXTIX_MSK)
#define SYSCFG_EXTIX_GET(REG) (((REG) & SYSCFG_EXTIX_MSK) >> 0)
/* These bits are written by software to select the source input for the 
   EXTIx external interrupt.
   0000: PA[x] pin
   0001: PB[x] pin
   0010: PC[x] pin
   0011: PD[x] pin
   0100: PE[x] pin
   0101: PF[x] pin
   0110: PG[x] pin
   0111: PH[x] pin
   1000: PI[x] pin  */

#define SYSCFG_EXTI0_MSK (0xf << 0)
#define SYSCFG_EXTI0_PA  (0x0 << 0)
#define SYSCFG_EXTI0_PB  (0x1 << 0)
#define SYSCFG_EXTI0_PC  (0x2 << 0)
#define SYSCFG_EXTI0_PD  (0x3 << 0)
#define SYSCFG_EXTI0_PE  (0x4 << 0)
#define SYSCFG_EXTI0_PF  (0x5 << 0)
#define SYSCFG_EXTI0_PG  (0x6 << 0)
#define SYSCFG_EXTI0_PH  (0x7 << 0)
#define SYSCFG_EXTI0_PI  (0x8 << 0)

#define SYSCFG_EXTI1_MSK (0xf << 4)
#define SYSCFG_EXTI1_PA (0x0 << 4)
#define SYSCFG_EXTI1_PB (0x1 << 4)
#define SYSCFG_EXTI1_PC (0x2 << 4)
#define SYSCFG_EXTI1_PD (0x3 << 4)
#define SYSCFG_EXTI1_PE (0x4 << 4)
#define SYSCFG_EXTI1_PF (0x5 << 4)
#define SYSCFG_EXTI1_PG (0x6 << 4)
#define SYSCFG_EXTI1_PH (0x7 << 4)
#define SYSCFG_EXTI1_PI (0x8 << 4)

#define SYSCFG_EXTI2_MSK (0xf << 8)
#define SYSCFG_EXTI2_PA (0x0 << 8)
#define SYSCFG_EXTI2_PB (0x1 << 8)
#define SYSCFG_EXTI2_PC (0x2 << 8)
#define SYSCFG_EXTI2_PD (0x3 << 8)
#define SYSCFG_EXTI2_PE (0x4 << 8)
#define SYSCFG_EXTI2_PF (0x5 << 8)
#define SYSCFG_EXTI2_PG (0x6 << 8)
#define SYSCFG_EXTI2_PH (0x7 << 8)
#define SYSCFG_EXTI2_PI (0x8 << 8)

#define SYSCFG_EXTI3_MSK (0xf << 12)
#define SYSCFG_EXTI3_PA (0x0 << 12)
#define SYSCFG_EXTI3_PB (0x1 << 12)
#define SYSCFG_EXTI3_PC (0x2 << 12)
#define SYSCFG_EXTI3_PD (0x3 << 12)
#define SYSCFG_EXTI3_PE (0x4 << 12)
#define SYSCFG_EXTI3_PF (0x5 << 12)
#define SYSCFG_EXTI3_PG (0x6 << 12)
#define SYSCFG_EXTI3_PH (0x7 << 12)
#define SYSCFG_EXTI3_PI (0x8 << 12)


/* SYSCFG external interrupt configuration register 2 */
#define STM32F_SYSCFG_EXTICR2 0x0C

/* [31..16] Reserved */

/* Bits [15..0] - EXTI x configuration (x = 4 to 7) */
#define SYSCFG_EXTIX_MSK (((1 << (3 + 1)) - 1) << 0)
#define SYSCFG_EXTIX_SET(VAL) (((VAL) << 0) & SYSCFG_EXTIX_MSK)
#define SYSCFG_EXTIX_GET(REG) (((REG) & SYSCFG_EXTIX_MSK) >> 0)
/* These bits are written by software to select the source input for the 
   EXTIx external interrupt.
   0000: PA[x] pin
   0001: PB[x] pin
   0010: PC[x] pin
   0011: PD[x] pin
   0100: PE[x] pin
   0101: PF[x] pin
   0110: PG[x] pin
   0111: PH[x] pin
   1000: PI[x] pin */

#define SYSCFG_EXTI4_MSK (0xf << 0)
#define SYSCFG_EXTI4_PA (0x0 << 0)
#define SYSCFG_EXTI4_PB (0x1 << 0)
#define SYSCFG_EXTI4_PC (0x2 << 0)
#define SYSCFG_EXTI4_PD (0x3 << 0)
#define SYSCFG_EXTI4_PE (0x4 << 0)
#define SYSCFG_EXTI4_PF (0x5 << 0)
#define SYSCFG_EXTI4_PG (0x6 << 0)
#define SYSCFG_EXTI4_PH (0x7 << 0)
#define SYSCFG_EXTI4_PI (0x8 << 0)

#define SYSCFG_EXTI5_MSK (0xf << 4)
#define SYSCFG_EXTI5_PA (0x0 << 4)
#define SYSCFG_EXTI5_PB (0x1 << 4)
#define SYSCFG_EXTI5_PC (0x2 << 4)
#define SYSCFG_EXTI5_PD (0x3 << 4)
#define SYSCFG_EXTI5_PE (0x4 << 4)
#define SYSCFG_EXTI5_PF (0x5 << 4)
#define SYSCFG_EXTI5_PG (0x6 << 4)
#define SYSCFG_EXTI5_PH (0x7 << 4)
#define SYSCFG_EXTI5_PI (0x8 << 4)

#define SYSCFG_EXTI6_MSK (0xf << 8)
#define SYSCFG_EXTI6_PA (0x0 << 8)
#define SYSCFG_EXTI6_PB (0x1 << 8)
#define SYSCFG_EXTI6_PC (0x2 << 8)
#define SYSCFG_EXTI6_PD (0x3 << 8)
#define SYSCFG_EXTI6_PE (0x4 << 8)
#define SYSCFG_EXTI6_PF (0x5 << 8)
#define SYSCFG_EXTI6_PG (0x6 << 8)
#define SYSCFG_EXTI6_PH (0x7 << 8)
#define SYSCFG_EXTI6_PI (0x8 << 8)

#define SYSCFG_EXTI7_MSK (0xf << 12)
#define SYSCFG_EXTI7_PA (0x0 << 12)
#define SYSCFG_EXTI7_PB (0x1 << 12)
#define SYSCFG_EXTI7_PC (0x2 << 12)
#define SYSCFG_EXTI7_PD (0x3 << 12)
#define SYSCFG_EXTI7_PE (0x4 << 12)
#define SYSCFG_EXTI7_PF (0x5 << 12)
#define SYSCFG_EXTI7_PG (0x6 << 12)
#define SYSCFG_EXTI7_PH (0x7 << 12)
#define SYSCFG_EXTI7_PI (0x8 << 12)

/* SYSCFG external interrupt configuration register 3 */
#define STM32F_SYSCFG_EXTICR3 0x10

/* [31..16] Reserved */

/* Bits [15..0] - EXTI x configuration (x = 8 to 11) */
#define SYSCFG_EXTIX_MSK (((1 << (3 + 1)) - 1) << 0)
#define SYSCFG_EXTIX_SET(VAL) (((VAL) << 0) & SYSCFG_EXTIX_MSK)
#define SYSCFG_EXTIX_GET(REG) (((REG) & SYSCFG_EXTIX_MSK) >> 0)
/* These bits are written by software to select the source input for the 
   EXTIx external interrupt.
   0000: PA[x] pin
   0001: PB[x] pin
   0010: PC[x] pin
   0011: PD[x] pin
   0100: PE[x] pin
   0101: PF[x] pin
   0110: PG[x] pin
   0111: PH[x] pin
   1000: PI[x] pin */

#define SYSCFG_EXTI8_MSK (0xf << 0)
#define SYSCFG_EXTI8_PA (0x0 << 0)
#define SYSCFG_EXTI8_PB (0x1 << 0)
#define SYSCFG_EXTI8_PC (0x2 << 0)
#define SYSCFG_EXTI8_PD (0x3 << 0)
#define SYSCFG_EXTI8_PE (0x4 << 0)
#define SYSCFG_EXTI8_PF (0x5 << 0)
#define SYSCFG_EXTI8_PG (0x6 << 0)
#define SYSCFG_EXTI8_PH (0x7 << 0)
#define SYSCFG_EXTI8_PI (0x8 << 0)

#define SYSCFG_EXTI9_MSK (0xf << 4)
#define SYSCFG_EXTI9_PA (0x0 << 4)
#define SYSCFG_EXTI9_PB (0x1 << 4)
#define SYSCFG_EXTI9_PC (0x2 << 4)
#define SYSCFG_EXTI9_PD (0x3 << 4)
#define SYSCFG_EXTI9_PE (0x4 << 4)
#define SYSCFG_EXTI9_PF (0x5 << 4)
#define SYSCFG_EXTI9_PG (0x6 << 4)
#define SYSCFG_EXTI9_PH (0x7 << 4)
#define SYSCFG_EXTI9_PI (0x8 << 4)

#define SYSCFG_EXTI10_MSK (0xf << 8)
#define SYSCFG_EXTI10_PA (0x0 << 8)
#define SYSCFG_EXTI10_PB (0x1 << 8)
#define SYSCFG_EXTI10_PC (0x2 << 8)
#define SYSCFG_EXTI10_PD (0x3 << 8)
#define SYSCFG_EXTI10_PE (0x4 << 8)
#define SYSCFG_EXTI10_PF (0x5 << 8)
#define SYSCFG_EXTI10_PG (0x6 << 8)
#define SYSCFG_EXTI10_PH (0x7 << 8)
#define SYSCFG_EXTI10_PI (0x8 << 8)

#define SYSCFG_EXTI11_MSK (0xf << 12)
#define SYSCFG_EXTI11_PA (0x0 << 12)
#define SYSCFG_EXTI11_PB (0x1 << 12)
#define SYSCFG_EXTI11_PC (0x2 << 12)
#define SYSCFG_EXTI11_PD (0x3 << 12)
#define SYSCFG_EXTI11_PE (0x4 << 12)
#define SYSCFG_EXTI11_PF (0x5 << 12)
#define SYSCFG_EXTI11_PG (0x6 << 12)
#define SYSCFG_EXTI11_PH (0x7 << 12)
#define SYSCFG_EXTI11_PI (0x8 << 12)

/* SYSCFG external interrupt configuration register 4 */
#define STM32F_SYSCFG_EXTICR4 0x14

/* [31..16] Reserved */

/* Bits [15..0] - EXTI x configuration (x = 12 to 15) */
#define SYSCFG_EXTIX_MSK (((1 << (3 + 1)) - 1) << 0)
#define SYSCFG_EXTIX_SET(VAL) (((VAL) << 0) & SYSCFG_EXTIX_MSK)
#define SYSCFG_EXTIX_GET(REG) (((REG) & SYSCFG_EXTIX_MSK) >> 0)
/* These bits are written by software to select the source input for the 
   EXTIx external interrupt.
   0000: PA[x] pin
   0001: PB[x] pin
   0010: PC[x] pin
   0011: PD[x] pin
   0100: PE[x] pin
   0101: PF[x] pin
   0110: PG[x] pin
   0111: PH[x] pin
   Note: PI[15:12] are not used. */

#define SYSCFG_EXTI12_MSK (0xf << 0)
#define SYSCFG_EXTI12_PA (0x0 << 0)
#define SYSCFG_EXTI12_PB (0x1 << 0)
#define SYSCFG_EXTI12_PC (0x2 << 0)
#define SYSCFG_EXTI12_PD (0x3 << 0)
#define SYSCFG_EXTI12_PE (0x4 << 0)
#define SYSCFG_EXTI12_PF (0x5 << 0)
#define SYSCFG_EXTI12_PG (0x6 << 0)
#define SYSCFG_EXTI12_PH (0x7 << 0)
#define SYSCFG_EXTI12_PI (0x8 << 0)

#define SYSCFG_EXTI13_MSK (0xf << 4)
#define SYSCFG_EXTI13_PA (0x0 << 4)
#define SYSCFG_EXTI13_PB (0x1 << 4)
#define SYSCFG_EXTI13_PC (0x2 << 4)
#define SYSCFG_EXTI13_PD (0x3 << 4)
#define SYSCFG_EXTI13_PE (0x4 << 4)
#define SYSCFG_EXTI13_PF (0x5 << 4)
#define SYSCFG_EXTI13_PG (0x6 << 4)
#define SYSCFG_EXTI13_PH (0x7 << 4)
#define SYSCFG_EXTI13_PI (0x8 << 4)

#define SYSCFG_EXTI14_MSK (0xf << 8)
#define SYSCFG_EXTI14_PA (0x0 << 8)
#define SYSCFG_EXTI14_PB (0x1 << 8)
#define SYSCFG_EXTI14_PC (0x2 << 8)
#define SYSCFG_EXTI14_PD (0x3 << 8)
#define SYSCFG_EXTI14_PE (0x4 << 8)
#define SYSCFG_EXTI14_PF (0x5 << 8)
#define SYSCFG_EXTI14_PG (0x6 << 8)
#define SYSCFG_EXTI14_PH (0x7 << 8)
#define SYSCFG_EXTI14_PI (0x8 << 8)

#define SYSCFG_EXTI15_MSK (0xf << 12)
#define SYSCFG_EXTI15_PA (0x0 << 12)
#define SYSCFG_EXTI15_PB (0x1 << 12)
#define SYSCFG_EXTI15_PC (0x2 << 12)
#define SYSCFG_EXTI15_PD (0x3 << 12)
#define SYSCFG_EXTI15_PE (0x4 << 12)
#define SYSCFG_EXTI15_PF (0x5 << 12)
#define SYSCFG_EXTI15_PG (0x6 << 12)
#define SYSCFG_EXTI15_PH (0x7 << 12)
#define SYSCFG_EXTI15_PI (0x8 << 12)


/*
 * SYSCFG configuration register 2 (SYSCFG_CFGR2) */

#define STM32_SYSCFG_CFGR2 0x001c
/* Bits 31:9 Reserved, must be kept at reset value */

/* Bit 8 SPF: SRAM2 parity error flag */

/* This bit is set by hardware when an SRAM2 parity error is detected. 
   It is cleared by software by writing ‘1’.
   0: No SRAM2 parity error detected
   1: SRAM2 parity error detected */

/* Bits 7:4 Reserved, must be kept at reset value */

/* Bit 3 ECCL: ECC Lock */
/* This bit is set by software and cleared only by a system reset. 
   It can be used to enable and lock the Flash ECC error connection to 
   TIM1/15/16 Break input.

0: ECC error disconnected from TIM1/15/16 Break input.
1: ECC error connected to TIM1/15/16 Break input. */

/* Bit 2 PVDL: PVD lock enable bit */
#define SYSCFG_PVDL (1 << 2)
/* This bit is set by software and cleared only by a system reset. It can 
   be used to enable and lock the PVD connection to TIM1/15/16 Break input, 
   as well as the PVDE and PLS[2:0] in the PWR_CR2 register. 
   0: PVD interrupt disconnected from TIM1/15/16 Break input. PVDE and 
   PLS[2:0] bits can be programmed by the application.
   1: PVD interrupt connected to TIM1/15/16 Break input, 
   PVDE and PLS[2:0] bits are read only. */

/* Bit 1 SPL: SRAM2 parity lock bit */
#define SYSCFG_SPL (1 << 2)
/* This bit is set by software and cleared only by a system reset. It can 
   be used to enable and lock the SRAM2 parity error signal connection to 
   TIM1/15/16 Break inputs.
   0: SRAM2 parity error signal disconnected from TIM1/15/16 Break inputs
   1: SRAM2 parity error signal connected to TIM1/15/16 Break inputs */

/* Bit 0 CLL: Cortex®-M4 LOCKUP (Hardfault) output enable bit */

/* This bit is set by software and cleared only by a system reset. It can 
   be used to enable and lock the connection of Cortex®-M4 LOCKUP 
   (Hardfault) output to TIM1/15/16 Break input
   0: Cortex®-M4 LOCKUP output disconnected from TIM1/15/16 Break inputs
   1: Cortex®-M4 LOCKUP output connected to TIM1/15/16 Break inputs */


#define STM32_SYSCFG_CFGR2 0x001c

#ifndef __ASSEMBLER__

#include <stdint.h>

struct stm32_syscfg {
	volatile uint32_t memrmp;
	volatile uint32_t cfgr1;
	union {
		struct {
			volatile uint32_t exticr1;
			volatile uint32_t exticr2;
			volatile uint32_t exticr3;
			volatile uint32_t exticr4;
		};
		volatile uint32_t exticr[4];
	};
	volatile uint32_t scsr;
	volatile uint32_t cfgr2;
    volatile uint32_t spwr;
    volatile uint32_t skr;
};

#endif /* __ASSEMBLER__ */

#endif /* __STM32L4_SYSCFG_H__ */

