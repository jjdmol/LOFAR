#!/bin/sh

echo "Checking for RSP driver"
rspdriver_pid=`swlevel|awk '/RSPDriver/ {print $4}'`
if test "$rspdriver_pid" = "DOWN" ; then
    swlevel 2 & sleep 50;
    rspdriver_pid=`swlevel|awk '/RSPDriver/ {print $4}'`
fi
echo "RSPDriver has PID $rspdriver_pid";

clock=`rspctl --clock 2>&1|grep "Sample frequency"|sed -e 's/.*clock=\(...\)MHz/\1/'`
echo "Clock = $clock MHz"
frequency=`echo "($clock / 4)"|bc -l`


station=`hostname -s`
let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
let nrcus=8*$rspboards
echo "The number of RCUs is "$nrcus

rspctl --rcuprsg=0
#rspctl --rspclear
#echo "Wait 50 seconds for clearing the RSPbuffers"
#sleep 50

for (( idx = 0; idx < $nrcus ; idx++)) ; do
    phaserad=`echo "( $idx * 2 * 3.141592654 ) / $nrcus" | bc -l`
    ampl=`echo "0.5 * (1.02^$idx)/(1.02^$nrcus)" | bc -l`
    #ampl='echo "0.5 * ($idx)/(1.02^$nrcus)"| bc -l' 
    echo "Amplitude [$idx]: $ampl"
    echo "Phase     [$idx]: $phaserad"
    rspctl --wg=${frequency}e6 --select=$idx --ampli=$ampl --phase=$phaserad
done

rspctl --xcsubband=256
echo ==========================
echo "Amplitudes" `hostname -s`
echo ==========================
rspctl --xcstatistics& 
sleep 20 && kill $!

echo ======================
echo "Phases" `hostname -s`
echo ======================
rspctl --xcangle --xcstatistics &
sleep 20 && kill $!

rspctl --wg=0
