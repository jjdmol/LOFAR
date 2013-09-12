#!/bin/bash
# startBGL.sh jobName executable workingDir parset observationID
#
# jobName
# executable      executable file (should be in a place that is readable from BG/L)
# workingDir      directory for output files (should be readable by BG/L)
# parset          the parameter file
# observationID   observation number
#
# This script is called by OnlineControl to start an observation.

PARSET_MAC="$4"
OBSID="$5"

# The name of what will be our parset
PARSET=$LOFARROOT/var/run/Observation$OBSID.parset

# The file to store the PID in
PIDFILE=$LOFARROOT/var/run/rtcp-$OBSID.pid

# Prepare environment, typically needed only once
mkdir -p $LOFARROOT/var/{run,log}

(
# Always print a header, to match errors to observations
echo "---------------"
echo "now:      " `date +"%F %T"`
echo "called as: $0 $@"
echo "pwd:       $PWD"
echo "LOFARROOT: $LOFARROOT"
echo "obs id:    $OBSID"
echo "parset:    $PARSET_MAC"
echo "---------------"

function error {
  echo "$@"
  exit 1
}

[ -n "$OBSID" ] || error "No observation ID provided on the command line"
[ -n "$PARSET_MAC" ] || error "No parset provided on the command line"

# Add static keys ($PARSET_MAC is last, to allow any key to be overridden in tests)
cat $LOFARROOT/etc/parset-additions.d/*.parset $PARSET_MAC > $PARSET || error "Could not create parset $PARSET"

# Start observation in the background
runObservation.sh "$PARSET" > $LOFARROOT/var/log/rtcp-$OBSID.log 2>&1 </dev/null &
PID=$!
echo "PID: $PID"

# Keep track of PID for stop script
echo "PID file: $PIDFILE"
echo $PID > $PIDFILE || error "Could not write PID file: $PIDFILE"

# Done
echo "Done"

) 2>&1 | tee -a $LOFARROOT/var/log/startBGL.log

# Return the status of our subshell, not of tee
exit ${PIPESTATUS[0]}
