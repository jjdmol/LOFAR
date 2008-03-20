   
# todo: this doesn't work yet with multiple rspboards per station
class Station(object):
    """
    Represents a real or virtual station. Position is a tuple of three floats
    representing ITRF x,y,z coordinates.
    """
    def __init__(self, name):
        self.name = name
        #stationNumber = int(name.split('_')[0].split('CS')[-1])
    def getName(self):
        return self.name

#define the stations as a list so we can easily add them
#Keep the antenna positions in ITRF coordinates.
#Storing a position on earth given as (lon,lat,height)

CS010_1_dipole0  = [Station('CS010_1.dipole0')]
CS010_1_dipole4  = [Station('CS010_1.dipole4')]
CS010_1_dipole8  = [Station('CS010_1.dipole8')]
CS010_1_dipole12 = [Station('CS010_1.dipole12')]
CS010_1_4dipoles = CS010_1_dipole0 + CS010_1_dipole4 + CS010_1_dipole8 + CS010_1_dipole12
CS010_3_HBA0 = [Station('CS010_3.HBA0')]
CS010_3_HBA1 = [Station('CS010_3.HBA1')]
CS010_3_HBA2 = [Station('CS010_3.HBA2')]
CS010_3_HBA3 = [Station('CS010_3.HBA3')]
CS010_3_HBAs = CS010_3_HBA0 + CS010_3_HBA1 + CS010_3_HBA2 + CS010_3_HBA3
CS010_4_us0  = [Station('CS010_4.us0')]
CS010_4_us1  = [Station('CS010_4.us1')]
CS010_4_us2  = [Station('CS010_4.us2')]
CS010_4_us3  = [Station('CS010_4.us3')]
CS010_4_4us  = CS010_4_us0 + CS010_4_us1 + CS010_4_us2 + CS010_4_us3
CS010_5_TILE1 = [Station('CS010_5.TILE1')]
CS010_5_TILE2 = [Station('CS010_5.TILE2')]
CS010_5_TILE3 = [Station('CS010_5.TILE3')]
CS010_5_TILE4 = [Station('CS010_5.TILE4')]
CS010_5_TILE5 = [Station('CS010_5.TILE5')]
CS010_TILE6 = [Station('CS010_TILE6')]
CS010_5_TILES = CS010_5_TILE5 + CS010_5_TILE2 + CS010_5_TILE3 + CS010_5_TILE4



CS008_1_dipole0  = [Station('CS008_1.dipole0')]
CS008_1_dipole4  = [Station('CS008_1.dipole4')]
CS008_1_dipole8  = [Station('CS008_1.dipole8')]
CS008_1_dipole12 = [Station('CS008_1.dipole12')]
CS008_1_4dipoles = CS008_1_dipole0 + CS008_1_dipole4 + CS008_1_dipole8 + CS008_1_dipole12
CS008_2_dipole2  = [Station('CS008_2.dipole2')]
CS008_2_dipole6  = [Station('CS008_2.dipole6')]
CS008_2_dipole10 = [Station('CS008_2.dipole10')]
CS008_2_dipole14 = [Station('CS008_2.dipole14')]
CS008_2_4dipoles = CS008_2_dipole2 + CS008_2_dipole6 + CS008_2_dipole10 + CS008_2_dipole14
CS008_3_HBA0 = [Station('CS008_3.HBA0')]
CS008_3_HBA1 = [Station('CS008_3.HBA1')]
CS008_3_HBA2 = [Station('CS008_3.HBA2')]
CS008_3_HBA3 = [Station('CS008_3.HBA3')]
CS008_3_HBAs = CS008_3_HBA0 + CS008_3_HBA1 + CS008_3_HBA2 + CS008_3_HBA3
CS008_4_us0  = [Station('CS008_4.us0')]
CS008_4_us1  = [Station('CS008_4.us1')]
CS008_4_us2  = [Station('CS008_4.us2')]
CS008_4_us3  = [Station('CS008_4.us3')]
CS008_4_4us  = CS008_4_us0 + CS008_4_us1 + CS008_4_us2 + CS008_4_us3

CS001_1_dipole0  = [Station('CS001_1.dipole0')]
CS001_1_dipole4  = [Station('CS001_1.dipole4')]
CS001_1_dipole8  = [Station('CS001_1.dipole8')]
CS001_1_dipole12 = [Station('CS001_1.dipole12')]
CS001_1_4dipoles = CS001_1_dipole0 + CS001_1_dipole4 + CS001_1_dipole8 + CS001_1_dipole12
CS001_2_dipole2  = [Station('CS001_2.dipole2')]
CS001_2_dipole6  = [Station('CS001_2.dipole6')]
CS001_2_dipole10 = [Station('CS001_2.dipole10')]
CS001_2_dipole14 = [Station('CS001_2.dipole14')]
CS001_2_4dipoles = CS001_2_dipole2 + CS001_2_dipole6 + CS001_2_dipole10 + CS001_2_dipole14
CS001_3_HBA0 = [Station('CS001_3.HBA0')]
CS001_3_HBA1 = [Station('CS001_3.HBA1')]
CS001_3_HBA2 = [Station('CS001_3.HBA2')]
CS001_3_HBA3 = [Station('CS001_3.HBA3')]
CS001_3_HBAs = CS001_3_HBA0 + CS001_3_HBA1 + CS001_3_HBA2 + CS001_3_HBA3
CS001_4_us0  = [Station('CS001_4.us0')]
CS001_4_us1  = [Station('CS001_4.us1')]
CS001_4_us2  = [Station('CS001_4.us2')]
CS001_4_us3  = [Station('CS001_4.us3')]
CS001_4_4us  = CS001_4_us0 + CS001_4_us1 + CS001_4_us2 + CS001_4_us3

CS016_1_dipole0  = [Station('CS016_1.dipole0')]
CS016_1_dipole4  = [Station('CS016_1.dipole4')]
CS016_1_dipole8  = [Station('CS016_1.dipole8')]
CS016_1_dipole12 = [Station('CS016_1.dipole12')]
CS016_1_4dipoles = CS016_1_dipole0 + CS016_1_dipole4 + CS016_1_dipole8 + CS016_1_dipole12
CS016_2_dipole2  = [Station('CS016_2.dipole2')]
CS016_2_dipole6  = [Station('CS016_2.dipole6')]
CS016_2_dipole10 = [Station('CS016_2.dipole10')]
CS016_2_dipole14 = [Station('CS016_2.dipole14')]
CS016_2_4dipoles = CS016_2_dipole2 + CS016_2_dipole6 + CS016_2_dipole10 + CS016_2_dipole14
CS016_3_HBA0 = [Station('CS016_3.HBA0')]
CS016_3_HBA1 = [Station('CS016_3.HBA1')]
CS016_3_HBA2 = [Station('CS016_3.HBA2')]
CS016_3_HBA3 = [Station('CS016_3.HBA3')]
CS016_3_HBAs = CS016_3_HBA0 + CS016_3_HBA1 + CS016_3_HBA2 + CS016_3_HBA3
CS016_4_us0  = [Station('CS016_4.us0')]
CS016_4_us1  = [Station('CS016_4.us1')]
CS016_4_us2  = [Station('CS016_4.us2')]
CS016_4_us3  = [Station('CS016_4.us3')]
CS016_4_4us  = CS016_4_us0 + CS016_4_us1 + CS016_4_us2 + CS016_4_us3

CS030_1_dipole0  = [Station('CS030_1.dipole0')]
CS030_1_dipole7  = [Station('CS030_1.dipole7')]
CS030_1_dipole10 = [Station('CS030_1.dipole10')]
CS030_1_dipole14 = [Station('CS030_1.dipole14')]
CS030_1_4dipoles = CS030_1_dipole0 + CS030_1_dipole7 + CS030_1_dipole10 + CS030_1_dipole14

CS031_1_dipole16 = [Station('CS031_1.dipole16')]
CS031_1_dipole22 = [Station('CS031_1.dipole22')]
CS031_1_dipole24 = [Station('CS031_1.dipole24')]
CS031_1_dipole31 = [Station('CS031_1.dipole31')]
CS031_1_4dipoles = CS031_1_dipole16 + CS031_1_dipole22 + CS031_1_dipole24 + CS031_1_dipole31

CS032_1_dipole34 = [Station('CS032_1.dipole34')]
CS032_1_dipole37 = [Station('CS032_1.dipole37')]
CS032_1_dipole42 = [Station('CS032_1.dipole42')]
CS032_1_dipole46 = [Station('CS032_1.dipole46')]
CS032_1_4dipoles = CS032_1_dipole34 + CS032_1_dipole37 + CS032_1_dipole42 + CS032_1_dipole46

CS001T_1_dipole0  = [Station('CS001T_1.dipole0')]
CS001T_1_dipole4  = [Station('CS001T_1.dipole4')]
CS001T_1_dipole8  = [Station('CS001T_1.dipole8')]
CS001T_1_dipole12 = [Station('CS001T_1.dipole12')]
CS001T_1_4dipoles = CS001T_1_dipole0 + CS001T_1_dipole4 + CS001T_1_dipole8 + CS001T_1_dipole12

CS010T_1_dipole0  = [Station('CS010T_1.dipole0')]
CS010T_1_dipole4  = [Station('CS010T_1.dipole4')]
CS010T_1_dipole8  = [Station('CS010T_1.dipole8')]
CS010T_1_dipole12 = [Station('CS010T_1.dipole12')]
CS010T_1_4dipoles = CS010T_1_dipole0 + CS010T_1_dipole4 + CS010T_1_dipole8 + CS010T_1_dipole12

Effelsberg_1_0 = [Station('Effelsberg_1.0')]
Effelsberg_1_1 = [Station('Effelsberg_1.1')]
Effelsberg_1_2 = [Station('Effelsberg_1.2')]
Effelsberg_1_3 = [Station('Effelsberg_1.3')]
Effelsberg_1 = Effelsberg_1_0 + Effelsberg_1_1 + Effelsberg_1_2 + Effelsberg_1_3

I001_us0 = [Station('I001_us0')]

B06_0 = [Station('B06_1.0')]
B06_1 = [Station('B06_1')]
B06_2 = [Station('B06_2')]
B06_3 = [Station('B06_3')]
B06 = B06_0 + B06_1 + B06_2 + B06_3

T0_0 = [Station('T0_0')]
T0_1 = [Station('T0_1')]
T0_2 = [Station('T0_2')]
T0_3 = [Station('T0_3')]
T0_4 = [Station('T0_4')]
T0_5 = [Station('T0_5')]
T0_6 = [Station('T0_6')]
T0_7 = [Station('T0_7')]
T0_8 = [Station('T0_8')]
T0_9 = [Station('T0_9')]
T0_10 = [Station('T0_10')]
T0_11 = [Station('T0_11')]
T0_12 = [Station('T0_12')]
T0_13 = [Station('T0_13')]
T0_14 = [Station('T0_14')]
T0_15 = [Station('T0_15')]
T0 = T0_0 + T0_1 + T0_2 + T0_3 + T0_4 + T0_5 + T0_6 + T0_7 + T0_8 + T0_9 + T0_10 + T0_11 + T0_12 + T0_13 + T0_14 + T0_15

I_43 = [Station('I_43')]
I_91 = [Station('I_91')]

slist            = CS010_5_TILES + CS001_3_HBAs + CS008_3_HBAs + CS016_3_HBAs
AllMicroStations = CS010_4_4us + CS001_4_4us + CS008_4_4us + CS016_4_4us
AllDipoles       = CS010_1_4dipoles + CS001_1_4dipoles + CS008_1_4dipoles + CS016_1_4dipoles
AllDipolesMixed  = CS010_1_4dipoles + CS001_2_4dipoles + CS008_2_4dipoles + CS016_2_4dipoles
AllHBAs          = CS010_5_TILES + CS001_3_HBAs + CS008_3_HBAs + CS016_3_HBAs
Mixed            = CS010_4_us0 + CS010_1_dipole4 + CS010_1_dipole8 + CS010_1_dipole12 + CS001_1_4dipoles + CS008_1_4dipoles + CS016_1_4dipoles
All24Dipoles     = CS001_1_4dipoles + CS008_1_4dipoles + CS016_1_4dipoles + CS030_1_4dipoles + CS031_1_4dipoles + CS032_1_4dipoles
All23Dipoles     = CS001_1_4dipoles + CS008_1_4dipoles + CS016_4_us0 + CS016_4_us1 + CS016_4_us2 + CS030_1_4dipoles + CS031_1_4dipoles + CS032_1_4dipoles
Generator        = CS010_3_HBA0 + CS010_3_HBA2 + CS010_3_HBA3 + CS008_3_HBA0 + CS008_3_HBA2 + CS008_3_HBA3 + CS001_3_HBA0 + CS001_3_HBA2 + CS001_3_HBA3 + CS016_3_HBA0 + CS016_3_HBA2 + CS016_3_HBA3
Test             = CS001_1_4dipoles + CS008_1_4dipoles + CS016_1_4dipoles
#Test             = CS001T_1_4dipoles + CS010_1_4dipoles + CS001_1_4dipoles + CS008_1_4dipoles + CS016_1_dipole0 + CS016_1_dipole4 + CS001_1_dipole8
Everything       = All23Dipoles + B06_0 + B06_2
