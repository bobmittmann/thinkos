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

OUTPUT_FORMAT ("elf32-littlearm")
ENTRY(_reset)
SEARCH_DIR(.)

__code_init = ORIGIN(CODE);
__data_init = ORIGIN(DATA);

__krn_data_init = ORIGIN(DATA);
__krn_data_end = ORIGIN(DATA) + __krn_data_size;

__krn_code_init = ORIGIN(CODE);
__krn_code_end = ORIGIN(CODE) + __krn_code_size;

_stack_top = ORIGIN(STACK) + LENGTH(STACK);
_stack = _stack_top - DEFINED(ice_trace_init) * 0x50;
_stack_limit = ORIGIN(STACK);
ice_comm_blk = _stack_top - DEFINED(ice_trace_init) * 0x50;
__heap_end = ORIGIN(HEAP) + LENGTH(HEAP);

__firmware_end = ORIGIN(CODE) + LENGTH(CODE);


EXTERN(_stack)
EXTERN(_reset)
EXTERN(ice_comm_blk)

EXTERN(arm_nmi_isr)
EXTERN(arm_hard_fault_isr)
EXTERN(arm_mem_manage_isr)
EXTERN(arm_bus_fault_isr)
EXTERN(arm_usage_fault_isr)
EXTERN(arm_svc_isr)
EXTERN(arm_debug_mon_isr)
EXTERN(arm_pendsv_isr)
EXTERN(arm_systick_isr)

SECTIONS
{
	. = 0;

	.dcclog :
	{
		. = ALIGN(4);
		KEEP(*(.dcclog))
	} >dcclog

	.dccdata :
	{
		. = ALIGN(4);
		KEEP(*(.rodata.__FUNCTION__*))
		KEEP(*(.dccdata))
	} >dcclog

	.init __code_init :
	{
		KEEP (*(.rom_vectors))
		KEEP (*(.boot))
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
		/* kernel traces */
		PROVIDE(__trace_krn_start = .);
		KEEP(*(SORT(.rodata.trace_krn.*)))
		PROVIDE(__trace_krn_end = .);

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
/*
	.image :
	{
		. = . + 500000;
		PROVIDE (__image_end = . );
	} >"CODE"
*/
	.bss :
	{
		/* define a global symbol marking the start of the .bss section */
		PROVIDE (__bss_start = . );
		/* all .bss sections  */
		*(.bss .bss*)
	} >"DATA"

	/* advance location counter to the next 32-bit boundary */
	. = ALIGN(4);
	PROVIDE (__bss_end = . );

	.krn_stack :
	{
		. = ALIGN(4);
		PROVIDE (__krn_stack_start = . );
		/* all .krn_stack sections  */
		*(.krn_stack .krn_stack*)
		/* define a global symbol marking the end of the .krn_stack section  */
		. = ALIGN(4);
		PROVIDE (__krn_stack_end = . );
	} >"DATA"

	.heap :
	{
		PROVIDE (__heap_start = . );
		/* all .heap sections  */
		*(.heap .heap*)
	} >"DATA"

	/* advance location counter to the next 32-bit boundary */
	. = ALIGN(4);
	PROVIDE (__heap_base = . );

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

