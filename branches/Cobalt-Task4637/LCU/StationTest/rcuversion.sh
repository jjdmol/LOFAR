#!/bin/bash
#
# check the modem software version of the receiver unit.
# modified for international and national
# version 10 or 11 are with DC on Y-connector
# version 12 is without DC on Y-connector
# 12-3-12, M.J.Norden

let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
let nrcus=8*$rspboards

version=12


if [ $nrcus -eq 96 ] ; then
   echo "RCU modem version V-$version check national station"  
   sleep 2
   python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/hba_client.py --client_acces r --client_reg version --data $version
else
   echo "RCU modem version V-$version check international station"  
   sleep 2
   python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11,rsp12,rsp13,rsp14,rsp15,rsp16,rsp17,rsp18,rsp19,rsp20,rsp21,rsp22,rsp23 --fpga blp0,blp1,blp2,blp3 --te tc/hba_client.py --client_acces r --client_reg version --data $version
fi


