#!/bin/sh
# 1.0 subband statistics LBL test 
# 15-05-09, M.J Norden
# LBL input with antennas

rspctl --specinv=0
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=2 
sleep 2

echo ==========================
echo "Subband Statistics LBL" `hostname -s`
echo ==========================
rspctl --stati& 
sleep 20 && kill $!

