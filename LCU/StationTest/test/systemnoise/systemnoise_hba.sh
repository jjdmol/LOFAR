#!/bin/sh
# 1.1 system noise HBA test 
# 29-06-09, M.J Norden
# HBA input with antenna (one antenna biased and one not)

rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=5 --sel=80
rspctl --rcu=0x1007a080 --sel=82
rspctl --specinv=1 --sel=80,82
rspctl --rcuenable=1
sleep 2

echo ==========================
echo "System noise HBA" `hostname -s`
echo ==========================
rspctl --stati --sel=80,82& 
sleep 20 && kill $!

rspctl --specinv=0

