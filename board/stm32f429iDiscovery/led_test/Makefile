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

BOARD_ROOT = ..
THINKOS_DIR = $(BOARD_ROOT)/../..
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
SDK_DIR = $(THINKOS_DIR)/sdk
TOOLS_DIR = $(THINKOS_DIR)/tools

PROG = led_test

CFILES = led_test.c

LIBDIRS = $(ARCH_DIR)/libstm32f $(SDK_DIR)/libcm3 \
		  $(SDK_DIR)/libutil \
		  $(SDK_DIR)/libthinkos-usr

LIBS = cm3 stm32f thinkos-usr util

CFLAGS = -g -O1 -Wall

MACH = stm32f407xg
THINKAPP := 0x08020000
LDCODE := $(THINKAPP)
LDDATA := 0x20001000
include $(ARCH_DIR)/mk/stm32.mk

