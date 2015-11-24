#!/bin/bash
#
# V3.1, M.J.Norden, 17-11-2015
# usage: ./poweruphba.sh 5  (or 6 or 7)
# Power up of the HBA Tiles needs to be slowed down because of high rush-in current. 
# for rcumode 6 you need to switch the clock seperately (rspctl --clock=160)
# from MAC version V2_12 and higher you can directly switch between rcumodes
# The setting of hbadelays with 253 is needed for the new HBA-FE that are default off
#
# - automatic clock switch when needed
# - automatic disble broken tiles from PVSS database


clock=`rspctl --clock 2>&1|grep "Sample frequency"|sed -e 's/.*clock=\(...\)MHz/\1/'`

if [ "$1" != "" ]; then
    if [ $1 -lt 5 ]; then
       echo -e "Usage: ./poweruphba.sh 5 (or 6 or 7)\n"
       exit  
    elif [ $1 -gt 7 ]; then
       echo -e "Usage: ./poweruphba.sh 5 (or 6 or 7)\n"
       exit
    else
       if [ $clock = "160" ]; then
         echo "wait 30 seconds for 200MHz clock switch"
         rspctl --clock=200  
         sleep 30
       fi
       hbamode=$1
    fi
else 
   echo -e "Usage: ./poweruphba.sh 5 (or 6 or 7)\n"
   exit
fi

if [ $hbamode -eq 6 ]; then
    if [ $clock = "200" ]; then
       echo "wait 30 seconds for 160MHz clock switch"
       rspctl --clock=160
       sleep 30
    fi
fi    
rspctl --rcumode=$hbamode
sleep 1
rspctl --rcuenable=1
sleep 1

DISABLED_RCU_LIST=`/opt/lofar/sbin/disabledRCUlist $hbamode 2</dev/null` 
if test "$DISABLED_RCU_LIST" == ""; then 
   echo "no disabled HBA tiles" 
else 
   rspctl --rcumode=0 --sel=$DISABLED_RCU_LIST 
   sleep 1
   rspctl --rcuenable=0 --select=$DISABLED_RCU_LIST 
   sleep 1
fi
rspctl --hbadelays=253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253

if [ $hbamode -eq 5 ]; then 
   rspctl --specinv=1
else
   rspctl --specinv=0
fi
