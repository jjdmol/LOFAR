#!/bin/bash
#
# This test is a duration test to test the modem communication between RCU and HBA.
# The beamserver is used in this test.
#
# Version 1.3  29-04-10   M.J.Norden

rm -f /Beamdata/output_*.dat
let hbamode=5
let points=10

killall beamctl
rspctl --splitter=0

station=`hostname -s`

# determine the number of rcu's 
if [ -e /opt/lofar/etc/RemoteStation.conf ]; then
  let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
  let rcus=$rspboards*8
else
  echo "Could not find /opt/lofar/etc/RemoteStation.conf"
  let rspboards=12
  let rcus=$rspboards*8
fi

# determine the time between each skyscan position
if [ -e /opt/lofar/etc/BeamServer.conf ]; then
  let waittime=`sed -n  's/^\s*BeamServer\.HBA_INTERVAL\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/BeamServer.conf`
else
  echo "Could not find /opt/lofar/etc/BeamServer.conf"
  let waittime=10
fi

swlevel 3

echo "This is station "$station
echo "The number of RCU's is "$rcus
echo "The rcumode is "$hbamode
echo "The wait time is "$waittime
echo "The number of scanpoints "$points*$points
let duration=$waittime*$points*$points
echo "The duration of this measurement is "$duration" seconds"
let nrcus=$rcus-1
sleep 10

beamctl --array=HBA --rcus=0:$nrcus --rcumode=$hbamode --subbands=300:310 --beamlets=0:10 --direction=$points,$points,SKYSCAN &
sleep 25

for (( passes=1; passes<($points*$points)+1; passes++)) 
do
	echo "this is loop nr "$passes
	rspctl --realdelays > ./Beamdata/output_$passes.dat
	sleep $waittime 
done




