#!/bin/bash
COMMAND=$1

PARTITION=R00
BINPATH=/opt/lofar/bin
LOG=/opt/lofar/log/BlueGeneControl.log
PIDFILE=/tmp/BlueGeneControl-$PARTITION.pid

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

case $COMMAND in
  start) if [ "$PID" != "DOWN" ]
         then
         (
           $BINPATH/LOFAR/Partitions.py -kfa $PARTITION

           $BINPATH/startOLAP.py -P $PARTITION &
           PID=$!
           echo $PID > $PIDFILE
         ) >> $LOG 2>&1   
         fi
         ;;

  stop)  (
           kill -2 `cat $PIDFILE` || $BINPATH/commandOLAP.py -P $PARTITION quit
           rm -f $PIDFILE
         ) >> $LOG 2>&1
         ;;

  status)
         SWLEVEL=$2
         echo "$SWLEVEL : BlueGeneControl           $PID"
         ;;

  *)     echo "usage: $0 {start|stop|status}"
         ;;
esac  
