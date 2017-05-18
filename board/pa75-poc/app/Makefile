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


PROG = pa75_poc

WWW_CFILES = www/zepto.min.js.c www/libpa75.js.c \
			 www/pa75_monitor.html.c \
			 www/pa75_status.html.c \
			 www/root.c 

CFILES = pa75_poc.c audio.c jitbuf.c rtcp.c rtp.c rtsp.c rtsp_hdr.c \
		 sdp.c sndbuf.c webserver.c supv.c console.c version.c \
		 $(WWW_CFILES) 

SDK = ../../../sdk

#LIBDIRS = $(SDK)/libcm3 $(SDK)/libice-comm \
#		  $(SDK)/libstm32f $(SDK)/libstm32drv $(SDK)/libutil \
#		  $(SDK)/libc $(SDK)/libconsole $(SDK)/libtcpip $(SDK)/libhttpd \
#		  $(SDK)/libtrace $(SDK)/libcrc

LIBS = gcc cm3 stm32f util stm32drv c console tty shell tcpip httpd trace crc

INCPATH = $(SDK)/include ../include .

VERSION_DATE = "2017-02-20"
VERSION_MAJOR = "0"
VERSION_MINOR = "1"

CDEFS = CONFIG_H
CFLAGS = -g -O1 -Wall -ffunction-sections -fdata-sections
LDFLAGS = -Wl,--gc-sections 

RELEASE_DIR := Release
DEBUG_DIR := Debug
LIBPATH = ../sdklibs/Debug
#LIBPATH = ../sdklibs/Release

JTAGTOOL_ADDR= 192.168.10.50

MACH = stm32f407
THINKAPP = 1
LDSCRIPT = pa75-poc.ld
LOAD_ADDR = 0x08010000
include ../../../mk/stm32f4xx.mk

%.js.c : %.js
ifeq ($(HOST),Cygwin)
else 
	$(Q)$(PYTHON) $(TOOLSDIR)/bin2hex.py -z -H www/www.h -o $@ $<
endif

%.html.c : %.html
ifeq ($(HOST),Cygwin)
else 
	$(Q)$(PYTHON) $(TOOLSDIR)/bin2hex.py -z -H www/www.h -o $@ $<
endif

