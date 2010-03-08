#!/bin/bash
#
# check the modem software version of the receiver unit.
# 4-3-10, M.J.Norden


echo "The modemtest"  
sleep 2
python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fpga blp0,blp1,blp2,blp3 --te tc/hba_client.py --client_acces r --client_reg version --data 10
