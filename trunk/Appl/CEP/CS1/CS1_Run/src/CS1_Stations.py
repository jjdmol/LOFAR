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

CS10_dipole0  = [Station('CS10_dipole0' ,  1, (0.119881183707,  0.920274058886, 6364096.65549 ), 0)]
CS10_dipole4  = [Station('CS10_dipole4' ,  1, (0.119888715288,  0.920277077416, 6364096.63359 ), 1)]
CS10_dipole8  = [Station('CS10_dipole8' ,  2, (0.119879699401,  0.920273668267, 6364096.7055  ), 2)]
CS10_dipole12 = [Station('CS10_dipole12',  3, (0.119885492973,  0.92027554014 , 6364096.62412 ), 3)]
CS10_4dipoles = CS10_dipole0 + CS10_dipole4 + CS10_dipole8 + CS10_dipole12

CS08_dipole0  = [Station('CS08_dipole0' ,  4, (0.119868257653,  0.920301867601, 6364095.96819 ), 0)]
CS08_dipole4  = [Station('CS08_dipole4' ,  4, (0.119874875505,  0.920305714116, 6364095.8893  ), 1)]
CS08_dipole8  = [Station('CS08_dipole8' ,  5, (0.119897959763,  0.92029772671 , 6364096.05311 ), 2)]
CS08_dipole12 = [Station('CS08_dipole12',  6, (0.119891341863,  0.920293880294, 6364096.13299 ), 3)]
CS08_4dipoles = CS08_dipole0 + CS08_dipole4 + CS08_dipole8 + CS08_dipole12

CS01_dipole0  = [Station('CS01_dipole0' ,  7, (0.11985110257 ,  0.920247392852, 6364097.49438 ), 0)]
CS01_dipole4  = [Station('CS01_dipole4' ,  7, (0.119857719888,  0.920251239615, 6364097.81449 ), 1)]
CS01_dipole8  = [Station('CS01_dipole8' ,  8, (0.119864071239,  0.920247231952, 6364097.89768 ), 2)]
CS01_dipole12 = [Station('CS01_dipole12',  9, (0.11985745392 ,  0.920243385441, 6364097.97757 ), 3)]
CS01_4dipoles = CS01_dipole0 + CS01_dipole4 + CS01_dipole8 + CS01_dipole12

CS16_dipole0  = [Station('CS16_dipole0' , 10, (0.119958428004,  0.920274972606, 6364096.62375 ), 0)]
CS16_dipole4  = [Station('CS16_dipole4' , 10, (0.119965046099,  0.920278818828, 6364096.54488 ), 1)]
CS16_dipole8  = [Station('CS16_dipole8' , 11, (0.11997139711 ,  0.920274810823, 6364096.62707 ), 2)]
CS16_dipole12 = [Station('CS16_dipole12', 12, (0.119964779015,  0.920270965005, 6364097.40695 ), 3)]
CS16_4dipoles = CS16_dipole0 + CS16_dipole4 + CS16_dipole8 + CS16_dipole12

Fourstations_4dipoles = CS10_4dipoles + CS01_4dipoles + CS08_4dipoles + CS16_4dipoles
