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
        self.mac = '00:00:00:%02x:01:%02x' % (stationNumber, RSPBoardNumber)
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
CS10_dipole0  = [Station('CS10_dipole0' ,  1, (3826445.092 , 460923.319 , 5064346.196 ), 0)]
CS10_dipole4  = [Station('CS10_dipole4' ,  1, (3826426.4071, 460950.3711, 5064357.7736), 1)]
CS10_dipole8  = [Station('CS10_dipole8' ,  2, (3826447.720 , 460917.9109, 5064344.7126), 2)]
CS10_dipole12 = [Station('CS10_dipole12',  3, (3826435.6425, 460938.9631, 5064351.8736), 3)]
CS10_4dipoles = CS10_dipole0 + CS10_dipole4 + CS10_dipole8 + CS10_dipole12

CS08_dipole0  = [Station('CS08_dipole0' ,  4, (3826314.932 , 460863.1097, 5064449.3196), 0)]
CS08_dipole4  = [Station('CS08_dipole4' ,  4, (3826292.4937, 460886.0964, 5064464.0812), 1)]
CS08_dipole8  = [Station('CS08_dipole8' ,  5, (3826322.1133, 460979.2737, 5064433.4283), 2)]
CS08_dipole12 = [Station('CS08_dipole12',  6, (3826344.552 , 460956.2871, 5064418.6678), 3)]
CS08_4dipoles = CS08_dipole0 + CS08_dipole4 + CS08_dipole8 + CS08_dipole12

CS01_dipole0  = [Station('CS01_dipole0' ,  7, (3826597.415 , 460830.5359, 5064240.2635), 0)]
CS01_dipole4  = [Station('CS01_dipole4' ,  7, (3826574.9777, 460853.5227, 5064255.0255), 1)]
CS01_dipole8  = [Station('CS01_dipole8' ,  8, (3826592.2508, 460880.2595, 5064239.6452), 2)]
CS01_dipole12 = [Station('CS01_dipole12',  9, (3826614.688 , 460857.2727, 5064224.8833), 3)]
CS01_4dipoles = CS01_dipole0 + CS01_dipole4 + CS01_dipole8 + CS01_dipole12

CS16_dipole0  = [Station('CS16_dipole0' , 10, (3826408.922 , 461224.4683, 5064346.1072), 0)]
CS16_dipole4  = [Station('CS16_dipole4' , 10, (3826386.4833, 461247.4549, 5064360.8674), 1)]
CS16_dipole8  = [Station('CS16_dipole8' , 11, (3826403.7559, 461274.192 , 5064345.4863), 2)]
CS16_dipole12 = [Station('CS16_dipole12', 12, (3826426.195 , 461251.2051, 5064330.7263), 3)]
CS16_4dipoles = CS16_dipole0 + CS16_dipole4 + CS16_dipole8 + CS16_dipole12

Fourstations_4dipoles = CS10_4dipoles + CS01_4dipoles + CS08_4dipoles + CS16_4dipoles
    
