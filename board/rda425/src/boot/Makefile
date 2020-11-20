# File:		Makefile
# Module: 
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

VERSION_DATE = "2020-11-18"
VERSION_MAJOR = "0"
VERSION_MINOR = "4"

THINKOS_DIR = ../../../..
SDK_DIR = $(THINKOS_DIR)/sdk
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
TOOLS_DIR = $(THINKOS_DIR)/tools
BIN2HEX = python $(TOOLS_DIR)/bin2hex.py

PROG = rda425-thinkos-$(VERSION_MAJOR).$(VERSION_MINOR)

BOOT_CFILES = boot.c monitor.c standby.c

CFILES = $(addprefix $(THINKOS_DIR)/boot/,$(BOOT_CFILES)) \
		 board.c otg_xflash_pic.c 

LIBS = stm32f util thinkos-krn thinkos-mon xmodem ice-comm

SDK_LIBS = util thinkos-krn thinkos-mon xmodem ice-comm
ARCH_LIBS = stm32f 
LIBS = $(SDK_LIBS) $(ARCH_LIBS)

LIBDIRS = $(addprefix $(SDK_DIR)/lib,$(SDK_LIBS)) \
		  $(addprefix $(ARCH_DIR)/lib,$(ARCH_LIBS))

INCPATH = $(SDK_DIR)/include ../include

# 
# PB_874_R03 is the current default hardware revision. If no HW variable is
# set then the macro PB_874_R03 will be set.
#
ifdef HWREV
  hardware := $(HWREV)
else
  hardware := PB_874_R03
endif

CDEFS := CONFIG_H $(hardware)

CFLAGS = -Os

MACH = stm32f446xe
KRN_CODE_SIZE = 65536
KRN_DATA_SIZE = 4096
include $(ARCH_DIR)/mk/stm32-krn.mk

