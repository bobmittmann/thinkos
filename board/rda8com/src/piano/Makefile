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
BIN2HEX = python $(TOOLS_DIR)/bin2hex.py
MKCMDTAB = python $(TOOLS_DIR)/mkcmd.py

PROG = piano

CFILES = ../common/spidrv.c piano.c dac.c keyboard.c  keyboard-cfgs.c \
		 addsynth-voice.c exp_envelope.c \
		 addsynth-instr.c \
		 addsynth-piano.c addsynth-xilophone.c \
		 cmdtab.c shell.c

ARCH_LIBS = stm32f

SDK_LIBS = thinkos-usr util trace c fixpt tty shell 

LIBDIRS = $(addprefix $(ARCH_DIR)/lib,$(ARCH_LIBS))\
		  $(addprefix $(SDK_DIR)/lib,$(SDK_LIBS))

LIBS = m $(ARCH_LIBS) $(SDK_LIBS)

INCPATH = ../include

CDEFS = CONFIG_H 

#LIBPATH = $(LIBMAD_DIR)

CFLAGS = -Og

MACH := stm32l433xc
LDCODE := 0x0800a000
LDDATA := 0x20001000

APP_INSTALLDIR = /c/tmp/release/rda8com

default: app-install

include $(ARCH_DIR)/mk/stm32-app.mk

CMDTAB_YAML = $(SRCDIR)/cmdtab.yaml
CMDTAB_C = $(SRCDIR)/cmdtab.c
CMDTAB_H = $(SRCDIR)/cmdtab.h

$(CMDTAB_H): $(CMDTAB_C)

$(CMDTAB_C): $(CMDTAB_YAML)
	$(MKCMDTAB) -l -o $@ $<

cmdtab: $(CMDTAB_H)

.PHONY = cmdtab

