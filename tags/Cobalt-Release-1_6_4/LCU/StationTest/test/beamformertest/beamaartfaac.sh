# beamaartfaac.sh
# beamformer script for 96 rcu's in LBA_OUTER mode (with splitter OFF)
# object zenith, 240 beams, 48 antennas with 5 subbands around 60MHz
# not used beamlets are 60, 121, 182 and 243
# version 1.2,  20 jun 2011, M.J.Norden and J.Romein
# 
# rcumode=1 or rcumode=2 --> LBA_OUTER and swapxy=1
# rcumode=3 or rcumode=4 --> LBA_INNER and swapxy=0
# X dipoles are always on the EVEN receivers and Y dipoles are always on the ODD receivers

# define rcumode here
rcumode=2

if [ $rcumode -lt 3 ] ; then
  rspctl --swapxy=1
  array="LBA_OUTER"
else
  rspctl --swapxy=0
  array="LBA_INNER"
fi

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
for ((i = 0; i < 48; i ++))
  do
     let "a = 2 * $i"
     let "b = 2 * $i + 1"
     for ((j = 0; j < 5; j ++))
       do
	 let "s = 306 + $j"
	 let "r = 5 * $i + 5 * $i / 60 + $j"
	 beamctl --antennaset=$array --rcumode=$rcumode --rcus=$a:$b --subbands=$s --beamlets=$r --digdir=0,1.5708,AZELGEO&
         sleep 2
       done
  done


