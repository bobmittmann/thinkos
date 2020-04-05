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
#   **                        Copyright 2018-2019                         **
#   ************************************************************************
#
#

#
# Author: Bob Mittmann <bmittmann@mircomgroup.com>
#

PROJECT_ROOT = ../../
THINKOS_DIR = $(PROJECT_ROOT)/../..
MK_DIR = $(THINKOS_DIR)/mk
SDK_DIR = $(THINKOS_DIR)/sdk
TOOLS_DIR = $(THINKOS_DIR)/tools

include $(MK_DIR)/config.mk

LIB_STATIC = mad
CDEFS := NDEBUG

CFILES = bit.c fixed.c frame.c huffman.c layer12.c layer3.c stream.c synth.c timer.c version.c

include $(MK_DIR)/lib.mk

