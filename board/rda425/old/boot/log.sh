#!/bin/sh

JTAGTOOL_ADDR=192.168.0.51
BUILD_NAME=rda420-thinkos-0.1
TOOLS_DIR=../../../tools

#${TOOLS_DIR}/tftp_reset.py -q -h ${JTAGTOOL_ADDR} 
#if [ $? = 0 ] ; then
	# Disable the halt debug mode by clearing C_DEBUGEN on DHCSR
#	${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR} \
#		'disable poll' 'let dhcsr 0xa05f0000' 'enable poll'
#fi

if [ $? = 0 ] ; then
	${TOOLS_DIR}/dcclog -h ${JTAGTOOL_ADDR} debug/${BUILD_NAME}.elf
fi

