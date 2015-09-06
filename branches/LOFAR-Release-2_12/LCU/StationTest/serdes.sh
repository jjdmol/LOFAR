#!/bin/bash
#
# check the serdes (INFINIBAND) connection with splitter is on and off.
# 20-8-14, V2.0,  M.J.Norden
#
splitter=$(grep RS.HBA_SPLIT /opt/lofar/etc/RemoteStation.conf | cut -d'=' -f2)
let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`


if [ $rspboards == 12 ]; then
     # This is a NL station
     if [ $splitter = "Yes" ]; then
        echo "This is a core station"
        echo "The splitter is turned on"  
        rspctl --splitter=1
        sleep 2
        python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --te tc/serdes.py
     else
        echo "This is a remote station"
     fi
     echo "The splitter is turned off"  
     rspctl --splitter=0
     sleep 2
     python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --te tc/serdes.py
else
     # This is a INT station
        echo "This is an international station"
        echo "The splitter is turned off"  
        rspctl --splitter=0
        sleep 2
        python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11,rsp12,rsp13,rsp14,rsp15,rsp16,rsp17,rsp18,rsp19,rsp20,rsp21,rsp22,rsp23 --te tc/serdes.py
fi



