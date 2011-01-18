#!/bin/sh
# 1.5 subband statistics HBA test 
# 18-01-2011, M.J Norden
# HBA input with antennas


echo "Set the clock speed at 160MHz (wait for 60 seconds)"
rspctl --clock=160 && sleep 60

rspctl --rcuprsg=0
rspctl --wg=0
rspctl --splitter=0
killall beamctl

swlevel 3
sleep 5
beamctl --antennaset=HBA_JOINED --rcus=0:95 --rcumode=6 --subbands=100:110 --beamlets=0:10 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&
sleep 15

echo ==========================
echo "Subband Statistics HBA rcumode=6" `hostname -s`
echo ==========================
rspctl --stati& 
sleep 20 && kill $!
killall beamctl
echo "Do not forget to switch the clock back to 200MHz"


