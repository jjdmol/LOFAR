#!/bin/bash





function createbus {
   TEMPFILE=`tempfile`
   HOSTNAME=`hostname`
   if (( $# >= 2 ))
   then
     BUSNAME=$1
     REMOTEHOST=$2
     if (( $# >2 ))
     then 
       HOSTNAME=$3
     fi
     ERRORSEEN=0
     # setup exchanges
     echo "qpid-config -b $HOSTNAME add exchange topic $BUSNAME\".proxy.deadletter\" --durable" >> $TEMPFILE
     qpid-config -b $HOSTNAME add exchange topic $BUSNAME".proxy.deadletter" --durable >> $TEMPFILE 2>&1
     (( $? >0 )) && ERRORSEEN=1

     echo "qpid-config -b $HOSTNAME add exchange direct $BUSNAME --durable --alternate-exchange=$BUSNAME\".proxy.deadletter\"" >> $TEMPFILE
     qpid-config -b $HOSTNAME add exchange direct $BUSNAME --durable --alternate-exchange=$BUSNAME".proxy.deadletter" >> $TEMPFILE 2>&1
     (( $? >0 )) && ERRORSEEN=1

     echo "qpid-config -b $REMOTEHOST add exchange topic $BUSNAME\".proxy.deadletter\" --durable" >> $TEMPFILE
     qpid-config -b $REMOTEHOST add exchange topic $BUSNAME".proxy.deadletter" --durable >> $TEMPFILE 2>&1
     (( $? >0 )) && ERRORSEEN=1

     echo "qpid-config -b $REMOTEHOST add exchange direct $BUSNAME --durable --alternate-exchange=$BUSNAME\".proxy.deadletter\"" >> $TEMPFILE
     qpid-config -b $REMOTEHOST add exchange direct $BUSNAME --durable --alternate-exchange=$BUSNAME".proxy.deadletter" >> $TEMPFILE 2>&1
     (( $? >0 )) && ERRORSEEN=1

     #setup deadletter queues
     echo "qpid-config -b $HOSTNAME add queue $BUSNAME\".deadletter\" --durable" >> $TEMPFILE
     qpid-config -b $HOSTNAME add queue $BUSNAME".deadletter" --durable >> $TEMPFILE 2>&1
     (( $? >0 )) && ERRORSEEN=1

     echo "qpid-config -b $HOSTNAME bind $BUSNAME\".proxy.deadletter\" $BUSNAME\".deadletter\" '*' --durable" >> $TEMPFILE
     qpid-config -b $HOSTNAME bind $BUSNAME".proxy.deadletter" $BUSNAME".deadletter" '*' --durable >> $TEMPFILE 2>&1
     (( $? >0 )) && ERRORSEEN=1

     echo "qpid-config -b $REMOTEHOST add queue $BUSNAME\".deadletter\" --durable" >> $TEMPFILE
     qpid-config -b $REMOTEHOST add queue $BUSNAME".deadletter" --durable >> $TEMPFILE 2>&1
     (( $? >0 )) && ERRORSEEN=1

     echo "qpid-config -b $REMOTEHOST bind $BUSNAME\".proxy.deadletter\" $BUSNAME\".deadletter\" '*' --durable" >> $TEMPFILE
     qpid-config -b $REMOTEHOST bind $BUSNAME".proxy.deadletter" $BUSNAME".deadletter" '*' --durable >> $TEMPFILE 2>&1
     (( $? >0 )) && ERRORSEEN=1

     #setup routing
     echo "qpid-route -d dynamic add $REMOTEHOST $HOSTNAME $BUSNAME" >> $TEMPFILE
     qpid-route -d dynamic add $REMOTEHOST $HOSTNAME $BUSNAME >> $TEMPFILE 2>&1
     (( $? >0 )) && ERRORSEEN=1

     echo "qpid-route -d dynamic add $HOSTNAME $REMOTEHOST $BUSNAME" >> $TEMPFILE
     qpid-route -d dynamic add $HOSTNAME $REMOTEHOST $BUSNAME >> $TEMPFILE 2>&1
     (( $? >0 )) && ERRORSEEN=1

     (( $ERRORSEEN >0 )) && echo "ERROR during setup, check "$TEMPFILE" for errors." 
     (( $ERRORSEEN == 0 )) && rm $TEMPFILE
   else
     echo " 'createbus' creates a dynamic routing bus between two nodes." 
     echo "Usage: createbus <busname> <remotenode>" 
     echo "   or: createbus <busname> <node1> <node2>" 
     echo " If only one hostname is given the second hostname is presumed to be the local node." 
     echo " to ensure one-time delivery of messages avoid loops in the bus topology." 
   fi
}