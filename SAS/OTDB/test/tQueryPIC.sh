#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
./runctest.sh tQueryPIC
rm -f DATABASENAME
