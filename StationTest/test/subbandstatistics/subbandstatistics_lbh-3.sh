#!/bin/sh
# 1.0 subband statistics LBH test 
# 15-05-09, M.J Norden
# LBH input with antennas

rspctl --specinv=0
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=3 
sleep 2

echo ==========================
echo "Subband Statistics LBH" `hostname -s`
echo ==========================
rspctl --stati& 
sleep 20 && kill $!

