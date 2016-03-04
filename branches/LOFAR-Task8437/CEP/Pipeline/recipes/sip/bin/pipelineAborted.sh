#!/bin/bash -e

# Signals a specific obs id as ABORTED
#
# The following chain is executed:
#
#   setStatus(ABORTED)
#
# Syntax:
#
#   runPipeline.sh <obsid> || pipelineAborted.sh <obsid>

OBSID=$1

if [ -z "$OBSID" ]; then
  echo "Usage: $0 <obsid>"
  exit 1
fi

# Queue on which to post status changes
SETSTATUS_BUS=lofar.otdb.setStatus

# Mark as aborted
setStatus.py -o $OBSID -s aborted -b $SETSTATUS_BUS || true

exit 0
