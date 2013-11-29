#!/bin/sh
# 1.1 xcstatistics test to check SerDes Ring with LBA antennas
# 24-06-11, M.J Norden
# LBA input with antenna

killall beamctl
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --splitter=0

swlevel 3
sleep 5
beamctl --antennaset=LBA_INNER --rcus=0:191 --rcumode=3 --subbands=100:110 --beamlets=0:10 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&
sleep 5
echo "check xcstat and xcangle"

rspctl --xcsubband=256
echo ==========================
echo "Amplitudes" `hostname -s`
echo ==========================
rspctl --xcstatistics& 
sleep 30 && kill $!

echo ======================
echo "Phases" `hostname -s`
echo ======================
rspctl --xcangle --xcstatistics &
sleep 30 && kill $!
killall beamctl

