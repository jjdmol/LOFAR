# beamformer test for 96 rcu's in LBA mode (with splitter OFF)
# object zenith
# version 1.0,  26 nov 2009, M.J.Norden


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

beamctl --array=LBA --rcumode=3 --rcus=0:1 --subbands=320:381 --beamlets=0:61 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=LBA --rcumode=3 --rcus=0:47 --subbands=320:381 --beamlets=62:123 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=LBA --rcumode=3 --rcus=48:49 --subbands=320:381 --beamlets=124:185 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=LBA --rcumode=3 --rcus=0:95 --subbands=320:381 --beamlets=186:247 --direction=0,0,LOFAR_LMN&

