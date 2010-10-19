#!/bin/bash
#
# Send delays from RCU to HBA and compare results with the expected golden result.
# To verify modem communication between RCU and HBA
#
# Version 1.2  18-01-10   M.J.Norden

rm -f hba_modem*.log
rm -f hba_modem*.diff

let ontime=4
let hbamode=5

station=`hostname -s`

if [ -e /opt/lofar/etc/RemoteStation.conf ]; then
  let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
  let rcus=$rspboards*8
else
  echo "Could not find /opt/lofar/etc/RemoteStation.conf"
  let rspboards=12
  let rcus=$rspboards*8
fi

echo "This is station "$station
echo "The number of RCU's is "$rcus

rspctl --rcumode=$hbamode --sel=0:31
sleep 2
rspctl --rcumode=$hbamode --sel=32:63
sleep 2
rspctl --rcumode=$hbamode --sel=64:95
sleep 2

if [ $rcus -eq 192 ]; then
  rspctl --rcumode=$hbamode --sel=96:127
  sleep 2
  rspctl --rcumode=$hbamode --sel=128:159
  sleep 2
  rspctl --rcumode=$hbamode --sel=160:191
  sleep 2
fi

echo "The rcumode is "$hbamode 
sleep 2

echo "rspctl --hbadelays=1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16"
eval "rspctl --hbadelays=1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16"
sleep $ontime

rspctl --realdelays > hba_modem1.log
diff hba_modem1.log gold/hba_modem1.gold > hba_modem1.diff

if [ -e hba_modem1.log ] && [ -e gold/hba_modem1.gold ] && [ -e hba_modem1.diff ] && ! [ -s hba_modem1.diff ]; then
    # The files exists AND the diff has size 0
    echo "HBA modem test went OK"
else
    rspctl --realdelays
    echo "HBA modem test went wrong"
fi

echo "rspctl --hbadelays=0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"
eval "rspctl --hbadelays=0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"
sleep $ontime

rspctl --realdelays > hba_modem2.log
diff hba_modem2.log gold/hba_modem2.gold > hba_modem2.diff

if [ -e hba_modem2.log ] && [ -e gold/hba_modem2.gold ] && [ -e hba_modem2.diff ] && ! [ -s hba_modem2.diff ]; then
    # The files exists AND the diff has size 0
    echo "HBA modem test went OK"
else
    rspctl --realdelays
    echo "HBA modem test went wrong"
fi

echo "rspctl --hbadelays=253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253"
eval "rspctl --hbadelays=253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253"
sleep $ontime

rspctl --realdelays > hba_modem3.log
diff hba_modem3.log gold/hba_modem3.gold > hba_modem3.diff

if [ -e hba_modem3.log ] && [ -e gold/hba_modem3.gold ] && [ -e hba_modem3.diff ] && ! [ -s hba_modem3.diff ]; then
    # The files exists AND the diff has size 0
    echo "HBA modem test went OK"
else
    rspctl --realdelays
    echo "HBA modem test went wrong"
fi


