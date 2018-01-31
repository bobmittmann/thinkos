#!/bin/sh

TOOLS_DIR=../../../tools
PYTHON=python
if [ -z "$JTAGTOOL_ADDR" ]; then
	JTAGTOOL_ADDR=192.168.10.51
fi
# Collect ".bin" files in the positional parameters
set -- `ls debug/*.bin`
# Get the last one
for PROG_BIN; do true; done
# The corresponding .elf
PROG_ELF=${PROG_BIN%%.bin}.elf

${PYTHON} ${TOOLS_DIR}/tftp_load.py -q -i -e -r -a 0x08000000 \
	-h ${JTAGTOOL_ADDR} ${PROG_BIN} 

if [ $? = 0 ] ; then
	# Disable the halt debug mode by clearing C_DEBUGEN on DHCSR
	${PYTHON} ${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR} \
	'nrst' 'connect' 'disable debug'
	if [ $? = 0 ] ; then
		# Trace
		${TOOLS_DIR}/dcclog -h ${JTAGTOOL_ADDR} ${PROG_ELF}
	fi
fi

