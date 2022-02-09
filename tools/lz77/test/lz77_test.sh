#!/bin/bash

IN=test.bin
LZ=test.lz
OUT=test.bout
LZ77=../lz77

function do_test {
	SIZ=$1 
	WND=$2

	dd if=/dev/urandom bs=1 count=${SIZ} of=${IN}

	${LZ77} -vv -w ${WND} -c -o ${LZ} ${IN}
	${LZ77} -vv -w ${WND} -o ${OUT} ${LZ}
	md5sum ${IN} ${OUT}
}

do_test 1234045 8192
do_test 1234045 1024

