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
#
# $Id$

if test "$LOFARROOT" == ""; then
  echo "LOFARROOT is not set! Exiting." >&2
  exit 1
fi

PARSET="$4"
OBSID="$5"

# The file to store the PID in
PIDFILE=$LOFARROOT/var/run/rtcp-$OBSID.pid

# The file we will log the observation output to
LOGFILE=$LOFARROOT/var/log/rtcp-$OBSID.log

function addlogprefix {
  ME="`basename "$0" .sh`@`hostname`"
  while read LINE
  do
    echo "$ME" "`date "+%F %T.%3N"`" "$LINE"
  done
}

(
# Always print a header, to match errors to observations
echo "---------------"
echo "called as: $0 $@"
echo "pwd:       $PWD"
echo "LOFARROOT: $LOFARROOT"
echo "obs id:    $OBSID"
echo "parset:    $PARSET"
echo "log file:  $LOGFILE"
echo "---------------"

function error {
  echo "$@" >&2
  exit 1
}

[ -n "$PARSET" ] || error "No parset provided"
[ -f "$PARSET" -a -r "$PARSET" ] || error "Cannot read parset: $PARSET"

TBB_PARSET=/globalhome/lofarsystem/log/L$OBSID.parset
echo "Copying parset to $TBB_PARSET for postprocessing"
cp "$PARSET" "$TBB_PARSET" || true
ln -sfT $TBB_PARSET /globalhome/lofarsystem/log/latest || true

# Start observation in the background
echo "Starting runObservation.sh -P $PIDFILE $PARSET"
runObservation.sh -P "$PIDFILE" "$PARSET" > $LOGFILE 2>&1 </dev/null &
PID=$!
echo "PID: $PID"

# Done
echo "Done"

) 2>&1 | addlogprefix | tee -a $LOFARROOT/var/log/startBGL.log

# Return the status of our subshell, not of tee
exit ${PIPESTATUS[0]}

