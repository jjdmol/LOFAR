#!/bin/bash
#
# Generates parset keys for the start and stop times of an observation,
# based on the current system time.
#

STARTDELAY=60
RUNTIME=120

function usage() {
  echo "Usage: $0 [-s startdelay] [-r runtime]"
  echo "       $0 -h"
  echo " "
  echo "-h              print this help message"
  echo "-s startdelay   start in 'startdelay' seconds [$STARTDELAY]"
  echo "-r runtime      stop after 'runtime' seconds [$RUNTIME]"

  exit 1;
}

while getopts "s:r:h" opt; do
  case $opt in
    s) STARTDELAY="$OPTARG"
       ;;
    r) RUNTIME="$OPTARG"
       ;;
    h) usage;
       ;;

    \?) echo "Invalid option: -$OPTARG" >&2
        exit 1;
        ;;

    :)  echo "Option requires an argument: -$OPTARG" >&2
        exit 1;
        ;;
  esac
done

# Calculate start and stop times in seconds since 1970 UTC
NOW=`date -u +%s`
START=$(($NOW + $STARTDELAY))
STOP=$(($START + $RUNTIME))

# The date format used in the parset
FORMAT="%F %T"

# Echo the resulting parset keys
echo "Observation.startTime = `date -u -d @$START +"$FORMAT"`"
echo "Observation.stopTime  = `date -u -d @$STOP  +"$FORMAT"`"

