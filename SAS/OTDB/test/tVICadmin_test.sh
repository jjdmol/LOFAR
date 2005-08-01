#!/bin/sh
cp ../../../test/VICcomponentFile.in .
./tVICadmin 2>&1 > tVICadmin_test.log
