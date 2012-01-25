#!/bin/bash

source locations.sh

function start() {
  # create a new log dir
  rm -f "$LOGSYMLINK" || true
  mkdir -p "$LOGDIR"
  ln -s "$LOGDIR" "$LOGSYMLINK"

  TMPDIR=`mktemp -d`
  PIDFILE="$TMPDIR/pid"

  # use a fifo to avoid race conditions
  mkfifo "$PIDFILE"

  (/bgsys/LOFAR/openmpi-ion/bin/mpirun -host "$PSETS"  --pernode -wd "$LOGDIR" "$IONPROC" "$ISPRODUCTION" 2>&1 &
  echo $! > "$PIDFILE") | LOFAR/Logger.py $LOGPARAMS "$LOGSYMLINK/IONProc.log" &

  PID=`cat $PIDFILE`
  rm -f "$PIDFILE"
  rmdir "$TMPDIR"

  if [ -z "$PID" ]
  then
    PID=DOWN
  fi   
}

function stop() {
  # graceful exit
  alarm 10 gracefullyStopBGProcessing.sh

  # ungraceful exit
  [ -e /proc/$PID ] && kill -15 "$PID" && (sleep 2; [ -e /proc/$PID ] && kill -9 "$PID")
}

. controller.sh
