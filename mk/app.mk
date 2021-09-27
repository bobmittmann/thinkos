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

ifndef PROG
  $(error PROG undefined!) 
endif

ifndef THINKAPP
  $(error THINKAPP undefined!) 
endif

ifndef PYTHON
  $(error PYTHON undefined!)
endif

ifeq ($(HOST),Windows)
  BIN2APP := $(subst /,\,$(TOOLSDIR)/bin2app.py)
else
  ifeq ($(HOST),Msys)
    ifeq ($(DIRMODE),msys)
      BIN2APP := $(subst /,\\,$(call windrv,$(TOOLSDIR)\\bin2app.py))
    else
      BIN2APP := $(TOOLSDIR)/bin2app.py
    endif
  else
    BIN2APP := $(TOOLSDIR)/bin2app.py
  endif
endif

%.app: %.bin
	$(ACTION) "APP: $@"
ifeq ($(HOST),Cygwin)
else
	$(Q)$(PYTHON) $(BIN2APP) -o $@ -t $(THINKAPP) $<
endif

PROG_APP := $(OUTDIR)/$(PROG).app

ifeq (Windows,$(HOST))
  CLEAN_APP := $(strip $(subst /,\,$(PROG_APP)))
else
  CLEAN_APP := $(strip $(PROG_APP))
endif

app: $(PROG_APP)

app-clean: 
	$(Q)$(RMALL) $(CLEAN_APP)

app-install: all
	$(ACTION) "INSTALL: $(addprefix $(APP_INSTALLDIR)/, $(notdir $(PROG_APP)))"
	$(Q)$(CP) $(PROG_APP) $(APP_INSTALLDIR)

.PHONY: app app-clean

