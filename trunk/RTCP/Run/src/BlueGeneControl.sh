# >> $LOG 2>&1 >> $LOG 2>&1!/bin/bash
COMMAND=$1

PARTITION=R00
BINPATH=/opt/lofar/bin
LOG=/opt/lofar/log/BlueGeneControl.log
PIDFILE=/tmp/BlueGeneControl-$PARTITION.pid

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

function stop_soft() {
  kill -2 $PID || (
    $BINPATH/commandOLAP.py -P $PARTITION cancel all
    $BINPATH/commandOLAP.py -P $PARTITION quit
  )

  rm -f $PIDFILE
}

function stop_hard() {
  sleep 30
  pkill -P $PID # kill children of startOLAP.py script -- should be our mpiruns
}

getpid

case $COMMAND in
  start) if [ "$PID" != "DOWN" ]
         then
         (
           start
         ) >> $LOG 2>&1
         fi
         ;;

  stop)  (
           stop_soft
           sleep 30
           stop_hard
         ) >> $LOG 2>&1
         ;;

  status)
         SWLEVEL=$2
         echo "$SWLEVEL : BlueGeneControl           $PID"
         ;;

  *)     echo "usage: $0 {start|stop|status}"
         ;;
esac  
