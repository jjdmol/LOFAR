from Hosts import *
import sys


listfen   = ClusterFEN(name = 'listfen'    ,
                       address = '129.125.99.50')
listfen.setSlavesByPattern('list%03d', '10.181.0.%d', [1,2])	# RO
#listfen.setSlavesByPattern('list%03d', '10.181.0.%d', [3,4])	# R&D
#listfen.setSlavesByPattern('list%03d', '10.170.200.%d', [1])	# bgsn
#listfen.setSlavesByPattern('list%03d', '10.170.2.%d', [1,5,9,13,2,6,10,14,17,21,25,29,18,22,26,30,33,37,41,45,34,38,42,46,49,53,57,61,50,54,58,62]) # R02-M0



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

# define the pset hierarchy

# setup is analogue to:
# R00-M0-N00-J00 = R00-M0-N00-J00-16 consists of a single pset
# R00-M0-N00-32  = R00-M0-N00-J00 + R00-M0-N00-J01
# R00-M0-N00-64  = R00-M0-N00-32  + R00-M0-N01-32
# R00-M0-N00-128 = R00-M0-N00-64  + R00-M0-N02-64
# R00-M0-N00-256 = R00-M0-N00-128 + R00-M0-N04-128
# R00-M0         = R00-M0-N00-256 + R00-M0-N08-256
# R00            = R00-M0 + R00-M1

IONodes = {}
for R in xrange(3):
  rack = "R%02d" % R
  for M in xrange(2):
    midplane = "%s-M%01d" % (rack,M)

    # individual psets
    for N in xrange(16):
       nodecard = "%s-N%02d" % (midplane,N)
       for J in xrange(2):
         # ip address for this pset
         ip = "10.170.%d.%d" % (R,(1+M*128+N*4+J))

         pset = "%s-J%02d" % (nodecard,J)
         IONodes[pset] = [ip]
         IONodes[pset+"-16"] = [ip]

    # groups smaller than a midplane
    for groupsize in (1,2,4,8):
      for N in xrange(0,16,groupsize):
        nodecard = "%s-N%02d" % (midplane,N)

        IONodes["%s-%d" % (nodecard,32*groupsize)] = sum( [
          IONodes["%s-N%02d-J00" % (midplane,x)] + IONodes["%s-N%02d-J01" % (midplane,x)]
         for x in xrange( N, N+groupsize) ], [] )

    # a midplane
    IONodes[midplane] = IONodes["%s-N00-256" % midplane] + IONodes["%s-N08-256" % midplane]

  # a rack
  IONodes[rack] = IONodes["%s-M0" % rack] + IONodes["%s-M1" % rack]
