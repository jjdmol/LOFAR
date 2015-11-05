#!/bin/bash
#
# Runs an observation, in the foreground, logging to stdout/stderr.
#
# This script takes care of running all the commands surrounding mpirun.sh,
# based on the given parset.
#
# $Id$


########  Functions  ########

function error {
  echo -e "$@" >&2
  exit 1
}

function getkey {
  KEY=$1
  DEFAULT=$2

  # grab the last key matching "^$KEY=", ignoring spaces.
  VALUE=`<$PARSET perl -ne '/^'$KEY'\s*=\s*"?(.*?)"?\s*$/ || next; print "$1\n";' | tail -n 1`

  if [ "$VALUE" == "" ]
  then
    echo "$DEFAULT"
  else
    echo "$VALUE"
  fi
}

function setkey {
  KEY=$1
  VAL=$2

  # In case already there, comment all out to avoid stale warnings. Then append.
  KEYESC=`echo "$KEY" | sed -r -e "s/([\.[])/\\\\\\\\\1/g"`  # escape '.' '[' chars in keys with enough '\'
  sed -i --follow-symlinks -r -e "s/^([[:blank:]]*$KEYESC[[:blank:]]*=)/#\1/g" "$PARSET"
  echo "$KEY = $VAL" >> "$PARSET"
}

function usage {
  error \
    "\nUsage: $0 [-A] [-C] [-F] [-P pidfile] [-l nprocs] [-p] PARSET"\
    "\n"\
    "\n  Run the observation specified by PARSET"\
    "\n"\
    "\n    -A: do NOT augment parset"\
    "\n    -C: run with check tool specified in environment variable"\
    "LOFAR_CHECKTOOL"\
    "\n    -F: do NOT send feedback to OnlineControl"\
    "\n    -P: create PID file"\
    "\n    -l: run solely on localhost using 'nprocs' MPI processes (isolated test)"\
    "\n    -p: enable profiling\n"
}

# command_retry expects a string it will execute as a subprocess
# It wait on the processes finish (using the PID) with a
# succesfull return value
# - On signals kill the child process
# - On non zero return value of the command it will retry
#   with increasingly larger wait periods between tries
function command_retry {
  COMMAND="$1"   
  SLEEP_DURATION=1                          # Increasing wait duration
  while :
  do 
    $COMMAND &                              # Run the command
    SSH_PID=$!                              # get the PID

    # Trap 'all' signals and forward to ssh process
    TRAP_COMMAND="kill $SSH_PID; break"
    trap "$TRAP_COMMAND" SIGTERM SIGINT SIGQUIT SIGHUP 2> /dev/null
    wait $SSH_PID && break       # wait while the ssh command is up
                                 # Break the loop if the command returned with exit value 0

    sleep $SLEEP_DURATION                  # Sleep if ssh failed
    SLEEP_DURATION=$((SLEEP_DURATION + 1)) # Increase duration   
  done
}

#############################

echo "Called as: $0 $@"

# ******************************
# Set default options
# ******************************

# Provide feedback to OnlineControl?
ONLINECONTROL_FEEDBACK=1

# Augment the parset with etc/parset-additions.d/* ?
AUGMENT_PARSET=1

# File to write PID to
PIDFILE=""

# Force running on localhost instead of the hosts specified
# in the parset?
FORCE_LOCALHOST=0
NRPROCS_LOCALHOST=0

# Parameters to pass to mpirun
MPIRUN_PARAMS=""

# Parameters to pass to rtcp
RTCP_PARAMS=""

# Avoid passing on "*" if it matches nothing
shopt -s nullglob

# ******************************
# Parse command-line options
# ******************************
while getopts ":ACFP:l:p" opt; do
  case $opt in
      A)  AUGMENT_PARSET=0
          ;;
      C)  CHECK_TOOL="$LOFAR_CHECKTOOL"
          ;;
      F)  ONLINECONTROL_FEEDBACK=0
          ;;
      P)  PIDFILE="$OPTARG"
          ;;
      l)  FORCE_LOCALHOST=1
          MPIRUN_PARAMS="$MPIRUN_PARAMS -np $OPTARG"
          ;;
      p)  RTCP_PARAMS="$RTCP_PARAMS -p"
          ;;
      \?) echo "Invalid option: -$OPTARG" >&2
          exit 1
          ;;
      :)  echo "Option requires an argument: -$OPTARG" >&2
          exit 1
          ;;
  esac
done

# Remove parsed options
shift $((OPTIND-1))

# Obtain parset parameter
PARSET="$1"

# Show usage if no parset was provided
[ -n "$PARSET" ] || usage

# Check if LOFARROOT is set.
[ -n "$LOFARROOT" ] || error "LOFARROOT is not set!"
echo "LOFARROOT is set to $LOFARROOT"

# ******************************
# Preprocess: initialise
# ******************************

# Write PID if requested
if [ "$PIDFILE" != "" ]
then
  echo $$ > "$PIDFILE"

  # We created the PIDFILE, so we
  # clean it up.
  trap "rm -f $PIDFILE" EXIT
fi

# Test the -k option of timeout(1). It only appeared since GNU coreutils 8.5 (fails on DAS-4).
timeout -k2 1 /bin/true 2> /dev/null && KILLOPT=-k2

# Read parset
[ -f "$PARSET" -a -r "$PARSET" ] || error "Cannot read parset: $PARSET"

OBSID=`getkey Observation.ObsID`
echo "Observation ID: $OBSID"

# Remove stale feedback file (useful for testing)
FEEDBACK_FILE=$LOFARROOT/var/run/Observation${OBSID}_feedback
rm -f $FEEDBACK_FILE

# ******************************
# Preprocess: augment parset
# ******************************

if [ "$AUGMENT_PARSET" -eq "1" ]
then
  AUGMENTED_PARSET=$LOFARROOT/var/run/rtcp-$OBSID.parset

  # Add static keys
  # Ignore sneaky .cobalt/ parset overrides in production (lofarsys).
  # Note: If you want such an override anyway, do it in your own account.
  if [ "$USER" == "lofarsys" ]; then
    ls $DOT_COBALT_DEFAULT $DOT_COBALT_OVERRIDE >/dev/null 2>&1 && \
      echo -e "WARNING: ignoring augmentation parset(s) in $HOME/.cobalt/" >&2
  else
    DOT_COBALT_DEFAULT=$HOME/.cobalt/default/*.parset
    DOT_COBALT_OVERRIDE=$HOME/.cobalt/override/*.parset
  fi
  cat $LOFARROOT/etc/parset-additions.d/default/*.parset \
      $DOT_COBALT_DEFAULT \
      $PARSET \
      $LOFARROOT/etc/parset-additions.d/override/*.parset \
      $DOT_COBALT_OVERRIDE \
      > $AUGMENTED_PARSET || error "Could not create parset $AUGMENTED_PARSET"
  unset DOT_COBALT_DEFAULT DOT_COBALT_OVERRIDE

  # Use the new one from now on
  PARSET="$AUGMENTED_PARSET"

  # If we force localhost, we need to remove the node list, or the first one will be used
  # Also set all other location relevant keys to a localhost value
  if [ "$FORCE_LOCALHOST" -eq "1" ]
  then
    setkey Cobalt.Nodes                               []

    setkey Cobalt.OutputProc.userName                 "$USER"
    setkey Cobalt.OutputProc.executable               "$LOFARROOT/bin/outputProc"
    setkey Cobalt.OutputProc.StaticMetaDataDirectory  "$LOFARROOT/etc"
    setkey Cobalt.FinalMetaDataGatherer.host          localhost
    setkey Cobalt.FinalMetaDataGatherer.executable    "$LOFARROOT/bin/FinalMetaDataGatherer"
    setkey Cobalt.FinalMetaDataGatherer.database.host localhost
    setkey Cobalt.Feedback.host                       localhost
    setkey Cobalt.Feedback.remotePath                 "$LOFARROOT/var/run"
  fi
fi

# ******************************
# Run the observation
# ******************************

# Determine node list to run on
if [ "$FORCE_LOCALHOST" -eq "1" ]
then
  HOSTS=localhost
else
  HOSTS=`mpi_node_list -n "$PARSET"`
fi

echo "Hosts: $HOSTS"

# Copy parset to all hosts
cksumline=`md5sum $PARSET`
for h in `echo $HOSTS | tr ',' ' '`
do
  # Ignore empty hostnames
  [ -z "$h" ] && continue;

  # Ignore hostnames that point to us
  [ "$h" == "localhost" ] && continue;
  [ "$h" == "`hostname`" ] && continue;

  # Ignore hosts that already have the same parset (for example, through NFS).
  timeout $KILLOPT 5s ssh -qn $h "[ -f $PARSET ] && echo \"$cksumline\" | md5sum -c --status" && continue

  # Copy parset to remote node
  echo "Copying parset to $h:$PARSET"
  timeout $KILLOPT 30s scp -Bq $PARSET $h:$PARSET || error "Could not copy parset to $h"
done

# ************************************
# Start outputProcs on receiving nodes
# ***********************************
# Get parameters from the parset
SSH_USER_NAME=$(getkey Cobalt.OutputProc.userName $USER)
SSH_PUBLIC_KEY=$(getkey Cobalt.OutputProc.sshPublicKey)
SSH_PRIVATE_KEY=$(getkey Cobalt.OutputProc.sshPrivateKey)
OUTPUT_PROC_EXECUTABLE=$(getkey Cobalt.OutputProc.executable)
OBSERVATIONID=$(getkey Observation.ObsID 0)

# If parameters are found in the parset create a key_string for ssh command
if [ "$SSH_PRIVATE_KEY" != "" ] 
then
  # Use -i to signal usage of the specific key
  KEY_STRING="-i $SSH_PRIVATE_KEY"   
elif [ "$SSH_PUBLIC_KEY" != "" ]
then
  KEY_STRING="-i $SSH_PUBLIC_KEY"
fi

# test the connection with local host: minimal test for valid credentials
ssh -l $SSH_USER_NAME $KEY_STRING "localhost" "/bin/true" || { echo "Failed to create a connection to localhost, ssh error" ; exit 1; }

# Create a helper function for delete child processes and
# a file containing the PID of these processes
PID_LIST_FILE="$LOFARROOT/var/run/outputProc-$OBSERVATIONID.pids"



# Function clean_up will clean op all PID in the
# PID_LIST_FILE and the seperately supplied additional list of PIDs
# in argument $2
# First using SIGTERM and then with a kill -9
# It will then exit with the state suplied in argument $1
function clean_up { 
  EXIT_STATE=$1
  PID_LIST=$2
  
  echo "Cleaning up child processes. Sending SIGTERM" 
  # THe kill statements might be called with an empty argument. This will 
  # result in an exit state 1. But the error is redirected to dev/null.
  kill $(cat $PID_LIST_FILE)  2> /dev/null
  kill $PID_LIST              2> /dev/null 
  
  echo "Waiting 2 seconds for soft shutdown"
  sleep 2
  
  echo "Sending SIGKILL"
  kill -9 $(cat $PID_LIST_FILE) 2> /dev/null
  kill -9 $PID_LIST             2> /dev/null
  
  echo "removing Childprocess pid list file"
  rm -f $PID_LIST_FILE
  
  exit $EXIT_STATE 
}

# We can now create a trap for signals, no arguments so only
# clean of the PID file list the list of sub shells
trap 'clean_up 1' SIGTERM SIGINT SIGQUIT SIGHUP

# Start output procs in a seperate function
# Save file for started child processes
# Use helper program to get the list of hosts from parset
echo "outputProc processes are appended to the file: $PID_LIST_FILE"
touch $PID_LIST_FILE

LIST_OF_HOSTS=$(getOutputProcHosts $PARSET)
RANK=0
for HOST in $LIST_OF_HOSTS
do
  COMMAND="ssh -tt -l $SSH_USER_NAME $KEY_STRING $SSH_USER_NAME@$HOST $OUTPUT_PROC_EXECUTABLE $OBSERVATIONID $RANK"
  # keep a counter to allow determination of the rank (needed for binding to rtcp)
  RANK=$(($RANK + 1))   
  
  command_retry "$COMMAND" &  # Start retrying function in the background
  PID=$!                      # get the pid 
  
  echo -n "$PID " >> $PID_LIST_FILE  # Save the pid for cleanup
done

# ************************************
# Start rtcp 
# ***********************************

# Run in the background to allow signals to propagate
#
# -x LOFARROOT    Propagate $LOFARROOT for rtcp to find GPU kernels, config files, etc.
# -H              The host list to run on, derived earlier.
mpirun.sh -x LOFARROOT="$LOFARROOT" \
          -H "$HOSTS" \
          $MPIRUN_PARAMS \
          $CHECK_TOOL \
          `which rtcp` $RTCP_PARAMS "$PARSET" &
PID=$!

# Propagate SIGTERM
trap 'echo runObservation.sh: Received signal cleaning up child processes; clean_up 1 $PID' SIGTERM SIGINT SIGQUIT SIGHUP

# Wait for $COMMAND to finish. We use 'wait' because it will exit immediately if it
# receives a signal.
#
# Return code:
#   signal:    >128
#   no signal: return code of mpirun.sh
wait $PID
OBSRESULT=$?

echo "Result code of observation: $OBSRESULT"

if [ $OBSRESULT -ne 0 -a -s $FEEDBACK_FILE ]
then
  # There is a feedback file! Consider the observation as succesful,
  # to prevent crashes in the tear down from ruining an otherwise
  # perfectly good observation
  echo "Found feed-back file $FEEDBACK_FILE, considering the observation succesful."

  OBSRESULT=0
fi

# ******************************
# Post-process the observation
# ******************************

if [ "$ONLINECONTROL_FEEDBACK" -eq "1" ]
then
  ONLINECONTROL_USER=`getkey Cobalt.Feedback.userName $USER`
  ONLINECONTROL_HOST=`getkey Cobalt.Feedback.host`

  if [ $OBSRESULT -eq 0 ]
  then
    # ***** Observation ran successfully

    # Copy LTA feedback file to ccu001
    FEEDBACK_DEST=$ONLINECONTROL_USER@$ONLINECONTROL_HOST:`getkey Cobalt.Feedback.remotePath`

    echo "Copying feedback to $FEEDBACK_DEST"
    timeout $KILLOPT 30s scp $FEEDBACK_FILE $FEEDBACK_DEST
    FEEDBACK_RESULT=$?
    if [ $FEEDBACK_RESULT -ne 0 ]
    then
      echo "Failed to copy file $FEEDBACK_FILE to $FEEDBACK_DEST (status: $FEEDBACK_RESULT)"
      OBSRESULT=$FEEDBACK_RESULT
    fi
  fi

  # Communicate result back to OnlineControl
  ONLINECONTROL_RESULT_PORT=$((21000 + $OBSID % 1000))

  if [ $OBSRESULT -eq 0 ]
  then
    # Signal success to OnlineControl
    echo "Signalling success to $ONLINECONTROL_HOST"
    echo -n "FINISHED" > /dev/tcp/$ONLINECONTROL_HOST/$ONLINECONTROL_RESULT_PORT
  else
    # ***** Observation or sending feedback failed for some reason
    # Signal failure to OnlineControl
    echo "Signalling failure to $ONLINECONTROL_HOST"
    echo -n "ABORT" > /dev/tcp/$ONLINECONTROL_HOST/$ONLINECONTROL_RESULT_PORT
  fi
else
  echo "Not communicating back to OnlineControl"
fi

# clean up outputProc children
echo "Allowing 120 second for normal end of outputProc"
#    Set trap to kill the sleep in case of signals                    save the pid of sleep
( trap 'kill $SLEEP_PID' SIGTERM SIGINT SIGQUIT SIGHUP ; sleep 120&  SLEEP_PID=$!; echo "Starting forced cleanup outputProc:"; clean_up 0 ) & 
KILLER_PID=$!

# Waiting for the child processes to finish
LIST_OF_PIDS_TO_WAIT_FOR=$(cat $PID_LIST_FILE)
if [ "$LIST_OF_PIDS_TO_WAIT_FOR" != "" ] # if there are outputProc pid working
then
  echo "waiting for output procs"
  wait $(cat $PID_LIST_FILE)        2> /dev/null
fi

# All OutputProcs are now killed, cleanup of the killer
kill $KILLER_PID                  2> /dev/null

# Our exit code is that of the observation
# In production this script is started in the background, so the exit code is for tests.
exit $OBSRESULT

