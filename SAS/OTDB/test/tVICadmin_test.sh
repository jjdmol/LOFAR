#!/bin/sh
cp ../../../test/VICcomponentFile.in .
$lofar_sharedir/runtest.sh tVICadmin 2>&1 > tVICadmin_test.log
