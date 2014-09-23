#!/bin/bash
#
# V2.0, M.J.Norden, 21-08-2014
# usage: ./poweruphba.sh 5  (or 6 or 7)
# Power up of the HBA Tiles needs to be slowed down because of high rush-in current. 
# for rcumode 6 you need to switch the clock seperately (rspctl --clock=160)
#

if [ "$1" != "" ]; then
    if   [ $1 -lt 5 ]; then
       echo -e "Usage: ./poweruphba.sh 5 (or 6 or 7)\n"
       exit  
    elif [ $1 -gt 7 ]; then
       echo -e "Usage: ./poweruphba.sh 5 (or 6 or 7)\n"
       exit
    else
       hbamode=$1
    fi
else 
   echo -e "Usage: ./poweruphba.sh 5 (or 6 or 7)\n"
   exit
fi

if [ -e /opt/lofar/etc/RemoteStation.conf ]; then
  let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
  let "last=$rspboards/2"
else
  echo "Could not find /opt/lofar/etc/RemoteStation.conf"
  let last=6
fi

for (( idx = 0; idx < $last ; idx++)) ; do
  let "a=16*$idx"
  let "b=$a + 15"
  rspctl --rcumode=$hbamode --sel=$a:$b
  sleep 1
done

sleep 1
rspctl --rcuenable=1
sleep 1

if [ $hbamode -eq 5 ]; then 
   rspctl --specinv=1
else
   rspctl --specinv=0
fi
