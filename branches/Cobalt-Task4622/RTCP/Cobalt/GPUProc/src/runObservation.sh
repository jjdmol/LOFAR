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

[ -n "$PARSET" ] || error "No parset specified"

# Determine start parameters
HOSTS=`mpi_node_list -n "$PARSET"`
echo "Hosts: $HOSTS"

# Run in the background to allow signals to propagate
mpirun.sh -H "$HOSTS" rtcp "$PARSET" &
PID=$!

# Propagate SIGTERM
trap "echo runObservation.sh: killing $PID; kill $PID" SIGTERM

# Wait for $COMMAND to finish. We use 'wait' because it will exit immediately if it
# receives a signal.
#
# Return code:
#   signal:    >128
#   no signal: return code of mpirun.sh
wait $PID

