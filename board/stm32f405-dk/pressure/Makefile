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

VERSION_DATE = "2019-01-16"
VERSION_MAJOR = "0"
VERSION_MINOR = "1"

THINKOS_DIR = ../../..
SDK_DIR = $(THINKOS_DIR)/sdk

PROG = airgun-$(VERSION_MAJOR).$(VERSION_MINOR)

CFILES = airgun.c io.c

SDK = ../../../sdk

LIBDIRS = $(SDK_DIR)/libstm32f $(SDK_DIR)/libcm3 $(SDK_DIR)/libc \
		  $(SDK_DIR)/libutil 

LIBS = gcc stm32f cm3 c util 

INCPATH = $(SDK)/include 

JTAGTOOL_ADDR = 192.168.10.50

CFLAGS = -g -O1


MACH := stm32f405
THINKAPP := 0x08020000
LDCODE := $(THINKAPP)
LDDATA := 0x20001000
#LDSCRIPT := amp4x25w.ld
include $(THINKOS_DIR)/mk/stm32.mk

