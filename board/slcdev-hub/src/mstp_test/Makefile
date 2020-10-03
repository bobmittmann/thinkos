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

CFILES = mstp_crc.c mstp_lnk.c mstp_test.c lattice.c ice40lp384.c

SDK = ../../../sdk

LIBS = cm3 stm32f util c crc stm32drv trace console

RELEASE_DIR := Release
DEBUG_DIR := Debug
INCPATH = $(SDK)/include ../include
LIBPATH = ../ ../sdklibs/Release

CDEFS = CONFIG_H NDEBUG
CFLAGS = -g -Os -Wall -fomit-frame-pointer -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections 

MACH = stm32f407
THINKAPP = 1
LDSCRIPT = slcdev-hub.ld
LOAD_ADDR = 0x08010000
include ../../../mk/stm32f4xx.mk

FPGA_DIR = C:\\mircom/rs485-class-a/rs485hub
#FPGA_BMP_DIR = $(FPGA_DIR)/rs485hub_Implmnt/sbt/outputs/bitmap
FPGA_BMP_DIR = .
FPGA_BIN = $(FPGA_BMP_DIR)/rs485hub_bitmap.bin

ice40lp384.c : $(FPGA_BIN) 
	$(PYTHON) $(TOOLSDIR)/bin2hex.py -n ice40lp384_bin -o $@ $<
