# File:		Makefile
# Author:   Robinson Mittmann (bobmittmann@gmail.com)
# Target:
# Comment:  
# Copyright(C) 2012 Bob Mittmann. All Rights Reserved.
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
# 

include ../../mk/config.mk

LIB_STATIC = thinkos-dbg

CFILES = null_comm.c usb_comm.c usb_comm_queue.c \
		 print_osinfo.c print_alloc.c print_thread.c \
		 print_context.c print_stack_usage.c \
		 app_exec.c flash_erase.c dmon_ymodem.c flash_ymodem.c \
		 print_exception.c dbgmon_thread.c dbgmon_memory.c \
		 dbgmon_printf.c dbgmon_scanf.c \
		 dbgmon_putc.c dbgmon_puts.c \
		 dbgmon_getc.c dbgmon_gets.c \
		 dbgmon_hexdump.c dbgmon_console_task.c \
		 dbgmon_comm_misc.c \
		 thinkos_dbgmon.c

SFILES = dbgmon_swap.S

include ../../mk/lib.mk

