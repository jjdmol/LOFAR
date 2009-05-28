#!/bin/sh
# 1.0 subband statistics HBA test 
# 15-05-09, M.J Norden
# HBA input with antennas


echo "Set the clock speed at 160MHz (rspctl --clock=160)"
echo "Before you run this test!!"

rspctl --specinv=0
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=6 
sleep 2

echo ==========================
echo "Subband Statistics HBA rcumode=6" `hostname -s`
echo ==========================
rspctl --stati& 
sleep 20 && kill $!
