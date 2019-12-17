# $Id: Makefile,v 1.4 2006/09/28 19:31:45 bob Exp $
#
# File:		Makefile
# Module:
# Project:
# Author:	Robinson Mittmann (bob@boreste.com)
# Target:
# Comment:
# Copyright(c) 2005-2006 BORESTE (www.boreste.com). All Rights Reserved.

PROJECT	= debruijn

LIBS =
#pthread

CFILES = $(wildcard ./*.c)
OFILES = $(CFILES:.c=.o)

INCPATH	=
LIBPATH =

DBGOPTS = -g

OPTIONS	= $(DBGOPTS)
#OPTIONS	= $(DBGOPTS) -mno-cygwin

SFLAGS	= -Wall
CFLAGS	= -Wall -Ofast
LDFLAGS	=

CC	= gcc
LD	= gcc
AS	= gcc
AR	= ar
OBJCOPY	= objcopy
OBJDUMP	= objdump
STRIP = strip

all: Makefile $(PROJECT) 

clean:
	@rm -fv $(OFILES) $(PROJECT) $(PROJECT).exe

$(PROJECT): Makefile $(OFILES)
	$(LD) $(OPTIONS) $(LDFLAGS) $(addprefix -L,$(LIBPATH)) -o $@ $(OFILES) \
	$(addprefix -l,$(LIBS))

%.o : %.c Makefile
	$(CC) $(OPTIONS) $(CFLAGS) $(addprefix -I,$(INCPATH)) -o $@ -c $<

