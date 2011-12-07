#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
./runctest.sh tConnection 2>&1 > tConnection_test.log
rm -f DATABASENAME
