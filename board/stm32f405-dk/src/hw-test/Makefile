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

PROG = hw-test

CFILES = io.c sanity.c

LIBDIRS = $(ARCH_DIR)/libstm32f \
		  $(SDK_DIR)/libutil \
		  $(SDK_DIR)/libc \
		  $(SDK_DIR)/libthinkos-usr

LIBS = gcc m stm32f util c thinkos-usr 

INCPATH = $(PROJECT_ROOT)/src/include

CDEFS = CONFIG_H 
CFLAGS = -O1 

MACH = stm32f415xg
include $(ARCH_DIR)/mk/stm32-app.mk

