#!/bin/bash
export PYHTONPATH=$PYTHONPATH:`pwd`

# if we want to use an inifite loop we should add ourself to the kill list.
#PIDLIST=$$
PIDLIST=""
for i in $( ls *.py | grep -v DBToQDeamon.py ) 
do
	./$i > ./$i.log 2>&1  &
	PIDLIST=$PIDLIST" "$!
done
./DBToQDeamon.py -H sas099.control.lofar -D LOFAR_4 >> ./DBoutput.log 2>&1 &
PIDLIST=$PIDLIST" "$!
echo $PIDLIST >> ./running_tasks.txt
