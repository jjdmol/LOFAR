from Host_Names import IONodes

class Station(object):
    """
    Represents a real or virtual station. Position is a tuple of three floats
    representing ITRF x,y,z coordinates.
    """
    def __init__(self, name, ionode, inputs):
        self.name	= name
        self.ionode	= ionode
        self.inputs	= inputs

    def getName(self):
        return self.name

    def getPset(self, partition):
	return IONodes.get(partition).index(self.ionode)


CS001_us0 = [Station('CS001_us0', '10.170.0.33', ['10.170.0.33:4346'])]
CS001_us1 = [Station('CS001_us1', '10.170.0.34', ['10.170.0.34:4347'])]
CS001_us2 = [Station('CS001_us2', '10.170.0.37', ['10.170.0.37:4348'])]
CS001_us3 = [Station('CS001_us3', '10.170.0.38', ['10.170.0.38:4349'])]
CS001_4us = CS001_us0 + CS001_us1 + CS001_us2 + CS001_us3

CS001_dipole0 = [Station('CS001_dipole0', '10.170.0.33', ['10.170.0.33:4346'])]
CS001_dipole4 = [Station('CS001_dipole4', '10.170.0.34', ['10.170.0.34:4347'])]
CS001_dipole8 = [Station('CS001_dipole8', '10.170.0.37', ['10.170.0.37:4348'])]
CS001_dipole12 = [Station('CS001_dipole12', '10.170.0.38', ['10.170.0.38:4349'])]
CS001_4dipoles = CS001_dipole0 + CS001_dipole4 + CS001_dipole8 + CS001_dipole12

CS001_HBA0 = [Station('CS001_HBA0', '10.170.0.33', ['10.170.0.33:4346'])]
CS001_HBA1 = [Station('CS001_HBA1', '10.170.0.34', ['10.170.0.34:4347'])]
CS001_HBA2 = [Station('CS001_HBA2', '10.170.0.37', ['10.170.0.37:4348'])]
CS001_HBA3 = [Station('CS001_HBA3', '10.170.0.38', ['10.170.0.38:4349'])]
CS001_4HBAs = CS001_HBA0 + CS001_HBA1 + CS001_HBA2 + CS001_HBA3

CS001 = [Station('CS001', '10.170.0.33', ['10.170.0.33:4346', '10.170.0.33:4347', '10.170.0.33:4348', '10.170.0.33:4349'])]

CS010_us0 = [Station('CS010_us0', '10.170.0.41', ['10.170.0.41:4346'])]
CS010_us1 = [Station('CS010_us1', '10.170.0.42', ['10.170.0.42:4351'])]
CS010_us2 = [Station('CS010_us2', '10.170.0.45', ['10.170.0.45:4352'])]
CS010_us3 = [Station('CS010_us3', '10.170.0.46', ['10.170.0.46:4353'])]
CS010_4us = CS010_us0 + CS010_us1 + CS010_us2 + CS010_us3

CS010_dipole0 = [Station('CS010_dipole0', '10.170.0.41', ['10.170.0.41:4346'])]
CS010_dipole4 = [Station('CS010_dipole4', '10.170.0.42', ['10.170.0.42:4351'])]
CS010_dipole8 = [Station('CS010_dipole8', '10.170.0.45', ['10.170.0.45:4352'])]
CS010_dipole12 = [Station('CS010_dipole12', '10.170.0.46', ['10.170.0.46:4353'])]
CS010_4dipoles = CS010_dipole0 + CS010_dipole4 + CS010_dipole8 + CS010_dipole12

CS010_HBA0 = [Station('CS010_HBA0', '10.170.0.41', ['10.170.0.41:4346'])]
CS010_HBA1 = [Station('CS010_HBA1', '10.170.0.42', ['10.170.0.42:4351'])]
CS010_HBA2 = [Station('CS010_HBA2', '10.170.0.45', ['10.170.0.45:4352'])]
CS010_HBA3 = [Station('CS010_HBA3', '10.170.0.46', ['10.170.0.46:4353'])]
CS010_4HBAs = CS010_HBA0 + CS010_HBA1 + CS010_HBA2 + CS010_HBA3

CS010_TILE1 = [Station('CS010_TILE1', '10.170.0.41', ['10.170.0.41:4346'])]
CS010_TILE2 = [Station('CS010_TILE2', '10.170.0.42', ['10.170.0.42:4351'])]
CS010_TILE5 = [Station('CS010_TILE5', '10.170.0.45', ['10.170.0.45:4352'])]
CS010_TILE6 = [Station('CS010_TILE6', '10.170.0.46', ['10.170.0.46:4353'])]
CS010_4Tiles = CS010_TILE1 + CS010_TILE2 + CS010_TILE5 + CS010_TILE6

CS010     = [Station('CS010', '10.170.0.41', ['10.170.0.41:4346', '10.170.0.41:4351', '10.170.0.41:4352', '10.170.0.41:4353'])]

CS016_us0 = [Station('CS016_us0', '10.170.0.49', ['10.170.0.49:4346'])]
CS016_us1 = [Station('CS016_us1', '10.170.0.50', ['10.170.0.50:4347'])]
CS016_us2 = [Station('CS016_us2', '10.170.0.53', ['10.170.0.53:4348'])]
CS016_us3 = [Station('CS016_us3', '10.170.0.54', ['10.170.0.54:4349'])]
CS016_4us = CS016_us0 + CS016_us1 + CS016_us2 + CS016_us3

CS016_dipole0 = [Station('CS016_dipole0', '10.170.0.49', ['10.170.0.49:4346'])]
CS016_dipole4 = [Station('CS016_dipole4', '10.170.0.50', ['10.170.0.50:4347'])]
CS016_dipole8 = [Station('CS016_dipole8', '10.170.0.53', ['10.170.0.53:4348'])]
CS016_dipole12 = [Station('CS016_dipole12', '10.170.0.54', ['10.170.0.54:4349'])]
CS016_4dipoles = CS016_dipole0 + CS016_dipole4 + CS016_dipole8 + CS016_dipole12

CS016_HBA0 = [Station('CS016_HBA0', '10.170.0.49', ['10.170.0.49:4346'])]
CS016_HBA1 = [Station('CS016_HBA1', '10.170.0.50', ['10.170.0.50:4347'])]
CS016_HBA2 = [Station('CS016_HBA2', '10.170.0.53', ['10.170.0.53:4348'])]
CS016_HBA3 = [Station('CS016_HBA3', '10.170.0.54', ['10.170.0.54:4349'])]
CS016_4HBAs = CS016_HBA0 + CS016_HBA1 + CS016_HBA2 + CS016_HBA3

CS016 = [Station('CS016', '10.170.0.49', ['10.170.0.49:4346', '10.170.0.49:4347', '10.170.0.49:4348', '10.170.0.49:4349'])]

for RSP in range(1,5):
  for IP in [1,2,5,6,9,10,13,14]:
    inputs = ['10.170.0.' + str(IP) + ':' + str(port) for port in range(4346, 4346 + RSP)]
    exec 'S' + str(IP) + '_' + str(RSP) + '=[Station(\'S' + str(IP) + '\',\'10.170.0.' + str(IP) + '\',' + str(inputs) + ')]'
  exec 'B00_' + str(RSP) + '=S1_' + str(RSP) + '+S2_' + str(RSP)
  exec 'B01_' + str(RSP) + '=S5_' + str(RSP) + '+S6_' + str(RSP)
  exec 'B02_' + str(RSP) + '=S9_' + str(RSP) + '+S10_' + str(RSP)
  exec 'B03_' + str(RSP) + '=S13_' + str(RSP) + '+S14_' + str(RSP)



AllStations      = CS001 + CS010 + CS016
AllMicroStations = CS001_4us + CS010_4us + CS016_4us
AllDipoles       = CS010_4dipoles + CS016_4dipoles
AllHBAs          = CS010_4HBAs + CS001_4HBAs + CS016_4HBAs
Mixed            = CS010_4Tiles + CS001_4HBAs + CS016_4HBAs

CS001CS016Dipoles = CS001_4dipoles + CS016_4dipoles

