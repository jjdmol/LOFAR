#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
./runctest.sh tVICcomp 2>&1 > tVICcomp_test.log
rm -f DATABASENAME
