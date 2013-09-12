#!/bin/bash
#
# Usage: runObservation.sh parset
#
# Runs an observation, in the foreground, logging to stdout/stderr.
#
# This script takes care of running all the commands surrounding mpirun.sh,
# based on the given parset.

PARSET="$1"

function error {
  echo "$@"
  exit 1
}

function getkey {
  KEY=$1
  <$PARSET perl -ne '/^'$KEY'\s*=\s*"?(.*?)"?\s*$/ || next; print "$1";'
}

[ -n "$PARSET" ] || error "No parset specified"
[ -r "$PARSET" ] || error "Cannot read parset: $PARSET"

OBSID=`getkey Observation.ObsID`
echo "Observation ID: $OBSID"

# ******************************
# Run the observation
# ******************************

# Determine start parameters
HOSTS=`mpi_node_list -n "$PARSET"`
echo "Hosts: $HOSTS"

# Copy parset to all hosts
for h in `echo $HOSTS | tr ',' ' '`
do
  # Ignore empty hostnames
  [ -z "$h" ] && continue;

  # Ignore hostnames that point to us
  [ "$h" == "localhost" ] && continue;
  [ "$h" == "`hostname`" ] && continue;

  # Ignore hosts that already have the parset
  # (for example, through NFS).
  timeout 5s ssh -qn $h [ -e $PWD/$PARSET ] && continue;

  # Copy parset to remote node
  echo "Copying parset to $h:$PWD"
  timeout 30s scp -Bq $PARSET $h:$PWD || error "Could not copy parset to $h"
done

# Run in the background to allow signals to propagate
mpirun.sh -H "$HOSTS" rtcp "$PARSET" &
PID=$!

# Propagate SIGTERM
trap "echo runObservation.sh: killing $PID; kill $PID" SIGTERM SIGINT SIGQUIT SIGHUP

# Wait for $COMMAND to finish. We use 'wait' because it will exit immediately if it
# receives a signal.
#
# Return code:
#   signal:    >128
#   no signal: return code of mpirun.sh
wait $PID
OBSRESULT=$?

echo "Result code of observation: $OBSRESULT"

# ******************************
# Post-process the observation
# ******************************
#
# Note: don't propagate errors here as observation failure,
#       because that would be too harsh and also makes testing
#       harder.

ONLINECONTROL_HOST=`getkey Cobalt.Feedback.host`
ONLINECONTROL_RESULT_PORT=$((21000 + $OBSID % 1000))

if [ $OBSRESULT -eq 0 ]
then
  # ***** Observation ran successfully

  # 1. Copy LTA feedback file to ccu001
  FEEDBACK_DEST=$ONLINECONTROL_HOST:`getkey Cobalt.Feedback.remotePath`
  echo "Copying feedback to $FEEDBACK_DEST"
  timeout 30s scp $LOFARROOT/var/run/Observation${OBSID}_feedback $FEEDBACK_DEST

  # 2. Signal success to OnlineControl
  echo "Signalling success to $ONLINECONTROL_HOST"
  echo -n "FINISHED" > /dev/tcp/$ONLINECONTROL_HOST/$ONLINECONTROL_RESULT_PORT
else
  # ***** Observation failed for some reason

  # 1. Signal failure to OnlineControl
  echo "Signalling failure to $ONLINECONTROL_HOST"
  echo -n "ABORT" > /dev/tcp/$ONLINECONTROL_HOST/$ONLINECONTROL_RESULT_PORT
fi

# Our exit code is that of the observation
exit $OBSRESULT

