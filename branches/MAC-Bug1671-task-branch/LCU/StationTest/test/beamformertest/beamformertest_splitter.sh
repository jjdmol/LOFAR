# beamformer test for 96 rcu's in HBA mode (with splitter ON)
# object zenith
# version 1.1,  3 dec 2010, M.J.Norden


rspctl --wg=0
sleep 1
rspctl --rcuprsg=0
sleep 1

killall beamctl
sleep 3
eval swlevel 3
sleep 2
rspctl --splitter=1
sleep 3

beamctl --antennaset=HBA_ZERO --rcumode=5 --rcus=0:1 --subbands=320:381 --beamlets=0:61 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&
sleep 3
beamctl --antennaset=HBA_ZERO --rcumode=5 --rcus=0:47 --subbands=320:381 --beamlets=62:123 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&
sleep 3
beamctl --antennaset=HBA_ONE --rcumode=5 --rcus=48:49 --subbands=320:381 --beamlets=1124:1185 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&
sleep 3
beamctl --antennaset=HBA_ONE --rcumode=5 --rcus=48:95 --subbands=320:381 --beamlets=1186:1247 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&

