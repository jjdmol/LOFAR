#!/bin/sh
# do a hard copy until a variable is available
cp ../../../test/DATABASENAME .
./runctest.sh tCampaign 2>&1 > tCampaign_test.log
rm -f DATABASENAME
