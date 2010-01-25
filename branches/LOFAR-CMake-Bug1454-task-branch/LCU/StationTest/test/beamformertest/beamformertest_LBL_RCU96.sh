# beamformer test for 96 rcu's in LBL mode (with splitter OFF)
# object zenith
# version 1.1,  13 jan 2010, M.J.Norden


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

beamctl --array=LBA_OUTER --rcumode=1 --rcus=0:1 --subbands=270:331 --beamlets=0:61 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=LBA_OUTER --rcumode=1 --rcus=0:47 --subbands=270:331 --beamlets=62:123 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=LBA_OUTER --rcumode=1 --rcus=48:49 --subbands=270:331 --beamlets=124:185 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=LBA_OUTER --rcumode=1 --rcus=0:95 --subbands=270:331 --beamlets=186:247 --direction=0,0,LOFAR_LMN&

