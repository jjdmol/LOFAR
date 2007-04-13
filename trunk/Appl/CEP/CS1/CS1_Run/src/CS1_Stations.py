# inputnode macadresses
inputMACs = [
    '00:E0:81:31:B3:EB',
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

destIPs = [
    '10.170.0.1',
    '10.170.0.2',
    '10.170.0.3',
    '10.170.0.4',
    '10.170.0.5',
    '10.170.0.6',
    '10.170.0.7',
    '10.170.0.8',
    '10.170.0.9',
    '10.170.0.10',
    '10.170.0.11',
    '10.170.0.12'
    ]
    
# todo: this doesn't work yet with multiple rspboards per station
class Station(object):
    """
    Represents a real or virtual station. Position is a tuple of three floats
    representing ITRF x,y,z coordinates.
    """
    def __init__(self, name, inputNode, dstPort, position, phase_centre, RSPBoardNumber, mac, ip):
        self.name = name
        self.inputNode = inputNode
        self.position = position
        self.phase_centre = phase_centre
        self.dstPort = dstPort
        self.srcIP = ip
        stationNumber = int(name.split('_')[0].split('CS')[-1])
	self.mac = mac
    def getName(self):
        return self.name
    def getInputNode(self):
        return self.inputNode
    def getPosition(self):
        return self.position
    def getPhaseCentre(self):
        return self.phase_centre
    def getSrcMAC(self):
        return self.mac
    def getDstMAC(self):
        return inputMACs[self.inputNode - 1]
    def getDstIP(self):
        return inputIPs[self.inputNode - 1]
    def getDstPort(self):
        return self.dstPort
    def getSrcIP(self):
        return self.srcIP

#define the stations as a list so we can easily add them
#Keep the antenna positions in ITRF coordinates.
#Storing a position on earth given as (lon,lat,height)

CS10_dipole0  = [Station('CS10_dipole0' ,  1, 4346, (0.119881248042,  0.920262545738, 6364621.18843 ), (0.119884530715, 0.920263520535, 6364621.19236), 0, '10:fa:00:0a:01:00', '10.10.0.1')]
CS10_dipole4  = [Station('CS10_dipole4' ,  1, 4347, (0.119888779622,  0.920265564336, 6364621.16629 ), (0.119884530715, 0.920263520535, 6364621.19236), 1, '10:fa:00:0a:01:01', '10.10.0.2')]
CS10_dipole8  = [Station('CS10_dipole8' ,  2, 4348, (0.119879764027,  0.920262155263, 6364621.23847 ), (0.119884530715, 0.920263520535, 6364621.19236), 2, '10:fa:00:0a:01:02', '10.10.0.3')]
CS10_dipole12 = [Station('CS10_dipole12',  3, 4349, (0.119885557551,  0.920264027099, 6364621.15694 ), (0.119884530715, 0.920263520535, 6364621.19236), 3, '10:fa:00:0a:01:03', '10.10.0.4')]

#CS10 HBA ant 16,17,18,19
#CS10_dipole0  = [Station('CS10_dipole0' ,  1, 4346, (0.119898420627 ,  0.920270974317, 6364619.80963 ), (0.119899436312, 0.920270333689, 6364619.82292), 0, '10:fa:00:0a:01:00', '10.10.0.1')]
#CS10_dipole4  = [Station('CS10_dipole4' ,  1, 4347, (0.119897522752 ,  0.92027154112 , 6364619.79786 ), (0.119899436312, 0.920270333689, 6364619.82292), 1, '10:fa:00:0a:01:01', '10.10.0.2')]
#CS10_dipole8  = [Station('CS10_dipole8' ,  2, 4348, (0.11989608528  ,  0.920272447907, 6364619.7791 ),  (0.119899436312, 0.920270333689, 6364619.82292), 2, '10:fa:00:0a:01:02', '10.10.0.3')]
#CS10_dipole12 = [Station('CS10_dipole12',  3, 4349, (0.119893749932 ,  0.920273921498, 6364619.74847 ), (0.119899436312, 0.920270333689, 6364619.82292), 3, '10:fa:00:0a:01:03', '10.10.0.4')]
CS10_4dipoles = CS10_dipole0 + CS10_dipole4 + CS10_dipole8 + CS10_dipole12

CS08_dipole0  = [Station('CS08_dipole0' ,  4, 4346, (0.119868321939,  0.920290354485, 6364620.49802 ), (0.119875714863, 0.920290262694, 6364620.51091), 0, '10:fa:00:08:01:00', '10.10.0.1')]
CS08_dipole4  = [Station('CS08_dipole4' ,  4, 4347, (0.119874940131,  0.920294201177, 6364620.43385 ), (0.119875714863, 0.920290262694, 6364620.51091), 1, '10:fa:00:08:01:01', '10.10.0.2')]
CS08_dipole8  = [Station('CS08_dipole8' ,  5, 4348, (0.11989802405 ,  0.920286213523, 6364620.68626 ), (0.119875714863, 0.920290262694, 6364620.51091), 2, '10:fa:00:08:01:02', '10.10.0.3')]
CS08_dipole12 = [Station('CS08_dipole12',  6, 4349, (0.119891406343,  0.920282367332, 6364620.78143 ), (0.119875714863, 0.920290262694, 6364620.51091), 3, '10:fa:00:08:01:03', '10.10.0.4')]
CS08_4dipoles = CS08_dipole0 + CS08_dipole4 + CS08_dipole8 + CS08_dipole12

CS01_dipole0  = [Station('CS01_dipole0' ,  7, 4346, (0.119851166807,  0.920235879575, 6364622.29932 ), (0.119858559246, 0.920235787778, 6364622.29921), 0, '10:fa:00:01:01:00', '10.10.0.1')]
CS01_dipole4  = [Station('CS01_dipole4' ,  7, 4347, (0.119857784514,  0.920239726254, 6364622.21414 ), (0.119858559246, 0.920235787778, 6364622.29921), 1, '10:fa:00:01:01:01', '10.10.0.2')]
CS01_dipole8  = [Station('CS01_dipole8' ,  8, 4348, (0.119864135573,  0.92023571831 , 6364622.27264 ), (0.119858559246, 0.920235787778, 6364622.29921), 2, '10:fa:00:01:01:02', '10.10.0.3')]
CS01_dipole12 = [Station('CS01_dipole12',  9, 4349, (0.119857518351,  0.920231872127, 6364622.37981 ), (0.119858559246, 0.920235787778, 6364622.29921), 3, '10:fa:00:01:01:03', '10.10.0.4')]
CS01_4dipoles = CS01_dipole0 + CS01_dipole4 + CS01_dipole8 + CS01_dipole12

CS16_dipole0  = [Station('CS16_dipole0' , 10, 4346, (0.119958492436,  0.920263459402, 6364621.32162 ), (0.119965884875, 0.920263367124, 6364621.33052), 0, '10:fa:00:10:01:00', '10.10.0.1')]
CS16_dipole4  = [Station('CS16_dipole4' , 10, 4347, (0.119965110628,  0.920267305601, 6364621.24345 ), (0.119965884875, 0.920263367124, 6364621.33052), 1, '10:fa:00:10:01:01', '10.10.0.2')]
CS16_dipole8  = [Station('CS16_dipole8' , 11, 4348, (0.119971461687,  0.920263297671, 6364621.33295 ), (0.119965884875, 0.920263367124, 6364621.33052), 2, '10:fa:00:10:01:02', '10.10.0.3')]
CS16_dipole12 = [Station('CS16_dipole12', 12, 4349, (0.119964843495,  0.920259451478, 6364621.42212 ), (0.119965884875, 0.920263367124, 6364621.33052), 3, '10:fa:00:10:01:03', '10.10.0.4')]
CS16_4dipoles = CS16_dipole0 + CS16_dipole4 + CS16_dipole8 + CS16_dipole12

Fourstations_4dipoles = CS10_4dipoles + CS01_4dipoles + CS08_4dipoles + CS16_4dipoles

#CS10_us0  = [Station('CS10_us0' ,  1, 4346, (0.119880751593, 0.920263316053, 6364621.18657), (0.119884530715, 0.920263520535, 6364621.19236), 0, '10:fa:00:0a:01:00', '10.10.0.1')]
# For CS10_us0 (48 dipoles) the physical position and phase centre are equal
CS10_us0  = [Station('CS10_us0' ,  1, 4346, (0.119884530715, 0.920263520535, 6364621.19236), (0.119884530715, 0.920263520535, 6364621.19236), 0, '10:fa:00:0a:01:00', '10.10.0.1')]
CS10_us1  = [Station('CS10_us1' ,  1, 4347, (0.11988976137 , 0.920265828067, 6364621.16087), (0.119884530715, 0.920263520535, 6364621.19236), 1, '10:fa:00:0a:01:01', '10.10.0.2')]
CS10_us2  = [Station('CS10_us2' ,  2, 4348, (0.119878900574, 0.920261296557, 6364621.25414), (0.119884530715, 0.920263520535, 6364621.19236), 2, '10:fa:00:0a:01:02', '10.10.0.3')]
CS10_us3 =  [Station('CS10_us3' ,  3, 4349, (0.119884742094, 0.920264493858, 6364621.16333), (0.119884530715, 0.920263520535, 6364621.19236), 3, '10:fa:00:0a:01:03', '10.10.0.4')]
CS10_4us = CS10_us0 + CS10_us1 + CS10_us2 + CS10_us3

CS08_us0  = [Station('CS08_us0' ,  4, 4346, (0.119869229995, 0.920290343316, 6364620.50225), (0.119875714863, 0.920290262694, 6364620.51091), 0, '10:fa:00:08:01:00', '10.10.0.1')]
CS08_us1  = [Station('CS08_us1' ,  4, 4347, (0.119875847702, 0.920294190008, 6364620.43708), (0.119875714863, 0.920290262694, 6364620.51091), 1, '10:fa:00:08:01:01', '10.10.0.2')]
CS08_us2  = [Station('CS08_us2' ,  5, 4348, (0.119898932106, 0.920286202343, 6364620.66949), (0.119875714863, 0.920290262694, 6364620.51091), 2, '10:fa:00:08:01:02', '10.10.0.3')]
CS08_us3 =  [Station('CS08_us3' ,  6, 4349, (0.119892314399, 0.920282356152, 6364620.76366), (0.119875714863, 0.920290262694, 6364620.51091), 3, '10:fa:00:08:01:03', '10.10.0.4')]
CS08_4us = CS08_us0 + CS08_us1 + CS08_us2 + CS08_us3

CS01_us0  = [Station('CS01_us0' ,  7, 4346, (0.119852074863, 0.920235868407, 6364622.30655), (0.119858559246, 0.920235787778, 6364622.29921), 0, '10:fa:00:01:01:00', '10.10.0.1')]
CS01_us1  = [Station('CS01_us1' ,  7, 4347, (0.119858692085, 0.920239715084, 6364622.21637), (0.119858559246, 0.920235787778, 6364622.29921), 1, '10:fa:00:01:01:01', '10.10.0.2')]
CS01_us2  = [Station('CS01_us2' ,  8, 4348, (0.119865043629, 0.920235707148, 6364622.29187), (0.119858559246, 0.920235787778, 6364622.29921), 2, '10:fa:00:01:01:02', '10.10.0.3')]
CS01_us3 =  [Station('CS01_us3' ,  9, 4349, (0.119858426407, 0.920231860473, 6364622.38405), (0.119858559246, 0.920235787778, 6364622.29921), 3, '10:fa:00:01:01:03', '10.10.0.4')]
CS01_4us = CS01_us0 + CS01_us1 + CS01_us2 + CS01_us3

CS16_us0  = [Station('CS16_us0' , 10, 4346, (0.119959400492, 0.920263448231, 6364621.32185), (0.119965884875, 0.920263367124, 6364621.33052), 0, '10:fa:00:10:01:00', '10.10.0.1')]
CS16_us1  = [Station('CS16_us1' , 10, 4347, (0.119966018199, 0.92026729443 , 6364621.24368), (0.119965884875, 0.920263367124, 6364621.33052), 1, '10:fa:00:10:01:01', '10.10.0.2')]
CS16_us2  = [Station('CS16_us2' , 11, 4348, (0.119972369258, 0.9202632865  , 6364621.33318), (0.119965884875, 0.920263367124, 6364621.33052), 2, '10:fa:00:10:01:02', '10.10.0.3')]
CS16_us3 =  [Station('CS16_us3' , 12, 4349, (0.119965751066, 0.920259440307, 6364621.42235), (0.119965884875, 0.920263367124, 6364621.33052), 3, '10:fa:00:10:01:03', '10.10.0.4')]
CS16_4us = CS16_us0 + CS16_us1 + CS16_us2 + CS16_us3

AllMicroStations = CS10_4us + CS01_4us + CS08_4us + CS16_4us
AllDipoles = CS10_4dipoles + CS01_4dipoles + CS08_4dipoles + CS16_4dipoles
Mixed = CS10_us0 + CS10_dipole4 + CS10_dipole8 + CS10_dipole12 + CS01_4dipoles + CS08_4dipoles + CS16_4dipoles
Apr06 = CS01_4dipoles + CS08_4dipoles + CS16_4dipoles
