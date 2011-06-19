# beamaartfaac.sh
# beamformer script for 96 rcu's in LBA_OUTER mode (with splitter OFF)
# object zenith, 240 beams, 48 antennas with 5 subbands around 60MHz
# version 1.1,  17 jun 2011, M.J.Norden


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
let r=0
for (( i=0; i<=239; i=i+5 ))
  do 
     let a=(2*$i)/5
     let b=$a+1
     beamctl --antennaset=LBA_OUTER --rcumode=2 --rcus=$a:$b --subbands=306 --beamlets=$r --digdir=0,1.5708,AZELGEO&
     sleep 2
     let r=$r+1
     beamctl --antennaset=LBA_OUTER --rcumode=2 --rcus=$a:$b --subbands=307 --beamlets=$r --digdir=0,1.5708,AZELGEO&
     sleep 2
     let r=$r+1
     beamctl --antennaset=LBA_OUTER --rcumode=2 --rcus=$a:$b --subbands=308 --beamlets=$r --digdir=0,1.5708,AZELGEO&
     sleep 2
     let r=$r+1
     beamctl --antennaset=LBA_OUTER --rcumode=2 --rcus=$a:$b --subbands=309 --beamlets=$r --digdir=0,1.5708,AZELGEO&
     sleep 2
     let r=$r+1
     beamctl --antennaset=LBA_OUTER --rcumode=2 --rcus=$a:$b --subbands=310 --beamlets=$r --digdir=0,1.5708,AZELGEO&
     sleep 2
     let r=$r+1
  done

