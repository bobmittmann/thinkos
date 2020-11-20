#
# stm32-app.mk 
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

ifndef PROG 
  $(error PROG undefined!)
endif

# Load the thinkos configuration script
include $(__THINKOS_DIR)/mk/config.mk
# Load the architecture configuration script
include $(__ARCH_DIR)/mk/stm32-cfg.mk

LDFLAGS += -Wl,--gc-sections -nostdlib -lgcc

OPTIONS	+= -g -mcpu=$(CPU) -mthumb -mthumb-interwork -mno-unaligned-access

WARN = -Wextra -Werror -Wdouble-promotion
NOWARN = -Wno-sign-compare -Wno-unused-parameter

CFLAGS += -Wall $(WARN) $(NOWARN) -fno-builtin -ffreestanding -fomit-frame-pointer -ffunction-sections -fdata-sections 

ifeq ($(CPU), cortex-m4)
  OPTIONS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard 
endif

ifdef CODE
    APP_CODE := $(CODE)
else 
  ifdef LDCODE
    APP_CODE := $(LDCODE)
  else 
    APP_CODE := $(STM32_APP_CODE)
  endif
endif

ifdef DATA
    APP_DATA := $(DATA)
else 
  ifdef LDDATA
    APP_DATA := $(LDDATA)
  else 
    APP_DATA := $(STM32_APP_DATA)
  endif
endif

ifndef LOAD_ADDR
  LOAD_ADDR := $(APP_CODE)
endif

# Add symbols to the linker to adjust the application's memory offsets
SYMDEFS += __lddata=$(APP_DATA) __ldcode=$(APP_CODE)

# Add a macro to the C compiler to indicate this is an application
CDEFS += THINKAPP

ifndef THINKAPP
  THINKAPP = "ThinkOSApp"
endif

ifeq ($(CPU), cortex-m4)
  OPTIONS += -mfpu=fpv4-sp-d16 -mfloat-abi=hard 
endif

ifdef LDSCRIPT
  LDFLAGS += -T $(LDSCRIPT) -lgcc
else
  LDFLAGS += -T $(MACH).ld -T arm-elf-thinkos-app.ld
endif

all: prog-all app

clean: clean-all app-clean 

include $(__THINKOS_DIR)/mk/prog.mk

include $(__THINKOS_DIR)/mk/app.mk

ifdef VERSION_MAJOR
  include $(__THINKOS_DIR)/mk/version.mk
endif

include $(__THINKOS_DIR)/mk/jtag.mk

