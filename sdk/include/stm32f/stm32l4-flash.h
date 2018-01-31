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
 * @file stm32l4-flash.h
 * @brief
 * @author Robinson Mittmann <bobmittmann@gmail.com>
 */ 

#ifndef __STM32L4_FLASH_H__
#define __STM32L4_FLASH_H__

/*-------------------------------------------------------------------------
  Flash Interface
  -------------------------------------------------------------------------*/

/* Flash access control register */
#define STM32_FLASH_ACR 0x00
/*	The Flash access control register is used to enable/disable 
	the acceleration features and control the Flash memory access 
	time according to CPU frequency. */

#define FLASH_SLEEP_PD (1 << 14)

#define FLASH_RUN_PD (1 << 13)

#define FLASH_DCRST (1 << 12)
/* Data cache reset
0: Data cache is not reset
1: Data cache is reset
This bit can be written only when the D cache is disabled.*/

#define FLASH_ICRST (1 << 11)
/* Instruction cache reset
0: Instruction cache is not reset
1: Instruction cache is reset
This bit can be written only when the I cache is disabled.*/

#define FLASH_DCEN (1 << 10)
/* Data cache enable
0: Data cache is disabled
1: Data cache is enabled*/

#define FLASH_ICEN (1 << 9)
/* Instruction cache enable
0: Instruction cache is disabled
1: Instruction cache is enabled*/

#define FLASH_PRFTEN (1 << 8)
/* Prefetch enable
0: Prefetch is disabled
1: Prefetch is enabled */

#define FLASH_LATENCY(WS) (((WS) & 0x7) << 0)
/* Latency. These bits represent the ratio of the CPU clock period to the Flash memory access time. 
000: Zero wait state
001: One wait state
010: Two wait states
011: Three wait states
100: Four wait states */


/* ------------------------------------------------------------------------- */
/* Power down key register (FLASH_PDKEYR) */
#define STM32_FLASH_PDKEYR 0x04
/* The Power down key register is used to unlock 
   the RUN_PD bit in FLASH_ACR. */
#define FLASH_PDKEY1 0x04152637
#define FLASH_PDKEY2 0xfafbfcfd

/* ------------------------------------------------------------------------- */
/* Flash key register */
#define STM32_FLASH_KEYR 0x08
/* The Flash key register is used to allow access to the Flash control 
   register and so, to allow program and erase operations. */
#define FLASH_KEY1 0x45670123
#define FLASH_KEY2 0xcdef89ab

/* Flash option key register */
#define STM32_FLASH_OPTKEYR 0x0c
/* The Flash option key register is used to allow program and erase 
   operations in the user configuration sector. */
#define FLASH_OPTKEY1 0x08192a3b
#define FLASH_OPTKEY2 0x4c5d6e7f

/* ------------------------------------------------------------------------- */
/* Flash status register */
#define STM32_FLASH_SR 0x10
/* The Flash status register gives information on ongoing program and 
   erase operations. */

#define FLASH_PEMPTY (1 << 17)
/* Set by hardware on power-on reset or after OBL_LAUNCH command execution
if the Flash is not programmed and the user intends to boot from the main Flash.
Cleared by hardware on power-on reset or after OBL_LAUNCH command
execution if the Flash is programmed and the user intends to boot from main
Flash. This bit can also be set and cleared by software.
1: The bit value is toggling
0: No effect
This bit can be set to clear the Program Empty bit if an OBL_LAUNCH is done by
software after Flash programming (boot in main flash selected). It finally forces
the boot in the main flash, without loosing the debugger connection.
*/

#define FLASH_BSY (1 << 16)
/* Busy. This bit indicates that a Flash memory operation is in progress. 
   It is set at the beginning of a Flash memory operation and cleared 
   when the operation finishes or an error occurs.
	0: no Flash memory operation ongoing
	1: Flash memory operation ongoing */

#define FLASH_OPTVERR (1 << 15)
/* Option validity error
   Set by hardware when the options read may not be the one configured by the
   user. If option haven’t been properly loaded, OPTVERR is set again after each
   system reset.
   Cleared by writing 1.
   */

#define FLASH_RDERR (1 << 14)
/* PCROP read error
   Set by hardware when an address to be read through the D-bus belongs to a
   read protected area of the flash (PCROP protection). An interrupt is generated if
   RDERRIE is set in FLASH_CR.
   Cleared by writing 1.
   */

#define FLASH_FASTERR (1 << 9)
/* Fast programming error
   Set by hardware when a fast programming sequence (activated by FSTPG) is
   interrupted due to an error (alignment, size, write protection or data miss). The
   corresponding status bit (PGAERR, SIZERR, WRPERR or MISSERR) is set at
   the same time.
   Cleared by writing 1.
   */

#define FLASH_MISERR (1 << 9)
/* Fast programming data miss error
In Fast programming mode, 32 double words must be sent to flash successively,
   and the new data must be sent to the flash logic control before the current data is
   fully programmed. MISSERR is set by hardware when the new data is not
   present in time.
   Cleared by writing 1.
   */

#define FLASH_PGSERR (1 << 7)
/* Programming sequence error. Set by hardware when a write access to 
   the Flash memory is performed by the code while the control register 
   has not been correctly configured.
   Cleared by writing 1.*/

#define FLASH_PGSIZERR (1 << 6)
/* Set by hardware when the size of the access is a byte or half-word during a
   program or a fast program sequence. Only double word programming is allowed
   (consequently: word access).
   Cleared by writing 1.
   */

#define FLASH_PGAERR (1 << 5)
/* Programming alignment error. Set by hardware when the data to program 
   cannot be contained in the same 128-bit Flash memory row.
   Cleared by writing 1.*/

#define FLASH_WRPERR (1 << 4)
/* Write protection error. Set by hardware when an address to be 
   erased/programmed belongs to a write-protected part of the Flash memory.
   Cleared by writing 1.*/

#define FLASH_PROGERR (1 << 3)
/* : Programming error
Set by hardware when a double-word address to be programmed contains a
value different from '0xFFFF FFFF' before programming, except if the data to
write is '0x0000 0000'.
Cleared by writing 1 */

#define FLASH_OPERR (1 << 1)
/* Operation error. Set by hardware when a flash operation 
   (programming / erase) request is detected and can not be run because 
   of parallelism, alignment, sequence or write protection error. This bit 
   is set only if error interrupts are enabled (ERRIE = 1).*/

#define FLASH_EOP (1 << 0)
/* End of operation. Set by hardware when one or more Flash memory 
   operations (program/erase) has/have completed successfully. 
   It is set only if the end of operation interrupts are 
   enabled (EOPIE = 1).
   Cleared by writing a 1. */

/* ------------------------------------------------------------------------- */
/* Flash control register */
#define STM32_FLASH_CR 0x14
/* The Flash control register is used to configure and start 
   Flash memory operations. */

#define FLASH_LOCK (1 << 31)
/* Lock. Write to 1 only. When it is set, this bit indicates that the 
   FLASH_CR register is locked. It is cleared by hardware after detecting 
   the unlock sequence. In the event of an unsuccessful unlock operation, 
   this bit remains set until the next reset. */

#define FLASH_OPTLOCK (1 << 30)
/* Option lock. Write to 1 only. When this bit is set, it indicates that 
   the FLASH_OPTCR register is locked. This bit is cleared by hardware 
   after detecting the unlock sequence. In the event of an unsuccessful 
   unlock operation, this bit remains set until the next reset. */

#define FLASH_OBL_LAUNCH (1 << 27)

/*
OBL_LAUNCH: Force the option byte loading
When set to 1, this bit forces the option byte reloading. This bit is cleared only
when the option byte loading is complete. It cannot be written if OPTLOCK is set.
0: Option byte loading complete
1: Option byte loading requested
*/

#define FLASH_RDERRIE (1 << 26)

#define FLASH_ERRIE (1 << 25)
/* Error interrupt enable. This bit enables the interrupt generation when 
   the OPERR bit in the FLASH_SR register is set to 1.
0: Error interrupt generation disabled
1: Error interrupt generation enabled */

#define FLASH_EOPIE (1 << 24)
/* End of operation interrupt enable. This bit enables the interrupt 
   generation when the EOP bit in the FLASH_SR register goes to 1.
0: Interrupt generation disabled
1: Interrupt generation enabled */

/* Bit 18 : Fast programming */
#define FLASH_FSTPG (1 << 18)
/* 
0: Fast programming disabled
1: Fast programming enabled
*/

/*Bit 17 : Options modification start */
#define FLASH_OPTSTRT (1 << 16)
/* This bit triggers an options operation when set.
This bit is set only by software, and is cleared when the BSY bit is cleared in
FLASH_SR.
*/

/*Bit 16 START: Start */
#define FLASH_STRT (1 << 16)
/* This bit triggers an erase operation when set. If MER1, MER2 and PER bits are
reset and the STRT bit is set, an unpredictable behavior may occur without
generating any error flag. This condition should be forbidden.
This bit is set only by software, and is cleared when the BSY bit is cleared in
FLASH_SR.
*/

/*Bits 15:11 Reserved, must be kept at reset value.
*/

/*Bits 10:3 Page number selection */
#define FLASH_PNB(N) (((N) & 0xff) << 3)
/* These bits select the page to erase:
00000000: page 0
00000001: page 1
...
11111111: page 255
Note: Bit 10 is used on STM32L45x and STM32L46x devices only. */

#define FLASH_MER1 (1 << 2)
/*Bit 2 MER1: Mass erase
This bit triggers the mass erase (all user pages) when set.
*/

#define FLASH_PER (1 << 1)
/* Page erase
0: page erase disabled
1: page erase enabled
*/

#define FLASH_PG (1 << 0)
/* Programming
0: Flash programming disabled
1: Flash programming enabled
*/

#ifndef __ASSEMBLER__

#include <stdint.h>

struct stm32_flash {
	volatile uint32_t acr;      
	volatile uint32_t pdkeyr;
	volatile uint32_t keyr;
	volatile uint32_t optkeyr;

	volatile uint32_t sr;
	volatile uint32_t cr;   
	volatile uint32_t eccr;   
	uint32_t res;   

	volatile uint32_t optr;
	volatile uint32_t pcrop1sr;
	volatile uint32_t pcrop1er;
	volatile uint32_t wrp1ar;

	volatile uint32_t wrp1br;
};

#endif /* __ASSEMBLER__ */

#endif /* __STM32L4_FLASH_H__ */

