# beamformer test for 96 rcu's in HBA mode (with splitter ON)
# object zenith
# version 1.2,  18 jan 2011, M.J.Norden


rspctl --wg=0
sleep 1
rspctl --rcuprsg=0
sleep 1

killall beamctl
sleep 1
swlevel 3
sleep 2
rspctl --splitter=1
sleep 3

beamctl --antennaset=HBA_ZERO --rcumode=5 --rcus=0:1 --subbands=320:380 --beamlets=0:60 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&
sleep 3
beamctl --antennaset=HBA_ZERO --rcumode=5 --rcus=0:47 --subbands=320:380 --beamlets=61:121 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&
sleep 3
beamctl --antennaset=HBA_ONE --rcumode=5 --rcus=48:49 --subbands=320:380 --beamlets=1122:1182 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&
sleep 3
beamctl --antennaset=HBA_ONE --rcumode=5 --rcus=48:95 --subbands=320:380 --beamlets=1183:1243 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&

