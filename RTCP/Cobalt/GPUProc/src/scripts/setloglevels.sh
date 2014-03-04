#!/bin/sh
# setloglevel.sh
#
# Change rootLogger=LOGLEVEL directives in .log_prop files.
# If no files are given, apply to installed .log_prop files under $LOFARROOT (if set).
#
# $Id$

SED=sed
LOGLEVEL_LIST='DEBUG|INFO|WARN|ERROR|FATAL'

usage()
{
  echo "Usage: $0 $LOGLEVEL_LIST [file.log_prop [...]]"
  echo
  echo 'If no filename(s) are given, but LOFARROOT is set, the change is applied to $LOFARROOT/etc/*.log_prop'
  exit 1
}


if [ $# -eq 0 ]; then
  echo -e 'Error: need at least 1 argument\n'
  usage
fi


LOGLEVEL=$1
shift
if [ "$LOGLEVEL" != "DEBUG" ] && [ "$LOGLEVEL" != "INFO"  ] && \
   [ "$LOGLEVEL" != "WARN"  ] && [ "$LOGLEVEL" != "ERROR" ] && \
   [ "$LOGLEVEL" != "FATAL" ]; then
  echo -e 'Error: Invalid log level argument specified\n'
  usage
fi

if [ $# -eq 0 ]; then  # $# is #filename args, i.e. after the shift above
  if [ "$LOFARROOT" = "" ]; then
    echo "Error: no filename set and LOFARROOT not set or empty"
    exit 1
  else
    FILENAMES="$LOFARROOT/etc/*.log_prop"  # default, relative to $LOFARROOT
  fi
else
  FILENAMES="$*"
fi

$SED -i --follow-symlinks -r -e "s/rootLogger=($LOGLEVEL_LIST)/rootLogger=$LOGLEVEL/g" $FILENAMES

