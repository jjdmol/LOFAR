#!/bin/sh
cp ../../../test/PICmasterfile.txt .
$lofar_sharedir/runtest.sh tPICadmin PICmasterfile.txt 2>&1 > tPICadmin_test.log
