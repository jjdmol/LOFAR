# inputnode macadresses
inputMACs = [
    '00:E0:81:34:93:3B',
    '00:E0:81:31:B0:E1',
    '00:E0:81:31:B2:F5',
    '00:E0:81:31:DE:8B',
    '00:E0:81:29:20:37',
    '00:E0:81:31:DE:9B',
    '00:E0:81:34:8C:D1',
    '00:E0:81:34:9B:87',
    '00:E0:81:31:B3:27',
    '00:E0:81:31:DF:D5',
    '00:E0:81:34:92:11',
    '00:E0:81:31:B2:15'
    ]

# todo: this doesn't work yet with multiple rspboards per station
class Station(object):
    """
    Represents a real or virtual station. Position is a tuple of three floats
    representing ITRF x,y,z coordinates. The RSPBoardNumber is used to determine
    the source mac address.
    """
    def __init__(self, name, inputNode, position, RSPBoardNumber):
        self.name = name
        self.inputNode = inputNode
        self.position = position
        stationNumber = int(name.split('_')[0].split('CS')[-1])
        self.mac = '10:fa:00:%02x:01:%02x' % (stationNumber, RSPBoardNumber)
    def getName(self):
        return self.name
    def getInputNode(self):
        return self.inputNode
    def getPosition(self):
        return self.position
    def getSrcMAC(self):
        return self.mac
    def getDstMAC(self):
        return inputMACs[self.inputNode - 1]

#define the stations as a list so we can easily add them
#Keep the antenna positions in ITRF coordinates.
#Storing a position on earth given as (lon,lat,height)

CS10_dipole0  = [Station('CS10_dipole0' ,  1, (0.119879729,  0.920274996, 6364097.32  ), 0)]
CS10_dipole4  = [Station('CS10_dipole4' ,  1, (0.119887278,  0.920278013, 6364097.26  ), 1)]
CS10_dipole8  = [Station('CS10_dipole8' ,  2, (0.119878255,  0.92027461 , 6364097.33  ), 2)]
CS10_dipole12 = [Station('CS10_dipole12',  3, (0.119884052,  0.920276475, 6364097.29  ), 3)]
CS10_4dipoles = CS10_dipole0 + CS10_dipole4 + CS10_dipole8 + CS10_dipole12

CS08_dipole0  = [Station('CS08_dipole0' ,  4, (0.119868258 , 0.920301868, 6364096.77  ), 0)]
CS08_dipole4  = [Station('CS08_dipole4' ,  4, (0.119874876 , 0.920305715, 6364096.69  ), 1)]
CS08_dipole8  = [Station('CS08_dipole8' ,  5, (0.11989796  , 0.920297727, 6364096.85  ), 2)]
CS08_dipole12 = [Station('CS08_dipole12',  6, (0.119891342 , 0.920293881, 6364096.93  ), 3)]
CS08_4dipoles = CS08_dipole0 + CS08_dipole4 + CS08_dipole8 + CS08_dipole12

CS01_dipole0  = [Station('CS01_dipole0' ,  7, (0.119851103 , 0.920247393, 6364097.89  ), 0)]
CS01_dipole4  = [Station('CS01_dipole4' ,  7, (0.11985772  , 0.92025124 , 6364097.81  ), 1)]
CS01_dipole8  = [Station('CS01_dipole8' ,  8, (0.119864071 , 0.920247232, 6364097.9   ), 2)]
CS01_dipole12 = [Station('CS01_dipole12',  9, (0.119857454 , 0.920243385, 6364097.98  ), 3)]
CS01_4dipoles = CS01_dipole0 + CS01_dipole4 + CS01_dipole8 + CS01_dipole12

CS16_dipole0  = [Station('CS16_dipole0' , 10, (0.119958428 , 0.920274973, 6364097.32  ), 0)]
CS16_dipole4  = [Station('CS16_dipole4' , 10, (0.119965046 , 0.920278819, 6364097.24  ), 1)]
CS16_dipole8  = [Station('CS16_dipole8' , 11, (0.119971397 , 0.920274811, 6364097.33  ), 2)]
CS16_dipole12 = [Station('CS16_dipole12', 12, (0.119964779 , 0.920270965, 6364097.41  ), 3)]
CS16_4dipoles = CS16_dipole0 + CS16_dipole4 + CS16_dipole8 + CS16_dipole12

Fourstations_4dipoles = CS10_4dipoles + CS01_4dipoles + CS08_4dipoles + CS16_4dipoles
