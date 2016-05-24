#!/bin/bash -e

# Signals a specific obs id as ABORTED
#
# The following chain is executed:
#
#   setStatus(ABORTED)
#
# Syntax:
#
#   runPipeline.sh -o <obsid> || pipelineAborted.sh -o <obsid>

# ======= Defaults

# Obs ID
OBSID=

# Queue on which to post status changes
SETSTATUS_BUS=lofar.otdb.command

# ======= Parse command-line parameters

function usage() {
  echo "$0 -o OBSID [options]"
  echo ""
  echo "  -o OBSID           Task identifier"
  echo "  -B busname         Bus name to post status changes on (default: $SETSTATUS_BUS)"
  exit 1
}

while getopts "o:c:p:B:" opt; do
  case $opt in
    h)  usage
        ;;
    o)  OBSID="$OPTARG"
        ;;
    B)  SETSTATUS_BUS="$OPTARG"
        ;;
    \?) error "Invalid option: -$OPTARG"
        ;;
    :)  error "Option requires an argument: -$OPTARG"
        ;;
  esac
done
[ -z "$OBSID" ] && usage

# ======= Run

# Mark as aborted
setStatus.py -o $OBSID -s aborted -B $SETSTATUS_BUS || true

exit 0
