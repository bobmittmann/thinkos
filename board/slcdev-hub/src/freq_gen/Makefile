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

VERSION_DATE = "2020-04-05"
VERSION_MAJOR = "0"
VERSION_MINOR = "1"

PROJECT_ROOT = ../../
THINKOS_DIR = $(PROJECT_ROOT)/../..
SDK_DIR = $(THINKOS_DIR)/sdk
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
TOOLS_DIR = $(THINKOS_DIR)/tools

PROG = freq_gen

CFILES = freq_gen.c

LIBDIRS = $(ARCH_DIR)/libstm32f $(ARCH_DIR)/libstm32drv $(SDK_DIR)/libcm3 \
		  $(SDK_DIR)/libthinkos-usr $(SDK_DIR)/libutil \
		  $(SDK_DIR)/libc 

LIBS = gcc m util c cm3 stm32f stm32drv thinkos-usr 

INCPATH = $(PROJECT_ROOT)/src/include .

CDEFS = CONFIG_H 

MACH = stm32f427xi
LDCODE := 0x08020000
LDDATA := 0x20001000
THINKAPP := $(LDCODE)
include $(ARCH_DIR)/mk/stm32.mk

