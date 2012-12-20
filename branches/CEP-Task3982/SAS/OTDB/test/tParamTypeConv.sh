#!/bin/sh
# do a hard copy until a variable is available
cp $srcdir/DATABASENAME .
./runctest.sh tParamTypeConv 2>&1 > tParamTypeConv.log
rm -f DATABASENAME
