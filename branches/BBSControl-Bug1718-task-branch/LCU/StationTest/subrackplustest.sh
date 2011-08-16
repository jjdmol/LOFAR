# !/bin/bash
# subrackplustest.sh, version 1.0, date 16-01-2009,  M.J.Norden


#export PYTHONPATH=/home/lofartest/subracktest/modules

######## definieeren en printen van de variabelen ####################
page=1
tbbpage=1
station=`hostname -s`
let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
let tbboards=`sed -n  's/^\s*RS\.N_TBBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`

echo "This station is "$station
echo "The number of rspboards is "$rspboards
echo "The number of tbboards is "$tbboards
echo "The selected image page is "$page

######## het resetten van RSP borden ####################
# in de nieuwe swlevel is dit niet meer nodig    

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

######## het opstarten van de RSP en TBBDriver ###########################

eval "swlevel 2"

####### terwijl RSPDriver opstart, start image 1 op in de TBB borden ###

if [ $tbbpage != 0 ] ; then
	tbbctl --config=$tbbpage 
    else 
       echo "When the TBB flash action was sucessful please"
       echo "set page to page=0 and reset the TBB boards"
       echo "with ./restart_tbb.sh"
fi

echo "wacht hier 50 seconden voor opstarten TBB en RSP borden"
sleep 50

######## het starten van de subrack test ####################
cd /opt/stationtest
python station_production.py -r $rspboards -t $tbboards




