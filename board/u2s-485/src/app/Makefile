# Makefile
# --------
#
#   ************************************************************************
#   **            Company Confidential - For Internal Use Only            **
#   **          Mircom Technologies Ltd. & Affiliates ("Mircom")          **
#   **                                                                    **
#   **   This information is confidential and the exclusive property of   **
#   ** Mircom.  It is intended for internal use and only for the purposes **
#   **   provided,  and may not be disclosed to any third party without   **
#   **                prior written permission from Mircom.               **
#   **                                                                    **
#   **                        Copyright 2017-2018                         **
#   ************************************************************************
#
#

#
# Author: Bob Mittmann <bmittmann@mircomgroup.com>
# 

PROJECT_ROOT = ../..
THINKOS_DIR = $(PROJECT_ROOT)/../..
ARCH_DIR = $(THINKOS_DIR)/arch/stm32
SDK_DIR = $(THINKOS_DIR)/sdk
TOOLS_DIR = $(THINKOS_DIR)/tools

PROG = sniffer

CFILES = sniffer.c

LIBDIRS = $(ARCH_DIR)/libstm32f \
		  $(SDK_DIR)/libutil \
		  $(SDK_DIR)/libc \
		  $(SDK_DIR)/libthinkos-usr

LIBS = gcc m stm32f util c thinkos-usr 

INCPATH = ../boot ../include

CDEFS = CONFIG_H 
CFLAGS = -O0 -Werror -Wall 
#-Wextra -Wdouble-promotion -Wno-unused-parameter

MACH = stm32f103x6
THINKAPP := 0x08004000
LDCODE := $(THINKAPP)
LDDATA := 0x20000800
include $(ARCH_DIR)/mk/stm32-app.mk

