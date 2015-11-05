#!/bin/csh -f
#starting ingest pipeline
#ar: 15 may 2013
cd /globalhome/ingest/LTAIngest
setenv PYTHONPATH /globalhome/ingest/LTAIngest
if (! `ps uxf | grep -v grep | grep -c slave.py` ) then
  nohup slave.py ingest_config >& nohup_slave.out &
else
  echo "Slave is running already"
endif
cd /globalhome/ingest
