#!/bin/csh -f
#starting ingest pipeline, slave only on target
# $Id$

set LTAINGEST_PATH = /home/lofarlocal/LTAIngest
echo "LTAINGEST_PATH=$LTAINGEST_PATH"
cd $LTAINGEST_PATH
if ( ! ($?PYTHONPATH) ) then
  setenv PYTHONPATH $LTAINGEST_PATH
else if ( $LTAINGEST_PATH !~ $PYTHONPATH ) then
  setenv PYTHONPATH $LTAINGEST_PATH\:$PYTHONPATH
endif

echo "PYTHONPATH=$PYTHONPATH"

if (! `ps uxf | grep -v grep | grep -c slave.py` ) then
  echo "Starting Slave"
  nohup python ./slave.py ingest_test >& nohup_slave.out &
else
  echo "Slave is running already"
endif
