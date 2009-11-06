from Hosts import *
import sys


listfen   = ClusterFEN(name = 'listfen'    ,
                       address = '129.125.99.50')
listfen.setSlavesByPattern('list%03d', '10.181.0.%d', [3,4])
#listfen.slaves.append(ClusterSlave('lifs001', '10.182.0.1'))
#listfen.slaves.append(ClusterSlave('lifs002', '10.182.0.2'))
#listfen.slaves.append(ClusterSlave('lifs003', '10.182.0.3'))
#listfen.slaves.append(ClusterSlave('lifs004', '10.182.0.4'))
#listfen.slaves.append(ClusterSlave('lifs005', '10.182.0.5'))


list001   = Host(name = 'list001'  , \
                 address = '10.181.0.1')
list002   = Host(name = 'list002'  , \
                 address = '10.181.0.2')
list003   = Host(name = 'list003'  , \
                 address = '10.181.0.3')
hpclf     = Host(name = 'hpclf'  , \
                 address = 'hpclf1.service.rug.nl')
bgfen1   = Host(name = 'bgfen1', \
                 address = 'bgfen1.bgnet.rug.nl')
bgfen2   = Host(name = 'bgfen2', \
                 address = 'bgfen2.bgnet.rug.nl')
bgfen3   = Host(name = 'bgfen3', \
                 address = 'bgfen3.bgnet.rug.nl')
bgfen0    = Host(name = 'bgfen0', \
                 address = 'bgfen0.bgnet.rug.nl')
bgsn      = Host(name = 'bgsn', \
                 address = 'bgsn')		 
CS10LCU   = Host(name = 'lcu', \
                 address = '10.151.18.1')
localhost = Host(name = 'localhost', \
                 address = 'localhost')

gels        = UserId(bgfen0,'gels')
romein      = UserId(bgfen0,'romein')
lofarsys    = UserId(bgfen0,'lofarsys')
broekema    = UserId(bgfen0,'broekema')
nieuwpoo    = UserId(bgfen0,'nieuwpoo')
mol         = UserId(bgfen0,'mol')

IONodes = dict({ \
  'R000-B00' : [ \
    '10.170.0.1', \
    '10.170.0.2', \
  ], \
  'R000-B01' : [ \
    '10.170.0.5', \
    '10.170.0.6', \
  ], \
  'R000-B02' : [ \
    '10.170.0.9', \
    '10.170.0.10', \
  ], \
  'R000-B03' : [ \
    '10.170.0.13', \
    '10.170.0.14', \
  ], \
  'R000_64_0' : [ \
    '10.170.0.1', \
    '10.170.0.2', \
    '10.170.0.5', \
    '10.170.0.6', \
  ], \
  'R000_64_1' : [ \
    '10.170.0.9', \
    '10.170.0.10', \
    '10.170.0.13', \
    '10.170.0.14', \
  ], \
  'R000_64_2' : [ \
    '10.170.0.17', \
    '10.170.0.18', \
    '10.170.0.21', \
    '10.170.0.22', \
  ], \
  'R000_64_3' : [ \
    '10.170.0.25', \
    '10.170.0.26', \
    '10.170.0.29', \
    '10.170.0.30', \
  ], \
  'R000_256_1' : [ \
    '10.170.0.33', \
    '10.170.0.34', \
    '10.170.0.37', \
    '10.170.0.38', \
    '10.170.0.41', \
    '10.170.0.42', \
    '10.170.0.45', \
    '10.170.0.46', \
    '10.170.0.49', \
    '10.170.0.50', \
    '10.170.0.53', \
    '10.170.0.54', \
    '10.170.0.57', \
    '10.170.0.58', \
    '10.170.0.61', \
    '10.170.0.62', \
  ], \
})
