#!/bin/bash

GET_VERSION=0
GET_ALL_VERSIONS=0
SET_VERSION=""
LIST_VERSIONS=0

function error() {
  echo "$@" >&2
  exit 1
}

if [ -z "$HOSTS" ]; then
  case `hostname` in
    cbt001|cbt002|cbt003|cbt004|cbt005|cbt006|cbt007|cbt008)
      # Production system -- switch test system as well, in case we use cbm009/cbm010!
      HOSTS="cbm001 cbm002 cbm003 cbm004 cbm005 cbm006 cbm007 cbm008 cbm009 cbm010"
      ;;

    cbt009|cbt010)
      # Test system
      HOSTS="cbm009 cbm010"
      ;;

    *)
      echo "WARNING: Cannot derive \$HOSTS variable. Will only operate on localhost"
      HOSTS="localhost"
      ;;
  esac
fi

function usage() {
  echo "$0 [-l] [-g] [-G] [-s VERSION]"
  echo ""
  echo "  -l            List available Cobalt versions (localhost)"
  echo "  -g            Get active Cobalt version (localhost)"
  echo "  -G            Get active Cobalt version on each Cobalt node (\$HOSTS)"
  echo "  -s VERSION    Set active Cobalt version (\$HOSTS)"
  echo ""
  echo "\$HOSTS is set to '$HOSTS'"
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
[ $OPTIND -eq 1 ] && usage

COBALT_VERSIONS_DIR=/localhome/lofar/lofar_versions

[ -d "$COBALT_VERSIONS_DIR" ] || error "Directory not found: $COBALT_VERSIONS_DIR"

# List Cobalt versions
if [ $LIST_VERSIONS -eq 1 ]; then
  ls -1 $COBALT_VERSIONS_DIR
fi

CURRENT_VERSION=`readlink -f /opt/lofar | awk -F/ '{ print $NF; }'`

# Get current Cobalt version
if [ $GET_VERSION -eq 1 ]; then
  echo "$CURRENT_VERSION"
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
  echo "Switching Cobalt to $SET_VERSION"

  function set_version {
    VERSION="$1"
    for HOST in $HOSTS; do
      echo "$HOST"
      ssh $HOST "[ -d \"${COBALT_VERSIONS_DIR}/${VERSION}\" ] && ln -sfT \"${COBALT_VERSIONS_DIR}/${VERSION}\" /localhome/lofarsystem/lofar/current" || return 1
    done

    return 0
  }

  if ! set_version "$SET_VERSION"; then
    echo "------------------------------------------------------------------------------"
    echo "ERROR Switching to $SET_VERSION. Switching back to $CURRENT_VERSION"
    echo "------------------------------------------------------------------------------"

    set_version "$CURRENT_VERSION"
  fi
fi

