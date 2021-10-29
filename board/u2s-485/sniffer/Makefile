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

THINKOS_DIR = ../../..
SDK_DIR = $(THINKOS_DIR)/sdk
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
TOOLS_DIR = $(THINKOS_DIR)/tools

VERSION_DATE = "2020-11-18"
VERSION_MAJOR = "0"
VERSION_MINOR = "4"

PROG = u2s-485-$(VERSION_MAJOR).$(VERSION_MINOR)

CFILES = u2s-485.c serial.c board.c leds.c usb_trace.c raw_trace.c \
		 sdu_trace.c xflash.c usb_xflash_pic.c profclk.c

LIBDIRS = $(ARCH_DIR)/libstm32f $(SDK_DIR)/libutil \
		  $(SDK_DIR)/libthinkos-krn $(SDK_DIR)/libthinkos-usr \
		  $(SDK_DIR)/libc $(SDK_DIR)/libice-comm $(SDK_DIR)/libcdc-acm 

LIBS =  stm32f util thinkos-krn thinkos-usr c ice-comm cdc-acm

INCPATH = ../include

CDEFS = CONFIG_H 

CFLAGS = -Os 

MACH = stm32f103x6
KRN_CODE_SIZE = 12288
KRN_DATA_SIZE = 2048
include $(ARCH_DIR)/mk/stm32-krn.mk

