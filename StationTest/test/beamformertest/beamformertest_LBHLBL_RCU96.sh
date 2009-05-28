# beamformermformer test for 96 rcu's in LBA mode (LBL & LBH input)
# two antennas per beam, four beams
# object zenith
# version 1.5  27 may 2009 M.J.Norden

killall beamctl

eval swlevel 3

beamctl --array=LBAINNER --rcumode=3 --rcus=0:1 --subbands=270:331 --beamlets=124:185 --direction=0,0,LOFAR_LMN&
beamctl --array=LBAINNER --rcumode=3 --rcus=0:47 --subbands=270:331 --beamlets=186:247 --direction=0,0,LOFAR_LMN&
beamctl --array=LBAOUTER --rcumode=1 --rcus=47:48 --subbands=270:331 --beamlets=0:61 --direction=0,0,LOFAR_LMN&
beamctl --array=LBAOUTER --rcumode=1 --rcus=48:95 --subbands=270:331 --beamlets=62:123 --direction=0,0,LOFAR_LMN&

sleep 5
rspctl --wg=0
rspctl --rcuprsg=0
rspctl --rcuenable=1
rspctl --rcumode=3 --sel=0:47
sleep 3
rspctl --rcumode=1 --sel=48:95
rspctl --stati=beamlet
