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

    def getIONode(self):
        return self.ionode


# Full station mode

RS106LBA  = [Station('RS106LBA',  '10.170.0.174', ['10.170.0.174:4346', '10.170.0.174:4347', '10.170.0.174:4348', '10.170.0.174:4349'])]
RS106HBA  = [Station('RS106HBA',  '10.170.0.174', ['10.170.0.174:4346', '10.170.0.174:4347', '10.170.0.174:4348', '10.170.0.174:4349'])]

RS208LBA  = [Station('RS208LBA',  '10.170.0.162', ['10.170.0.162:4346', '10.170.0.162:4347', '10.170.0.162:4348', '10.170.0.162:4349'])]
RS208HBA  = [Station('RS208HBA',  '10.170.0.162', ['10.170.0.162:4346', '10.170.0.162:4347', '10.170.0.162:4348', '10.170.0.162:4349'])]

CS302LBA  = [Station('CS302LBA',  '10.170.0.21', ['10.170.0.21:4346', '10.170.0.21:4347', '10.170.0.21:4348', '10.170.0.21:4349'])]
CS302HBA  = [Station('CS302HBA',  '10.170.0.21', ['10.170.0.21:4346', '10.170.0.21:4347', '10.170.0.21:4348', '10.170.0.21:4349'])]
CS302HBA0 = [Station('CS302HBA0', '10.170.0.21', ['10.170.0.21:4346', '10.170.0.21:4347', '10.170.0.21:4348', '10.170.0.21:4349'])]
CS302HBA1 = [Station('CS302HBA1', '10.170.0.22', ['10.170.0.22:4352', '10.170.0.22:4353', '10.170.0.22:4354', '10.170.0.22:4355'])]

RS307LBA  = [Station('RS307LBA',  '10.170.0.189', ['10.170.0.189:4346', '10.170.0.189:4347', '10.170.0.189:4348', '10.170.0.189:4349'])]
RS307HBA  = [Station('RS307HBA',  '10.170.0.189', ['10.170.0.189:4346', '10.170.0.189:4347', '10.170.0.189:4348', '10.170.0.189:4349'])]

RS503LBA  = [Station('RS503LBA',  '10.170.0.170', ['10.170.0.170:4346', '10.170.0.170:4347', '10.170.0.170:4348', '10.170.0.170:4349'])]
RS503HBA  = [Station('RS503HBA',  '10.170.0.170', ['10.170.0.170:4346', '10.170.0.170:4347', '10.170.0.170:4348', '10.170.0.170:4349'])]

DE601LBA  = [Station('DE601LBA',  '10.170.0.178', ['10.170.0.178:4358', '10.170.0.178:4359', '10.170.0.178:4363', '10.170.0.178:4364'])]

#
# Experimental part, not edited by Michiel Brentjens:
#

# S17_8: 8 full stations (4 RSP boards), starting from 10.170.0.17
# s9_2 : 2 microstations (1 RSP board ), starting from 10.170.0.9

for ip in [1,2,5,6,9,10,13,14,17,18,21,22,25,26,29,30,33,34,37,38,41,42,45,46,49,50,53,54,57,58,61,62,129,130,133,134,137,138,141,142,145,146,149,150,153,154,157,158,161,162,165,166,169,170,173,174,177,178,181,182,185,186,189,190]:
  inputs = ['10.170.0.' + str(ip) + ':4346']
  exec 's' + str(ip) + '_1=[Station(\'S' + str(ip) + '\', \'10.170.0.' + str(ip) + '\', ' + str(inputs) + ')]'
  inputs = ['10.170.0.' + str(ip) + ':' + str(port) for port in range(4346, 4350)]
  #inputs = ['raw:eth0:' + str(port) for port in range(4346, 4350)]
  exec 'S' + str(ip) + '_1=[Station(\'S' + str(ip) + '\', \'10.170.0.' + str(ip) + '\', ' + str(inputs) + ')]'

for ip in range(1,65,4) + range(129,193,4):
  exec 's' + str(ip) + '_2 = s' + str(ip) + '_1 + s' + str(ip + 1) + '_1'
  exec 'S' + str(ip) + '_2 = S' + str(ip) + '_1 + S' + str(ip + 1) + '_1'

for ip in range(1,65,8) + range(129,193,8):
  exec 's' + str(ip) + '_4 = s' + str(ip) + '_2 + s' + str(ip + 4) + '_2'
  exec 'S' + str(ip) + '_4 = S' + str(ip) + '_2 + S' + str(ip + 4) + '_2'

for ip in range(1,65,16) + range(129,193,16):
  exec 's' + str(ip) + '_8 = s' + str(ip) + '_4 + s' + str(ip + 8) + '_4'
  exec 'S' + str(ip) + '_8 = S' + str(ip) + '_4 + S' + str(ip + 8) + '_4'

for ip in range(1,65,32) + range(129,193,32):
  exec 's' + str(ip) + '_16 = s' + str(ip) + '_8 + s' + str(ip + 16) + '_8'
  exec 'S' + str(ip) + '_16 = S' + str(ip) + '_8 + S' + str(ip + 16) + '_8'

for ip in range(1,65,64) + range(129,193,64):
  exec 's' + str(ip) + '_32 = s' + str(ip) + '_16 + s' + str(ip + 32) + '_16'
  exec 'S' + str(ip) + '_32 = S' + str(ip) + '_16 + S' + str(ip + 32) + '_16'

for ip in [1]:
  exec 's' + str(ip) + '_64 = s' + str(ip) + '_32 + s' + str(ip + 128) + '_32'
  exec 'S' + str(ip) + '_64 = S' + str(ip) + '_32 + S' + str(ip + 128) + '_32'


Pulsar = [Station('Pulsar', '10.170.0.30', ['tcp:10.170.0.30:4346'])]

# full rack tests

R01=[]

for IP in [1,2,5,6,9,10,13,14,17,18,21,22,25,26,29,30,33,34,37,38,41,42,45,46,49,50,53,54,57,58,61,62,129,130,133,134,137,138,141,142,145,146,149,150,153,154,157,158,161,162,165,166,169,170,173,174,177,178,181,182,185,186,189,190]:
  inputs = ['10.170.1.' + str(IP) + ':' + str(port) for port in range(4346, 4350)]

  #if IP != 1:
    #inputs = 4 * ['null:']

  exec 'T' + str(IP) + '=[Station(\'S' + str(IP) + '\',\'10.170.1.' + str(IP) + '\',' + str(inputs) + ')]'
  exec 'R01=R01+T' + str(IP)
  

#
#   End of experimental part
#


# Standard configurations:

LBA              = CS010LBA + CS302LBA + RS307LBA + RS503LBA

# Space for experimental, one-of configurations:


#End of File
