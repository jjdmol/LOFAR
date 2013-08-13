#!/bin/bash

# Script to kill an observation in production mode
# 
# Usage:
#   cancelObservation obsid

OBSID=$1

PIDFILE=$LOFARROOT/var/run/Observation_$OBSID.pid

if [ ! -r $PIDFILE ]
then
  echo "File not found: $PIDFILE"
  exit 1
fi

PID=`cat $PIDFILE`
kill $PID

