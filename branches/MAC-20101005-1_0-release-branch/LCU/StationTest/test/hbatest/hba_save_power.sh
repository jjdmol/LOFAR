#!/bin/bash
#
# This script can be used to power-up save the HBA tiles for the first time (short circuit detection).
#
# Version 1.0, 18-09-2009,  M.J.Norden

rm -f hba_modem*.log
rm -f hba_modem*.diff

station=`hostname -s`

if [ -e /opt/lofar/etc/RemoteStation.conf ]; then
  let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
  let rcus=$rspboards*8
else
  echo "Could not find /opt/lofar/etc/RemoteStation.conf"
  let rspboards=12
  let rcus=$rspboards*8
fi

let offtime=5
let ontime=10
let hbaoff=0
let hbamode=5

echo "This is station "$station
echo "The number of RCU's is "$rcus

#eval "rspctl --stati&"
sleep 1 
eval "rspctl --rcumode=$hbaoff"
sleep 2
eval "rspctl --rcuenable=0"
sleep 1 
eval "rspctl --rcuprsg=0"

      for ((ind=0; ind < $rcus-1; ind=ind+2)) do
         let ind2=$ind+1
         eval "rspctl --rcumode=$hbamode --sel=$ind,$ind2" 
         echo "RCU "$ind" and RCU "$ind2" on"
         eval "rspctl --rcuenable=1 --sel=$ind,$ind2"
         sleep $ontime
         eval "rspctl --rcumode=$hbaoff --sel=$ind,$ind2"
         eval "rspctl --rcuenable=0 --sel=$ind,$ind2"
         echo "RCU "$ind"and RCU "$ind2" off"
         sleep $offtime 
      done



