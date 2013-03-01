#!/bin/bash -l
#
# This script is called by MAC PythonControl to start a pipeline, using the
# following parameters:
#
#   <pythonProgram> <parsetname> <3 fields with communication settings> 
#
# For example:
#
#   startPython.sh ./pythonProgram /opt/lofar/var/run/Observation6118 \
#                  MCU001T MCU001T:PythonControl[0]{6118}:listener  \
#                  PythonServer{6118}@MCU001T 
#
# For the time being, we will ignore the 3 communication settings, because
# we will not be under control of MAC yet.
#

# Make sure aliases are expanded, which is not the default for non-interactive
# shells.
shopt -s expand_aliases

# Enable debugging messages
debug=on

# Log-file used for logging output of this script
logFile=/opt/lofar/var/log/startPython.log

usage()
{
  echo "Usage: $0 <pythonProgram> <parsetname> <MAC-Python-control-host> \\"
  echo "         <MAC-Python-control-listener> <MAC-Python-control-server>"
  exit 1
}

# Check for correct number of input arguments.
[ $# -eq 5 ] || usage

# Initialize the environment. We will assume here that we can use the
# Lofar Login Environment (LLE).
use Lofar

pythonProgram="${1}"
parsetFile="${2}"
controlHost="${3}"

echo "**** $(date) ****" >> ${logFile}
# Try to run a script that resets the invironment based on a parset
# value
if [ -r $(dirname $0)/startPythonVersion.sh ]; then
  . $(dirname $0)/startPythonVersion.sh >> ${logFile}
else
  echo "startPythonVersion.sh not found, parset software version ignored"
fi

programOptions=" \
 -d \
 -c ${LOFARROOT}/share/pipeline/pipeline.cfg \
 -t ${LOFARROOT}/share/pipeline/tasks.cfg \
 -r ${LOFARROOT}/var/run/pipeline \
 -w /data/scratch/${USER}"
  
# Print some debugging information if debugging is enabled.
if [ -n "$debug" ]; then
  echo "$0 $@" >> ${logFile}
  echo "PATH=${PATH}" >> ${logFile}
  echo "PYTHONPATH=${PYTHONPATH}" >> ${logFile}
  echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}" >> ${logFile}
  echo "${pythonProgram} ${programOptions} ${parsetFile} ${controlHost}" \
    >> ${logFile}
fi

# Start the Python program in the background. 
# This script should return ASAP so that MAC can set the task to ACTIVE.
# STDERR will be redirected to the log-file.
${pythonProgram} ${programOptions} ${parsetFile} ${controlHost} \
    1> /dev/null 2>> ${logFile} &

# Check if the Python program died early. If so, this indicates an error.
sleep 1
if ! kill -0 $! 2> /dev/null; then
  echo "$(date): FATAL ERROR: ${pythonProgram} died unexpectedly."
  exit 1
fi

