#!/bin/bash
#
# Script to stop recording of the TBB boards and set them to recording mode.
# Should be run when TBB images have been loaded and TBBDriver is running
#
# $Id:$
#

level=`swlevel -S`
if [ $level -lt 2 ]; then 
  echo "Swlevel must be 2 or higher, and RSPDriver and TBBDriver must be running"
  exit
fi
 
tbbctl --rcui --select=0 2>&1 | grep 0x0 | egrep "(S|R)" > /dev/null 2>&1; 
if [ $? -eq 0 ]; then 
   tbbctl --stop > /dev/null 2>&1 && \
   echo "TBB recording stopped" && exit
else
   echo "TBBs not in recording mode"
fi
