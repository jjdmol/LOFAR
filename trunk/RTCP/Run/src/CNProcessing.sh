#!/bin/bash

source locations.sh

function start() {
  set_psetinfo

  TMPDIR="`mktemp -d`"
  PIDFILE="$TMPDIR/pid"

  # use a fifo to avoid race conditions
  mkfifo "$PIDFILE"

  (mpirun -mode VN -partition "$PARTITION" -env DCMF_COLLECTIVES=0 -env BG_MAPPING=XYZT -env LD_LIBRARY_PATH=/bgsys/drivers/ppcfloor/comm/lib:/bgsys/drivers/ppcfloor/runtime/SPI:/globalhome/romein/lib.bgp -cwd "$LOGSYMLINK" -exe "$CNPROC" 2>&1 &
  echo $! > "$PIDFILE") | LOFAR/Logger.py $LOGPARAMS "$LOGSYMLINK/CNProc.log" &

  PID=`cat "$PIDFILE"`
  rm -f "$PIDFILE"
  rmdir "$TMPDIR"

  if [ -z "$PID" ]
  then
    PID=DOWN
  fi   
}

function stop() {
  set_psetinfo

  # graceful exit
  alarm 10 gracefullyStopBGProcessing.sh

  # ungraceful exit
  [ -e /proc/$PID ] && (
    # mpikill only works when mpirun has started running the application
    mpikill "$PID" ||

    # ask DNA to kill the job
    (bgjobs -u $USER -s | awk "/$PARTITION/ { print \$1; }" | xargs -L 1 bgkilljob) ||

    # kill -9 is the last resort
    kill -9 "$PID"
  ) && sleep 10

  # wait for job to die
  while true
  do
    JOBSTATUS=`bgjobs -u $USER -s | awk "/$PARTITION/ { print \\$6; }"`
    JOBID=`bgjobs -u $USER -s | awk "/$PARTITION/ { print \\$1; }"`

    if [ -z "$JOBID" ]
    then
      # job is gone
      break
    fi  

    case "$JOBSTATUS" in
      dying)
        sleep 1
        continue ;;

      *)
        echo "Failed to kill BG/P job $JOBID. Status is $JOBSTATUS"
        break ;;
    esac
  done
}

. controller.sh
