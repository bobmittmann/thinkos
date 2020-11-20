#!/bin/sh

PYTHON=python
if [ -z "$JTAGTOOL_ADDR" ]; then
	JTAGTOOL_ADDR=192.168.10.50
fi
JTAGTOOL_ADDR=192.168.10.50
BUILD_NAME=pa75_poc
TOOLS_DIR=../../../tools

${PYTHON} ${TOOLS_DIR}/tftp_load.py -q -i -e -r  -a 0x08020000 -h \
		${JTAGTOOL_ADDR} Release/${BUILD_NAME}.bin 

