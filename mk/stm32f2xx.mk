#
# stm32f2xx.mk 
#
# Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
# 
# This file is part of the YARD-ICE.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3.0 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You can receive a copy of the GNU Lesser General Public License from 
# http://www.gnu.org/

THISDIR := $(dir $(lastword $(MAKEFILE_LIST)))

include $(THISDIR)/config.mk

ifndef MACH 
MACH = stm32f207
endif

ifeq ($(findstring $(MACH), stm32f205, stm32f207),)
  $(error "Unsupported machine type: MACH=$(MACH)")
endif

ARCH = cm3
CPU = cortex-m3
STM32 = stm32f

export STM32

CDEFS += $(call uc,$(MACH))
ifdef HCLK_HZ
CDEFS += "HCLK_HZ=$(HCLK_HZ)" 
endif
ifdef HSE_HZ
CDEFS += "HSE_HZ=$(HSE_HZ)" 
endif

CROSS_COMPILE = arm-none-eabi-

OPTIONS	+= -mcpu=$(CPU) -mthumb -mthumb-interwork 

ifndef APPADDR
APPADDR := 0x08020000
endif

ifdef THINKAPP
CDEFS += THINKAPP
SYMDEFS += __thinkapp=$(APPADDR)
else
OPTIONS += -mno-unaligned-access
endif

ifdef LDSCRIPT
LDFLAGS += -nostdlib -T $(LDSCRIPT)
else
LDFLAGS += -nostdlib -T $(MACH).ld
endif

ifndef LOAD_ADDR
LOAD_ADDR := $(APPADDR)
endif

include $(THISDIR)/prog.mk

include $(THISDIR)/jtag.mk

