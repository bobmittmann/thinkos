#!/bin/sh

PYTHON=python
JTAGTOOL_ADDR=192.168.10.51
BUILD_NAME=thinkos
TOOLS_DIR=../../../tools

${PYTHON} ${TOOLS_DIR}/tftp_load.py -q -i -e -r -a 0x08000000 \
		-h ${JTAGTOOL_ADDR} debug/${BUILD_NAME}.bin 

if [ $? = 0 ] ; then
	# Disable the halt debug mode by clearing C_DEBUGEN on DHCSR
	${PYTHON} ${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR} \
		'disable poll' 'let dhcsr 0xa05f0000' 'enable poll' 'sleep 100' 'reset' 
fi

if [ $? = 0 ] ; then
	${TOOLS_DIR}/dcclog -h ${JTAGTOOL_ADDR} debug/${BUILD_NAME}.elf | tee dbg.log
fi

