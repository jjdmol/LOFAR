#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
$lofar_sharedir/runtest.sh tVHvalue 2>&1 > tVHvalue_test.log
rm -f DATABASENAME
