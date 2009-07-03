#!/bin/sh
# 1.1 subband statistics HBA test 
# 29-06-09, M.J Norden
# HBA input with antennas


echo "Set the clock speed at 160MHz (rspctl --clock=160)"
echo "Before you run this test!!"

rspctl --specinv=0
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=6 
sleep 2
rspctl --rcuenable=1
sleep 2

echo ==========================
echo "Subband Statistics HBA rcumode=6" `hostname -s`
echo ==========================
rspctl --stati& 
sleep 20 && kill $!
