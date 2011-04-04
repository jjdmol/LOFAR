# beamformermformer test for 96 rcu's in LBA mode (LBL & LBH input)
# two antennas per beam, four beams
# object zenith
# version 1.9  3 dec 2010 M.J.Norden


rspctl --wg=0
rspctl --rcuprsg=0

killall beamctl
swlevel 3
sleep 5

beamctl --antennaset=LBA_OUTER --rcumode=1 --rcus=48:49 --subbands=270:331 --beamlets=0:61 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&
sleep 3
beamctl --antennaset=LBA_INNER --rcumode=3 --rcus=0:1 --subbands=270:331 --beamlets=62:123 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&
sleep 3
beamctl --antennaset=LBA_OUTER --rcumode=1 --rcus=48:95 --subbands=270:331 --beamlets=124:185 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&
sleep 3
beamctl --antennaset=LBA_INNER --rcumode=3 --rcus=0:47 --subbands=270:331 --beamlets=186:247 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&

