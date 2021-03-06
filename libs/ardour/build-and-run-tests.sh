#!/bin/bash
#
# Run libardour test suite.
#

if [ ! -f './tempo.cc' ]; then
    echo "This script must be run from within the libs/ardour directory";
    exit 1;
fi

waft --targets libardour-tests
if [ "$?" != 0 ]; then
  exit
fi

srcdir=`pwd`
cd ../../build/default

libs='libs'

export LD_LIBRARY_PATH=$libs/audiographer:$libs/vamp-sdk:$libs/surfaces:$libs/surfaces/control_protocol:$libs/ardour:$libs/midi++2:$libs/pbd:$libs/rubberband:$libs/soundtouch:$libs/gtkmm2ext:$libs/appleutility:$libs/taglib:$libs/evoral:$libs/evoral/src/libsmf:$libs/timecode:$LD_LIBRARY_PATH

export ARDOUR_PANNER_PATH=$libs/panners/2in2out:$libs/panners/1in2out:$libs/panners/vbap

if [ "$1" == "--debug" ]; then
        gdb ./libs/ardour/run-tests
else
        ./libs/ardour/run-tests
fi
