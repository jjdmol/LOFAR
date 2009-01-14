# !/bin/bash
# version 2.2, date 05-11-2008,  M.J.Norden
eval "swlevel 1"
page=15
station=`hostname -s`
let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
bphexfile=bp3b_v5_3.hex
aphexfile=ap3b_v5_3.hex

echo "This station is "$station
echo "The number of rspboards is "$rspboards
echo "The selected image page is "$page
echo "The bp hex file is "$bphexfile
echo "The ap hex file is "$aphexfile

for ((ind=0; ind < $rspboards; ind++)) do
  MACadr=$(printf "10:FA:00:00:%02x:00" $ind)
  sudo rsuctl3 -w -q -p $page -b $bphexfile -a $aphexfile -m $MACadr -F
done


if [ $page != 0 ] ; then
	for ((ind=0; ind < $rspboards; ind++)) do
  		MACadr=$(printf "10:FA:00:00:%02x:00" $ind)
  		sudo rsuctl3_reset -m $MACadr -p $page -x -q;
        done
    else 
       echo "When the RSP flash action was sucessful please"
       echo "set page to page=0 and reset the RSP boards"
       echo "with ./restart_images.sh"
fi       
