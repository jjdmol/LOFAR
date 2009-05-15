#!/bin/sh
# 1.0 system noise HBA test 
# 15-05-09, M.J Norden
# HBA input with antenna (one antenna biased and one not)


rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=5 --sel=0
rspctl --rcu=0x1007a080 --sel=2
rspctl --specinv=1 --sel=0,2
sleep 2

echo ==========================
echo "System noise HBA" `hostname -s`
echo ==========================
rspctl --stati --sel=0,2& 
sleep 20 && kill $!

rspctl --specinv=0

