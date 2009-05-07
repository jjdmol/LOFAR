#!/bin/bash
#
# Send delays from RCU to HBA and compare results with the expected golden result.
# To verify modem communication between RCU and HBA
#
# Version 1.0   13 feb 2009   M.J.Norden

rm -f hba_modem*.log
rm -f hba_modem*.diff

declare ontime=4

let hbamode=5

eval "rspctl --rcumode=$hbamode"

echo "The rcumode is "$hbamode 
sleep 2

echo "rspctl --hbadelays=1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16"
eval "rspctl --hbadelays=1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16"
sleep $ontime

rspctl --realdelays > hba_modem1.log
diff hba_modem1.log gold/hba_modem1.gold > hba_modem1.diff

if [ -e hba_modem1.log ] && [ -e gold/hba_modem1.gold ] && [ -e hba_modem1.diff ] && ! [ -s hba_modem1.diff ]; then
    # The files exists AND the diff has size 0
    echo "HBA modem test went OK"
else
    rspctl --realdelays
    echo "HBA modem test went wrong"
fi

echo "rspctl --hbadelays=0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"
eval "rspctl --hbadelays=0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"
sleep $ontime

rspctl --realdelays > hba_modem2.log
diff hba_modem2.log gold/hba_modem2.gold > hba_modem2.diff

if [ -e hba_modem2.log ] && [ -e gold/hba_modem2.gold ] && [ -e hba_modem2.diff ] && ! [ -s hba_modem2.diff ]; then
    # The files exists AND the diff has size 0
    echo "HBA modem test went OK"
else
    rspctl --realdelays
    echo "HBA modem test went wrong"
fi

echo "rspctl --hbadelays=253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253"
eval "rspctl --hbadelays=253,253,253,253,253,253,253,253,253,253,253,253,253,253,253,253"
sleep $ontime

rspctl --realdelays > hba_modem3.log
diff hba_modem3.log gold/hba_modem3.gold > hba_modem3.diff

if [ -e hba_modem3.log ] && [ -e gold/hba_modem3.gold ] && [ -e hba_modem3.diff ] && ! [ -s hba_modem3.diff ]; then
    # The files exists AND the diff has size 0
    echo "HBA modem test went OK"
else
    rspctl --realdelays
    echo "HBA modem test went wrong"
fi


