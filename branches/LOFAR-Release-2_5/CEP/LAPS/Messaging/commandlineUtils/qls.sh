#!/bin/bash

HEADER="Queues:"
DESCRIP="  queue                                     dur  autoDel  excl  msg   msgIn  msgOut  bytes  bytesIn  bytesOut  cons  bind"
BREAKLINE="  ========================================================================================================================="

if [ "$1" != "" ]
then
	echo "$HEADER"
	echo "$DESCRIP"
	echo "$BREAKLINE"
	qpid-stat -q |grep $1
else
	qpid-stat -q
fi
