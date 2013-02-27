# !/bin/bash
# version 2.1, date 03-11-2008,  M.J.Norden
station=`hostname -s`
let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`

echo "This station is "$station
echo "The number of rspboards is "$rspboards

for ((ind=0; ind < $rspboards; ind++)) do
  MACadr=$(printf "10:FA:00:00:%02x:00" $ind)
  sudo rsuctl3 -q -m $MACadr -V
  sudo rsuctl3 -q -m $MACadr -l
done
