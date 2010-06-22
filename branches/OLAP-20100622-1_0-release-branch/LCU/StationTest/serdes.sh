#!/bin/bash

#
# check the serdes (INFINIBAND) connection with splitter is on and off.
# 30-10-09, M.J.Norden
#

echo "The splitter is turned off"  
rspctl --splitter=0
sleep 2
python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --te tc/serdes.py
sleep 2
echo "The splitter is turned on"
sleep 2
rspctl --splitter=1
sleep 2
python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --te tc/serdes.py
