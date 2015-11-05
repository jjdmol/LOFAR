#!/bin/bash

QUEUES=`qls |grep Y |awk '{ print $1 }'`
for i in $QUEUES
do
 delqueue $i
done

