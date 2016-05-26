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

# The FIFO used for communication with rtcp
COMMANDPIPE=$LOFARROOT/var/run/rtcp-$OBSID.pipe

source cobalt_functions.sh

(
# Always print a header, to match errors to observations
echo "---------------"
echo "called as: $0 $@"
echo "pwd:       $PWD"
echo "LOFARROOT: $LOFARROOT"
echo "LOFARENV:  $LOFARENV"
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

# Export a copy of the parset to the TBB software
TBB_PARSET=/globalhome/lofarsystem/log/L$OBSID.parset
echo "Copying parset to $TBB_PARSET for postprocessing"
cp "$PARSET" "$TBB_PARSET" || true
ln -sfT $TBB_PARSET /globalhome/lofarsystem/log/latest || true

# Create the command pipe
[ -e "$COMMANDPIPE" ] && rm -f "$COMMANDPIPE"
mkfifo -m 0660 "$COMMANDPIPE" || true

# Construct command line
COMMAND="env LOFARENV=$LOFARENV runObservation.sh -P $PIDFILE -o Cobalt.commandStream=file:$COMMANDPIPE $PARSET"

# Process cluster requirements
read_cluster_model

if $SLURM; then
  # We need to issue "salloc" on the target cluster, and once the resources
  # are available, the job should first SSH back here to start the observation.

  # Note that we need to marshall some SLURM environment variables as well, hence the use of bash.
  for s in SLURM_JOB_ID SLURM_JOB_NODELIST SLURM_JOB_NUM_NODES; do
    SLURM_VARS+=" $s=\$$s"
  done

  COMMAND="ssh -tt $HEADNODE salloc -N $NRCOMPUTENODES -J $OBSID bash -c 'ssh `hostname -f` -tt $SLURM_VARS $COMMAND'"
fi

# Start observation in the background
echo "Starting $COMMAND"
$COMMAND > $LOGFILE 2>&1 </dev/null &
PID=$!
echo "PID: $PID"

# Done
echo "Done"

) 2>&1 | addlogprefix | tee -a $LOFARROOT/var/log/startBGL.log

# Return the status of our subshell, not of tee
exit ${PIPESTATUS[0]}

