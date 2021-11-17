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

VERSION_DATE = "2021-10-27"
VERSION_MAJOR = "0"
VERSION_MINOR = "18"

PROJECT_ROOT = ../..
THINKOS_DIR = $(PROJECT_ROOT)/../..
SDK_DIR = $(THINKOS_DIR)/sdk
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
TOOLS_DIR = $(THINKOS_DIR)/tools
MKCMD  = python $(TOOLS_DIR)/mkcmd.py
BIN2HEX = python $(TOOLS_DIR)/bin2hex.py

PROG := stm32f405-thinkos-$(VERSION_MAJOR).$(VERSION_MINOR)

CFILES = boot.c board.c monitor.c uart1-comm.c shell.c cmd_tab.c usb-cdc-comm.c

LIBDIRS = $(addprefix $(SDK_DIR)/lib,$(SDK_LIBS)) \
		  $(addprefix $(ARCH_DIR)/lib,$(ARCH_LIBS)) 

SDK_LIBS = util thinkos-krn thinkos-btl thinkos-mon thinkos-dbg thinkos-usr \
		   xmodem ice-comm

ARCH_LIBS = stm32f 

LIBS = $(SDK_LIBS) $(ARCH_LIBS)

INCPATH = ../include

CDEFS = CONFIG_H 

CFLAGS = -Ofast

MACH = stm32f415xg
KRN_DATA_SIZE = 8192
KRN_CODE_SIZE = 65536
include $(ARCH_DIR)/mk/stm32-krn.mk

CMD_TAB_YAML = $(SRCDIR)/cmd_tab.yaml
CMD_TAB_C = $(SRCDIR)/cmd_tab.c
CMD_TAB_H = $(SRCDIR)/cmd_tab.h
$(CMD_TAB_H): $(CMD_TAB_C)

$(CMD_TAB_C): $(CMD_TAB_YAML)
	$(MKCMD) -e "struct btl_shell_env" -l -p btl -o $@ $<


