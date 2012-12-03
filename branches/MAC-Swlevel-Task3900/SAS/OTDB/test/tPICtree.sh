#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
./runctest.sh tPICtree 2>&1 > tPICtree_test.log
rm -f DATABASENAME
