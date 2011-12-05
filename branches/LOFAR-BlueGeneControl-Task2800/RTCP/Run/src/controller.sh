#!/bin/bash
COMMAND=$1

type getpid >&/dev/null || function getpid() {
  PID=DOWN
  PIDFILE="/tmp/`procname`-$USER.pid"

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
  PIDFILE="/tmp/`procname`-$USER.pid"

  if [ "x$PID" == "x" ]
  then
    exit
  fi

  echo "$PID" > "$PIDFILE"
}

type delpid >&/dev/null || function delpid() {
  PIDFILE="/tmp/`procname`-$USER.pid"
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

getpid

case $COMMAND in
  start) if ! isstarted
         then
           PID=

           start || exit $!

           FUNCPID=$!
           if [ -z "$PID" ]
           then
             PID=$FUNCPID
           fi  
           setpid $PID
         fi
         ;;

  stop)  if isstarted
         then
           stop || exit $!
           delpid
         fi
         ;;

  status)
         SWLEVEL=$2
         printf "%d : %-25s %s\n" "$SWLEVEL" "`procname`" "$PID"
         ;;

  *)     echo "usage: $0 {start|stop|status}"
         ;;
esac

