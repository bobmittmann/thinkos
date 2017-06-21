#
# stm32f4xx.mk 
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

ARCH = cm3

ifndef MACH 
  $(error "Missing MACH definition...")
endif

ifneq ($(findstring $(MACH), stm32f405 stm32f407 stm32f427 stm32f429),)
  CPU = cortex-m4
  STM32 = stm32f4
endif

ifneq ($(findstring $(MACH), stm32f446xc stm32f446xe),)
  CPU = cortex-m4
  STM32 = stm32f4
endif

ifneq ($(findstring $(MACH), stm32f302 stm32f303 stm32f303xb stm32f303xc),)
  CPU = cortex-m4
  STM32 = stm32f3
endif

ifneq ($(findstring $(MACH), stm32f100 stm32f103 stm32f103x6),)
  CPU = cortex-m3
  STM32 = stm32f1
endif

ifneq ($(findstring $(MACH), stm32f205 stm32f207),)
  CPU = cortex-m3
  STM32 = stm32f2
endif

ifneq ($(findstring $(MACH), stm32l151x6 stm32l151x8 stm32l151xb stm32l151xc),)
  CPU = cortex-m3
  STM32 = stm32l1
endif

ifneq ($(findstring $(MACH), stm32l433xc stm32l433xb),)
  CPU = cortex-m4
  STM32 = stm32l4
endif

ifndef CPU
  $(error "Unsupported machine type: MACH=$(MACH)")
endif

export STM32

CDEFS += $(call uc,$(MACH))
  ifdef HCLK_HZ
CDEFS += "STM32_HCLK_HZ=$(HCLK_HZ)" 
endif

ifdef HSE_HZ
  CDEFS += "STM32_HSE_HZ=$(HSE_HZ)" 
endif

CROSS_COMPILE = arm-none-eabi-

OPTIONS	+= -mcpu=$(CPU) -mthumb -mthumb-interwork 
ifeq ($(CPU), cortex-m4)
	OPTIONS	+= -mfpu=fpv4-sp-d16 -mfloat-abi=hard 
endif

ifdef THINKAPP
  CDEFS += THINKAPP
  SYMDEFS += __thinkapp=$(THINKAPP)
  ifndef APPADDR
    APPADDR := THINKAPP
  endif
else
  OPTIONS += -mno-unaligned-access
endif

ifndef APPADDR
  APPADDR := 0x08010000
endif

CFLAGS += -ffunction-sections -fdata-sections 

ifdef LDSCRIPT
  LDFLAGS += -Wl,--gc-sections -nostdlib -T $(LDSCRIPT)
else
  LDFLAGS += -Wl,--gc-sections -nostdlib -T $(MACH).ld
endif

include $(THISDIR)/prog.mk

ifndef LOAD_ADDR
  LOAD_ADDR := $(APPADDR)
endif

include $(THISDIR)/jtag.mk

