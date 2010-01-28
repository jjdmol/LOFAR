#!/bin/sh
# 1.1 xcstatistics test to check SerDes Ring with LBH antennas
# 18-01-10, M.J Norden
# HBA input with antenna

killall beamctl
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --splitter=1

swlevel 3
beamctl --array=HBA --rcus=0:47 --rcumode=5 --subbands=100:110 --beamlets=0:10 --direction=0,0,LOFAR_LMN&
beamctl --array=HBA --rcus=48:95 --rcumode=5 --subbands=100:110 --beamlets=1000:1010 --direction=0,0,LOFAR_LMN&
sleep 5

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


