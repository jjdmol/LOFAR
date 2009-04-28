# beamformer test for 192 rcu's in LBA mode (LBH input)
# twentyfour antenna's per beam, four beams
# object zenith
# version 1.3  23 maa 2009 M.J.Norden

killall beamctl

beamctl --array=HBA --rcumode=5 --rcus=0:47 --subbands=100:139 --beamlets=0:39 --direction=0,0,LOFAR_LMN&
beamctl --array=HBA --rcumode=5 --rcus=48:95 --subbands=150:189 --beamlets=50:89 --direction=0,0,LOFAR_LMN&
beamctl --array=HBA --rcumode=5 --rcus=96:143 --subbands=200:239 --beamlets=100:139 --direction=0,0,LOFAR_LMN&
beamctl --array=HBA --rcumode=5 --rcus=144:191 --subbands=250:289 --beamlets=150:189 --direction=0,0,LOFAR_LMN&

sleep 5
rspctl --wg=0
rspctl --rcuprsg=0
rspctl --rcuenable=1
rspctl --rcumode=5

