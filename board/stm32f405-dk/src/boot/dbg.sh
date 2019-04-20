#!/bin/sh

TOOLS_DIR=../../../../tools
PYTHON=python
if [ -z "$JTAGTOOL_ADDR" ]; then
	JTAGTOOL_ADDR=192.168.10.50
fi
# JtagTool remote calls
ICE_RUN="${PYTHON} ${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR}" 
ICE_LOAD="${PYTHON} ${TOOLS_DIR}/tftp_load.py -h ${JTAGTOOL_ADDR} -q -e"
DCCLOG="${TOOLS_DIR}/dcclog -h ${JTAGTOOL_ADDR}"

# Collect ".bin" files in the positional parameters
set -- `ls debug/*.bin`
# Get the last one
for PROG_BIN; do true; done
# The corresponding .elf
PROG_ELF=${PROG_BIN%%.bin}.elf

${ICE_RUN} 'tgt 9 f c' 

${ICE_LOAD} -a 0x08000000 ${PROG_BIN} 

if [ $? = 0 ] ; then
	${ICE_RUN} 'tgt stm32f f c' 'run' 'disable debug'

	if [ $? = 0 ] ; then
		# Trace
		${DCCLOG} ${PROG_ELF} | tee out.log
	fi
fi

