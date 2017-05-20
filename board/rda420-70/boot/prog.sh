#!/bin/sh

PYTHON=python
JTAGTOOL_ADDR=192.168.0.51
BUILD_NAME=rda420-thinkos-0.1
TOOLS_DIR=../../../tools

${PYTHON} ${TOOLS_DIR}/tftp_load.py -q -i -e -r  -a 0x08000000 \
		-h ${JTAGTOOL_ADDR} release/${BUILD_NAME}.bin 

if [ $? = 0 ] ; then
	# Disable the halt debug mode by clearing C_DEBUGEN on DHCSR
	${PYTHON} ${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR} \
		'disable poll' 'let dhcsr 0xa05f0000' 'enable poll'
fi

