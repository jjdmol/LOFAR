#!/bin/sh
# 1.1 xcstatistics test to check SerDes Ring
# 21-04-09, M.J Norden

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

