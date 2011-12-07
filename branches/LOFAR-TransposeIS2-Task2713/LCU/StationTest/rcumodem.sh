# RCU modem check version V11
# This version disable the DC from the HBA connector
# led on VDC=3.3V, led off VDC=0.04V
# note communication with the TILES only with LED ON !!! 
# M.J. Norden, 18-07-2011

# First script to switch on the LEDS
echo "This script switched the led on on all Y-RCU's"
python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fp blp0,blp1,blp2,blp3 --te tc/hba_client.py --client_acces w --client_reg led --data 00

# Second script to readout version RCU modem (V11 is new, V10 is old)
#echo "This script reads version number of RCU mode (V11)"
#python verify.py --brd rsp0,rsp1,rsp2,rsp3,rsp4,rsp5,rsp6,rsp7,rsp8,rsp9,rsp10,rsp11 --fp blp0,blp1,blp2,blp3 --te tc/hba_client.py --client_acces r --client_reg version --data 11

# Thirth script to read RSP0 version (V11)
#echo "This script switched the led on on all Y-RCU's"
#python verify.py --brd rsp0 --fp blp0,blp1,blp2,blp3 --te tc/hba_client.py --client_acces r --client_reg version --data 10







