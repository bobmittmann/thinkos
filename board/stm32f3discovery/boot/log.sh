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

# Disable the halt debug mode by clearing C_DEBUGEN on DHCSR
${PYTHON} ${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR} 'disable debug'

if [ $? = 0 ] ; then
	# Trace
	${TOOLS_DIR}/dcclog -h ${JTAGTOOL_ADDR} ${PROG_ELF}
fi
