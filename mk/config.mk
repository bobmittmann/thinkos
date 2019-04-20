#
# config.mk 
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
#
#

CONFIG_MK := $(lastword $(MAKEFILE_LIST))
SOURCE_MK := $(firstword $(MAKEFILE_LIST))

THISDIR := $(dir $(CONFIG_MK))
SCRPTDIR := $(abspath $(THISDIR))
BASEDIR := $(abspath $(THISDIR)/..)
TOOLSDIR := $(abspath $(THISDIR)/../tools)
LDDIR := $(abspath $(THISDIR)/../ld)

#------------------------------------------------------------------------------ 
# default output directories
#------------------------------------------------------------------------------ 
ifndef RELEASE_DIR 
  RELEASE_DIR := release
endif

ifndef DEBUG_DIR
  DEBUG_DIR := debug
endif

#------------------------------------------------------------------------------ 
# parameters
# <V> verbose, can be either 0 or 1
# <D> debug level: (0 == release, no debug)
# <O> output directory
# <T> trace level: (0 == quiet)
#------------------------------------------------------------------------------ 
ifdef V
  verbose := $(V)
else
  verbose := 0
endif

ifdef D
  dbg_level := $(D)
else
  dbg_level := 0
endif

ifdef O
  out_dir := $(O)
else
  # if the output directory is not set select the appropriate one according
  # to debug level.
  ifneq ($(dbg_level),0)
    out_dir := $(DEBUG_DIR)
  else
    out_dir := $(RELEASE_DIR)
  endif
endif

ifdef T
  trace_level := $(T)
else
  trace_level := 0
endif

ifeq ($(trace_level),0)
  trace1 = 
  trace2 = 
  trace3 = 
else
  ifeq ($(trace_level),1)
    trace1 = $(info $1)
    trace2 = 
    trace3 = 
  else
    ifeq ($(trace_level),2)
      trace1 = $(info $1)
      trace2 = $(info $1)
      trace3 = 
    else
      trace1 = $(info $1)
      trace2 = $(info $1)
      trace3 = $(info $1)
    endif
  endif
endif

ifeq ($(trace_level),4)
  $(info !!!!!!!!!!! ++++++++++++++++++++++++++++++++++++++++++++++++++++++++) 
  $(foreach v, $(.VARIABLES), $(info $(v) = '$($(v))'))
  $(info !!!!!!!!!!! ++++++++++++++++++++++++++++++++++++++++++++++++++++++++) 
endif

#------------------------------------------------------------------------------ 
# Host OS detection
#------------------------------------------------------------------------------ 

ifeq (, $(OS))
  UNAME := $(shell uname -o) 
  ifneq (,$(findstring Linux, $(UNAME)))
#   $(info Linux Host)
    HOST := Linux
    DIRMODE := unix
  else
    $(error Unsuported OS)
  endif
else
  # check if we are in a windows environment
  ifneq (,$(findstring Windows, $(OS)))
    ifeq ($(PROGRAMFILES\(X86\))$(ProgramFiles\(x86\)),)
      $(call trace1,Windows 32bits)
      WIN := Win32
    else
      $(call trace1,Windows 64bits)
      WIN := Win64
    endif
    $(call trace3,MSYSTEM = '$(MSYSTEM)')
    $(call trace3,MSYSCON = '$(MSYSCON)')
    ifneq (,$(findstring MINGW, $(MSYSTEM)))
      HOST := Msys
      ifeq (+++, $(firstword $(subst /,+++ ,$(BASEDIR))))
        $(call trace1,UNIX style paths: BASEDIR = '$(BASEDIR)'...)
        DIRMODE := msys
      else
        $(call trace1,Msys style paths: BASEDIR = '$(BASEDIR)'...)
        DIRMODE := windows
      endif
      ifneq (, $(MSYSCON))
        $(call trace1,MSYSCON = '$(MSYSCON)')
      endif
      ifneq (, $(MAKE_MODE))
        $(call trace1,MAKE_MODE = '$(MAKE_MODE)')
      endif
      SHELL := sh.exe
      export SHELL
    else 
      ifeq (+++, $(firstword $(subst /,+++ ,$(BASEDIR))))
        $(call trace1,UNIX style paths: BASEDIR = '$(BASEDIR)'...)
	    # UNIX style path, it can be MinGW or Cygwin
        SHELL := /bin/sh.exe
        UNAME := $(shell uname -s) 
        $(call trace3,UNAME = '$(UNAME)')
        ifneq (,$(findstring MINGW, $(UNAME)))
          $(call trace1,Windows MinGW/Msys Host)
          HOST := Msys
          DIRMODE := msys
        else
          ifneq (,$(findstring CYGWIN, $(UNAME)))
            $(call trace1,Windows Cygwin Host, $(shell cygpath -W))
            HOST := Cygwin
            DIRMODE := unix
          else
            $(error Unable to detect the host OS type)
          endif
        endif
      else
#        ifdef SHELL 
#          $(call trace1,SHELL = '$(SHELL)')
#        endif
        $(call trace1,Windows Native Host)
#      ifdef SHELL 
##          HOST := Msys
#          DIRMODE := windows
#          $(info SHELL --> '$(flavor SHELL)')
#        else
        HOST := Windows
        DIRMODE := windows
        ifdef ComSpec
           # Force the shell to the Windows Command Prompt
            SHELL := $(ComSpec)
          export SHELL
        else
          ifdef COMSPEC
            SHELL := $(COMSPEC)
            export SHELL
          endif
        endif
#        $(call trace1,SHELL --> '$(flavor SHELL) $(origin SHELL)')
#        $(call trace1,SHELL = '$(SHELL)')
#        endif
      endif
    endif
  else
    ifndef HOST 
      $(error Unable to detect the host OS type)
    endif
  endif
endif

#------------------------------------------------------------------------------ 
# Other macros
#------------------------------------------------------------------------------ 

ifeq ($(verbose),0) 
  Q := @
  ACTION := @echo
else
  Q :=
  ifeq ($(HOST),Windows)
    ACTION := @rem
  else
    ACTION := @\#
  endif
endif

#------------------------------------------------------------------------------ 
# OS specific helper macros
#------------------------------------------------------------------------------ 

ifeq ($(HOST),Linux)
  CAT := cat
  RMALL := rm -f 
  RMDIR := rmdir
  MKDIR := mkdir -p
  CP := cp
  CPDIR := cp -rd
  MV := mv    
  ECHO := echo
  DEVNULL := /dev/null
else
ifeq ($(HOST),Cygwin)
  CAT := cat
  RMALL := rm -f 
  RMDIR := rmdir
  MKDIR := mkdir -p
  CP := cp
  CPDIR := cp -rd
  MV := mv    
  ECHO := echo
  DEVNULL := /dev/null
else
ifeq ($(HOST),Msys)
  CAT := cat
  RMALL := rm -f
  RMDIR := rmdir
  MKDIR := mkdir -p
  CP := cp
  CPDIR := cp -rd
  MV := mv    
  ECHO := echo
  DEVNULL := /dev/null
else
ifeq ($(HOST),Windows)
  CAT := type
  RMALL := del /F /Q 
  RMDIR := rmdir
  MKDIR := mkdir
  CP := copy /Y
  CPDIR := xcopy /Y /R /E /I
  MV := ren
  ECHO := echo
  DEVNULL := NUL:
else
  $(error Unsuported HOST)
endif
endif
endif
endif

ifeq ($(HOST),Windows)
#  PYTHON := "C:\Python27\python"
  PYTHON := python
else
  PYTHON := python
endif

# ----------------------------------------------------------------------------
# Function: windrv
#   Return: windows driver letter
# This macro converts an absolute path from UNIX into windows style, ex:
# "/c/directory/file" will be turn into: "c:/directory/file".
# It works by adding the prefix '+' into the path and searching for a 
# matching patters: "+/c/", "+/d/" ... "+/z/", then replacing the pattern
# by driver letter followed by colon and a slash: "c:/", "d:/" .. "z:/"
# ----------------------------------------------------------------------------
windrv = $(patsubst +/z/%,z:/%,$(patsubst +/y/%,y:/%,$(patsubst +/w/%,w:/%,$(patsubst +/v/%,v:/%,$(patsubst +/u/%,u:/%,$(patsubst +/t/%,t:/%,$(patsubst +/s/%,s:/%,$(patsubst +/r/%,r:/%,$(patsubst +/q/%,q:/%,$(patsubst +/p/%,p:/%,$(patsubst +/o/%,o:/%,$(patsubst +/n/%,n:/%,$(patsubst +/m/%,m:/%,$(patsubst +/l/%,l:/%,$(patsubst +/k/%,k:/%,$(patsubst +/j/%,j:/%,$(patsubst +/i/%,i:/%,$(patsubst +/h/%,h:/%,$(patsubst +/h/%,h:/%,$(patsubst +/g/%,g:/%,$(patsubst +/f/%,f:/%,$(patsubst +/e/%,e:/%,$(patsubst +/d/%,d:/%,$(patsubst +/c/%,c:/%,$(addprefix +,$1)))))))))))))))))))))))))

# ----------------------------------------------------------------------------
# Function: lc
#   Return: text in lower case
# ----------------------------------------------------------------------------
lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

# ----------------------------------------------------------------------------
# Function: uc
#   Return: text in upper case
# ----------------------------------------------------------------------------
uc = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))

# ----------------------------------------------------------------------------
# Function:  first (same as LISP's car, or head)
# Arguments: 1: A list
# Returns:   Returns the first element of a list
# ----------------------------------------------------------------------------
first = $(firstword $1)

# ----------------------------------------------------------------------------
# Function:  rest (same as LISP's cdr, or tail)
# Arguments: 1: A list
# Returns:   Returns the list with the first element removed
# ----------------------------------------------------------------------------
rest = $(wordlist 2,$(words $1),$1)

# ----------------------------------------------------------------------------
# Function:  reverse
# Arguments: 1: A list to reverse
# Returns:   The list with its elements in reverse order
# ----------------------------------------------------------------------------
reverse = $(strip $(if $1,$(call reverse,$(call rest,$1)) $(call first,$1)))

# ----------------------------------------------------------------------------
# Define useful strings
# ----------------------------------------------------------------------------
comma := ,
space :=
space +=

# ----------------------------------------------------------------------------
# Adjust source and output directory variables
# SRCDIR --> Location of the Makefile
# OUTDIR --> Location of the building generated files 
# ----------------------------------------------------------------------------

ifeq ($(HOST),Windows)
  fullpath = $(subst /,\,$(abspath $1))
else
  ifeq ($(DIRMODE),msys)
    fullpath = $(call windrv,$(abspath $1))
  else
    fullpath = $(abspath $1)
  endif
endif

ifndef SRCDIR
  src_dir := $(abspath $(dir $(SOURCE_MK)))
  ifeq ($(HOST),Windows)
    SRCDIR := $(subst /,\,$(src_dir))
  else
  ifeq ($(DIRMODE),msys)
    SRCDIR := $(call windrv,$(src_dir))
# SRCDIR := $(src_dir)
  else
    SRCDIR := $(src_dir)
   endif
  endif
endif

ifndef OUTDIR
  ifeq ($(HOST),Windows)
    OUTDIR := $(subst /,\,$(abspath $(out_dir)))
  else
  ifeq ($(DIRMODE),msys)
    OUTDIR := $(call windrv,$(abspath $(out_dir)))
  else
    OUTDIR := $(abspath $(out_dir))
  endif
  endif
endif

export Q ACTION

