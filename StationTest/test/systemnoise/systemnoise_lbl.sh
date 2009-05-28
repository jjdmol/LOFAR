#!/bin/sh
# 1.0 system noise LBL test 
# 15-05-09, M.J Norden
# LBL input with antenna (one antenna biased and one not)


rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=1 --sel=0
rspctl --rcu=0x10017880 --sel=1
sleep 2

echo ==========================
echo "System noise LBL" `hostname -s`
echo ==========================
rspctl --stati --sel=0,1& 
sleep 20 && kill $!

