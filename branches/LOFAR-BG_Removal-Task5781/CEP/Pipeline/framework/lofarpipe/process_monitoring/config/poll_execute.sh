#! /bin/bash

PID=$(echo $1)

uid=`ls -ld /proc/${PID} | awk '{ print $3 }' `
exe=` ls -l /proc/${PID}/exe | awk '{print $10}'`

rb=`cat /proc/${PID}/io | grep read_bytes | awk '{print $2}'`
wb=`cat /proc/${PID}/io | grep write_bytes | grep -v cancelled | awk '{print $2}'`
cwb=`cat /proc/${PID}/io | grep cancelled_write_bytes | awk '{print $2}'`

echo =============
echo UID: ${uid}
echo CMD: ${exe}
echo IO R: ${rb} W: ${wb} - ${cwb}
pageSize=$(getconf PAGE_SIZE)
printf "VMSize(KB)\tReserved(KB)\tShared(KB)\tCode\tLibrary\tData/Stack\tDirty\n"

vals=`cat /proc/$1/statm`

totalVM=`echo $vals $pageSize | awk '{print $1*$8/1024}'`
reserved=`echo $vals $pageSize | awk '{print $2*$8/1024}'`
shared=`echo $vals $pageSize | awk '{print $3*$8/1024}'`
code=`echo $vals $pageSize | awk '{print $4*$8/1024}'`
datastack=`echo $vals $pageSize | awk '{print $5*$8/1024}'`
library=`echo $vals $pageSize | awk '{print $6*$8/1024}'`
dirty=`echo $vals $pageSize | awk '{print $7*$8/1024}'`
printf "$totalVM\t\t$reserved\t\t$shared\t\t$code\t$datastack\t$library\t\t$dirty\n"

printf "CPU user time, CPU kernel time, children in user time, children in kernel time \n"
cat stat | awk '{print $14","$15","$16","$17}'
echo =============
