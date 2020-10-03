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

LIB_STATIC = thinkos-mon

COMM_CFILES = null_comm.c usb_comm.c usb_comm_queue.c

COMMIO_CFILES = monitor_putc.c monitor_puts.c \
		monitor_getc.c monitor_gets.c \
		monitor_printf.c monitor_scanf.c \
		monitor_ymodem.c monitor_send_misc.c \
		print_osinfo.c print_alloc.c print_thread.c \
		print_context.c print_stack_usage.c \
		print_exception.c print_profile.c  \
		monitor_hexdump.c

CFILES = thinkos_monitor.c monitor_thread.c monitor_memory.c \
	 thinkos_console.c monitor_console_task.c \
	 monitor_flash.c \
	 app_exec.c app_erase.c flash_ymodem.c \
	 $(addprefix comm/,$(COMM_CFILES)) \
	 $(addprefix commio/,$(COMMIO_CFILES))

SFILES = monitor_swap.S

include ../../mk/lib.mk

