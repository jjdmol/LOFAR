#! /bin/bash

PID=$(echo $1)
uid=`ls -ld /proc/${PID} | awk '{ print $3 }' `
exe=` ls -l /proc/${PID}/exe | awk '{print $10}'`
rb=`cat /proc/${PID}/io | grep read_bytes | awk '{print $2}'`
wb=`cat /proc/${PID}/io | grep write_bytes | grep -v cancelled | awk '{print $2}'`
cwb=`cat /proc/${PID}/io | grep cancelled_write_bytes | awk '{print $2}'`
TIME=$(($(date +%s%3N))) 
PSOUTPUT=($(ps -p ${PID}man -o %cpu,%mem ))  # could be gotten from proc. This works for now ( Optionally use resident -  man ps)
# print the PID, TIME, executable, readbytes,writebytes,cancelled bytes, the cpu% , memory%
echo "['$TIME','${rb}','${wb}','${cwb}','${PSOUTPUT[0]}','${PSOUTPUT[1]}']" #,'${exe}'  # The executable could be added as aditional information is very spammy though


