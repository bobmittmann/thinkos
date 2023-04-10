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
# Author: Bob Mittmann <bobmitmann@mircomgroup.com>
#

PROJECT_ROOT = ../../../
THINKOS_DIR = $(PROJECT_ROOT)/thinkos
MK_DIR = $(THINKOS_DIR)/mk

include $(MK_DIR)/config.mk

LIB_STATIC = mstplnk

CFILES = mstp_lnk_comm.c mstp_lnk_core.c mstp_lnk_crc.c mstp_lnk_debug.c\
		 mstp_lnk_master.c mstp_lnk_mgmt.c mstp_lnk_slave.c\
		 mstp_lnk_sniffer.c
#mstp_lnk_task.c

include $(MK_DIR)/lib.mk

