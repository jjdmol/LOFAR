#!/bin/bash
COMMAND=$1

type getpid >&/dev/null || function getpid() {
  PID=DOWN

  if [ -f "$PIDFILE" ]
  then
    PID=`cat -- "$PIDFILE"`
  fi

  if [ ! -e /proc/$PID ]
  then
    PID=DOWN
  fi  
}

function isstarted() {
  [ "DOWN" != "$PID" ]
}

type setpid >&/dev/null || function setpid() {
  PID=$1

  if [ "x$PID" == "x" ]
  then
    exit
  fi

  echo "$PID" > "$PIDFILE"
}

type delpid >&/dev/null || function delpid() {
  rm -f -- "$PIDFILE"
}

function procname() {
  # the basename of this script, without its extension
  basename -- "$0" | sed 's/[.][^.]*$//g'
}

type start >&/dev/null || function start() {
  tail -F / >&/dev/null &
}

type stop >&/dev/null || function stop() {
  kill -15 "$PID"
}

PIDFILE="/tmp/`procname`-$USER.pid"
getpid

case $COMMAND in
  start) if ! isstarted
         then
           start && setpid $!
         fi
         ;;

  stop)  if isstarted
         then
           stop && delpid
         fi
         ;;

  status)
         SWLEVEL=$2
         printf "%d : %-25s %s\n" "$SWLEVEL" "`procname`" "$PID"
         ;;

  *)     echo "usage: $0 {start|stop|status}"
         ;;
esac

