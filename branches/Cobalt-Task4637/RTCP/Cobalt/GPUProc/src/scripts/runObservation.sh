#!/bin/bash

# Script to run an observation in production mode
# 
# Usage:
#   runObservation obsid parset

OBSID=$1
PARSET=$2

PIDFILE=$LOFARROOT/var/run/Observation_$OBSID.pid

# Run the observation, and record the PID for cancelObservation
rtcp $PARSET &
PID=$!
echo $PID > $PIDFILE
wait $PID
SUCCESS=$?
rm -f $PIDFILE

# The port to which to send the final signal to OnlineControl
ONLINECONTROL_STATUS_PORT=$((21000 + $OBSID % 1000))

# Inform OnlineControl 
if [ "$SUCCESS" == "0" ]
then
  scp $LOFARROOT/var/run/Observation_$OBSID.feedback ccu001:/opt/lofar/var/run
  echo -n "FINISHED" | netcat ccu001 $ONLINECONTROL_STATUS_PORT
else
  echo -n "ABORT" | netcat ccu001 $ONLINECONTROL_STATUS_PORT
fi

