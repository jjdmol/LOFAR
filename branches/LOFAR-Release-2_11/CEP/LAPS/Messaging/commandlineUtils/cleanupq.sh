#!/bin/bash

QUEUES=`qls |grep Y |awk '{ print $1 }'`

if [ '$1' = "" ]
then
   QUEUES=`qls |grep Y |awk '{ print $1 }'`
else
   QUEUES=`qls |grep $1 |awk '{ print $1 }'`
fi

for i in $QUEUES
do
 echo deleting queue $i
 delqueue $i
done

