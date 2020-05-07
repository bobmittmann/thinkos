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
LIBMAD_DIR = $(PROJECT_ROOT)/src/libmad
BIN2HEX = python $(TOOLS_DIR)/bin2hex.py

PROG = piano

CFILES = piano.c dac.c spi.c f32sin.c\
		 encoder.c tonegen.c

LIBDIRS = $(ARCH_DIR)/libstm32f $(SDK_DIR)/libcm3 \
		  $(SDK_DIR)/libthinkos-usr $(SDK_DIR)/libtty $(SDK_DIR)/libutil \
		  $(SDK_DIR)/libc $(SDK_DIR)/libtrace $(SDK_DIR)/libfixpt $(LIBMAD_DIR)

LIBS = gcc m stm32f cm3 util c thinkos-usr fixpt tty mad

INCPATH = $(SDK_DIR)/include $(PROJECT_ROOT)/src/include .

CDEFS = CONFIG_H 
CFLAGS = -Ofast -fomit-frame-pointer -Werror -Wall -Wdouble-promotion -Wno-unused-parameter -Wextra -Wpedantic 
OPTIONS = -g -mno-unaligned-access 
LDFLAGS = -Wl,--gc-sections 

RELEASE_DIR := Release
DEBUG_DIR := Debug
#LIBPATH = ../sdklibs/Debug
#LIBPATH = ../sdklibs/Release

LIBPATH += $(MAD_LIB_DIR)

MACH = stm32l433xc
THINKAPP  = 0x08010000
LDCODE := $(THINKAPP)
LDDATA := 0x20001000
include $(ARCH_DIR)/mk/stm32.mk

%.c : %.mp3
	$(BIN2HEX) -H mp3lib.h -o $@ $<

