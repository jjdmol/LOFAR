#!/bin/bash

function start() {
  source locations.sh

  # list both the partition directly (small partitions) and recursively (large partitions) to get all -32 subpartitions
  # bghierarchy needs a valid stdin for some reason and will read from it, so provide a fake one
  SUBPARTITIONS="`(bghierarchy -s $PARTITION;bghierarchy -s \`bghierarchy -s $PARTITION\`) </dev/null`"

  # xxx-32 means both xxx-J00 and xxx-J01
  PSETS=`for i in $SUBPARTITIONS; do echo $i; done|grep -- "-32$"|sort -u|sed 's/-32$/-J00/;p;s/-J00$/-J01/'|xargs -L 1 host -4|cut -d\  -f 4|tr '\n' ','`

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

. controller.sh
