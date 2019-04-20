#!/bin/sh

TOOLS_DIR=../../../../tools
PYTHON=python
if [ -z "$JTAGTOOL_ADDR" ]; then
	JTAGTOOL_ADDR=192.168.0.51
fi

# Disable the halt debug mode by clearing C_DEBUGEN on DHCSR
${PYTHON} ${TOOLS_DIR}/tftp_cmd.py -h ${JTAGTOOL_ADDR} 'nrst' 'nrst 1' 'tgt 9 f' 'trst' 'nrst 0' 'tgt c' 'connect' 'halt' 'init' 'erase flash'


