#!/bin/bash

PIDLIST=$( cat ./running_tasks.txt )
for i in $PIDLIST 
do
	kill -KILL $i
done
rm ./running_tasks.txt

