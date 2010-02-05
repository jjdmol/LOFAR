# beamformer test for 96 rcu's in HBA mode (with splitter ON)
# object zenith
# version 1.0,  1 oct 2009, M.J.Norden


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

beamctl --array=HBA --rcumode=5 --rcus=0:1 --subbands=320:381 --beamlets=0:61 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=HBA --rcumode=5 --rcus=0:47 --subbands=320:381 --beamlets=62:123 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=HBA --rcumode=5 --rcus=48:49 --subbands=320:381 --beamlets=1124:1185 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=HBA --rcumode=5 --rcus=48:95 --subbands=320:381 --beamlets=1186:1247 --direction=0,0,LOFAR_LMN&

