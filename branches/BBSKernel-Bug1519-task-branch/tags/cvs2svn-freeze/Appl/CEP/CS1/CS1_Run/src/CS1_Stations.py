   
# todo: this doesn't work yet with multiple rspboards per station
class Station(object):
    """
    Represents a real or virtual station. Position is a tuple of three floats
    representing ITRF x,y,z coordinates.
    """
    def __init__(self, name):
        self.name = name
        stationNumber = int(name.split('_')[0].split('CS')[-1])
    def getName(self):
        return self.name

#define the stations as a list so we can easily add them
#Keep the antenna positions in ITRF coordinates.
#Storing a position on earth given as (lon,lat,height)

CS010_HBA0 = [Station('CS010_HBA0')]
CS010_HBA1 = [Station('CS010_HBA1')]
CS010_HBA2 = [Station('CS010_HBA2')]
CS010_HBA3 = [Station('CS010_HBA3')]
CS010_HBAs = CS010_HBA0 + CS010_HBA1 + CS010_HBA2 + CS010_HBA3

CS008_HBA0 = [Station('CS008_HBA0')]
CS008_HBA1 = [Station('CS008_HBA1')]
CS008_HBA2 = [Station('CS008_HBA2')]
CS008_HBA3 = [Station('CS008_HBA3')]
CS008_HBAs = CS008_HBA0 + CS008_HBA1 + CS008_HBA2 + CS008_HBA3

CS001_HBA0 = [Station('CS001_HBA0')]
CS001_HBA1 = [Station('CS001_HBA1')]
CS001_HBA2 = [Station('CS001_HBA2')]
CS001_HBA3 = [Station('CS001_HBA3')]
CS001_HBAs = CS001_HBA0 + CS001_HBA1 + CS001_HBA2 + CS001_HBA3

CS016_HBA0 = [Station('CS016_HBA0')]
CS016_HBA1 = [Station('CS016_HBA1')]
CS016_HBA2 = [Station('CS016_HBA2')]
CS016_HBA3 = [Station('CS016_HBA3')]
CS016_HBAs = CS016_HBA0 + CS016_HBA1 + CS016_HBA2 + CS016_HBA3

CS010_dipole0  = [Station('CS010_dipole0')]
CS010_dipole4  = [Station('CS010_dipole4')]
CS010_dipole8  = [Station('CS010_dipole8')]
CS010_dipole12 = [Station('CS010_dipole12')]
CS010_4dipoles0_4_8_12 = CS010_dipole0 + CS010_dipole4 + CS010_dipole8 + CS010_dipole12

CS008_dipole0  = [Station('CS008_dipole0')]
CS008_dipole4  = [Station('CS008_dipole4')]
CS008_dipole8  = [Station('CS008_dipole8')]
CS008_dipole12 = [Station('CS008_dipole12')]
CS008_4dipoles0_4_8_12 = CS008_dipole0 + CS008_dipole4 + CS008_dipole8 + CS008_dipole12

CS008_dipole2  = [Station('CS008_dipole2')]
CS008_dipole6  = [Station('CS008_dipole6')]
CS008_dipole10 = [Station('CS008_dipole10')]
CS008_dipole14 = [Station('CS008_dipole14')]
CS008_4dipoles2_6_10_14 = CS008_dipole2 + CS008_dipole6 + CS008_dipole10 + CS008_dipole14

CS001_dipole0  = [Station('CS001_dipole0')]
CS001_dipole4  = [Station('CS001_dipole4')]
CS001_dipole8  = [Station('CS001_dipole8')]
CS001_dipole12 = [Station('CS001_dipole12')]
CS001_4dipoles0_4_8_12 = CS001_dipole0 + CS001_dipole4 + CS001_dipole8 + CS001_dipole12

CS001_dipole2  = [Station('CS001_dipole2')]
CS001_dipole6  = [Station('CS001_dipole6')]
CS001_dipole10 = [Station('CS001_dipole10')]
CS001_dipole14 = [Station('CS001_dipole14')]
CS001_4dipoles2_6_10_14 = CS001_dipole2 + CS001_dipole6 + CS001_dipole10 + CS001_dipole14

CS016_dipole0  = [Station('CS016_dipole0')]
CS016_dipole4  = [Station('CS016_dipole4')]
CS016_dipole8  = [Station('CS016_dipole8')]
CS016_dipole12 = [Station('CS016_dipole12')]
CS016_4dipoles0_4_8_12 = CS016_dipole0 + CS016_dipole4 + CS016_dipole8 + CS016_dipole12

CS016_dipole2  = [Station('CS016_dipole2')]
CS016_dipole6  = [Station('CS016_dipole6')]
CS016_dipole10 = [Station('CS016_dipole10')]
CS016_dipole14 = [Station('CS016_dipole14')]
CS016_4dipoles2_6_10_14 = CS016_dipole2 + CS016_dipole6 + CS016_dipole10 + CS016_dipole14

CS030_dipole0  = [Station('CS030_dipole0')]
CS030_dipole7  = [Station('CS030_dipole7')]
CS030_dipole10 = [Station('CS030_dipole10')]
CS030_dipole14 = [Station('CS030_dipole14')]
CS030_4dipoles = CS030_dipole0 + CS030_dipole7 + CS030_dipole10 + CS030_dipole14

CS031_dipole16 = [Station('CS031_dipole16')]
CS031_dipole22 = [Station('CS031_dipole22')]
CS031_dipole24 = [Station('CS031_dipole24')]
CS031_dipole31 = [Station('CS031_dipole31')]
CS031_4dipoles = CS031_dipole16 + CS031_dipole22 + CS031_dipole24 + CS031_dipole31

CS032_dipole34 = [Station('CS032_dipole34')]
CS032_dipole37 = [Station('CS032_dipole37')]
CS032_dipole42 = [Station('CS032_dipole42')]
CS032_dipole46 = [Station('CS032_dipole46')]
CS032_4dipoles = CS032_dipole34 + CS032_dipole37 + CS032_dipole42 + CS032_dipole46

CS010_us0  = [Station('CS010_us0')]
CS010_us1  = [Station('CS010_us1')]
CS010_us2  = [Station('CS010_us2')]
CS010_us3 =  [Station('CS010_us3')]
CS010_4us = CS010_us0 + CS010_us1 + CS010_us2 + CS010_us3

CS008_us0  = [Station('CS008_us0')]
CS008_us1  = [Station('CS008_us1')]
CS008_us2  = [Station('CS008_us2')]
CS008_us3 =  [Station('CS008_us3')]
CS008_4us = CS008_us0 + CS008_us1 + CS008_us2 + CS008_us3

CS001_us0  = [Station('CS001_us0')]
CS001_us1  = [Station('CS001_us1')]
CS001_us2  = [Station('CS001_us2')]
CS001_us3 =  [Station('CS001_us3')]
CS001_4us = CS001_us0 + CS001_us1 + CS001_us2 + CS001_us3

CS016_us0  = [Station('CS016_us0')]
CS016_us1  = [Station('CS016_us1')]
CS016_us2  = [Station('CS016_us2')]
CS016_us3 =  [Station('CS016_us3')]
CS016_4us = CS016_us0 + CS016_us1 + CS016_us2 + CS016_us3

AllMicroStations = CS010_4us + CS001_4us + CS008_4us + CS016_4us
AllDipoles0_4_8_12 = CS010_4dipoles0_4_8_12 + CS001_4dipoles0_4_8_12 + CS008_4dipoles0_4_8_12 + CS016_4dipoles0_4_8_12
AllDipolesMixed = CS010_4dipoles0_4_8_12 + CS001_4dipoles2_6_10_14 + CS008_4dipoles2_6_10_14 + CS016_4dipoles2_6_10_14
AllHBAs = CS010_HBAs + CS001_HBAs + CS008_HBAs + CS016_HBAs
Mixed = CS010_us0 + CS010_dipole4 + CS010_dipole8 + CS010_dipole12 + CS001_4dipoles0_4_8_12 + CS008_4dipoles0_4_8_12 + CS016_4dipoles0_4_8_12
All24Dipoles = CS001_4dipoles0_4_8_12 + CS008_4dipoles0_4_8_12 + CS016_4dipoles0_4_8_12 + CS030_4dipoles + CS031_4dipoles + CS032_4dipoles
Test = CS001_4dipoles0_4_8_12 + CS008_4dipoles0_4_8_12 + CS016_4dipoles0_4_8_12
