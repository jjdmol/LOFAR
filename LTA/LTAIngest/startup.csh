#!/bin/csh -f
#starting ingest pipeline
# $Id$

set LTAINGEST_PATH = /globalhome/ingest/LTAIngest
echo "LTAINGEST_PATH=$LTAINGEST_PATH"
cd $LTAINGEST_PATH
if ( ! ($?PYTHONPATH) ) then
  setenv PYTHONPATH $LTAINGEST_PATH
else if ( $LTAINGEST_PATH !~ $PYTHONPATH ) then
  setenv PYTHONPATH $LTAINGEST_PATH\:$PYTHONPATH
endif

echo "PYTHONPATH=$PYTHONPATH"

if (! `ps uxf | grep -v grep | grep -c master.py` ) then
  echo "Starting Master"
  nohup python ./master.py ingest_config >& nohup.out &
else
  echo "Master running already"
endif
sleep 1
if (! `ps uxf | grep -v grep | grep -c slave.py` ) then
  echo "Starting Slave"
  nohup python ./slave.py ingest_config >& nohup_slave.out &
else
  echo "Slave is running already"
endif
