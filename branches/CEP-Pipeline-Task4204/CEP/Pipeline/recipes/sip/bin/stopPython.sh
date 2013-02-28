#!/bin/bash
#
# This script is called by MAC PythonControl to stop a pipeline.
# Usage:
#
#   stopPython.sh <observationID> 
#
# The script will try to kill the process with the PID that is stored in the
# file /opt/lofar/var/run/pipeline/<ObservationID>/pid
#
# If it succeeds, it will remove the PID-file and return a zero exit status;
# otherwise it will return a non-zero exit status.
#
# Exit statuses:
#  -1: incorrect number of arguments supplied
#   0: success; process was successfully killed 
#   1: PID-file does not exist or is unreadable
#   2: failed to send kill signal (no such process, or operation not permitted)
#   3: process did not terminate within timeout period (30 seconds)
#

# Print usage message and exit with an error status.
usage()
{
  echo "Usage: $0 <observationID>"
  exit -1
}

# Print an error message and exit with an error status.
# The first argument must be the exit status, the remaining arguments will
# printed as error message.
error()
{
  local status="$1"
  shift
  echo "$(date +'%F %T') ERROR: $*" | tee -a ${logFile}
  exit $status
}

# Check for correct number of input arguments.
[ $# -eq 1 ] || usage

# Log-file used for logging output of this script.
logFile="/opt/lofar/var/log/stopPython.log"

# File containing the ID of the process that must be killed.
pidFile="/opt/lofar/var/run/pipeline/${1}/pid"

# Check if the PID-file exists.
[ -r ${pidFile} ] || error 1 "${pidFile} does not exist or is unreadable"

# Read the PID from the PID-file.
pid=$(cat ${pidFile})

# Try to kill the process with ID $pid.
kill ${pid} 2>/dev/null || error 2 "kill (${pid}) - no such process, or" \
                                   "operation not permitted"

# Give the process some time to stop.
seconds=30
while kill -0 ${pid} 2>/dev/null
do
  ((seconds-- > 0)) || error 3 "process (${pid}) did not terminate within" \
                               "30 seconds"
  sleep 1
done

# Remove the PID-file
rm -f ${pidFile}

exit 0
