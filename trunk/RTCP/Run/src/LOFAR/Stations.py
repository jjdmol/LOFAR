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

  defaultInputs = ['0.0.0.0:%s' % (4346+i) for i in xrange(4)]

  s.update( {
    # CS001
    "CS001_lba0":  [Station('CS001LBA_LBA0',  '10.170.0.1',  ['0.0.0.0:4346'])],
    "CS001_lba2":  [Station('CS001LBA_LBA2',  '10.170.0.1',  ['0.0.0.0:4346'])],
    "CS001_lba4":  [Station('CS001LBA_LBA4',  '10.170.0.2',  ['0.0.0.0:4347'])],
    "CS001_lba6":  [Station('CS001LBA_LBA6',  '10.170.0.2',  ['0.0.0.0:4347'])],
    "CS001_lba8":  [Station('CS001LBA_LBA8',  '10.170.0.17', ['0.0.0.0:4348'])],
    "CS001_lba10": [Station('CS001LBA_LBA10', '10.170.0.17', ['0.0.0.0:4348'])],
    "CS001_lba12": [Station('CS001LBA_LBA12', '10.170.0.18', ['0.0.0.0:4349'])],
    "CS001_lba14": [Station('CS001LBA_LBA14', '10.170.0.18', ['0.0.0.0:4349'])],
    "CS001_lba":   "CS001_lba0 + CS001_lba4 + CS001_lba8 + CS001_lba12",
    "CS001_lba_rotated": "CS001_lba2 + CS001_lba6 + CS001_lba10 + CS001_lba14",

    "CS001_hba0":  [Station('CS001HBA_HBA0', '10.170.0.1',  ['0.0.0.0:4346'])],
    "CS001_hba1":  [Station('CS001HBA_HBA1', '10.170.0.2',  ['0.0.0.0:4347'])],
    "CS001_hba2":  [Station('CS001HBA_HBA2', '10.170.0.17', ['0.0.0.0:4348'])],
    "CS001_hba3":  [Station('CS001HBA_HBA3', '10.170.0.18', ['0.0.0.0:4349'])],
    "CS001_hba":   "CS001_hba0 + CS001_hba1 + CS001_hba2 + CS001_hba3",

    "CS001":       [Station('CS001', '10.170.0.1', defaultInputs)],

    # CS010
    "CS010_lba0":  [Station('CS010LBA_LBA0',  '10.170.0.5',  ['0.0.0.0:4346'])],
    "CS010_lba2":  [Station('CS010LBA_LBA2',  '10.170.0.5',  ['0.0.0.0:4346'])],
    "CS010_lba4":  [Station('CS010LBA_LBA4',  '10.170.0.6',  ['0.0.0.0:4351'])],
    "CS010_lba6":  [Station('CS010LBA_LBA6',  '10.170.0.6',  ['0.0.0.0:4351'])],
    "CS010_lba8":  [Station('CS010LBA_LBA8',  '10.170.0.21', ['0.0.0.0:4352'])],
    "CS010_lba10": [Station('CS010LBA_LBA10', '10.170.0.21', ['0.0.0.0:4352'])],
    "CS010_lba12": [Station('CS010LBA_LBA12', '10.170.0.22', ['0.0.0.0:4353'])],
    "CS010_lba14": [Station('CS010LBA_LBA14', '10.170.0.22', ['0.0.0.0:4353'])],
    "CS010_lba":   "CS010_lba0 + CS010_lba4 + CS010_lba8 + CS010_lba12",
    "CS010_lba_rotated": "CS010_lba",

    "CS010_hba0":  [Station('CS010HBA_HBA0', '10.170.0.5',  ['0.0.0.0:4346'])],
    "CS010_hba1":  [Station('CS010HBA_HBA1', '10.170.0.6',  ['0.0.0.0:4351'])],
    "CS010_hba2":  [Station('CS010HBA_HBA2', '10.170.0.21', ['0.0.0.0:4352'])],
    "CS010_hba3":  [Station('CS010HBA_HBA3', '10.170.0.22', ['0.0.0.0:4353'])],
    "CS010_hba":   "CS010_hba0 + CS010_hba1 + CS010_hba2 + CS010_hba3",

    "CS010_TILE1": [Station('CS010HBA_TILE1', '10.170.0.5',  ['0.0.0.0:4346'])],
    "CS010_TILE2": [Station('CS010HBA_TILE2', '10.170.0.6',  ['0.0.0.0:4351'])],
    "CS010_TILE5": [Station('CS010HBA_TILE5', '10.170.0.21', ['0.0.0.0:4352'])],
    "CS010_TILE6": [Station('CS010HBA_TILE6', '10.170.0.22', ['0.0.0.0:4353'])],
    "CS010_tiles": "CS010_TILE1 + CS010_TILE2 + CS010_TILE5 + CS010_TILE6",

    "CS010":       [Station('CS010', '10.170.0.5', ['0.0.0.0:4346', '0.0.0.0:4351', '0.0.0.0:4352', '0.0.0.0:4353'])],

    # CS016
    "CS016_lba0":  [Station('CS016LBA_LBA0', '10.170.0.9', ['0.0.0.0:4346'])],
    "CS016_lba2":  [Station('CS016LBA_LBA2', '10.170.0.9', ['0.0.0.0:4346'])],
    "CS016_lba4":  [Station('CS016LBA_LBA4', '10.170.0.10', ['0.0.0.0:4347'])],
    "CS016_lba6":  [Station('CS016LBA_LBA6', '10.170.0.10', ['0.0.0.0:4347'])],
    "CS016_lba8":  [Station('CS016LBA_LBA8', '10.170.0.25', ['0.0.0.0:4348'])],
    "CS016_lba10": [Station('CS016LBA_LBA10', '10.170.0.25', ['0.0.0.0:4348'])],
    "CS016_lba12": [Station('CS016LBA_LBA12', '10.170.0.26', ['0.0.0.0:4349'])],
    "CS016_lba14": [Station('CS016LBA_LBA14', '10.170.0.26', ['0.0.0.0:4349'])],
    "CS016_lba":   "CS016_lba0 + CS016_lba4 + CS016_lba8 + CS016_lba12",
    "CS016_lba_rotated": "CS016_lba2 + CS016_lba6 + CS016_lba10 + CS016_lba14",

    "CS016_hba0":  [Station('CS016_HBA0', '10.170.0.9',  ['0.0.0.0:4346'])],
    "CS016_hba1":  [Station('CS016_HBA1', '10.170.0.10', ['0.0.0.0:4347'])],
    "CS016_hba2":  [Station('CS016_HBA2', '10.170.0.25', ['0.0.0.0:4348'])],
    "CS016_hba3":  [Station('CS016_HBA3', '10.170.0.26', ['0.0.0.0:4349'])],
    "CS016_hba":   "CS016_hba0 + CS016_hba1 + CS016_hba2 + CS016_hba3",

    "CS016":       [Station('CS016', '10.170.0.9', defaultInputs)],

    # CS302
    "CS302LBA":    [Station('CS302LBA',  '10.170.0.21', defaultInputs)],
    "CS302HBA":    [Station('CS302HBA',  '10.170.0.21', defaultInputs)],
    "CS302HBA0":   [Station('CS302HBA0', '10.170.0.21', defaultInputs)],
    "CS302HBA1":   [Station('CS302HBA1', '10.170.0.21', defaultInputs)],

    # RS307
    "RS307LBA":   [Station('RS307LBA',  '10.170.0.189', defaultInputs)],
    "RS307HBA":   [Station('RS307HBA',  '10.170.0.189', defaultInputs)],

    # RS503
    "RS503LBA":   [Station('RS503LBA',  '10.170.0.170', defaultInputs)],
    "RS503HBA":   [Station('RS503HBA',  '10.170.0.170', defaultInputs)],
  } )

  # Simulated stations for experimentation.

  # IP suffixes of all psets
  psetsuffixes = [1,2,5,6,9,10,13,14,17,18,21,22,25,26,29,30,33,34,37,38,41,42,45,46,49,50,53,54,57,58,61,62,129,130,133,134,137,138,141,142,145,146,149,150,153,154,157,158,161,162,165,166,169,170,173,174,177,178,181,182,185,186,189,190]

  for suffix in psetsuffixes:
     # Rack R00
     ip = "10.170.0.%s" % (suffix,)

     # sXX_1, sXX: 1 full station (1 RSP board), starting from 10.170.0.XX, input received from station
     inputs = ["0.0.0.0:4346"]
     s["s%s_1" % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]
     s["s%s"   % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]

     # SXX_1, SXX: 1 full station (4 RSP boards), starting from 10.170.0.XX, input received from station
     inputs = ["0.0.0.0:%s" % (port,) for port in [4346,4347,4348,4349]]
     s["S%s_1" % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]
     s["S%s"   % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]

     # Rack R01
     ip = "10.170.1.%s" % (suffix,)

     # tXX_1: 1 full station (1 RSP board), starting from 10.170.1.XX, input received from station
     inputs = ["0.0.0.0:4346"]
     s["t%s_1" % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]

     # TXX_1: 1 full station (4 RSP boards), starting from 10.170.1.XX, input received from station
     inputs = ["0.0.0.0:%s" % (port,) for port in [4346,4347,4348,4349]]
     s["T%s_1" % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]

  # define sets of various sizes 
  for setsize in [2,4,8,16,32,64]:
     for first_index in xrange( 0, len(psetsuffixes), setsize ):
       suffixes = psetsuffixes[first_index:first_index+setsize]

       for prefix in ["s","S","t","T"]:
         s["%s%s_%s" % (prefix,suffixes[0],setsize)] = sum( (s["%s%s_1" % (prefix,suffix,)] for suffix in suffixes), [] )

  # Special stations, one-time stations, etc.
  s.update( {
   "Pulsar": [Station('Pulsar', '10.170.0.30', ['tcp:0.0.0.0:4346'])],
   "twoears":  [Station('CS302HBA0', '10.170.0.21', ['0.0.0.0:4346']),
                Station('CS302HBA1', '10.170.0.22', ['0.0.0.0:4347'])],
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

