/* 
 * stm32f4xx.ld
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You can receive a copy of the GNU Lesser General Public License from 
 * http://www.gnu.org/
 */

__code_init = DEFINED(__ldcode) ? __ldcode : ORIGIN(CODE);
__data_init = DEFINED(__lddata) ? __lddata : ORIGIN(DATA);

EXTERN(_reset)
ENTRY(_reset)

SECTIONS
{
	. = 0;

	.init __code_init :
	{
		KEEP (*(.init))
	} >"CODE" = 0xffffffff

	.text :
	{
		/* all .text sections (code)  */
		*(.text .text.* .gnu.linkonce.t.*)
		/* all .glue_7t sections (interworking thumb helpers) */
		*(.glue_7) *(.glue_7t) *(.vfp11_veneer) *(.v4_bx)
		/* all .rodata sections (constants, strings, etc.)  */
		*(.rodata)
		*(.rodata.__func__.*)

		. = ALIGN(4);
		/* user traces */
		PROVIDE(__trace_usr_start = .);
		KEEP(*(SORT(.rodata.trace.*)))
		PROVIDE(__trace_usr_end = .);

		*(.rodata.* .gnu.linkonce.r.*)
		*(.rodata1)

		. = ALIGN(4);
		/* constructors */
		PROVIDE(__ctors_start = .);
		KEEP (*(SORT(.init_array.*))) /* gcc 4.6 */
		KEEP (*(.init_array))
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		PROVIDE(__ctors_end = .);
		. = ALIGN(8);
	} >"CODE" = 0xffffffff

	.ARM.extab : 
	{ 
		*(.ARM.extab* .gnu.linkonce.armextab.*) 
	} >"CODE" = 0xffffffff
	PROVIDE_HIDDEN (__exidx_start = .);

	.ARM.exidx : 
	{ 
		*(.ARM.exidx* .gnu.linkonce.armexidx.*) 
	} >"CODE" = 0xffffffff
	PROVIDE_HIDDEN (__exidx_end = .);

	/* define a global symbol marking the end of read only data sections */
	PROVIDE(__text_end = .);

	.data __data_init :
	{
		. = ALIGN(4);
		/* create a global symbol marking the start of the .data section  */
		PROVIDE (__data_start = . );
		KEEP (*(.ram_vectors))
		/* all .data sections  */
		*(.data .data*)
		/* define a global symbol marking the end of the .data section  */
		. = ALIGN(4);
		PROVIDE (__data_end = . );
	} >"DATA" AT >"CODE"

	PROVIDE(__image_size = SIZEOF(.init) + SIZEOF(.text) + SIZEOF(.data));

	.bss __data_end :
	{
		/* define a global symbol marking the start of the .bss section */
		PROVIDE (__bss_start = . );
		/* all .bss sections  */
		*(.bss .bss*)
	} >"DATA"

	/* advance location counter to the next 32-bit boundary */
	. = ALIGN(4);
	PROVIDE (__bss_end = . );

	.heap :
	{
		PROVIDE (__heap_start = . );
		/* all .heap sections  */
		*(.heap .heap*)
	} >"DATA"

	/* advance location counter to the next 32-bit boundary */
	. = ALIGN(4);
	PROVIDE (__heap_base = . );

	.stack :
	{
		PROVIDE (__stack_start = . );
		/* all .stack sections  */
		*(.stack .stack*)
	} >"STACK"

	/* Backed up control section */
	.bkpctl :
	{
		PROVIDE (__bkpctl_start = . );
		*(.bkpctl .bkpctl*)
		PROVIDE (__bkpctl_end = . );
	} >"BKPCTL"

	/* Backed up data section */
	.bkpdat :
	{
		PROVIDE (__bkpdat_start = . );
		*(.bkpdat .bkpdat*)
		PROVIDE (__bkpdat_end = . );
	} >"BKPDAT"

}

