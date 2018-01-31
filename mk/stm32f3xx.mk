#
# stm32f3xx.mk 
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

include $(THISDIR)config.mk

ifndef MACH 
MACH = stm32f303
endif

ifeq ($(findstring $(MACH), stm32f302 stm32f303 stm32f303xb stm32f303xc),)
  $(error "Unsupported machine type: MACH=$(MACH)")
endif

ARCH = cm3
CPU = cortex-m4
STM32 = stm32f3

export STM32

CDEFS += $(call uc,$(MACH))
ifdef HCLK_HZ
  CDEFS += "STM32_HCLK_HZ=$(HCLK_HZ)" 
endif
ifdef HSE_HZ
  CDEFS += "STM32_HSE_HZ=$(HSE_HZ)" 
endif

OPTIONS	= -mcpu=$(CPU) -mthumb -mthumb-interwork 
CROSS_COMPILE = arm-none-eabi-

ifdef THINKAPP
  CDEFS += THINKAPP
  SYMDEFS += __thinkapp=$(THINKAPP)
else
  OPTIONS += -mno-unaligned-access
endif

ifndef APPADDR
  APPADDR := 0x08010000
endif

ifdef LDSCRIPT
  LDFLAGS += -nostdlib -T $(LDSCRIPT)
else
  LDFLAGS += -nostdlib -T $(MACH).ld
endif

include $(THISDIR)/prog.mk

ifndef LOAD_ADDR
  LOAD_ADDR := $(APPADDR)
endif

include $(THISDIR)/jtag.mk

