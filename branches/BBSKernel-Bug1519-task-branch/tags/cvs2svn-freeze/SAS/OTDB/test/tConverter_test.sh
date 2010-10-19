#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
$lofar_sharedir/runtest.sh tConverter 2>&1 > tConverter.log
rm -f DATABASENAME
