#!/bin/bash

function start() {
  . locations.sh

  TMPDIR=`mktemp -d`
  PIDFILE="$TMPDIR/pid"

  mkfifo $PIDFILE

  (mpirun -mode VN -partition $PARTITION -env DCMF_COLLECTIVES=0 -env BG_MAPPING=XYZT -env LD_LIBRARY_PATH=/bgsys/drivers/ppcfloor/comm/lib:/bgsys/drivers/ppcfloor/runtime/SPI:/globalhome/romein/lib.bgp -cwd $LOGSYMLINK -exe $CNPROC 2>&1 &
  echo $! > $PIDFILE) | LOFAR/Logger.py $LOGSYMLINK/CNProc.log &

  PID=`cat $PIDFILE`
  rm -f $PIDFILE
  rmdir $TMPDIR

  if [ -z "$PID" ]
  then
    PID=DOWN
  fi   
}

function stop() {
  # ask DNA to kill our jobs
  #bgjobs -u $USER -s | cut -d' '  -f 1 | xargs -L 1 bgkilljob

  # mpikill only works when mpirun has started running the application
  mpikill "$PID" || kill -9 "$PID"
}

. controller.sh
