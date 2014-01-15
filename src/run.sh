#!/bin/bash
# reads MIPS assembly language code from stdin, runs it in the MIPS emulator

dir=`dirname $0`
if [ ! "$1" ]; then
    echo "Usage: $0 [asm_file]"
    exit 1
elif [ ! -x "$dir/assemble" ]; then
    echo "assemble does not exist or is not executable; run make"
    exit 2
elif [ ! -x "$dir/emulate" ]; then
    echo "bin/emulate does not exist or is not executable; run make"
    exit 3
fi

tmpfile=`mktemp`
$dir/assemble $1 > $tmpfile
if [ "$?" -ne 0 ]; then rm $tmpfile; exit 3; fi
$dir/emulate $tmpfile
if [ "$?" -ne 0 ]; then rm $tmpfile; exit 4; fi
rm $tmpfile
