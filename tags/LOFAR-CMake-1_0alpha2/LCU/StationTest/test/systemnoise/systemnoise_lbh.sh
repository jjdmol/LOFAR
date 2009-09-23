#!/bin/sh
# 1.1 system noise LBH test 
# 29-06-09, M.J Norden
# LBH input with antenna (one antenna biased and one not)

rspctl --specinv=0
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=3 --sel=86
rspctl --rcu=0x10037880 --sel=87
rspctl --rcuenable=1
sleep 2

echo ==========================
echo "System noise LBH" `hostname -s`
echo ==========================
rspctl --stati --sel=86,87& 
sleep 20 && kill $!

