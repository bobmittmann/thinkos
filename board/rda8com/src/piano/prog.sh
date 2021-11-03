#!/bin/sh

TOOLS_DIR=../../../../tools
PYTHON=python
if [ -z "$JTAGTOOL_ADDR" ]; then
	JTAGTOOL_ADDR=192.168.10.51
fi
# Collect ".bin" files in the positional parameters
set -- `ls release/*.app`
# Get the last one
for PROG_BIN; do true; done

${PYTHON} ${TOOLS_DIR}/tftp_load.py -q -i -e -r  -a 0x0800a000 \
	-h ${JTAGTOOL_ADDR} ${PROG_BIN} 

if [ $? = 0 ] ; then
	# Disable the halt debug mode by clearing C_DEBUGEN on DHCSR
	${PYTHON} ${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR} 'disable debug'
fi

