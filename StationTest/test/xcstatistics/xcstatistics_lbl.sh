#!/bin/sh
# 1.2 xcstatistics test to check SerDes Ring
# 7-05-09, M.J Norden
# LBL input with Antenna

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

