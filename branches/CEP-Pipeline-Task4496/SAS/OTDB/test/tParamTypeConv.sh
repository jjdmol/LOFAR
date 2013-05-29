#!/bin/sh
# do a hard copy until a variable is available
cp $srcdir/DATABASENAME .
./runctest.sh tParamTypeConv
rm -f DATABASENAME
