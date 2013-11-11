#!/bin/bash
 

 # default values
START=""
STOP=""
LEVEL=0
UPDATE="no"
SERVICE="yes"
HELP="no"

# get command line options
while getopts s:e:tTurh option
do
    case "${option}"
    in
        s) START=${OPTARG};;
        e) STOP=${OPTARG};;
        t) LEVEL=1;;
        T) LEVEL=2;;
        u) UPDATE="yes";;
        r) SERVICE="yes";;
        h) HELP="yes";;
    esac
done

if [ $HELP == "yes" ]
then
    echo "Usage:"
    echo "   doStationTest.sh -s 20130624_04:00:00 -e 20130624_06:00:00 -u"
    echo "   -s : start time"
    echo "   -e : end time"
    echo "   -u : update pvss"
    echo "   -t : do short test L1"
    echo "   -T : do long test L2"
    echo "   -r : do service test and show results (default)"
    echo "   -h : show this screen"
    exit
fi

host=`hostname -s`

# set filenames and dirs
locallogdir="/opt/stationtest/data/"
globallogdir="/globalhome/log/"

if [ $LEVEL -ne 0 ]
then
    SERVICE="no"
fi

filenameNow=$host"_StationTest.csv"
if [ $SERVICE == "yes" ]
then
    LEVEL=2
    filenameLocal=$host"_S_StationTest.csv"
    filenameLocalHistory=$host"_S_StationTestHistory.csv"
else
    filenameLocal=$host"_L"$LEVEL"_StationTest.csv"
    filenameLocalHistory=$host"_L"$LEVEL"_StationTestHistory.csv"
fi
filenameBadRCUs=$host"_bad_rcus.txt"

# set test level
level="-l="$LEVEL

# set start and stop time if given
start=""
stop=""
if [ -n "$STOP" ]
then
    echo "STOP not empty"
    if [ -n "$START" ]
    then
        echo "START not empty"
        start="-start="$START
    fi
    stop="-stop="$STOP
fi

# Check hardware
checkHardware.py $level $start $stop

err=$?
echo $err
if [ $err -eq 0 ]
then
    # Add test results too PVSS and make bad_rcu_file
    #updatePVSS.py -N=5,50,1 -J=5,50,2 -S=20 -E
    #new settings by Wilfred, 9-7-2013
    if [ $UPDATE == "yes" ]
    then
        updatePVSS.py -N=5,50,3 -J=5,50,3 -E -S=10 -LBLN=5,50,3 -LBLJ=5,50,3 -LBLS=10 -LBHN=5,50,3 -LBHJ=5,50,3 -LBHS=10
    else
        updatePVSS.py -no_update -N=5,50,3 -J=5,50,3 -E -S=10 -LBLN=5,50,3 -LBLJ=5,50,3 -LBLS=10 -LBHN=5,50,3 -LBHJ=5,50,3 -LBHS=10
    fi
    
    # Copy to local filename  file in local dir
    cp $locallogdir$filenameNow $locallogdir$filenameLocal
    
    # Add to history
    cat $locallogdir$filenameNow >> $locallogdir$filenameLocalHistory

    # Copy from local to global dir
    cp $locallogdir$filenameLocal $globallogdir
    cp $locallogdir$filenameLocalHistory $globallogdir
    cp $locallogdir$filenameBadRCUs $globallogdir
fi

if [ $SERVICE == "yes" ]
then
    # Show last results on screen
    showTestResult.py -f=$locallogdir$filenameNow
fi