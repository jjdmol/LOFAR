#!/bin/bash
#qls |  awk '{print $1}'
#arr=($( qls | grep "klijn" | awk '{print $1}'))
arr=($( qpid-stat -e | grep "klijn" | awk '{print $1}'))

echo ${#arr[@]} 

## now loop through the above array
for i in "${arr[@]}"
do
   #echo "$i"
   python /home/klijn/source/7629/LOFAR/LCS/MessageMasterNodeDaemon/test/clearQueue.py ${i} 
   #delqueue ${i}
   deltopic ${i}
   # or do whatever with individual element of the array
done
