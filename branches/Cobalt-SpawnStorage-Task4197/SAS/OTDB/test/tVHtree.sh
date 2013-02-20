#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
./runctest.sh tVHtree 2>&1 > tVHtree_test.log
rm -f DATABASENAME
