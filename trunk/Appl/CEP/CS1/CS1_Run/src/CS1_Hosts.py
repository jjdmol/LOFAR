from Hosts import *
import sys


liifen    = ClusterFEN(name = 'liifen',
                       address = '129.125.99.51')
liifen.setSlavesByPattern('lii%03d', '10.162.0.%d', [1,2,3,4,5,6,7,8,9,10,11,12,13])

listfen   = ClusterFEN(name = 'listfen'    ,
                       address = '129.125.99.50')
listfen.setSlavesByPattern('list%03d', '10.181.0.%d', [1,2])
#listfen.slaves.append(ClusterSlave('lifs001', '10.182.0.1'))
#listfen.slaves.append(ClusterSlave('lifs002', '10.182.0.2'))
#listfen.slaves.append(ClusterSlave('lifs003', '10.182.0.3'))
#listfen.slaves.append(ClusterSlave('lifs004', '10.182.0.4'))
#listfen.slaves.append(ClusterSlave('lifs005', '10.182.0.5'))


list001   = Host(name = 'list001'  , \
                 address = '10.181.0.1')
list002   = Host(name = 'list002'  , \
                 address = '10.181.0.2')
hpclf     = Host(name = 'hpclf'  , \
                 address = 'hpclf1.service.rug.nl')
bglfen1   = Host(name = 'bglfen1', \
                 address = 'bglfen1.service.rug.nl')
bglfen2   = Host(name = 'bglfen2', \
                 address = 'bglfen2.service.rug.nl')
bglfen3   = Host(name = 'bglfen3', \
                 address = 'bglfen3.service.rug.nl')
bglfen0   = Host(name = 'bglfen0', \
                 address = 'bglfen0.service.rug.nl')
CS10LCU   = Host(name = 'lcu', \
                 address = '10.151.18.1')
localhost = Host(name = 'localhost', \
                 address = 'localhost')

gels        = UserId(bglfen1,'gels')
romein      = UserId(bglfen2,'romein')
lofarsystem = UserId(bglfen3,'lofarsystem')
broekema    = UserId(bglfen2,'broekema')
