#!/bin/bash
COMMAND=$1

PARTITION=R00
BINPATH=/opt/lofar/bin
LOG=/opt/lofar/log/BlueGeneControl.log
PIDFILE=/tmp/BlueGeneControl-$PARTITION.pid

case $COMMAND in
  start) (
           $BINPATH/LOFAR/Partitions.py -kfa $PARTITION

           $BINPATH/startOLAP.py -P $PARTITION &
           PID=$!
           echo $PID > $PIDFILE
         ) >> $LOG 2>&1   
         ;;

  stop)  (
           kill -2 `cat $PIDFILE` || $BINPATH/commandOLAP.py -P $PARTITION quit
           rm -f $PIDFILE
         ) >> $LOG 2>&1
         ;;

  *)     echo "usage: $0 {start|stop}"
         ;;
esac  
