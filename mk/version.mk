#
# version.mk 
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

ifndef TOOLSDIR
  $(error TOOLSDIR undefined!)
endif	

ifndef VERSION_MAJOR
  $(error VERSION_MAJOR undefined!) 
endif

ifdef PROG
  VERSION_NAME = $(PROG)
else
  VERSION_NAME = 
endif	

ifndef VERSION_MINOR
  VERSION_MINOR = 0
endif

ifndef VERSION_DATE
  VERSION_DATE = "2014-05-29"
endif

ifndef PRODUCT
  PRODUCT = "YARD-ICE"
endif

ifndef REVISION
  REVISION = "0.1"
endif

ifndef ASSEMBLY
  ASSEMBLY = "A"
endif

ifndef PYTHON
  $(error PYTHON undefined!)
endif

ifeq ($(HOST),Windows)
  MKVER := $(subst /,\,$(TOOLSDIR)/mkver.py)
else
  ifeq ($(HOST),Msys)
    ifeq ($(DIRMODE),msys)
      MKVER := $(subst /,\\,$(call windrv,$(TOOLSDIR)\\mkver.py))
    else
      MKVER := $(TOOLSDIR)/mkver.py
    endif
  else
    MKVER := $(TOOLSDIR)/mkver.py
  endif
endif

ifndef VERSION_TAG
  VERSION_TAG := $(VERSION_H:.h=.tag)
endif

$(VERSION_H): 
	$(ACTION) "Creating: $@"
	$(Q)$(PYTHON) $(MKVER) -o $@ -n $(VERSION_NAME) $(VERSION_MAJOR) $(VERSION_MINOR) $(VERSION_DATE) > $(VERSION_TAG)

ifeq (Windows,$(HOST))
  CLEAN_VERSION := $(strip $(subst /,\,$(VERSION_H) $(VERSION_TAG)))
else
  CLEAN_VERSION := $(strip $(VERSION_H) $(VERSION_TAG))
endif

version: 
	$(Q)$(RMALL) $(VERSION_H) $(VERSION_TAG)
	$(Q)$(MAKE) $(VERSION_H)

version-clean: 
	$(Q)$(RMALL) $(CLEAN_VERSION)

.PHONY: version version-clean

