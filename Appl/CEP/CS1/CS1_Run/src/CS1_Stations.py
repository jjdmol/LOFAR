   
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

CS010_HBA10 = [Station('CS010_HBA10')]
CS010_HBA17 = [Station('CS010_HBA17')]
CS010_HBA25 = [Station('CS010_HBA25')]
CS010_HBA29 = [Station('CS010_HBA29')]
CS010_HBA = CS010_HBA10 + CS010_HBA17 + CS010_HBA25 + CS010_HBA29

CS010_dipole0  = [Station('CS010_dipole0')]
CS010_dipole4  = [Station('CS010_dipole4')]
CS010_dipole8  = [Station('CS010_dipole8')]
CS010_dipole12 = [Station('CS010_dipole12')]
CS010_4dipoles = CS010_dipole0 + CS010_dipole4 + CS010_dipole8 + CS010_dipole12

CS008_dipole0  = [Station('CS008_dipole0')]
CS008_dipole4  = [Station('CS008_dipole4')]
CS008_dipole8  = [Station('CS008_dipole8')]
CS008_dipole12 = [Station('CS008_dipole12')]
CS008_4dipoles = CS008_dipole0 + CS008_dipole4 + CS008_dipole8 + CS008_dipole12

CS001_dipole0  = [Station('CS001_dipole0')]
CS001_dipole4  = [Station('CS001_dipole4')]
CS001_dipole8  = [Station('CS001_dipole8')]
CS001_dipole12 = [Station('CS001_dipole12')]
CS001_4dipoles = CS001_dipole0 + CS001_dipole4 + CS001_dipole8 + CS001_dipole12

CS016_dipole0  = [Station('CS016_dipole0')]
CS016_dipole4  = [Station('CS016_dipole4')]
CS016_dipole8  = [Station('CS016_dipole8')]
CS016_dipole12 = [Station('CS016_dipole12')]
CS016_4dipoles = CS016_dipole0 + CS016_dipole4 + CS016_dipole8 + CS016_dipole12

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
AllDipoles = CS010_4dipoles + CS001_4dipoles + CS008_4dipoles + CS016_4dipoles
Mixed = CS010_us0 + CS010_dipole4 + CS010_dipole8 + CS010_dipole12 + CS001_4dipoles + CS008_4dipoles + CS016_4dipoles
