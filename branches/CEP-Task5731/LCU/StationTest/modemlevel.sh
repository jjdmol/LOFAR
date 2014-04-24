#!/bin/bash
#
# The modem DC line level is measured by varying the comparator VREF reference level (1 bit ADC).
# Exact level determination with --data 2,16
# Version 1.1, 20-5-10, M.J.Norden

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


rspctl --rcumode=$hbamode --sel=0:15
sleep 1
rspctl --rcumode=$hbamode --sel=16:31
sleep 1
rspctl --rcumode=$hbamode --sel=32:47
sleep 1
rspctl --rcumode=$hbamode --sel=48:63
sleep 1
rspctl --rcumode=$hbamode --sel=64:79
sleep 1
rspctl --rcumode=$hbamode --sel=80:95
sleep 1

if [ $rcus -eq 192 ]; then
  rspctl --rcumode=$hbamode --sel=96:111
  sleep 1
  rspctl --rcumode=$hbamode --sel=112:127
  sleep 1
  rspctl --rcumode=$hbamode --sel=128:143
  sleep 1
  rspctl --rcumode=$hbamode --sel=144:159
  sleep 1
  rspctl --rcumode=$hbamode --sel=160:175
  sleep 1
  rspctl --rcumode=$hbamode --sel=176:191
fi

python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3  -v 11 --data 0,1 --te tc/hba_line_level.py 
#python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3  -v 11 --data 2,16 --te tc/hba_line_level.py 
