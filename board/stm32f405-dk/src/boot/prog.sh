#!/bin/sh

TOOLS_DIR=../../../../tools
PYTHON=python
if [ -z "$JTAGTOOL_ADDR" ]; then
	JTAGTOOL_ADDR=192.168.0.51
fi
# Collect ".bin" files in the positional parameters
set -- `ls release/*.bin`
# Get the last one
for PROG_BIN; do true; done

# Short remote calls
ICE_RUN="${PYTHON} ${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR}" 
ICE_LOAD="${PYTHON} ${TOOLS_DIR}/tftp_load.py -h ${JTAGTOOL_ADDR} -q -e"

#${ICE_RUN} 'nrst' 'nrst 1' 'tgt 9 f' 'trst' 'nrst 0' 'tgt c' 'connect' 'halt' 'init' 'erase flash' 'nrst' 'sleep 100' 

${ICE_RUN} 'nrst' 'nrst 1' 'tgt 9 f' 'trst' 'nrst 0' 'tgt c' 'connect' 'halt' 'init' 

${ICE_LOAD} -a 0x08000000 ${PROG_BIN} 

#if [ $? = 0 ] ; then
	# Disable the halt debug mode by clearing C_DEBUGEN on DHCSR
#	echo ${ICE_RUN} 'disable debug'
#fi

