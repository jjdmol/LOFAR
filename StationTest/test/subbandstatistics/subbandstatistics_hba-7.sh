#!/bin/sh
# 1.0 subband statistics HBA test 
# 15-05-09, M.J Norden
# HBA input with antennas

rspctl --specinv=0
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=7 
sleep 2

echo ==========================
echo "Subband Statistics HBA rcumode=7" `hostname -s`
echo ==========================
rspctl --stati& 
sleep 20 && kill $!

