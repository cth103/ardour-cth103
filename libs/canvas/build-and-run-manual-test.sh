#!/bin/bash
#

if [ ! -f './canvas.cc' ]; then
    echo "This script must be run from within the libs/canvas directory";
    exit 1;
fi

debug=0
valgrind=0

if [ "$1" == "--debug" ]; then
    debug=1
    name=$2
elif [ "$1" == "--valgrind" ]; then
    valgrind=1
    name=$2
else
    name=$1
fi

waft --targets libcanvas-manual-test-$name
if [ "$?" != 0 ]; then
  exit
fi

srcdir=`pwd`
cd ../../build/default

libs='libs'

export LD_LIBRARY_PATH=$libs/audiographer:$libs/vamp-sdk:$libs/surfaces:$libs/surfaces/control_protocol:$libs/ardour:$libs/midi++2:$libs/pbd:$libs/rubberband:$libs/soundtouch:$libs/gtkmm2ext:$libs/appleutility:$libs/taglib:$libs/evoral:$libs/evoral/src/libsmf:$libs/timecode:$libs/canvas:$LD_LIBRARY_PATH

if [ "$debug" == "1" ]; then
    gdb ./libs/canvas/test/$name
elif [ "$valgrind" == "1" ]; then
    valgrind --tool="helgrind" ./libs/canvas/test/$name
else
    ./libs/canvas/test/$name
fi


