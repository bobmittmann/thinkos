#
# stm32f103.mk 
#
# Copyright(C) 2013 Robinson Mittmann. All Rights Reserved.
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
MACH = stm32f103
endif

ifeq ($(findstring $(MACH), stm32f103 stm32f103x6),)
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
OPTIONS	= -mcpu=$(CPU) -mthumb -mthumb-interwork 
CROSS_COMPILE = arm-none-eabi-

ifdef LDSCRIPT
LDFLAGS += -nostdlib -T $(LDSCRIPT)
else
LDFLAGS += -nostdlib -T $(MACH).ld
endif

include $(THISDIR)/prog.mk

include $(THISDIR)/jtag.mk
