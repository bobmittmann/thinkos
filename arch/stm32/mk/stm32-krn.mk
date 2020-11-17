#
# stm32-krn.mk 
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

__THISDIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
__ARCH_DIR := $(abspath $(__THISDIR)/..)
__THINKOS_DIR := $(abspath $(__ARCH_DIR)/../..)

# Load the thinkos configuration script
include $(__THINKOS_DIR)/mk/config.mk
# Load the architecture configuration script
include $(__ARCH_DIR)/mk/stm32-cfg.mk

LDFLAGS += -Wl,--gc-sections -nostdlib 

OPTIONS	+= -g -mcpu=$(CPU) -mthumb -mthumb-interwork -mno-unaligned-access

WARN = -Wextra -Werror -Wpedantic 
NOWARN = -Wno-sign-compare -Wno-unused-parameter

CFLAGS += -Wall $(WARN) $(NOWARN) -fno-builtin -ffreestanding -fomit-frame-pointer -ffunction-sections -fdata-sections 

ifeq ($(CPU), cortex-m4)
  OPTIONS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard 
endif

ifdef RAM_VECTORS
  CDEFS += CM3_RAM_VECTORS
  BOOTLD = arm-elf-thinkos-ramvec.ld
else
  BOOTLD = arm-elf-thinkos-boot.ld
endif

ifdef KRN_DATA_SIZE
  SYMDEFS += __krn_data_size=$(KRN_DATA_SIZE)
endif

ifdef KRN_CODE_SIZE
  SYMDEFS += __krn_code_size=$(KRN_CODE_SIZE)
endif

ifndef KRN_CODE
  KRN_CODE := $(STM32_KRN_CODE)
endif

ifndef KRN_DATA
  KRN_DATA := $(STM32_KRN_DATA)
endif

ifndef LOAD_ADDR
  LOAD_ADDR := $(KRN_CODE)
endif

ifdef LDSCRIPT
  LDFLAGS += -T $(LDSCRIPT)
else
  LDFLAGS += -T $(MACH).ld -T $(STM32)-vec.ld -T $(BOOTLD)
endif

all: prog-all

clean: clean-all

include $(__THINKOS_DIR)/mk/prog.mk

ifdef VERSION_MAJOR
  include $(__THINKOS_DIR)/mk/version.mk
endif

include $(__THINKOS_DIR)/mk/jtag.mk

