#!/bin/bash
COMMAND=$1

CONFIG=/opt/lofar/etc/BlueGeneControl.conf

. $CONFIG

function getpid() {
  if [ -e $PIDFILE ]
  then
    PID=`cat $PIDFILE`
    if [ ! -e /proc/$PID ]
    then
      PID=DOWN
    fi
  else
    PID=DOWN
  fi
}

function start() {
  $BINPATH/LOFAR/Partitions.py -kfa $PARTITION

  $BINPATH/startOLAP.py -P $PARTITION &
  PID=$!
  echo $PID > $PIDFILE
}

function wait_for_graceful_exit() {
  # wait for correlator to stop
  for i in `seq 1 30`
  do
    if [ -e /proc/$PID ]
    then
      break
    fi  
  done
}

function stop() {
  $BINPATH/commandOLAP.py -P $PARTITION cancel all
  $BINPATH/commandOLAP.py -P $PARTITION quit

  wait_for_graceful_exit

  if [ -e /proc/$PID ]
  then
    # nudge startOLAP
    kill -2 $PID

    wait_for_graceful_exit

    # kill startOLAP.py script and all its children (mpiruns)
    pkill -P $PID
    $BINPATH/LOFAR/Partitions.py -k $PARTITION
  fi

  rm -f $PIDFILE
}

getpid

case $COMMAND in
  start) if [ "$PID" = "DOWN" ]
         then
         (
           start
         ) >> $LOGFILE 2>&1
         fi
         ;;

  stop)  if [ "$PID" != "DOWN" ]
         then
         (
           stop
         ) >> $LOGFILE 2>&1
         fi
         ;;

  status)
         SWLEVEL=$2
         echo "$SWLEVEL : BlueGeneControl           $PID"
         ;;

  *)     echo "usage: $0 {start|stop|status}"
         ;;
esac

