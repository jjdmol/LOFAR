# beamformer test for 96 rcu's in LBL mode (with splitter OFF)
# object zenith
# version 1.3,  18-01-2011, M.J.Norden


rspctl --wg=0
sleep 1
rspctl --rcuprsg=0
sleep 1

killall beamctl
sleep 3
swlevel 3
sleep 2
rspctl --splitter=0
sleep 3

beamctl --antennaset=LBA_OUTER --rcumode=1 --rcus=0:1 --subbands=270:330 --beamlets=0:60 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&
sleep 3
beamctl --antennaset=LBA_OUTER --rcumode=1 --rcus=0:47 --subbands=270:330 --beamlets=61:121 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&
sleep 3
beamctl --antennaset=LBA_OUTER --rcumode=1 --rcus=48:49 --subbands=270:330 --beamlets=122:182 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&
sleep 3
beamctl --antennaset=LBA_OUTER --rcumode=1 --rcus=0:95 --subbands=270:330 --beamlets=183:243 --anadir=0,1.5708,AZELGEO --digdir=0,1.5708,AZELGEO&

