#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
$lofar_sharedir/runtest.sh tVHtree 2>&1 > tVHtree_test.log
rm -f DATABASENAME
