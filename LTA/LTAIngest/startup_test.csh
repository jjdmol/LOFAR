#!/bin/csh -f
#starting ingest pipeline
#ar: 15 may 2013
cd /globalhome/ingesttest/LTAIngest
setenv PYTHONPATH /globalhome/ingesttest/LTAIngest
if (! `ps uxf | grep -v grep | grep -c master.py` ) then
  nohup ./master.py ingest_config_test >& nohup.out &
else
  echo "Master running already"
endif
sleep 1
if (! `ps uxf | grep -v grep | grep -c slave.py` ) then
  nohup ./slave.py ingest_config_test >& nohup_slave.out &
else
  echo "Slave is running already"
endif
