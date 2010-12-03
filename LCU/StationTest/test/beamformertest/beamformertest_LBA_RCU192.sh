# beamformer test for 192 rcu's in LBA mode (with splitter OFF)
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
rspctl --splitter=0
sleep 3

beamctl --antennaset=LBA_INNER --rcumode=3 --rcus=0:1 --subbands=270:331 --beamlets=0:61 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&
sleep 3
beamctl --antennaset=LBA_INNER --rcumode=3 --rcus=0:95 --subbands=270:331 --beamlets=62:123 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&
sleep 3
beamctl --antennaset=LBA_INNER --rcumode=3 --rcus=96:97 --subbands=270:331 --beamlets=124:185 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&
sleep 3
beamctl --antennaset=LBA_INNER --rcumode=3 --rcus=0:191 --subbands=270:331 --beamlets=186:247 --anadir=0,1.5708,AZEL --digdir=0,1.5708,AZEL&

