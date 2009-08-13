#!/usr/bin/env python

from Partitions import PartitionPsets

__all__ = ["Stations"]

class UnknownStationError(StandardError):
    pass

class Station(object):
    """
    Represents a real or virtual station.
    """
    def __init__(self, name, ionode, inputs):
        self.name	= name
        self.ionode	= ionode
        self.inputs	= inputs

    def getPsetIndex(self, partition):
        assert partition in PartitionPsets, "Unknown partition: %s" % (partition,)

        psets = PartitionPsets[partition]

        assert self.ionode in psets, "IONode %s not in partition %s" % (self.ionode,partition)

	return psets.index(self.ionode)

class Stations(dict):
  def __init__(self):
     self.stations = {}

  def parse( self, str ):
    """ Parse a string defining a set of stations, such as 'CS001 + CS010'. """

    # use no globals to avoid eval adding "__builtins__" to self, and thus as a station
    try:
      return eval( str, {}, self )  
    except Exception,e:
      raise UnknownStationError("%s (while parsing '%s')" % (e,str))

  def parseAll( self ):
    """ Parse all station definitions that were given as a string and replace them with Station lists. """

    changed = True

    # not everything parses in one go due to interdependencies
    while changed:
      changed = False
      for k,v in self.items():
        if isinstance( v, str ):
          try:
            self[k] = self.parse( v )
          except UnknownStationError:
             continue
          else:
             changed = True

    # reparse all strings left to trigger any exceptions that are left
    for k,v in self.items():
      if isinstance( v, str ):
        self[k] = self.parse( v )

def defineStations( s ):
  # Actual, physical stations. """

  def ports( portnrs ):
    return ['0.0.0.0:%s' % (port,) for port in portnrs]

  defaultPorts = ports( [4346, 4347, 4348, 4349] )

  s.update( {
    # CS302
    "CS302LBA":    [Station('CS302LBA',  '10.170.0.21', defaultPorts)],
    "CS302HBA":    [Station('CS302HBA',  '10.170.0.21', defaultPorts)],
    "CS302HBA0":   [Station('CS302HBA0', '10.170.0.21', defaultPorts)],
    "CS302HBA1":   [Station('CS302HBA1', '10.170.0.22', ports( [4352,4353,4354,4355] ))],

    # RS307
    "RS307LBA":   [Station('RS307LBA',  '10.170.0.189', defaultPorts)],
    "RS307HBA":   [Station('RS307HBA',  '10.170.0.189', defaultPorts)],

    # RS503
    "RS503LBA":   [Station('RS503LBA',  '10.170.0.170', defaultPorts)],
    "RS503HBA":   [Station('RS503HBA',  '10.170.0.170', defaultPorts)],

    # DE601, a.k.a. Effelsberg
    "DE601LBA"    [Station('DE601LBA',  '10.170.0.178', ports( [4358,4359,4363,4364] )],
  } )

  # Simulated stations for experimentation.

  # IP suffixes of all psets
  psetsuffixes = [1,2,5,6,9,10,13,14,17,18,21,22,25,26,29,30,33,34,37,38,41,42,45,46,49,50,53,54,57,58,61,62,129,130,133,134,137,138,141,142,145,146,149,150,153,154,157,158,161,162,165,166,169,170,173,174,177,178,181,182,185,186,189,190]

  for suffix in psetsuffixes:
     # Rack R00
     ip = "10.170.0.%s" % (suffix,)

     # sXX_1, sXX: 1 full station (1 RSP board), starting from 10.170.0.XX, input received from station
     inputs = ports( [4346] )
     s["s%s_1" % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]
     s["s%s"   % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]

     # SXX_1, SXX: 1 full station (4 RSP boards), starting from 10.170.0.XX, input received from station
     inputs = defaultPorts
     s["S%s_1" % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]
     s["S%s"   % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]

     # Rack R01
     ip = "10.170.1.%s" % (suffix,)

     # tXX_1: 1 full station (1 RSP board), starting from 10.170.1.XX, input received from station
     inputs = ports( [4346] )
     s["t%s_1" % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]

     # TXX_1: 1 full station (4 RSP boards), starting from 10.170.1.XX, input received from station
     inputs = defaultPorts
     s["T%s_1" % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]

  # define sets of various sizes 
  for setsize in [2,4,8,16,32,64]:
     for first_index in xrange( 0, len(psetsuffixes), setsize ):
       suffixes = psetsuffixes[first_index:first_index+setsize]

       for prefix in ["s","S","t","T"]:
         s["%s%s_%s" % (prefix,suffixes[0],setsize)] = sum( (s["%s%s_1" % (prefix,suffix,)] for suffix in suffixes), [] )

  # Special stations, one-time stations, etc.
  s.update( {
   "Pulsar": [Station('Pulsar', '10.170.0.30', ['0.0.0.0:4346'])],
   "twoears":  [Station('CS302HBA0', '10.170.0.133', ['0.0.0.0:4346']),
                Station('CS302HBA1', '10.170.0.134', ['0.0.0.0:4347'])],
  } )

  # Standard configurations
  s.update( {
    "LBA":          "CS001_lba + CS010_lba + CS016_lba",
    "LBAROTATED":   "CS001_lba_rotated + CS010_lba_rotated + CS016_lba_rotated",
    "HBA":          "CS001_hba + CS010_hba + CS016_hba",
    "HBATILES":     "CS001_hba + CS010_tiles + CS016_hba",
    "HBATILESONLY": "CS010_tiles",
  } )

# Construct everything

Stations = Stations()
defineStations( Stations )
Stations.parseAll()

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import sys

  # parse the command line
  parser = OptionParser()
  parser.add_option( "-l", "--list",
  			dest = "list",
			action = "store_true",
			default = False,
  			help = "list all known stations" )
  parser.add_option( "-S", "--station",
  			dest = "station",
			action = "store",
			type = "string",
  			help = "list the inputs of a certain station" )

  # parse arguments
  (options, args) = parser.parse_args()

  if not options.list and not options.station:
    parser.print_help()
    sys.exit(0)

  # print a list of all known stations
  if options.list:
    for name in sorted(Stations.keys()):
      print name

  # print the inputs of a single station
  if options.station:
    for s in Stations[options.station]:
      print s.name,s.ionode," ".join(s.inputs)

