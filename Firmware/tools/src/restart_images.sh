# !/bin/bash
# version 2.2, date 05-11-2008,  M.J.Norden
eval "swlevel 1"
page=15
station=`hostname -s`
let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`

echo "This station is "$station

for ((ind=0; ind < $rspboards; ind++)) do
  MACadr=$(printf "10:FA:00:00:%02x:00" $ind)
  sudo rsuctl3_reset -q -x -p $page -m $MACadr;
done
