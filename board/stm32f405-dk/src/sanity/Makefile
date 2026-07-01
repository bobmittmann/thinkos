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

<<<<<<< HEAD
THINKOSDIR = ../../../..
SDKDIR = $(THINKOSDIR)/sdk

VERSION_DATE = "2019-01-16"
VERSION_MAJOR = "0"
VERSION_MINOR = "28"

PROG = sanity-$(VERSION_MAJOR).$(VERSION_MINOR)

CFILES = sanity.c

#CFILES = $(THINKOSDIR)/boot/boot.c $(THINKOSDIR)/boot/monitor.c

LIBDIRS = $(SDKDIR)/libstm32f $(SDKDIR)/libcm3 $(SDKDIR)/libutil\
		  $(SDKDIR)/libthinkos $(SDKDIR)/libice-comm $(SDKDIR)/libconsole\
		  $(SDKDIR)/libc 

LIBS = cm3 stm32f util thinkos ice-comm gcc console  c

INCPATH = $(SDKDIR)/include ../include

CDEFS = CONFIG_H 
#CM3_RAM_VECTORS

CFLAGS = -g -Os -Wall -fno-builtin -ffreestanding -fomit-frame-pointer
OPTIONS = -mno-unaligned-access 

MACH = stm32f405
include $(THINKOSDIR)/mk/stm32.mk
=======

PROG = sanity

THINKOS_DIR = ../../../..
SDK_DIR = $(THINKOS_DIR)/sdk
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
TOOLS_DIR = $(THINKOS_DIR)/tools
TEST_DIR = $(THINKOS_DIR)/test

CFILES = $(wildcard $(TEST_DIR)/sanity/*.c)

LIBS = thinkos-usr util tty c

LIBDIRS = $(addprefix $(SDK_DIR)/lib,$(LIBS))

INCPATH = ../include

CDEFS = CONFIG_H 

CFLAGS = -O1 

MACH = stm32f415xg
LDCODE := 0x08020000
LDDATA := 0x20002000
include $(ARCH_DIR)/mk/stm32-app.mk

>>>>>>> krnsvc
