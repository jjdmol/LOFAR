#!/bin/bash
PARTITION="R01-M0-N04-64"


function start() {
  source locations.sh

  # list both the partition directly (small partitions) and recursively (large partitions) to get all -32 subpartitions
  SUBPARTITIONS=`bghierarchy -s $PARTITION;bghierarchy -s \`bghierarchy -s $PARTITION\`` 

  # xxx-32 means both xxx-J00 and xxx-J01
  PSETS=`for i in $SUBPARTITIONS; do echo $i; done|grep -- "-32$"|sort -u|sed 's/-32$/-J00/;p;s/-J00$/-J01/'|xargs -L 1 host -4|cut -d\  -f 4|tr '\n' ','`

  # create a new log dir
  rm -f $LOGSYMLINK || true
  mkdir -p $LOGDIR
  ln -s $LOGDIR $LOGSYMLINK

  # todo: log rotation and log server support (also in CNProc.sh)
  # production logserver: "tcp:ccu001:24500"

  /bgsys/LOFAR/openmpi-ion/bin/mpirun -host $PSETS --pernode -wd $LOGDIR $IONPROC $ISPRODUCTION >& $LOGSYMLINK/IONProc.log &
}

. controller.sh
