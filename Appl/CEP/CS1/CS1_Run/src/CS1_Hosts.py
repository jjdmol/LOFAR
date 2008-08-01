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
bglfen1   = Host(name = 'bglfen1', \
                 address = 'bglfen1.service.rug.nl')
bglfen2   = Host(name = 'bglfen2', \
                 #address = 'bglfen2.service.rug.nl')
                 address = 'bglfen2')
bglfen3   = Host(name = 'bglfen3', \
                 address = 'bglfen3.service.rug.nl')
bglfen0   = Host(name = 'bglfen0', \
                 address = 'bglfen0.service.rug.nl')
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
lofarsystem = UserId(bgfen0,'lofarsystem')
broekema    = UserId(bgfen0,'broekema')

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

psetDimensions = dict({ \
  'R000-B00' : [ \
    4, 2, 2, \
  ], \
  'R000-B01' : [ \
    4, 2, 2, \
  ], \
  'R000-B02' : [ \
    4, 2, 2, \
  ], \
  'R000-B03' : [ \
    4, 2, 2, \
  ], \
  'R000_256_1' : [ \
    4, 2, 2, \
  ], \
})
