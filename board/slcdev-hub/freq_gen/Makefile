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


PROG = mstp_test

CFILES = freq_gen.c 

THINKOS_DIR = ../../..
SDK_DIR = $(THINKOS_DIR)/sdk

LIBDIRS = $(SDK_DIR)/libcm3 $(SDK_DIR)/libstm32f $(SDK_DIR)/libutil \
	$(SDK_DIR)/libc $(SDK_DIR)/libtrace $(SDK_DIR)/libconsole \
	$(SDK_DIR)/libstm32drv

LIBS = cm3 stm32f util c trace console stm32drv
LIBPATH = ..

INCPATH = $(SDK_DIR)/include ../include

CDEFS = CONFIG_H NDEBUG
CFLAGS = -g -Os -Wall -fomit-frame-pointer -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections 

MACH = stm32f407
THINKAPP = 1
THINKAPP := 0x08010000
LDSCRIPT = slcdev-hub.ld
include $(THINKOS_DIR)/mk/stm32.mk

