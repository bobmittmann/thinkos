/* 
 * File:	stm32l4-dma.c
 * Author:  Robinson Mittmann (bobmittmann@gmail.com)
 * Target:  ThinkOS
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

#ifdef CONFIG_H
#include "config.h"
#endif

#include <sys/stm32f.h>

#if (THINKOS_DMA_MAX > 0)

void stm32l4_dma_isr(unsigned int irq) 
{
#if (THINKOS_ENABLE_IRQ_CYCCNT)
	/* set the thread's return value to cyle count */
	uint32_t cyccnt = CM3_DWT->cyccnt;
#endif
	struct stm32f_dma *dma = STM32F_DMA2;
	unsigned int th;
	uint32_t isr;

	dma = STM32F_DMA2;
	isr = dma->isr;
	if (isr & DMA_TEIF1) {
		/* clear the DMA transfer error flag */
		dma->ifcr = DMA_CTEIF1;
	} else {
		if ((isr & (DMA_TCIF1 | DMA_CHTIF1)) != 0) {
			/* clear the DMA transfer complete interrupt flags */
			dma->ifcr = DMA_TCIF1 | DMA_CHTIF1;

			th = thinkos_rt.irq_th[irq];
			thinkos_rt.irq_th[irq] = THINKOS_THREAD_IDLE;

			/* insert the thread into ready queue */
			__bit_mem_wr(&thinkos_rt.wq_ready, th, 1);

#if (THINKOS_ENABLE_IRQ_CYCCNT)
			/* set the thread's return value */
			thinkos_rt.ctx[th]->r1 = cyccnt;
#endif
		}
	}

	/* disable this interrupt source */
	cm3_irq_disable(irq);

#if (THINKOS_ENABLE_WQ_DMA)
	/* remove from the wait queue */
	__thinkos_wq_remove(THINKOS_WQ_DMA, th);  
#endif
	/* signal the scheduler ... */
	__thinkos_preempt();
}

