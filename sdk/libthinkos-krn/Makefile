# File:		Makefile
# Author:   Robinson Mittmann (bobmittmann@gmail.com)
# Target:
# Comment:  
# Copyright(C) 2012 Bob Mittmann. All Rights Reserved.
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 

include ../../mk/config.mk

LIB_STATIC = thinkos-krn

MISC_CFILES = thinkos_type.c obj_type_get.c \
			  alloc_bmp_get.c bmp_alloc.c strlen.c \
			  memcpy.c memcpy32.c memset32.c active.c suspended.c \
			  thread_getnext.c thread_get.c \
			  thread_isfaulty.c thread_ispaused.c \
			  thread_isalive.c pause_all.c resume_all.c xcpt_util.c \
			  cpu-info.c

CFILES = thinkos_core.c thinkos_svc.c thinkos_init.c \
		 thinkos_thread.c thinkos_mutex.c thinkos_cond.c \
		 thinkos_semaphore.c thinkos_event.c thinkos_irq.c \
		 thinkos_flag.c thinkos_except.c thinkos_debug.c \
		 thinkos_sleep.c thinkos_break.c thinkos_pause.c \
		 thinkos_exit.c thinkos_join.c thinkos_cancel.c \
		 thinkos_gate.c thinkos_ctl.c thinkos_idle.c thinkos_mpu.c \
		 thinkos_systmr.c thinkos_ffs.c \
		 thinkos_profile.c thinkos_error.c thinkos_memory.c \
		 thinkos_dma.c thinkos_alloc.c thinkos_flash_drv.c \
		 thinkos_krnsvc.c \
		 $(addprefix misc/,$(MISC_CFILES)) \

ifeq ($(CPU), cortex-m4)
  SFILES = thinkos_vector.S thinkos_isr.S \
		   cortex-m4f/thinkos_sched-cortex-m4f.S
#		   cortex-m4f/thinkos_sched-cortex-m4f-ndbg.S
endif

ifeq ($(CPU), cortex-m3)
  SFILES = thinkos_vector.S thinkos_isr.S \
		   cortex-m3/thinkos_sched-cortex-m3.S
#		   cortex-m3/thinkos_sched-cortex-m3-basic.S
endif

include ../../mk/lib.mk


