/* 
 * stm324xx.ld
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

_stack_top = ORIGIN(STACK) + LENGTH(STACK);
_stack = _stack_top - DEFINED(ice_trace_init) * 0x50;
_stack_limit = ORIGIN(STACK);
ice_comm_blk = _stack_top - DEFINED(ice_trace_init) * 0x50;
__heap_end = _stack;

EXTERN(_stack)
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

EXTERN(stm32_wwdg_isr)
EXTERN(stm32_pvd_isr)
EXTERN(stm32_tamp_stamp_isr)
EXTERN(stm32_rtc_wkup_isr)
EXTERN(stm32_flash_isr)
EXTERN(stm32_rcc_isr)
EXTERN(stm32_exti0_isr)
EXTERN(stm32_exti1_isr)
EXTERN(stm32_exti2_isr)
EXTERN(stm32_exti3_isr)
EXTERN(stm32_exti4_isr)

EXTERN(stm32_dma1_channel1_isr)
EXTERN(stm32_dma1_channel2_isr)
EXTERN(stm32_dma1_channel3_isr)
EXTERN(stm32_dma1_channel4_isr)
EXTERN(stm32_dma1_channel5_isr)

EXTERN(stm32_dma1_channel6_isr)
EXTERN(stm32_dma1_channel7_isr)


EXTERN(stm32_adc1_isr)
EXTERN(stm32_can1_tx_isr)
EXTERN(stm32_can1_rx0_isr)
EXTERN(stm32_can1_rx1_isr)
EXTERN(stm32_can1_sce_isr)
EXTERN(stm32_exti9_5_isr)

EXTERN(stm32_tim1_brk_tim15_isr)
EXTERN(stm32_tim1_up_tim16_isr)
EXTERN(stm32_tim1_trg_com_isr)
EXTERN(stm32_tim1_cc_isr)
EXTERN(stm32_tim2_isr)
EXTERN(stm32_tim3_isr)

EXTERN(stm32_i2c1_ev_isr)
EXTERN(stm32_i2c1_er_isr)
EXTERN(stm32_i2c2_ev_isr)
EXTERN(stm32_i2c2_er_isr)

EXTERN(stm32_spi1_isr)
EXTERN(stm32_spi2_isr)
EXTERN(stm32_usart1_isr)
EXTERN(stm32_usart2_isr)
EXTERN(stm32_usart3_isr)
EXTERN(stm32_exti15_10_isr)
EXTERN(stm32_rtc_alarm_isr)

EXTERN(stm32_sdmcc1_isr)

EXTERN(stm32_spi3_isr)

EXTERN(stm32_spi3_isr)
EXTERN(stm32_uart4_isr)

EXTERN(stm32_tim6_dac_isr)
EXTERN(stm32_tim7_isr)
EXTERN(stm32_dma2_channel1_isr)
EXTERN(stm32_dma2_channel2_isr)
EXTERN(stm32_dma2_channel3_isr)
EXTERN(stm32_dma2_channel4_isr)
EXTERN(stm32_dma2_channel5_isr)
EXTERN(stm32_dfsdm1_flt0_isr)
EXTERN(stm32_dfsdm1_flt1_isr)

EXTERN(stm32_comp_isr)

EXTERN(stm32_lptim1_isr)
EXTERN(stm32_lptim2_isr)
EXTERN(stm32_usb_fs_isr)

EXTERN(stm32_dma1_channel6_isr)
EXTERN(stm32_dma1_channel7_isr)

EXTERN(stm32_lpuart1_isr)
EXTERN(stm32_quadspi_isr)

EXTERN(stm32_i2c3_ev_isr)
EXTERN(stm32_i2c3_er_isr)

EXTERN(stm32_sai1r_isr)

EXTERN(stm32_tsc_isr)
EXTERN(stm32_lcd_isr)
EXTERN(stm32_aes_isr)
EXTERN(stm32_rng_isr)
EXTERN(stm32_fpu_isr)
EXTERN(stm32_crs_isr)
EXTERN(stm32_i2c4_ev_isr)
EXTERN(stm32_i2c4_er_isr)

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
		*(.rodata .rodata.* .gnu.linkonce.r.*)
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

	.data :
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

	.heap :
	{
		PROVIDE (__heap_start = . );
		/* all .heap sections  */
		*(.heap .heap*)
	} >"DATA"

	/* advance location counter to the next 32-bit boundary */
	. = ALIGN(4);
	PROVIDE (__heap_base = . );

   .sram2 :
    {
        /* define a global symbol marking the start of the .sram2 section */
        PROVIDE (__sram2_start = . );
        PROVIDE (__heap_end = . );
        *(.sram2 .sram2*)
        *(.stack .stack*)
    } >"STACK"

   /* Backed up control section */
    .bkpctl :
    {
        PROVIDE (__bkpctl_start = . );
        *(.bkpctl .bkpctl*)
        PROVIDE (__bkpctl_end = . );
    } >"rtcbkr"


    /* Backed up data section */
    .bkpdat :
    {
        PROVIDE (__bkpdat_start = . );
        *(.bkpdat .bkpdat*)
        PROVIDE (__bkpdat_end = . );
    } >"DATA"

}

