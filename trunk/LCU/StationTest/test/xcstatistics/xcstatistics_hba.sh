#!/bin/sh
# 1.0 xcstatistics test to check SerDes Ring with HBA antennas
# 8-10-09, M.J Norden
# HBA input with antenna


rspctl --rcuprsg=0
rspctl --wg=0
rspctl --rcumode=5
rspctl --rcuenable=1
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

