from Hosts import *


liifen    = ClusterFEN(name = 'liifen',
                       address = '129.125.99.51')
liifen.setSlavesByPattern('lii%03d', '10.20.150.%d', [1,2,3,4,5,6,7,8,9,10,11,12])



listfen   = ClusterFEN(name = 'listfen'    ,
                       address = '129.125.99.50')
listfen.setSlavesByPattern('list%03d', '10.20.170.%d', [1,2,3,4,5,6,7,8,9,10,11,12])


list002   = Host(name = 'list002'  , \
                 address = '10.20.170.2')
list012   = Host(name = 'list012'  , \
                 address = '10.20.170.12')		 
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
		 
gels        = UserId(bglfen1,'gels' ,'/cephome/gels/LOFAR/installed/gnu_bgl/bin/')
lofarsystem = UserId(bglfen3,'lofarsystem' ,'/bglhome2/lofarsystem/')
romein      = UserId(bglfen2,'romein' ,'/cephome/romein/LOFAR/installed/gnu_bgl/bin/')
broekema    = UserId(bglfen2,'broekema' ,'/cephome/broekema/LOFAR/installed/gnu_bgl/bin/')
		 
