#!/bin/sh

TOOLS_DIR=../../../../tools
PYTHON=python
if [ -z "$JTAGTOOL_ADDR" ]; then
	JTAGTOOL_ADDR=192.168.0.51
fi
# Collect ".bin" files in the positional parameters
set -- `ls debug/*.bin`
# Get the last one
for PROG_BIN; do true; done
# The corresponding .elf
PROG_ELF=${PROG_BIN%%.bin}.elf
# Short remote calls
ICE_RUN="${PYTHON} ${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR}" 
ICE_LOAD="${PYTHON} ${TOOLS_DIR}/tftp_load.py -h ${JTAGTOOL_ADDR} -q -e"

${ICE_RUN} 'nrst' 'nrst 1' 'tgt 9 f' 'trst' 'nrst 0' 'tgt c' 'connect' 'halt' 'init' 'sleep 100' 

${ICE_LOAD} -a 0x08000000 ${PROG_BIN} 

if [ $? = 0 ] ; then
#	${ICE_RUN} 'rst core' 'step' 'run'
#	${ICE_RUN} 'run'
	# Disable the halt debug mode by clearing C_DEBUGEN on DHCSR
#	${ICE_RUN} 'disable debug'
	if [ $? = 0 ] ; then
		# Trace
		${TOOLS_DIR}/dcclog -h ${JTAGTOOL_ADDR} ${PROG_ELF}
	fi
fi

