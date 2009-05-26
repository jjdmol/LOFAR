#!/bin/sh
# 1.3 xcstatistics test to check SerDes Ring with LBL antennas
# 15-05-09, M.J Norden
# LBL input with antenna


rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=1
sleep 2

echo "check xcstat and xcangle"

rspctl --xcsubband=256
echo ==========================
echo "Amplitudes" `hostname -s`
echo ==========================
rspctl --xcstatistics& 
sleep 20 && kill $!

echo ======================
echo "Phases" `hostname -s`
echo ======================
rspctl --xcangle --xcstatistics &
sleep 20 && kill $!

