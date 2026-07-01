#!/bin/sh

<<<<<<< HEAD
TOOLS_DIR=../../../thinkos/tools
=======
TOOLS_DIR=../../../../tools
>>>>>>> krnsvc
PYTHON=python
if [ -z "$JTAGTOOL_ADDR" ]; then
	JTAGTOOL_ADDR=192.168.10.50
fi
<<<<<<< HEAD
# Collect ".bin" files in the positional parameters
set -- `ls release/*.bin`
# Get the last one
for PROG_BIN; do true; done

${PYTHON} ${TOOLS_DIR}/tftp_load.py -q -i -e -r  -a 0x0800C000 \
	-h ${JTAGTOOL_ADDR} ${PROG_BIN} 
=======
# Collect ".app" files in the positional parameters
set -- `ls release/*.app`
# Get the last one
for PROG_APP; do true; done

${PYTHON} ${TOOLS_DIR}/tftp_load.py -q -i -e -r  -a 0x08020000 \
	-h ${JTAGTOOL_ADDR} ${PROG_APP} 
>>>>>>> krnsvc

if [ $? = 0 ] ; then
	# Disable the halt debug mode by clearing C_DEBUGEN on DHCSR
	${PYTHON} ${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR} 'disable debug'
fi

