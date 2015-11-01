#!/bin/bash
#
# V3.0, M.J.Norden, 10-09-2015
# usage: ./poweruphba.sh 5  (or 6 or 7)
# Power up of the HBA Tiles needs to be slowed down because of high rush-in current. 
# for rcumode 6 you need to switch the clock seperately (rspctl --clock=160)
# from MAC version V2_12 and higher you can directly switch between rcumodes
# The setting of hbadelays with 253 is needed for the new HBA-FE that are default off
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

if [ $hbamode -eq 6 ]; then
    echo "wait 30 seconds for 160MHz clock switch"
    rspctl --clock=160
    sleep 30
fi    
rspctl --rcumode=$hbamode
sleep 1
rspctl --rcuenable=1
sleep 1
rspctl --hbadelays=253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253

if [ $hbamode -eq 5 ]; then 
   rspctl --specinv=1
else
   rspctl --specinv=0
fi
