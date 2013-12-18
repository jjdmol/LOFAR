#!/bin/sh
# 1.4 xcstatistics test to check SerDes Ring with LBH antennas
# 18-01-2011, M.J Norden
# HBA input with antenna

killall beamctl
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --splitter=1

swlevel 3
sleep 5
beamctl --antennaset=HBA_ZERO --rcus=0:47 --rcumode=5 --subbands=100:110 --beamlets=0:10 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&
sleep 3
beamctl --antennaset=HBA_ONE --rcus=48:95 --rcumode=5 --subbands=100:110 --beamlets=1000:1010 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&
sleep 3
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
killall beamctl

