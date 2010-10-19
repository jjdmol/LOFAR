#!/bin/sh
./Example3 -s &> /dev/null &
$lofar_sharedir/runtest.sh Example3 -r > Example2_test.log 2>&1
