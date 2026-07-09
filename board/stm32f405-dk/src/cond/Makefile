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

PROG = cond_test

CFILES = cond_test.c

LIBDIRS = $(SDK_DIR)/libc \
		  $(SDK_DIR)/libutil \
		  $(SDK_DIR)/libthinkos-usr \

LIBS = gcc m c util thinkos-usr 

INCPATH = $(PROJECT_ROOT)/src/include

CDEFS = CONFIG_H 
CFLAGS = -O0 -Werror -Wall -Wextra -Wdouble-promotion -Wno-unused-parameter
OPTIONS = -g -mno-unaligned-access 

MACH = stm32f415xg
LDCODE := 0x08020000
LDDATA := 0x20003000
include $(ARCH_DIR)/mk/stm32-app.mk

