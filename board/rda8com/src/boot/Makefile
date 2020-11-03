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

PROJECT_ROOT = ../../
THINKOS_DIR = $(PROJECT_ROOT)/../..
SDK_DIR = $(THINKOS_DIR)/sdk
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
TOOLS_DIR = $(THINKOS_DIR)/tools
BIN2HEX = python $(TOOLS_DIR)/bin2hex.py
LZ77 = /c/opt/bin/lz77
LZ_WINDOW_SIZE = 32768

VERSION_DATE = "2020-05-18"
VERSION_MAJOR = "0"
VERSION_MINOR = "3"

PROG = rda8com-thinkos-$(VERSION_MAJOR).$(VERSION_MINOR)

CFILES = $(THINKOS_DIR)/boot/boot.c $(THINKOS_DIR)/boot/monitor.c\
		 $(THINKOS_DIR)/boot/standby.c board.c fpga.c ice40hx1k.c 

LIBDIRS = $(ARCH_DIR)/libstm32f $(SDK_DIR)/libcm3 $(SDK_DIR)/libutil \
		  $(SDK_DIR)/libthinkos-krn $(SDK_DIR)/libthinkos-dbg \
		  $(SDK_DIR)/libthinkos-mon \
		  $(SDK_DIR)/libgdbrsp $(SDK_DIR)/libice-comm 

LIBS = cm3 stm32f util thinkos-krn thinkos-mon thinkos-dbg gcc gdbrsp ice-comm 

INCPATH = $(SDK_DIR)/include ../include

CDEFS = CONFIG_H LZ_WINDOW_SIZE=$(LZ_WINDOW_SIZE) HWREV=0

CFLAGS = -Os -Werror -Wall -Wextra -Wpedantic -Wdouble-promotion -Wno-unused-parameter -fno-builtin -ffreestanding -fomit-frame-pointer
OPTIONS = -g -mno-unaligned-access 

MACH = stm32l433xc
KRN_CODE_SIZE = 32768
KRN_DATA_SIZE = 4096
include $(ARCH_DIR)/mk/stm32.mk

ICE40_DIR = ice40/ice40_comm_Implmnt
ICE40_BIN = $(ICE40_DIR)/sbt/outputs/bitmap/ice40_comm_bitmap.bin
# Uncomment ICE40_C to make changes in FPGA
ICE40_LZ = $(OUTDIR)/ice40hx1k.lz
#ICE40_C = $(SRCDIR)/ice40hx1k.c
ICE40_H = $(SRCDIR)/ice40hx1k.h

$(ICE40_LZ): $(ICE40_BIN)
	$(LZ77) -w $(LZ_WINDOW_SIZE) -c -o $@ $<

$(ICE40_C): $(ICE40_LZ)
	$(BIN2HEX) -H ice40hx1k.h -n ice40hx1k_bin -o $@ $<

#$(ICE40_C): $(ICE40_BIN)
#	$(BIN2HEX) -H ice40hx1k.h -n ice40hx1k_bin -o $@ $<

$(ICE40_H): $(ICE40_C)

