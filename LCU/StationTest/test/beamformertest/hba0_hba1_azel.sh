#!/bin/sh
# 1.2 test script to verify HBA0 and HBA1 settings
# North, East, South, West positions with AZELGEO
# 18-01-11, M.J Norden
# HBA input with antenna

killall beamctl
rspctl --rcuprsg=0
rspctl --wg=0
rspctl --splitter=1

swlevel 3
sleep 10

# rotation CS002
let rotation_hba0=52
let rotation_hba1=0

echo "rotation_hba0" $rotation_hba0
echo "rotation_hba1" $rotation_hba1

HBAnorth0=`(echo - | awk -v K=$rotation_hba0 '{print 0 + (K/360)*2*3.14159}')`;
HBAnorth1=`(echo - | awk -v K=$rotation_hba1 '{print 0 + (K/360)*2*3.14159}')`;

HBAeast0=`(echo - | awk -v K=$rotation_hba0 '{print 1.5708 + (K/360)*2*3.14159}')`;
HBAeast1=`(echo - | awk -v K=$rotation_hba1 '{print 1.5708 + (K/360)*2*3.14159}')`;

HBAsouth0=`(echo - | awk -v K=$rotation_hba0 '{print 3.1459 + (K/360)*2*3.14159}')`;
HBAsouth1=`(echo - | awk -v K=$rotation_hba1 '{print 3.1459 + (K/360)*2*3.14159}')`;

HBAwest0=`(echo - | awk -v K=$rotation_hba0 '{print 4.7124 + (K/360)*2*3.14159}')`;
HBAwest1=`(echo - | awk -v K=$rotation_hba1 '{print 4.7124 + (K/360)*2*3.14159}')`;


sleep 3

# pointing to horizon north
beamctl --antennaset=HBA_ZERO --rcus=0:47 --rcumode=5 --subbands=300:360 --beamlets=0:60 --anadir=$HBAnorth0,0,AZELGEO --digdir=$HBAnorth0,0,AZELGEO&
sleep 3
beamctl --antennaset=HBA_ONE --rcus=48:95 --rcumode=5 --subbands=300:360 --beamlets=1000:1060 --anadir=$HBAnorth1,0,AZELGEO --digdir=$HBAnorth1,0,AZELGEO&
sleep 20
rspctl --realdelays > realdelays_north_`date +%F_%T`.log
killall beamctl
sleep 10

# pointing to horizon east
beamctl --antennaset=HBA_ZERO --rcus=0:47 --rcumode=5 --subbands=300:360 --beamlets=0:60 --anadir=$HBAeast0,0,AZELGEO --digdir=$HBAeast0,0,AZELGEO&
sleep 3
beamctl --antennaset=HBA_ONE --rcus=48:95 --rcumode=5 --subbands=300:360 --beamlets=1000:1060 --anadir=$HBAeast1,0,AZELGEO --digdir=$HBAeast1,0,AZELGEO&
sleep 20
rspctl --realdelays > realdelays_east_`date +%F_%T`.log
killall beamctl
sleep 10

# pointing to horizon south
beamctl --antennaset=HBA_ZERO --rcus=0:47 --rcumode=5 --subbands=300:360 --beamlets=0:60 --anadir=$HBAsouth0,0,AZELGEO --digdir=$HBAsouth0,0,AZELGEO&
sleep 3
beamctl --antennaset=HBA_ONE --rcus=48:95 --rcumode=5 --subbands=300:360 --beamlets=1000:1060 --anadir=$HBAsouth1,0,AZELGEO --digdir=$HBAsouth1,0,AZELGEO&
sleep 20
rspctl --realdelays > realdelays_south_`date +%F_%T`.log
killall beamctl
sleep 10

# pointing to horizon west
beamctl --antennaset=HBA_ZERO --rcus=0:47 --rcumode=5 --subbands=300:360 --beamlets=0:60 --anadir=$HBAwest0,0,AZELGEO --digdir=$HBAwest0,0,AZELGEO&
sleep 3
beamctl --antennaset=HBA_ONE --rcus=48:95 --rcumode=5 --subbands=300:360 --beamlets=1000:1060 --anadir=$HBAwest1,0,AZELGEO --digdir=$HBAwest1,0,AZELGEO&
sleep 20
rspctl --realdelays > realdelays_west_`date +%F_%T`.log
killall beamctl
sleep 10

# pointing to zenith and north
beamctl --antennaset=HBA_ZERO --rcus=0:47 --rcumode=5 --subbands=300:360 --beamlets=0:60 --anadir=$HBAnorth,1.5708,AZELGEO --digdir=$HBAnorth,1.5708,AZELGEO&
sleep 3
beamctl --antennaset=HBA_ONE --rcus=48:95 --rcumode=5 --subbands=300:360 --beamlets=1000:1060 --anadir=$HBAnorth,1.5708,AZELGEO --digdir=$HBAnorth,1.5708,AZELGEO&
sleep 20
rspctl --realdelays > realdelays_zenith_`date +%F_%T`.log
killall beamctl
sleep 10




