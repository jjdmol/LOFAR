# !/bin/bash
#version 2.0, date 18-08-2009,  M.J.Norden


#export PYTHONPATH=/opt/stationtest/modules

######## vragen subrack, batchnr en serienr ####################
declare offset1
declare offset2
declare offset3
declare offset4


vraag1="welk subrack ga je testen [0, 1, 2, 3, 4, of 5] "
echo -n "$vraag1"
read subracknr

case $subracknr in
[0]*)
 offset1="00"
 offset2="01"
 offset3="02"
 offset4="03"
 ;;
[1]*)
 offset1="04"
 offset2="05"
 offset3="06"
 offset4="07"
 ;;
[2]*)
 offset1="08"
 offset2="09"
 offset3="0A"
 offset4="0B"
 ;;
[3]*)
 offset1="0C"
 offset2="0D"
 offset3="0E"
 offset4="0F"
 ;;
[4]*)
 offset1="10"
 offset2="11"
 offset3="12"
 offset4="13"
 ;;
[5]*)
 offset1="14"
 offset2="15"
 offset3="16"
 offset4="17"
 ;;
esac
######## definieeren en printen van de variabelen ####################
page=1
station=`hostname -s`
let rspboards=`sed -n  's/^\s*RS\.N_RSPBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`
let tbboards=`sed -n  's/^\s*RS\.N_TBBOARDS\s*=\s*\([0-9][0-9]*\).*$/\1/p' /opt/lofar/etc/RemoteStation.conf`

echo "This station is "$station
echo "The number of rspboards is "$rspboards
echo "The number of tbboards is "$tbboards
echo "The selected image page is "$page
echo "The offset 1 is "$offset1
echo "The offset 3 is "$offset3

######## het flashen en resetten van RSP borden ####################

eval "swlevel 1"
#for ((ind=$subracknr*4; ind < ($subracknr+1)*4; ind++)) do
#  MACadr=$(printf "10:FA:00:00:%02x:00" $ind)
#  #sudo rsuctl3b -w -q -p $page -b $bphexfile -a $aphexfile -m $MACadr -F
#  echo " rsp board = "$ind
#done
#
if [ $page != 0 ] ; then
	for ((ind=$subracknr*4; ind < ($subracknr+1)*4; ind++)) do
  		MACadr=$(printf "10:FA:00:00:%02x:00" $ind)
  		sudo rsuctl3_reset -m $MACadr -p $page -x -q;
             #echo " rsp board = "$ind
          done
    else 
       echo "When the RSP flash action was sucessful please"
       echo "set page to page=0 and reset the RSP boards"
       echo "with ./restart_images.sh"
fi       

######## het aanpassen van RSPDriver.conf  #########################

sed -i "s/MAC_ADDR_0=10:FA:00:00:..:00/MAC_ADDR_0=10:FA:00:00:$offset1:00/g" /opt/lofar/etc/RSPDriver.conf
sed -i "s/MAC_ADDR_1=10:FA:00:00:..:00/MAC_ADDR_1=10:FA:00:00:$offset2:00/g" /opt/lofar/etc/RSPDriver.conf
sed -i "s/MAC_ADDR_2=10:FA:00:00:..:00/MAC_ADDR_2=10:FA:00:00:$offset3:00/g" /opt/lofar/etc/RSPDriver.conf
sed -i "s/MAC_ADDR_3=10:FA:00:00:..:00/MAC_ADDR_3=10:FA:00:00:$offset4:00/g" /opt/lofar/etc/RSPDriver.conf

######## het aanpassen van TBBDriver.conf  #########################

sed -i "s/MAC_ADDR_0=10:FA:00:00:..:02/MAC_ADDR_0=10:FA:00:00:$offset1:02/g" /opt/lofar/etc/TBBDriver.conf
sed -i "s/MAC_ADDR_1=10:FA:00:00:..:02/MAC_ADDR_1=10:FA:00:00:$offset3:02/g" /opt/lofar/etc/TBBDriver.conf

let bn0=$subracknr*2
let bn1=$bn0+1
let bn2=$bn0*2
let bn3=$bn1*2

sed -i "s/MAC_ADDR_$bn0=10:FA:00:00:..:02/MAC_ADDR_$bn0=10:FA:00:00:00:02/g" /opt/lofar/etc/TBBDriver.conf
sed -i "s/MAC_ADDR_$bn1=10:FA:00:00:..:02/MAC_ADDR_$bn1=10:FA:00:00:02:02/g" /opt/lofar/etc/TBBDriver.conf

######## het opstarten van de TBBDriver ###########################

#sed -i "s/2:u:d:r::RSPDriver/#2:u:d:r::RSPDriver/g" /opt/lofar/etc/swlevel.conf
eval "swlevel 2"
#rspctl --regstat
echo "wacht even 50 seconden voor het opstarten van de RSP & TBB borden"
sleep 50

######## het opstarten van de RSPDriver en TBBDriver ###############

sed -i "s/#2:u:d:r::RSPDriver/2:u:d:r::RSPDriver/g" /opt/lofar/etc/swlevel.conf
sed -i "s/##2:u:d:r::RSPDriver/2:u:d:r::RSPDriver/g" /opt/lofar/etc/swlevel.conf


#eval "swlevel 2"

######## het terugzetten van TBBDriver.conf  #########################

sed -i "s/MAC_ADDR_0=10:FA:00:00:..:00/MAC_ADDR_0=10:FA:00:00:00:00/g" /opt/lofar/etc/RSPDriver.conf
sed -i "s/MAC_ADDR_1=10:FA:00:00:..:00/MAC_ADDR_1=10:FA:00:00:01:00/g" /opt/lofar/etc/RSPDriver.conf
sed -i "s/MAC_ADDR_2=10:FA:00:00:..:00/MAC_ADDR_2=10:FA:00:00:02:00/g" /opt/lofar/etc/RSPDriver.conf
sed -i "s/MAC_ADDR_3=10:FA:00:00:..:00/MAC_ADDR_3=10:FA:00:00:03:00/g" /opt/lofar/etc/RSPDriver.conf

sed -i "s/MAC_ADDR_0=10:FA:00:00:..:02/MAC_ADDR_0=10:FA:00:00:00:02/g" /opt/lofar/etc/TBBDriver.conf
sed -i "s/MAC_ADDR_1=10:FA:00:00:..:02/MAC_ADDR_1=10:FA:00:00:02:02/g" /opt/lofar/etc/TBBDriver.conf

sed -i "s/MAC_ADDR_$bn0=10:FA:00:00:..:02/MAC_ADDR_$bn0=10:FA:00:00:$offset1:02/g" /opt/lofar/etc/TBBDriver.conf
sed -i "s/MAC_ADDR_$bn1=10:FA:00:00:..:02/MAC_ADDR_$bn1=10:FA:00:00:$offset3:02/g" /opt/lofar/etc/TBBDriver.conf

####### het starten van de subrack test ####################
cd /opt/stationtest

rspctl --version
sleep 1
tbbctl --version
