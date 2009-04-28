# beamformermformer test for 96 rcu's in LBA mode (LBL & LBH input)
# two antennas per beam, four beams
# object zenith
# version 1.3  23 maa 2009 M.J.Norden

killall beamctl

beamctl --array=DUMMY48 --rcumode=3 --rcus=0 --subbands=100:139 --beamlets=0:39 --direction=0,0,LOFAR_LMN&
beamctl --array=DUMMY48 --rcumode=3 --rcus=0:47 --subbands=150:189 --beamlets=50:89 --direction=0,0,LOFAR_LMN&
beamctl --array=DUMMY48 --rcumode=1 --rcus=48 --subbands=200:239 --beamlets=100:139 --direction=0,0,LOFAR_LMN&
beamctl --array=DUMMY48 --rcumode=1 --rcus=48:95 --subbands=250:289 --beamlets=150:189 --direction=0,0,LOFAR_LMN&

sleep 5
rspctl --wg=0
rspctl --rcuprsg=0
rspctl --rcuenable=1
rspctl --rcumode=3 --sel=0:95
rspctl --rcumode=1 --sel=48:95

