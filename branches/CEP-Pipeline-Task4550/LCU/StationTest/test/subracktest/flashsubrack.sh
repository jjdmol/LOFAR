# !/bin/bash
# version 2.1, date 08-10-2009,  M.J.Norden


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
bphexfile=/localhome/firmware/images/bp3b_v6_0.hex
aphexfile=/localhome/firmware/images/ap3b_v6_0.hex
tphexfile=/localhome/firmware/images/tp18_11.hex
mphexfile=/localhome/firmware/images/mp12_8.hex

mplphexfile=/localhome/firmware/images/mp_lp2_2.hex

echo "This station is "$station
echo "The number of rspboards is "$rspboards
echo "The number of tbboards is "$tbboards
echo "The selected image page is "$page
echo "Het subrack nummer= "$subracknr
echo "The bp hex file is "$bphexfile
echo "The ap hex file is "$aphexfile
echo "The mp hex file is "$mphexfile
echo "The tp hex file is "$tphexfile
echo "The offset 1 is "$offset1
echo "The offset 3 is "$offset3

######## het flashen en resetten van RSP borden ####################

eval "swlevel 1"
for ((ind=$subracknr*4; ind < ($subracknr+1)*4; ind++)) do
  MACadr=$(printf "10:FA:00:00:%02x:00" $ind)
   sudo rsuctl3 -w -q -p $page -b $bphexfile -a $aphexfile -m $MACadr -F
  #echo " rsp board = "$ind
done

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

sed -i "s/2:u:d:r::RSPDriver/#2:u:d:r::RSPDriver/g" /opt/lofar/etc/swlevel.conf
eval "swlevel 2"

echo "wacht even 50 seconden voor het opstarten van de TBB borden"
sleep 50

######## het flashen en resetten van TBB borden ####################

if [ $tbboards != 0 ] ; then
tbbctl --writeimage=0,$page,4.7,$tphexfile,$mphexfile,0
tbbctl --writeimage=1,$page,4.7,$tphexfile,$mphexfile,0
tbbctl --config=1
echo " wacht even 30 seconden voor opstart TBB image 1" 
sleep 30
tbbctl --writeimage=0,0,4.7,$tphexfile,$mplphexfile,ad001234
tbbctl --writeimage=1,0,4.7,$tphexfile,$mplphexfile,ad001234
fi

######## restart image 1 RSP borden #################################
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


######## het opstarten van de RSPDriver en TBBDriver ###############

sed -i "s/#2:u:d:r::RSPDriver/2:u:d:r::RSPDriver/g" /opt/lofar/etc/swlevel.conf
sed -i "s/##2:u:d:r::RSPDriver/2:u:d:r::RSPDriver/g" /opt/lofar/etc/swlevel.conf

eval "swlevel 2"

######## het terugzetten van TBBDriver.conf  #########################

sed -i "s/MAC_ADDR_0=10:FA:00:00:..:00/MAC_ADDR_0=10:FA:00:00:00:00/g" /opt/lofar/etc/RSPDriver.conf
sed -i "s/MAC_ADDR_1=10:FA:00:00:..:00/MAC_ADDR_1=10:FA:00:00:01:00/g" /opt/lofar/etc/RSPDriver.conf
sed -i "s/MAC_ADDR_2=10:FA:00:00:..:00/MAC_ADDR_2=10:FA:00:00:02:00/g" /opt/lofar/etc/RSPDriver.conf
sed -i "s/MAC_ADDR_3=10:FA:00:00:..:00/MAC_ADDR_3=10:FA:00:00:03:00/g" /opt/lofar/etc/RSPDriver.conf

sed -i "s/MAC_ADDR_0=10:FA:00:00:..:02/MAC_ADDR_0=10:FA:00:00:00:02/g" /opt/lofar/etc/TBBDriver.conf
sed -i "s/MAC_ADDR_1=10:FA:00:00:..:02/MAC_ADDR_1=10:FA:00:00:02:02/g" /opt/lofar/etc/TBBDriver.conf

sed -i "s/MAC_ADDR_$bn0=10:FA:00:00:..:02/MAC_ADDR_$bn0=10:FA:00:00:$offset1:02/g" /opt/lofar/etc/TBBDriver.conf
sed -i "s/MAC_ADDR_$bn1=10:FA:00:00:..:02/MAC_ADDR_$bn1=10:FA:00:00:$offset3:02/g" /opt/lofar/etc/TBBDriver.conf

######### het setten van de juiste ID in tbb_version.gold  #####################
# sed -i "s/0.......V/0   $bn2   V/g" /home/lofartest/subracktest/gold/tbb_version.gold
# sed -i "s/1.......V/1   $bn3   V/g" /home/lofartest/subracktest/gold/tbb_version.gold

####### terwijl RSPDriver opstart, start image 1 op in de TBB borden ###

#if [ $page != 0 ] ; then
#	tbbctl --config=$page --sel=0,1
#	#tbbctl --config=9 --sel=0,1
#    else 
#       echo "When the TBB flash action was sucessful please"
#       echo "set page to page=0 and reset the TBB boards"
#       echo "with ./restart_tbb.sh"
#fi

#else
#   echo "No TBB boards are placed in the SUBRACK"
#fi

echo " wacht hier 50 seconden" 
sleep 50

tbbctl --config=1

echo " wacht hier 20 seconden"
sleep 20


######## het starten van de subrack test ####################
cd /opt/subracktest

#python subrack_production.py -b $batchnr -s $serienr &

rspctl --version

tbbctl --version




