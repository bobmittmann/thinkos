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
VERSION_MINOR = "5"

THINKOS_DIR = ../../../..

SDK_DIR = $(THINKOS_DIR)/sdk
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
TOOLS_DIR = $(THINKOS_DIR)/tools
BIN2HEX = python $(TOOLS_DIR)/bin2hex.py
MKCMD  = python $(TOOLS_DIR)/mkcmd.py
LZ77 = /c/opt/bin/lz77
LZ_WINDOW_SIZE = 16384
PROG = rda8com-thinkos-$(VERSION_MAJOR).$(VERSION_MINOR)

#BOOT_CFILES = boot.c monitor.c standby.c
#CFILES = $(addprefix $(THINKOS_DIR)/boot/,$(BOOT_CFILES)) board.c fpga.c ice40hx1k.c 
CFILES = board.c monitor.c shell.c cmd_tab.c fpga.c ice40hx1k.c 


LIBDIRS = $(addprefix $(SDK_DIR)/lib,$(SDK_LIBS)) \
		  $(addprefix $(ARCH_DIR)/lib,$(ARCH_LIBS))

SDK_LIBS = util thinkos-krn thinkos-mon thinkos-dbg thinkos-usr gdbrsp \
		   xmodem ice-comm
ARCH_LIBS = stm32f 

LIBS = $(SDK_LIBS) $(ARCH_LIBS)

INCPATH = ../include

#
# select the hardware revision
# 
ifdef HWREV
  ifeq ($(findstring $(HWREV), 1 2),)
    $(error invalid HWREV number, must be 1 or 2) 
  endif
  __hwrev=$(HWREV)
else
  # assign a default hardware if not specified in the command line
  __hwrev=2
endif

CDEFS = CONFIG_H LZ_WINDOW_SIZE=$(LZ_WINDOW_SIZE) HWREV=$(__hwrev)

CFLAGS = -Os

MACH = stm32l433xc
KRN_CODE_SIZE = 32768
KRN_DATA_SIZE = 4096
include $(ARCH_DIR)/mk/stm32-krn.mk

ICE40_DIR = ice40/ice40_comm_Implmnt
ICE40_BIN = $(ICE40_DIR)/sbt/outputs/bitmap/ice40_comm_bitmap.bin
# Uncomment ICE40_C to make changes in FPGA
ICE40_LZ = $(OUTDIR)/ice40hx1k.lz
#ICE40_C = $(SRCDIR)/ice40hx1k.c
ICE40_H = $(SRCDIR)/ice40hx1k.h

$(ICE40_LZ): $(ICE40_BIN) $(MAKEFILE_LIST)
	$(LZ77) -w $(LZ_WINDOW_SIZE) -c -o $@ $<

$(ICE40_C): $(ICE40_LZ)
	$(BIN2HEX) -H ice40hx1k.h -n ice40hx1k_bin -o $@ $<

#$(ICE40_C): $(ICE40_BIN)
#	$(BIN2HEX) -H ice40hx1k.h -n ice40hx1k_bin -o $@ $<

$(ICE40_H): $(ICE40_C)

CMD_TAB_YAML = $(SRCDIR)/cmd_tab.yaml
CMD_TAB_C = $(SRCDIR)/cmd_tab.c
CMD_TAB_H = $(SRCDIR)/cmd_tab.h

$(CMD_TAB_H): $(CMD_TAB_C)

$(CMD_TAB_C): $(CMD_TAB_YAML)
	$(MKCMD) -p boot -o $@ $<

cmdtab: $(CMD_TAB_C)

.PHONY = cmdtab

