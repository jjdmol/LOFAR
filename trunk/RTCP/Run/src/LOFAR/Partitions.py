#!/usr/bin/env python

# PartitionPsets:	A dict which maps partitions to I/O node IP addresses.
__all__ = [ "PartitionPsets" ]

# the pset hierarchy is is analogue to:
# R00-M0-N00-J00 = R00-M0-N00-J00-16 consists of a single pset
# R00-M0-N00-32  = R00-M0-N00-J00 + R00-M0-N00-J01
# R00-M0-N00-64  = R00-M0-N00-32  + R00-M0-N01-32
# R00-M0-N00-128 = R00-M0-N00-64  + R00-M0-N02-64
# R00-M0-N00-256 = R00-M0-N00-128 + R00-M0-N04-128
# R00-M0         = R00-M0-N00-256 + R00-M0-N08-256
# R00            = R00-M0 + R00-M1

PartitionPsets = {}
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
         if R == 0: PartitionPsets[pset] = [ip] # single psets without -16 suffix only work on R00
         PartitionPsets[pset+"-16"] = [ip]

    # groups smaller than a midplane
    for groupsize in (1,2,4,8):
      for N in xrange(0,16,groupsize):
        nodecard = "%s-N%02d" % (midplane,N)

        PartitionPsets["%s-%d" % (nodecard,32*groupsize)] = sum( [
          PartitionPsets["%s-N%02d-J00" % (midplane,x)] + PartitionPsets["%s-N%02d-J01" % (midplane,x)]
         for x in xrange( N, N+groupsize) ], [] )

    # a midplane
    PartitionPsets[midplane] = PartitionPsets["%s-N00-256" % midplane] + PartitionPsets["%s-N08-256" % midplane]

  # a rack
  PartitionPsets[rack] = PartitionPsets["%s-M0" % rack] + PartitionPsets["%s-M1" % rack]

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import sys

  # parse the command line
  parser = OptionParser()
  parser.add_option( "-l", "--list",
  			dest = "list",
			action = "store_true",
			default = False,
  			help = "list all known BlueGene partitions, or the psets in the provided partition" )
  parser.add_option( "-P", "--partition",
  			dest = "partition",
			action = "store",
			type = "string",
  			help = "use a certain partition" )

  # parse arguments
  (options, args) = parser.parse_args()

  if not options.list:
    parser.print_help()
    sys.exit(0)

  if options.list:
    if options.partition:
      # print the psets of a single partition
      for ip in PartitionPsets[options.partition]:
        print ip
    else:
      # print a list of all known partitions
      for name in sorted(PartitionPsets.keys()):
        print name
