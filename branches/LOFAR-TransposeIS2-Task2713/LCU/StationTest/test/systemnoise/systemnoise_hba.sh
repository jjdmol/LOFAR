#!/bin/sh
# 1.2 system noise HBA test 
# 02-10-09, M.J Norden
# HBA input with antenna (one antenna biased and one not)

let bias=26
let nobias=28

rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=5 --sel=$bias
rspctl --rcu=0x1007a080 --sel=$nobias
rspctl --specinv=1 --sel=$bias,$nobias
rspctl --rcuenable=1
sleep 2

echo ==========================
echo "System noise HBA" `hostname -s`
echo ==========================
rspctl --stati --sel=$bias,$nobias& 
sleep 20 && kill $!

rspctl --specinv=0

