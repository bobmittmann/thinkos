# File:		Makefile
# Copyright(C) 2012 Bob Mittmann. All Rights Reserved.

THINKOSDIR = ../../..
SDKDIR = $(THINKOSDIR)/sdk

PROG = cc_test

CFILES = cc_test.c

SDK = ../../../sdk

LIBS = cm3 stm32f stm32drv util c console

INCPATH = $(SDKDIR)/include ../include

LIBPATH = ../sdklibs/Debug

CDEFS = CONFIG_H CM3_RAM_VECTORS
CFLAGS = -g -O0 -Wall -Wdouble-promotion
OPTIONS = -mno-unaligned-access 

LDSCRIPT = ../stm32f303xc.ld
THINKAPP = 0x08010000
MACH = stm32f303xc
include $(THINKOSDIR)/mk/stm32.mk

