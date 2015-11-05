#!/bin/bash

GET_VERSION=0
GET_ALL_VERSIONS=0
SET_VERSION=""
LIST_VERSIONS=0

function error() {
  echo "$@" >&2
  exit 1
}

function usage() {
  echo "$0 [-l] [-g] [-G] [-s VERSION]"
  echo ""
  echo "  -l            List available Cobalt versions"
  echo "  -g            Get active Cobalt version"
  echo "  -G            Get active Cobalt version on each Cobalt node"
  echo "  -s VERSION    Set active Cobalt version"
  exit 1
}

while getopts "hgGls:" opt; do
  case $opt in
    h)  usage
        ;;
    g)  GET_VERSION=1
        ;;
    G)  GET_ALL_VERSIONS=1
        ;;
    l)  LIST_VERSIONS=1
        ;;
    s)  SET_VERSION="$OPTARG"
        ;;
    \?) error "Invalid option: -$OPTARG"
        ;;
    :)  error "Option requires an argument: -$OPTARG"
        ;;
  esac
done

COBALT_VERSIONS_DIR=/localhome/lofar/lofar_versions
HOSTS="cbm001 cbm002 cbm003 cbm004 cbm005 cbm006 cbm007 cbm008 cbm009"

[ -d "$COBALT_VERSIONS_DIR" ] || error "Directory not found: $COBALT_VERSIONS_DIR"

# List Cobalt versions
if [ $LIST_VERSIONS -eq 1 ]; then
  ls -1 $COBALT_VERSIONS_DIR
fi

# Get current Cobalt version
if [ $GET_VERSION -eq 1 ]; then
  readlink -f /opt/lofar | awk -F/ '{ print $NF; }'
fi

# Get current Cobalt version on ALL hosts
if [ $GET_ALL_VERSIONS -eq 1 ]; then
  for HOST in $HOSTS; do
    echo -n "$HOST: "
    ssh $HOST "readlink -f /opt/lofar | awk -F/ '{ print $NF; }'" 2>/dev/null
  done
fi

# Set current Cobalt version
if [ -n "$SET_VERSION" ]; then
  [ -d $COBALT_VERSIONS_DIR/$SET_VERSION ] || error "Cobalt version $SET_VERSION not found."

  echo "Switching Cobalt to $SET_VERSION"

  RELEASE_NAME="$SET_VERSION" Cobalt_setcurrent.sh
fi

