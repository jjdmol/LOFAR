#!/bin/sh
# 1.3 subband statistics HBA test 
# 18-02-10, M.J Norden
# HBA input with antennas

rspctl --rcuprsg=0
rspctl --wg=0
rspctl --splitter=0
killall beamctl

swlevel 3
sleep 5
beamctl --array=HBA --rcus=0:95 --rcumode=7 --subbands=100:110 --beamlets=0:10 --direction=0,0,LOFAR_LMN&
sleep 2

echo ==========================
echo "Subband Statistics HBA rcumode=7" `hostname -s`
echo ==========================
rspctl --stati& 
sleep 20 && kill $!
killall beamctl
