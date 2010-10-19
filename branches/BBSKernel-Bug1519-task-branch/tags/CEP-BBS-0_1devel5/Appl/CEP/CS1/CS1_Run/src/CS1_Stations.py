   
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

CS010_dipole0  = [Station('CS010_dipole0')]
CS010_dipole4  = [Station('CS010_dipole4')]
CS010_dipole8  = [Station('CS010_dipole8')]
CS010_dipole12 = [Station('CS010_dipole12')]
CS010_4dipoles_1 = CS010_dipole0 + CS010_dipole4 + CS010_dipole8 + CS010_dipole12

CS010_HBA0 = [Station('CS010_HBA0')]
CS010_HBA1 = [Station('CS010_HBA1')]
CS010_HBA2 = [Station('CS010_HBA2')]
CS010_HBA3 = [Station('CS010_HBA3')]
CS010_4HBAs = CS010_HBA0 + CS010_HBA1 + CS010_HBA2 + CS010_HBA3

CS010_us0  = [Station('CS010_us0')]
CS010_us1  = [Station('CS010_us1')]
CS010_us2  = [Station('CS010_us2')]
CS010_us3  = [Station('CS010_us3')]
CS010_4us  = CS010_us0 + CS010_us1 + CS010_us2 + CS010_us3

CS010_TILE1 = [Station('CS010_TILE1')]
CS010_TILE2 = [Station('CS010_TILE2')]
CS010_TILE3 = [Station('CS010_TILE3')]
CS010_TILE4 = [Station('CS010_TILE4')]
CS010_TILE5 = [Station('CS010_TILE5')]
CS010_TILE6 = [Station('CS010_TILE6')]
CS010_TILES = CS010_TILE5 + CS010_TILE2 + CS010_TILE3 + CS010_TILE4

CS008_dipole0  = [Station('CS008_dipole0')]
CS008_dipole4  = [Station('CS008_dipole4')]
CS008_dipole8  = [Station('CS008_dipole8')]
CS008_dipole12 = [Station('CS008_dipole12')]
CS008_4dipoles_1 = CS008_dipole0 + CS008_dipole4 + CS008_dipole8 + CS008_dipole12

CS008_dipole2  = [Station('CS008_dipole2')]
CS008_dipole6  = [Station('CS008_dipole6')]
CS008_dipole10 = [Station('CS008_dipole10')]
CS008_dipole14 = [Station('CS008_dipole14')]
CS008_4dipoles_2 = CS008_dipole2 + CS008_dipole6 + CS008_dipole10 + CS008_dipole14

CS008_HBA0 = [Station('CS008_HBA0')]
CS008_HBA1 = [Station('CS008_HBA1')]
CS008_HBA2 = [Station('CS008_HBA2')]
CS008_HBA3 = [Station('CS008_HBA3')]
CS008_4HBAs = CS008_HBA0 + CS008_HBA1 + CS008_HBA2 + CS008_HBA3

CS008_us0  = [Station('CS008_us0')]
CS008_us1  = [Station('CS008_us1')]
CS008_us2  = [Station('CS008_us2')]
CS008_us3  = [Station('CS008_us3')]
CS008_4us  = CS008_us0 + CS008_us1 + CS008_us2 + CS008_us3

CS001_dipole0  = [Station('CS001_dipole0')]
CS001_dipole4  = [Station('CS001_dipole4')]
CS001_dipole8  = [Station('CS001_dipole8')]
CS001_dipole12 = [Station('CS001_dipole12')]
CS001_4dipoles_1 = CS001_dipole0 + CS001_dipole4 + CS001_dipole8 + CS001_dipole12

CS001_dipole2  = [Station('CS001_dipole2')]
CS001_dipole6  = [Station('CS001_dipole6')]
CS001_dipole10 = [Station('CS001_dipole10')]
CS001_dipole14 = [Station('CS001_dipole14')]
CS001_4dipoles_2 = CS001_dipole2 + CS001_dipole6 + CS001_dipole10 + CS001_dipole14

CS001_HBA0 = [Station('CS001_HBA0')]
CS001_HBA1 = [Station('CS001_HBA1')]
CS001_HBA2 = [Station('CS001_HBA2')]
CS001_HBA3 = [Station('CS001_HBA3')]
CS001_4HBAs = CS001_HBA0 + CS001_HBA1 + CS001_HBA2 + CS001_HBA3

CS001_us0  = [Station('CS001_us0')]
CS001_us1  = [Station('CS001_us1')]
CS001_us2  = [Station('CS001_us2')]
CS001_us3  = [Station('CS001_us3')]
CS001_4us  = CS001_us0 + CS001_us1 + CS001_us2 + CS001_us3

CS016_dipole0  = [Station('CS016_dipole0')]
CS016_dipole4  = [Station('CS016_dipole4')]
CS016_dipole8  = [Station('CS016_dipole8')]
CS016_dipole12 = [Station('CS016_dipole12')]
CS016_4dipoles_1 = CS016_dipole0 + CS016_dipole4 + CS016_dipole8 + CS016_dipole12

CS016_dipole2  = [Station('CS016_dipole2')]
CS016_dipole6  = [Station('CS016_dipole6')]
CS016_dipole10 = [Station('CS016_dipole10')]
CS016_dipole14 = [Station('CS016_dipole14')]
CS016_4dipoles_2 = CS016_dipole2 + CS016_dipole6 + CS016_dipole10 + CS016_dipole14

CS016_HBA0 = [Station('CS016_HBA0')]
CS016_HBA1 = [Station('CS016_HBA1')]
CS016_HBA2 = [Station('CS016_HBA2')]
CS016_HBA3 = [Station('CS016_HBA3')]
CS016_4HBAs = CS016_HBA0 + CS016_HBA1 + CS016_HBA2 + CS016_HBA3

CS016_us0  = [Station('CS016_us0')]
CS016_us1  = [Station('CS016_us1')]
CS016_us2  = [Station('CS016_us2')]
CS016_us3  = [Station('CS016_us3')]
CS016_4us  = CS016_us0 + CS016_us1 + CS016_us2 + CS016_us3

CS030_dipole0  = [Station('CS030_dipole0')]
CS030_dipole7  = [Station('CS030_dipole7')]
CS030_dipole10 = [Station('CS030_dipole10')]
CS030_dipole14 = [Station('CS030_dipole14')]
CS030_4dipoles = CS030_dipole0 + CS030_dipole7 + CS030_dipole10 + CS030_dipole14

CS030_HBA_S02  = [Station('CS030_HBA_S02')]
CS030_HBA_S04  = [Station('CS030_HBA_S04')]
CS030_HBA_S10  = [Station('CS030_HBA_S10')]
CS030_HBA_S15  = [Station('CS030_HBA_S15')]
CS030_4HBAs     = CS030_HBA_S02 + CS030_HBA_S04 + CS030_HBA_S10 + CS030_HBA_S15

CS031_dipole16 = [Station('CS031_dipole16')]
CS031_dipole22 = [Station('CS031_dipole22')]
CS031_dipole24 = [Station('CS031_dipole24')]
CS031_dipole31 = [Station('CS031_dipole31')]
CS031_4dipoles = CS031_dipole16 + CS031_dipole22 + CS031_dipole24 + CS031_dipole31

CS031_HBA_S18 = [Station('CS031_HBA_S18')]
CS031_HBA_S24 = [Station('CS031_HBA_S24')]
CS031_HBA_S25 = [Station('CS031_HBA_S25')]
CS031_HBA_S27 = [Station('CS031_HBA_S27')]
CS031_4HBAs    = CS031_HBA_S18 + CS031_HBA_S24 + CS031_HBA_S25 + CS031_HBA_S27

CS032_dipole34 = [Station('CS032_dipole34')]
CS032_dipole37 = [Station('CS032_dipole37')]
CS032_dipole42 = [Station('CS032_dipole42')]
CS032_dipole46 = [Station('CS032_dipole46')]
CS032_4dipoles = CS032_dipole34 + CS032_dipole37 + CS032_dipole42 + CS032_dipole46

CS032_TILE1 = [Station('CS032_TILE1')]
CS032_TILE2 = [Station('CS032_TILE2')]
CS032_TILE3 = [Station('CS032_TILE3')]
CS032_TILE4 = [Station('CS032_TILE4')]
CS032_4Tiles = CS032_TILE1 + CS032_TILE2 + CS032_TILE3 + CS032_TILE4

CS001T_dipole0  = [Station('CS001T_dipole0')]
CS001T_dipole4  = [Station('CS001T_dipole4')]
CS001T_dipole8  = [Station('CS001T_dipole8')]
CS001T_dipole12 = [Station('CS001T_dipole12')]
CS001T_4dipoles = CS001T_dipole0 + CS001T_dipole4 + CS001T_dipole8 + CS001T_dipole12

CS010T_dipole0  = [Station('CS010T_dipole0')]
CS010T_dipole4  = [Station('CS010T_dipole4')]
CS010T_dipole8  = [Station('CS010T_dipole8')]
CS010T_dipole12 = [Station('CS010T_dipole12')]
CS010T_4dipoles = CS010T_dipole0 + CS010T_dipole4 + CS010T_dipole8 + CS010T_dipole12

Effelsberg_0 = [Station('Effelsberg_0')]
Effelsberg_1 = [Station('Effelsberg_1')]
Effelsberg_2 = [Station('Effelsberg_2')]
Effelsberg_3 = [Station('Effelsberg_3')]
Effelsberg = Effelsberg_0 + Effelsberg_1 + Effelsberg_2 + Effelsberg_3

I001_us0 = [Station('I001_us0')]

B00_0 = [Station('B00_0')]
B00_1 = [Station('B00_1')]
B01_0 = [Station('B01_0')]
B01_1 = [Station('B01_1')]
B02_0 = [Station('B02_0')]
B02_1 = [Station('B02_1')]
B03_0 = [Station('B03_0')]
B03_1 = [Station('B03_1')]
B00 = B00_0 + B00_1
B01 = B01_0 + B01_1
B02 = B02_0 + B02_1
B03 = B03_0 + B03_1

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

CS001  = [Station('CS001')]
CS008  = [Station('CS008')]
CS010  = [Station('CS010')]
CS016  = [Station('CS016')]

# CS0XX_4dipoles_1 =  CS0XX_dipole0 + CSXX_dipole4 + CS0XX_dipole8  + CS0XX_dipole12
# CS0XX_4dipoles_2 =  CS0XX_dipole2 + CSXX_dipole6 + CS0XX_dipole10 + CS0XX_dipole14

AllStations      = CS001 + CS010 + CS016
AllMicroStations = CS010_4us + CS001_4us + CS016_4us
AllDipoles       = CS010_4dipoles_1 + CS001_4dipoles_1 + CS008_4dipoles_1 + CS016_4dipoles_1
AllDipolesMixed  = CS010_4dipoles_1 + CS001_4dipoles_2 + CS008_4dipoles_2 + CS016_4dipoles_2
AllHBAs          = CS010_4HBAs + CS001_4HBAs + CS008_4HBAs + CS016_4HBAs
All24HBAs        = CS030_4HBAs + CS031_4HBAs + CS032_4Tiles + CS001_4HBAs + CS008_4HBAs + CS016_4HBAs
Mixed            = CS010_us0 + CS010_dipole4 + CS010_dipole8 + CS010_dipole12 + CS001_4dipoles_1 + CS008_4dipoles_1 + CS016_4dipoles_1
All24Dipoles     = CS001_4dipoles_1 + CS008_4dipoles_1 + CS016_4dipoles_1 + CS030_4dipoles + CS031_4dipoles + CS032_4dipoles
All23Dipoles     = CS001_4dipoles_1 + CS008_4dipoles_1 + CS016_us0 + CS016_us1 + CS016_us2 + CS030_4dipoles + CS031_4dipoles + CS032_4dipoles
Generator        = CS010_HBA0 + CS010_HBA2 + CS010_HBA3 + CS008_HBA0 + CS008_HBA2 + CS008_HBA3 + CS001_HBA0 + CS001_HBA2 + CS001_HBA3 + CS016_HBA0 + CS016_HBA2 + CS016_HBA3
Everything       = All23Dipoles
