#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
./runctest.sh tVTtree 2>&1 > tVTtree_test.log
rm -f DATABASENAME
