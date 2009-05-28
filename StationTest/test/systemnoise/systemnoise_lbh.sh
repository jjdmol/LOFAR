#!/bin/sh
# 1.0 system noise LBH test 
# 15-05-09, M.J Norden
# LBH input with antenna (one antenna biased and one not)


rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=3 --sel=88
rspctl --rcu=0x10037880 --sel=89
sleep 2

echo ==========================
echo "System noise LBH" `hostname -s`
echo ==========================
rspctl --stati --sel=88,89& 
sleep 20 && kill $!

