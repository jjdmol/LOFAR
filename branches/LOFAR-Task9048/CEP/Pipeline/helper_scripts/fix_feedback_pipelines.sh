#!/bin/bash
#
# Arguments:
# $1 = SASID

if [ -z $1 ]; then
  echo "Please provide SASID"
  exit
fi

sasid=$1

if [ "$USER" != "lofarsys" ]; then
  echo "Only as lofarsys"
  exit
fi

if [ ! -e /opt/lofar/var/run/Observation${sasid}_feedback ]; then 
  echo "Cannot find feedback file /opt/lofar/var/run/Observation${sasid}_feedback"
  exit
fi

scp -q /opt/lofar/var/run/Observation${sasid}_feedback ccu001:/opt/lofar/var/run/Observation${sasid}_feedback

if [ $? != 0 ]; then 
  echo "Problem copying feedback to ccu001:/opt/lofar/var/run/Observation${sasid}_feedback"
  exit
fi

ssh ccu001 uploadMetadata LOFAR_4 sas003 $sasid /opt/lofar/var/run/Observation${sasid}_feedback 1>/dev/null 

if [ $? != 0 ]; then
  echo "Problem putting feedback into OTDB using:"
  echo "ssh ccu001 uploadMetadata LOFAR_4 sas003 $sasid /opt/lofar/var/run/Observation${sasid}_feedback"
  exit
fi  

echo "Done"

