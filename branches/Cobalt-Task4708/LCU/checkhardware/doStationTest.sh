#!/bin/bash
# 
# nohup lcurun -s today -c '/opt/stationtest/doStationTest.sh start=20130624_04:00:00 stop=20130624_06:00:00 &' &
#
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

err=$?
echo $err
if [ $err -eq 0 ]
then
    # Add to history
    cat $filenameNow >> $filenameHistory

    # Copy to L2 file in local dir
    cp $filenameNow $locallogdir$host"_L2_StationTest.csv"

    # Copy from local to global dir
    cp $filenameNow $globallogdir$host"_L2_StationTest.csv"
    cp $filenameHistory $globallogdir

    # Add test results too PVSS and make bad_rcu_file
    #updatePVSS.py -N=5,50,1 -J=5,50,2 -S=20 -E
    #new settings by Wilfred, 9-7-2013
    updatePVSS.py -N=5,50,3 -J=5,50,3 -E -S=10

fi
