#!/bin/sh
# 1.2 system noise LBH test 
# 02-10-09, M.J Norden
# LBH input with antenna (one antenna biased and one not)

let rcux=26
let rcuy=27

rspctl --specinv=0
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=3 --sel=$rcux
rspctl --rcu=0x10037880 --sel=$rcuy
rspctl --rcuenable=1
sleep 2

echo ==========================
echo "System noise LBH" `hostname -s`
echo ==========================
rspctl --stati --sel=$rcux,$rcuy& 
sleep 20 && kill $!

