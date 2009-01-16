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



CS001_lba0  = [Station('CS001LBA_LBA0', '10.170.0.33', ['10.170.0.33:4346'])]
CS001_lba2  = [Station('CS001LBA_LBA2', '10.170.0.33', ['10.170.0.33:4346'])]
CS001_lba4  = [Station('CS001LBA_LBA4', '10.170.0.34', ['10.170.0.34:4347'])]
CS001_lba6  = [Station('CS001LBA_LBA6', '10.170.0.34', ['10.170.0.34:4347'])]
CS001_lba8  = [Station('CS001LBA_LBA8', '10.170.0.37', ['10.170.0.37:4348'])]
CS001_lba10 = [Station('CS001LBA_LBA10', '10.170.0.37', ['10.170.0.37:4348'])]
CS001_lba12 = [Station('CS001LBA_LBA12', '10.170.0.38', ['10.170.0.38:4349'])]
CS001_lba14 = [Station('CS001LBA_LBA14', '10.170.0.38', ['10.170.0.38:4349'])]
CS001_lba = CS001_lba0 + CS001_lba4 + CS001_lba8 + CS001_lba12
CS001_lba_rotated = CS001_lba2 + CS001_lba6 + CS001_lba10 + CS001_lba14

CS001_hba0 = [Station('CS001HBA_HBA0', '10.170.0.33', ['10.170.0.33:4346'])]
CS001_hba1 = [Station('CS001HBA_HBA1', '10.170.0.34', ['10.170.0.34:4347'])]
CS001_hba2 = [Station('CS001HBA_HBA2', '10.170.0.37', ['10.170.0.37:4348'])]
CS001_hba3 = [Station('CS001HBA_HBA3', '10.170.0.38', ['10.170.0.38:4349'])]
CS001_hba = CS001_hba0 + CS001_hba1 + CS001_hba2 + CS001_hba3

CS001 = [Station('CS001', '10.170.0.33', ['10.170.0.33:4346', '10.170.0.33:4347', '10.170.0.33:4348', '10.170.0.33:4349'])]


CS010_lba0  = [Station('CS010LBA_LBA0', '10.170.0.41', ['10.170.0.41:4346'])]
CS010_lba2  = [Station('CS010LBA_LBA2', '10.170.0.41', ['10.170.0.41:4346'])]
CS010_lba4  = [Station('CS010LBA_LBA4', '10.170.0.42', ['10.170.0.42:4351'])]
CS010_lba6  = [Station('CS010LBA_LBA6', '10.170.0.42', ['10.170.0.42:4351'])]
CS010_lba8  = [Station('CS010LBA_LBA8', '10.170.0.45', ['10.170.0.45:4352'])]
CS010_lba10 = [Station('CS010LBA_LBA10', '10.170.0.45', ['10.170.0.45:4352'])]
CS010_lba12 = [Station('CS010LBA_LBA12', '10.170.0.46', ['10.170.0.46:4353'])]
CS010_lba14 = [Station('CS010LBA_LBA14', '10.170.0.46', ['10.170.0.46:4353'])]
CS010_lba = CS010_lba0 + CS010_lba4 + CS010_lba8 + CS010_lba12
CS010_lba_rotated = CS010_lba


CS010_hba0 = [Station('CS010HBA_HBA0', '10.170.0.41', ['10.170.0.41:4346'])]
CS010_hba1 = [Station('CS010HBA_HBA1', '10.170.0.42', ['10.170.0.42:4351'])]
CS010_hba2 = [Station('CS010HBA_HBA2', '10.170.0.45', ['10.170.0.45:4352'])]
CS010_hba3 = [Station('CS010HBA_HBA3', '10.170.0.46', ['10.170.0.46:4353'])]
CS010_hba = CS010_hba0 + CS010_hba1 + CS010_hba2 + CS010_hba3

CS010_TILE1 = [Station('CS010HBA_TILE1', '10.170.0.41', ['10.170.0.41:4346'])]
CS010_TILE2 = [Station('CS010HBA_TILE2', '10.170.0.42', ['10.170.0.42:4351'])]
CS010_TILE5 = [Station('CS010HBA_TILE5', '10.170.0.45', ['10.170.0.45:4352'])]
CS010_TILE6 = [Station('CS010HBA_TILE6', '10.170.0.46', ['10.170.0.46:4353'])]
CS010_tiles = CS010_TILE1 + CS010_TILE2 + CS010_TILE5 + CS010_TILE6

CS010     = [Station('CS010', '10.170.0.41', ['10.170.0.41:4346', '10.170.0.41:4351', '10.170.0.41:4352', '10.170.0.41:4353'])]


CS016_lba0  = [Station('CS016LBA_LBA0', '10.170.0.49', ['10.170.0.49:4346'])]
CS016_lba2  = [Station('CS016LBA_LBA2', '10.170.0.49', ['10.170.0.49:4346'])]
CS016_lba4  = [Station('CS016LBA_LBA4', '10.170.0.50', ['10.170.0.50:4347'])]
CS016_lba6  = [Station('CS016LBA_LBA6', '10.170.0.50', ['10.170.0.50:4347'])]
CS016_lba8  = [Station('CS016LBA_LBA8', '10.170.0.53', ['10.170.0.53:4348'])]
CS016_lba10 = [Station('CS016LBA_LBA10', '10.170.0.53', ['10.170.0.53:4348'])]
CS016_lba12 = [Station('CS016LBA_LBA12', '10.170.0.54', ['10.170.0.54:4349'])]
CS016_lba14 = [Station('CS016LBA_LBA14', '10.170.0.54', ['10.170.0.54:4349'])]
CS016_lba = CS016_lba0 + CS016_lba4 + CS016_lba8 + CS016_lba12
CS016_lba_rotated = CS016_lba2 + CS016_lba6 + CS016_lba10 + CS016_lba14

CS016_hba0 = [Station('CS016_HBA0', '10.170.0.49', ['10.170.0.49:4346'])]
CS016_hba1 = [Station('CS016_HBA1', '10.170.0.50', ['10.170.0.50:4347'])]
CS016_hba2 = [Station('CS016_HBA2', '10.170.0.53', ['10.170.0.53:4348'])]
CS016_hba3 = [Station('CS016_HBA3', '10.170.0.54', ['10.170.0.54:4349'])]
CS016_hba = CS016_hba0 + CS016_hba1 + CS016_hba2 + CS016_hba3

CS016 = [Station('CS016', '10.170.0.49', ['10.170.0.49:4346', '10.170.0.49:4347', '10.170.0.49:4348', '10.170.0.49:4349'])]


#
# Experimental part, not edited by Michiel Brentjens:
#

for RSP in range(1, 5):
  for IP in [1,2,5,6,9,10,13,14,17,18,21,22,25,26,29,30,33,34,37,38,41,42,45,46,49,50,53,54,57,58,61,62]:
    inputs = ['10.170.0.' + str(IP) + ':' + str(port) for port in range(4346, 4346 + RSP)]

    #if IP != 62:
      #inputs = RSP * ['null:']

    exec 'S' + str(IP) + '_' + str(RSP) + '=[Station(\'S' + str(IP) + '\',\'10.170.0.' + str(IP) + '\',' + str(inputs) + ')]'

  for block in range(0, 16):
    exec 'B%02u_%u=S%u_%u+S%u_%u' % (block, RSP, 4*block+1, RSP, 4*block+2, RSP)

  for block in range(0, 8):
    exec 'R000_64_%u_%u=B%02u_%u+B%02u_%u' % (block, RSP, 2*block, RSP, 2*block+1, RSP)

  for block in range(0, 4):
    exec 'R000_128_%u_%u=R000_64_%u_%u+R000_64_%u_%u' % (block, RSP, 2*block, RSP, 2*block+1, RSP)

  for block in range(0, 2):
    exec 'R000_256_%u_%u=R000_128_%u_%u+R000_128_%u_%u' % (block, RSP, 2*block, RSP, 2*block+1, RSP)

  exec 'R00_M0_ION_%u=R000_256_0_%u+R000_256_1_%u' % (RSP, RSP, RSP)

Pulsar = [Station('Pulsar', '10.170.0.62', ['tcp:10.170.0.62:4346'])]

#
#   End of experimental part
#


# Standard configurations:

LBA              = CS001_lba + CS010_lba + CS016_lba
LBAROTATED       = CS001_lba_rotated + CS010_lba_rotated + CS016_lba_rotated
HBA              = CS001_hba + CS010_hba + CS016_hba
HBATILES         = CS001_hba + CS010_tiles + CS016_hba
HBATILESONLY     = CS010_tiles

# Space for experimental, one-of configurations:


#End of File
