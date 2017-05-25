#!/bin/sh

PYTHON=python
if [ -z "$JTAGTOOL_ADDR" ]; then
	JTAGTOOL_ADDR=192.168.10.51
fi
BUILD_NAME=usbamp
TOOLS_DIR=../../../tools

${PYTHON} ${TOOLS_DIR}/tftp_load.py -q -i -e -r  -a 0x08020000 -h \
		${JTAGTOOL_ADDR} Release/${BUILD_NAME}.bin 

