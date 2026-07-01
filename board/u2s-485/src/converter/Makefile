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

THINKOS_DIR = ../../../..
SDK_DIR = $(THINKOS_DIR)/sdk
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
TOOLS_DIR = $(THINKOS_DIR)/tools

VERSION_DATE = "2026-06-04"
VERSION_MAJOR = "1"
VERSION_MINOR = "40"

PROG = u2s-converter-$(VERSION_MAJOR).$(VERSION_MINOR)

COMMON_CFILES =	serial2.c leds.c xflash.c 
XFLASH_PIC_DIR = ../../../../arch/stm32/libxflash/usb-xflash
XFLASH_PIC_C = $(XFLASH_PIC_DIR)/usb_xflash_pic.c 

CFILES = u2s-converter.c board.c usb_printf.c \
		$(addprefix ../common/,$(COMMON_CFILES)) \
		$(XFLASH_PIC_C)

LIBDIRS = $(ARCH_DIR)/libstm32f $(SDK_DIR)/libutil \
		  $(SDK_DIR)/libthinkos-krn \
		  $(SDK_DIR)/libice-comm \
		  $(SDK_DIR)/libcdc-acm

PREBUILD = $(ARCH_DIR)/libxflash/usb-xflash

LIBS = stm32f thinkos-krn cdc-acm util ice-comm

INCPATH = ../include .

CDEFS = CONFIG_H 

CFLAGS = -O1 

MACH = stm32f103x6
KRN_CODE_SIZE = 10240
KRN_DATA_SIZE = 2048
include $(ARCH_DIR)/mk/stm32-krn.mk

$(XFLASH_PIC_C): xflash_bin

xflash_bin:
	$(ACTION) "Building : $@"
	$(Q)$(MAKE) -C $(XFLASH_PIC_DIR) all

#	$(XFLASH_PIC_DIR)/usb_xflash_pic.bin
