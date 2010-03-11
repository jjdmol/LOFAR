#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
./runctest.sh tPICvalue 2>&1 > tPICvalue_test.log
rm -f DATABASENAME
