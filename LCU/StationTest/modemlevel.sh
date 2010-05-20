#!/bin/bash
#
# The modem DC line level is measured by varying the comparator VREF reference level (1 bit ADC).
# Exact level determination with --data 2,16
# Version 1.0, 19-5-10, M.J.Norden

if [ -e /opt/lofar/etc/RemoteStation.conf ]; then
  let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
  let rcus=$rspboards*8
else
  echo "Could not find /opt/lofar/etc/RemoteStation.conf"
  let rspboards=12
  let rcus=$rspboards*8
fi

let hbamode=5
station=`hostname -s`
echo "This is station "$station
echo "The number of RCU's is "$rcus
echo "The rcumode is "$hbamode 
echo "The modem line level test" 


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

python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3  -v 11 --data 0,1 --te tc/hba_line_level.py 
#python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3  -v 11 --data 2,16 --te tc/hba_line_level.py 
