#!/bin/csh -f
#starting ingest pipeline
#ar: 28 august 2014
cd /home/lofarlocal/LTAIngest
setenv PYTHONPATH /home/lofarlocal/LTAIngest
if (! `ps uxf | grep -v grep | grep -c slave.py` ) then
  nohup ./slave.py ingest_config_test >& nohup_slave.out &
else
  echo "Slave is running already"
endif
