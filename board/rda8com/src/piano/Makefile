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

THINKOS_DIR = ../../../..
SDK_DIR = $(THINKOS_DIR)/sdk
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
TOOLS_DIR = $(THINKOS_DIR)/tools
BIN2HEX = python $(TOOLS_DIR)/bin2hex.py
MKCMD  = python $(TOOLS_DIR)/mkcmd.py

PROG = piano-$(VERSION_MAJOR).$(VERSION_MINOR)

CFILES = ../common/spidrv.c ../common/encoder.c \
		 piano.c dac.c keyboard.c addsynth-voice.c exp_envelope.c \
		 addsynth-instr.c \
		 addsynth-piano.c addsynth-xilophone.c  \
		filt.c

#tonegen.c 
#f32sin.c

LIBDIRS = $(addprefix $(SDK_DIR)/lib,$(SDK_LIBS)) \
		  $(addprefix $(ARCH_DIR)/lib,$(ARCH_LIBS))

SDK_LIBS = util thinkos-usr c fixpt

ARCH_LIBS = stm32f 

LIBS = $(SDK_LIBS) $(ARCH_LIBS) m stm32f util c thinkos-usr fixpt

INCPATH = ../include

CDEFS = CONFIG_H 
CFLAGS = -Ofast -fomit-frame-pointer -Werror -Wall -Wdouble-promotion -Wno-unused-parameter -Wextra 
#-Wpedantic 
OPTIONS = -g -mno-unaligned-access 
LDFLAGS = -Wl,--gc-sections 

MACH = stm32l433xc
LDCODE = 0x08010000
LDDATA := 0x20001000
include $(ARCH_DIR)/mk/stm32-app.mk

%.c : %.mp3
	$(BIN2HEX) -H mp3lib.h -o $@ $<

