from Hosts import *


liifen    = ClusterFEN(name = 'liifen',
                       address = '129.125.99.51')
liifen.setSlavesByPattern('lii%03d', '10.20.150.%d', range(1, 13))



listfen   = ClusterFEN(name = 'listfen'    ,
                       address = '129.125.99.50')
listfen.setSlavesByPattern('list%03d', '10.20.170.%d', range(1, 7) + range(8, 13))


list002   = Host(name = 'list002'  , \
                 address = '10.20.170.2')
list012   = Host(name = 'list012'  , \
                 address = '10.20.170.12')		 
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
