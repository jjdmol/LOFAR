#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
./runctest.sh tConverter 2>&1 > tConverter.log
rm -f DATABASENAME
