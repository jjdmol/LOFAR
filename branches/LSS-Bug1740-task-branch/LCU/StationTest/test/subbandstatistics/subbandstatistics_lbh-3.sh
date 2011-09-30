#!/bin/sh
# 1.1 subband statistics LBH test 
# 29-06-09, M.J Norden
# LBH input with antennas

rspctl --specinv=0
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=3
sleep 2 
rspctl --rcuenable=1
sleep 2

echo ==========================
echo "Subband Statistics LBH" `hostname -s`
echo ==========================
rspctl --stati& 
sleep 20 && kill $!

