#
# stm32-cfg.mk 
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

# Architecture 
ARCH = stm32

# Kernel memory locations (common to all chips)
STM32_KRN_CODE = 0x08000000
STM32_KRN_DATA = 0x20000000

ifndef MACH 
  $(error "Missing MACH definition...")
endif

ifndef __ARCH_DIR
  $(error "Missing __ARCH_DIR definition...")
endif

ifndef __THINKOS_DIR
  $(error "Missing __THINKOS_DIR definition...")
endif

ifneq ($(findstring $(MACH), stm32f446xc stm32f446xe),)
  CPU = cortex-m4
  STM32 = stm32f446x
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifneq ($(findstring $(MACH), stm32f405xg stm32f407xg stm32f407xe),)
  CPU = cortex-m4
  STM32 = stm32f4x
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifneq ($(findstring $(MACH), stm32f427xg stm32f427xi),)
  CPU = cortex-m4
  STM32 = stm32f4x
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifneq ($(findstring $(MACH), stm32f429xg stm32f429xe stm32f429xi),)
  CPU = cortex-m4
  STM32 = stm32f4x
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifneq ($(findstring $(MACH), stm32f415xg),)
  CPU = cortex-m4
  STM32 = stm32f4x
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifneq ($(findstring $(MACH), stm32f302 stm32f303 stm32f303xb stm32f303xc),)
  CPU = cortex-m4
  STM32 = stm32f3
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifneq ($(findstring $(MACH), stm32f100 stm32f103 stm32f103x6),)
  CPU = cortex-m3
  STM32 = stm32f1
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifneq ($(findstring $(MACH), stm32f207xe stm32f207xg),)
  CPU = cortex-m3
  STM32 = stm32f2
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifneq ($(findstring $(MACH), stm32l151x6 stm32l151x8 stm32l151xb stm32l151xc),)
  CPU = cortex-m3
  STM32 = stm32l1
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifneq ($(findstring $(MACH), stm32l433xc stm32l433xb),)
  CPU = cortex-m4
  STM32 = stm32l4
  STM32_APP_CODE = 0x08010000
  STM32_APP_DATA = 0x20001000
endif

ifneq ($(findstring $(MACH), stm32f031x4 stm32f031x6),)
  CPU = cortex-m0
  STM32 = stm32f0
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifneq ($(findstring $(MACH), stm32f072x8 stm32f072xb),)
  CPU = cortex-m0
  STM32 = stm32f0
  STM32_APP_CODE = 0x08020000
  STM32_APP_DATA = 0x20002000
endif

ifndef CPU
  $(error "Unsupported machine type: MACH=$(MACH)")
endif

export MACH
export ARCH
export STM32
export CPU

CDEFS += $(call uc,$(MACH))

ifdef HCLK_HZ
  CDEFS += "STM32_HCLK_HZ=$(HCLK_HZ)" 
endif

ifdef HSE_HZ
  CDEFS += "STM32_HSE_HZ=$(HSE_HZ)" 
endif

CROSS_COMPILE = arm-none-eabi-

INCPATH += $(__ARCH_DIR)/include $(__THINKOS_DIR)/sdk/include

LDDIR := $(abspath $(THISDIR)/../ld)

LIBPATH += $(__ARCH_DIR)/ld $(__THINKOS_DIR)/ld

