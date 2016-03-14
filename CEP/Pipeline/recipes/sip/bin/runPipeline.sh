#!/bin/bash -e

# Entry script from MAC on non-CEP2 clusters (CEP2 uses startPython.sh).
#
# The following chain is executed:
#
#   setStatus(STARTED)
#   getParset()
#   (execute pipeline as specified in parset)
#   setStatus(COMPLETING)
#   setStatus(FINISHED)
#
# Syntax:
#
#   runPipeline.sh <obsid> || pipelineAborted.sh <obsid>

OBSID=$1
shift

if [ -z "$OBSID" ]; then
  echo "Usage: $0 <obsid> <pipeline parameters>"
  exit 1
fi

# Queue on which to post status changes
SETSTATUS_BUS=lofar.otdb.setStatus

# Mark as started
setStatus.py -o $OBSID -s active -b $SETSTATUS_BUS || true

# Fetch parset
PARSET=${LOFARROOT}/var/run/Observation${OBSID}.parset
getParset.py -o $OBSID >$PARSET

# Fetch parameters from parset
PROGRAM_NAME=$(getparsetvalue $PARSET "ObsSW.Observation.ObservationControl.PythonControl.programName")

# Run pipeline
OPTIONS=" -d $@"
  
# Set up the environment (information to propagate to the node scripts for monitoring and logging)
export LOFAR_OBSID="$OBSID"

# Start the Python program
echo "**** $(date) ****"
echo "Executing: ${PROGRAM_NAME} ${OPTIONS} ${PARSET}"
${PROGRAM_NAME} ${OPTIONS} ${PARSET}
RESULT=$?

# Process the result
setStatus.py -o $OBSID -s completing -b $SETSTATUS_BUS || true

if [ $RESULT -eq 0 ]; then
  # Wait for feedback to propagate
  sleep 60

  # Mark as succesful
  setStatus.py -o $OBSID -s finished -b $SETSTATUS_BUS || true
fi

# Propagate result to caller
exit $RESULT
