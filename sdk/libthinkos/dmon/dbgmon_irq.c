/* 
 * File:	 dmon_irq.c
 * Author:   Robinson Mittmann (bobmittmann@gmail.com)
 * Target:
 * Comment:
 * Copyright(C) 2011 Bob Mittmann. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#define __THINKOS_DMON__
#include <thinkos_dmon.h>
#include <thinkos.h>
#include <sys/dcclog.h>

/**
  * __dmon_irq_force_enable:
  *
  * Enable interrupts listed in the IRQ forced enable list.
  */
void __dmon_irq_force_enable(void)
{
	int cnt = 0;

	while (cnt < 4) {
		int irq;
		int i;
		int k;

		/* get the next irq from the list */
		irq = thinkos_dmon_rt.irq_en_lst[cnt++];
		i = irq / 32;
		k = irq % 32;

		thinkos_dmon_rt.nvic_ie[i] |= (1 << k);
		CM3_NVIC->iser[i] = (1 << k);
	}
}

/**
  * __dmon_irq_disable_all:
  *
  * Disable all interrupts by clearing the interrupt enable bit
  * of all interrupts on the Nested Vector Interrupt Controller (NVIC).
  *
  * Also the interrupt enable backup is cleared to avoid 
  * interrupts being reenabled by calling __dmon_irq_restore_all().
  *
  * The systick interrupt is not disabled.
  */
void __dmon_irq_disable_all(void)
{
	int i;

	for (i = 0; i < NVIC_IRQ_REGS; ++i) {
		thinkos_dmon_rt.nvic_ie[i] = 0;
		CM3_NVIC->icer[i] = 0xffffff; /* disable interrupts */
	}
}

/**
  * __dmon_irq_pause_all:
  *
  * Save the state of the interrupt enable registers and 
  * disable all interrupts.
  */
void __dmon_irq_pause_all(void)
{
	int i;

	for (i = 0; i < NVIC_IRQ_REGS; ++i) {
		/* save interrupt state */
		thinkos_dmon_rt.nvic_ie[i] = CM3_NVIC->iser[i];
		CM3_NVIC->icer[i] = 0xffffffff; /* disable all interrupts */
	}
}

/**
  * __dmon_irq_restore_all:
  *
  * Restore the state of the interrupt enable registers.
  */
void __dmon_irq_restore_all(void)
{
	int i;

	for (i = 0; i < NVIC_IRQ_REGS; ++i) {
		/* restore interrupt state */
		CM3_NVIC->iser[i] = thinkos_dmon_rt.nvic_ie[i];
	}
}

/**
  * __dmon_irq_init:
  *
  * Initialize the IRQ track subsystem
  */
void __dmon_irq_init(void)
{
	int cnt;
	int i;
	
	cnt = 0;
	for (i = 0; i < NVIC_IRQ_REGS; ++i) {
		uint32_t mask;
		int k;

		mask = CM3_NVIC->iser[i];
		/* save the current interrupts */
		thinkos_dmon_rt.nvic_ie[i] = mask;
		DCC_LOG1(LOG_INFO, "nvic.iser[i]=0x%08x.", mask);

		/* probe interrupts */
		while ((cnt < 4) && (mask != 0)) {
			int irq;

			k = __clz(__rbit(mask));
			mask &= ~(1 << k);

			irq = (i * 32) + k;
			thinkos_dmon_rt.irq_en_lst[cnt++] = irq;

			DCC_LOG1(LOG_TRACE, "IRQ %d always enabled.", irq);
		}	
	}

	while (cnt < 4) {
		thinkos_dmon_rt.irq_en_lst[cnt++] = 0xff;
	}	
}

#if (THINKOS_ENABLE_RESET_RAM_VECTORS)

/**
 * __reset_ram_vectors:
 *
 * Copy the default values for the IRQ vectors from the flash into RAM. 
 * 
 * When the a new application replaces the existing one through the GDB
 * or Ymodem some interrupts can be fired due to wrong sequencig of
 * interrupt programming in the application. To avoid potential system
 * crashes the vectors should be initialized to a default value.
 *
 */

void __reset_ram_vectors(void)
{
	/* XXX: 
	   this function assumes the excpetion vectors defaults to be located 
	   just after the .text section! */
	extern unsigned int __text_end;
	extern unsigned int __ram_vectors;
	extern unsigned int __sizeof_ram_vectors;

	unsigned int size = __sizeof_ram_vectors;
	void * src = &__text_end;
	void * dst = &__ram_vectors;

	DCC_LOG3(LOG_TRACE, "dst=%08x src=%08x size=%d", dst, src, size); 
	__thinkos_memcpy32(dst, src, size); 
}
#endif

