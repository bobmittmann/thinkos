# File:		Makefile
# Copyright(C) 2012 Bob Mittmann. All Rights Reserved.

PROG = fire_alarm

CFILES = gpio.c ui.c net.c fire_alarm.c 

SDK = ../../../sdk

LIBS = cm3 stm32f util c console

INCPATH = $(SDK)/include ../include

LIBPATH = ../sdklibs/Debug

CDEFS =

CFLAGS = -g -O1 -Wall

MACH = stm32f303xb
HSE_HZ = 8000000
THINKAPP = 1
include ../../../mk/stm32f3xx.mk

