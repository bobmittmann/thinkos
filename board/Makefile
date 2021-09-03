#
# Copyright(C) 2012 Robinson Mittmann. All Rights Reserved.
# 
# This file is part of the YARD-ICE.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3.0 of the License, or (at your option) any later version.
# 
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
# 
# You can receive a copy of the GNU Lesser General Public License from 
# http://www.gnu.org/

#
# File:   Makefile
# Author: Robinson Mittmann <bobmittmann@gmail.com>
# 

BOARDS = slcdev-hub stm32f405-dk u2s-485 rda8com rda425

OUTDIR := $(abspath build)

all: boot-all

clean: boot-clean

boot-all: $(BOARDS:=-boot-all)
boot-clean: $(BOARDS:=-boot-clean)

sanity-all: $(BOARDS:=-sanity-all)
sanity-clean: $(BOARDS:=-sanity-clean)

%-boot-all: 
	$(MAKE) -C $(@:%-boot-all=%)/src/boot O=$(OUTDIR)/$(@:%-boot-all=%)/boot all

%-boot-clean:
	$(MAKE) -C $(@:%-boot-clean=%)/src/boot O=$(OUTDIR)/$(@:%-boot-clean=%)/boot clean


%-sanity-all: 
	$(MAKE) -C $(@:%-sanity-all=%)/src/sanity O=$(OUTDIR)/$(@:%-sanity-all=%)/sanity all

%-sanity-clean: 
	$(MAKE) -C $(@:%-sanity-all=%)/src/sanity O=$(OUTDIR)/$(@:%-sanity-all=%)/sanity all

help:
	@echo 'Targets:'
	@echo
	@echo '  all        - Build all boards'
	@echo '  clean      - Clean all boards'
	@echo

.PHONY: help boot-all boot-clean sanity-all sanity-clean

