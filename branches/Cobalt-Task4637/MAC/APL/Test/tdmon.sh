#!/bin/sh

logfile=`date -u +%Y-%m-%d-%H-%M-%S`_tdmon.log
echo > $logfile
while (true); do
	date -u >> $logfile
	rspctl --tdstatus 2> /dev/null | fgrep -v controlling >> $logfile
	sleep 1
done
