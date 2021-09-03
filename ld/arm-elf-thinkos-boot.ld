/* 
 * arm-elf-thinkos-boot.ld
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

__krn_data_start = ORIGIN(DATA);
__krn_data_end = ORIGIN(DATA) + __krn_data_size;

__krn_code_start = ORIGIN(CODE);
__krn_code_end = ORIGIN(CODE) + __krn_code_size;

_stack_top = ORIGIN(STACK) + LENGTH(STACK);
__krn_stack_end = _stack_top - DEFINED(ice_trace_init) * 0x50;
__krn_stack_start = ORIGIN(STACK);
__krn_stack_size = __krn_stack_end - __krn_stack_start;
_stack = __krn_stack_end;

__heap_start = __krn_data_end;
__heap_end = ORIGIN(HEAP) + LENGTH(HEAP);

__firmware_end = ORIGIN(CODE) + LENGTH(CODE);

ice_comm_blk = _stack_top - DEFINED(ice_trace_init) * 0x50;

EXTERN(_reset)
EXTERN(ice_comm_blk)
EXTERN(cm3_nmi_isr)
EXTERN(cm3_hard_fault_isr)
EXTERN(cm3_mem_manage_isr)
EXTERN(cm3_bus_fault_isr)
EXTERN(cm3_usage_fault_isr)
EXTERN(cm3_svc_isr)
EXTERN(cm3_debug_mon_isr)
EXTERN(cm3_pendsv_isr)
EXTERN(cm3_systick_isr)

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

	.vect :
	{
		. = ALIGN(4);
		PROVIDE (__text_start = .);
		PROVIDE (__vcts_start = .);
		KEEP (*(.core_vectors))
		KEEP (*(.nvic_vectors))
		PROVIDE (__vcts_end = .);
	} >"CODE" = 0xffffffff

	PROVIDE(__rom_vectors_size = SIZEOF(.vect));

	.init :
	{
		. = ALIGN(4);
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
		*(.rodata.* .gnu.linkonce.r.*)
		*(.rodata1)

		. = ALIGN(3);
		/* constructors */
		PROVIDE(__ctors_start = .);
		KEEP (*(SORT(.init_array.*))) /* gcc 4.6 */
		KEEP (*(.init_array))
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		PROVIDE(__ctors_end = .);

		. = ALIGN(3);
		/* kernel traces */
		PROVIDE(__trace_krn_start = .);
		KEEP(*(SORT(.rodata.trace_krn.*)))
		PROVIDE(__trace_krn_end = .);

		. = ALIGN(3);
		/* user traces */
		PROVIDE(__trace_usr_start = .);
		KEEP(*(SORT(.rodata.trace.*)))
		PROVIDE(__trace_usr_end = .);

	} >"CODE" = 0xffffffff

	. = ALIGN(3);
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

	. = ALIGN(3);

	.data :
	{
		/* create a global symbol marking the start of the .data section  */
		PROVIDE (__data_start = . );
		/* all .data sections  */
		*(.data .data*)
		/* define a global symbol marking the end of the .data section  */
	} >"DATA" AT >"CODE"

	. = ALIGN(3);
	PROVIDE (__data_end = . );
	/* define a global symbol marking the start of the .bss section */
	PROVIDE (__bss_start = . );

	.bss :
	{
		/* all .bss sections  */
		*(.bss .bss*)
	} >"DATA"

	/* advance location counter to the next 32-bit boundary */
	. = ALIGN(3);
	PROVIDE (__bss_end = . );

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

	PROVIDE(__image_size = SIZEOF(.init) + SIZEOF(.text) + SIZEOF(.data));

}

