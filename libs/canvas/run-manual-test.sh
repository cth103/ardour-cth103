#!/bin/bash
#

if [ ! -f './canvas.cc' ]; then
    echo "This script must be run from within the libs/canvas directory";
    exit 1;
fi

srcdir=`pwd`
cd ../../build/default

libs='libs'

export LD_LIBRARY_PATH=$libs/canvas:$LD_LIBRARY_PATH

if [ "$1" == "--debug" ]; then
    gdb ./libs/canvas/test/$2
else
    ./libs/canvas/test/$1
fi


