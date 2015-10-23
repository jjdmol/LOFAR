#!/bin/echo Usage: source lofarlogger.sh
# 
# Usage: source lofarlogger.sh
# then e.g.: log INFO "foo $bar"
# logs e.g.: 2015-10-16 16:00:46,186 INFO foo bar
#
# $Id$

log() {
  loglevel=$1  # one of: DEBUG INFO WARNING ERROR CRITICAL
  message=$2
  ts=`date --utc '+%F %T,%3N'`  # e.g. 2015-10-16 16:00:46,186
  echo "$ts $loglevel $message" >&2
}

