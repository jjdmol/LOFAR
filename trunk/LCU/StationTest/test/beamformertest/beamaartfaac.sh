# beamaartfaac.sh
# beamformer script for 96 rcu's in LBA_OUTER mode (with splitter OFF)
# object zenith, 48 beams of 5 subbands around 60MHz
# version 1.0,  16 jun 2011, M.J.Norden


rspctl --wg=0
sleep 1
rspctl --rcuprsg=0
sleep 1

killall beamctl
sleep 1
swlevel 3
sleep 2
rspctl --splitter=0
sleep 3

for (( i=0; i<=239; i=i+5 ))
  do 
     let b=i+4
     echo "Define beam and wait 1 seconds"
     beamctl --antennaset=LBA_OUTER --rcumode=2 --rcus=0:95 --subbands=306:310 --beamlets=$i:$b --digdir=0,1.5708,AZELGEO&
     sleep 1 
  done

