#!/bin/bash -l
#
# This script is started by the MAC Python Controller using the following
# parameters:
#
#   <pythonProgram> <parsetname> <3 fields with communication settings> 
#
# For example:
#
#   startPython.sh ./pythonProgram /opt/lofar/share/Observation6118 \
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
use LofIm

pythonProgram="${1}"
parsetFile="${2}"
programOptions=" \
 -d \
 -c ${LOFARROOT}/share/pipeline.cfg \
 -t ${LOFARROOT}/share/tasks.cfg \
 -r ${LOFARROOT}/share"
 
# Print some debugging information if debugging is enabled.
if [ -n "$debug" ]; then
  echo "PATH=${PATH}"
  echo "PYHONTPATH=${PYTHONPATH}"
  echo "LD_LIBRARY_PATH=${LD_LIBRARY_PATH}"
  echo "${pythonProgram} ${programOptions} ${parsetFile}"
fi

# Start the Python program.
${pythonProgram} ${programOptions} ${parsetFile}
