#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
$lofar_sharedir/runtest.sh tVICcomp 2>&1 > tVICcomp_test.log
rm -f DATABASENAME
