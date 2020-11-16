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

VERSION_DATE = "2020-04-16"
VERSION_MAJOR = "1"
VERSION_MINOR = "0"

PROG = stm32f405-thinkos-$(VERSION_MAJOR).$(VERSION_MINOR)

BOOT_CFILES = boot.c monitor.c standby.c 

CFILES = $(addprefix $(THINKOS_DIR)/boot/,$(BOOT_CFILES)) board.c

LIBDIRS = $(SDK_DIR)/libthinkos-krn $(SDK_DIR)/libutil \
		  $(SDK_DIR)/libthinkos-dbg $(ARCH_DIR)/libstm32f \
		  $(SDK_DIR)/libthinkos-mon $(SDK_DIR)/libxmodem \
		  $(SDK_DIR)/libgdbrsp $(SDK_DIR)/libice-comm 

LIBS = stm32f util thinkos-krn thinkos-mon thinkos-dbg gcc gdbrsp xmodem \
	   ice-comm 

INCPATH = ../include

CDEFS = CONFIG_H 

CFLAGS = -Os

MACH = stm32f415xg
KRN_DATA_SIZE = 8192
KRN_CODE_SIZE = 65536
include $(ARCH_DIR)/mk/stm32-krn.mk

