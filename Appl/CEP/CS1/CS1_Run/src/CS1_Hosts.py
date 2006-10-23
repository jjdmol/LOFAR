from Hosts import *


liifen    = ClusterFEN(name = 'liifen',
                       address = '129.125.99.51')
liifen.setSlavesByPattern('lii%03d', '10.20.150.%d', range(1, 13))



listfen   = ClusterFEN(name = 'listfen'    ,
                       address = '129.125.99.50')
listfen.setSlavesByPattern('list%03d', '10.20.170.%d', range(1, 13))



hpclf     = Host(name = 'hpclf'  , \
                 address = 'hpclf1.service.rug.nl')
bglfen1   = Host(name = 'bglfen1', \
                 address = 'bglfen1.service.rug.nl')
bglfen3   = Host(name = 'bglfen3', \
                 address = 'bglfen3.service.rug.nl')
CS10LCU   = Host(name = 'lcu', \
                 address = '10.151.18.1')
localhost = Host(name = 'localhost', \
                 address = 'localhost')
