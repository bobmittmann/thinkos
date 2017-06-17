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


PROG = test

CFILES = test.c

SDK = ../../../sdk

LIBDIRS = $(SDK)/libcm3 $(SDK)/libstm32f \
		  $(SDK)/libconsole $(SDK)/libtty $(SDK)/libutil $(SDK)/libc

LIBS = gcc m cm3 stm32f util c console tty 

INCPATH = $(SDK)/include ../include .

VERSION_DATE = "2017-02-20"
VERSION_MAJOR = "0"
VERSION_MINOR = "1"

CDEFS = CONFIG_H
CFLAGS = -g -O3 -Wall -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections 

RELEASE_DIR := Release
DEBUG_DIR := Debug
LIBPATH = ../sdklibs/Debug
#LIBPATH = ../sdklibs/Release

MACH = stm32l433xc
HSE_HZ = 11289600
HCLK_HZ = 80000000
THINKAPP = 1
LDSCRIPT = test.ld
APPADDR = 0x0800c000
include ../../../mk/stm32l4xx.mk

