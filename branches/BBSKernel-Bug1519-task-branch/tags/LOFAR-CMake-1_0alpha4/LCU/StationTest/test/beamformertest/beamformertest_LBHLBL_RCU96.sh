# beamformermformer test for 96 rcu's in LBA mode (LBL & LBH input)
# two antennas per beam, four beams
# object zenith
# version 1.6  1 jul 2009 M.J.Norden


rspctl --wg=0
rspctl --rcuprsg=0

killall beamctl
eval swlevel 3
sleep 2

beamctl --array=LBAOUTER --rcumode=1 --rcus=48:49 --subbands=270:331 --beamlets=0:61 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=LBAINNER --rcumode=3 --rcus=0:1 --subbands=270:331 --beamlets=124:185 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=LBAOUTER --rcumode=1 --rcus=48:95 --subbands=270:331 --beamlets=62:123 --direction=0,0,LOFAR_LMN&
sleep 3
beamctl --array=LBAINNER --rcumode=3 --rcus=0:47 --subbands=270:331 --beamlets=186:247 --direction=0,0,LOFAR_LMN&

