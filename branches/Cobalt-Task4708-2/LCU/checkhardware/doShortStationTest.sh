#!/bin/bash

host=`hostname -s`

locallogdir="/opt/stationtest/data/"
globallogdir="/globalhome/log/"

filenameNow=$locallogdir$host"_StationTest.csv"
filenameHistory=$locallogdir$host"_StationTestHistory.csv"

# Check hardware in CheckLevel 1, only antennas
checkHardware.py -l=1

# Add too history
cat $filenameNow >> $filenameHistory

# Add test results too PVSS and make bad_rcu_file
#updatePVSS.py -N=5,50,1 -J=5,50,2 -reset -S=20 -E


#UNCOMMENT NEXT LINE WHEN PVSS UPDATE MUST BE ENABLED AGAIN
#updatePVSS.py -N=5,50,3 -J=5,50,3 -E -S=10 # new settings by Wilfred, 9-7-2013

# Make old station log files
makeStationLogFile.py

# Copy from local to global dir
cp $filenameNow $globallogdir
cp $filenameHistory $globallogdir

showTestResult.py -f=$filenameNow
