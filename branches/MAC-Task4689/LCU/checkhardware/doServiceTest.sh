#!/bin/bash

# usage
#
# doServiceTest.sh
# doServiceTest.sh start=20130629_13:00:45 stop=20130629_16:00:45 
# doServiceTest.sh stop=20130629_16:00:45 

host=`hostname -s`

locallogdir="/opt/stationtest/data/"
globallogdir="/globalhome/log/"

filenameNow=$locallogdir$host"_StationTest.csv"
filenameHistory=$locallogdir$host"_L2_StationTestHistory.csv"

# Check hardware in CheckLevel 2, do all tests
start=""
stop=""
if [ $# -eq "2" ]
then
    start="-$1"
    stop="-$2"
fi
if [ $# -eq "1" ]
then
    stop="-$1"
fi
checkHardware.py -l=2 $start $stop

# Add to history
cat $filenameNow >> $filenameHistory

# Copy from local to global dir
cp $filenameNow $globallogdir$host"_L2_StationTest.csv"
cp $filenameHistory $globallogdir

# Show results on screen
showTestResult.py -f=$filenameNow