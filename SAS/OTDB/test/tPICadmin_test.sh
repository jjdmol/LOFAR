#!/bin/sh
cp ../../../test/PICmasterfile.txt .
./tPICadmin PICmasterfile.txt 2>&1 > tPICadmin_test.log
