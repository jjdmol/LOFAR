#!/usr/bin/env python

from Partitions import PartitionPsets
import os
import sys

# allow ../util to be found, a bit of a hack
sys.path += [os.path.dirname(__file__)+"/.."]

from util.Commands import SyncCommand,backquote

__all__ = ["packetAnalysis","stationInPartition","Stations"]

def packetAnalysis( name, ip, port ):
  # locate packetanalysis binary, since its location differs per usage, mainly because
  # nobody runs these scripts from an installed environment
  locations = map( os.path.abspath, [
    "%s/../packetanalysis"    % os.path.dirname(__file__), # when running straight from a source tree
    "%s/../../packetanalysis" % os.path.dirname(__file__), # when running in an installed environment
    "%s/../../build/gnu/src/packetanalysis" % os.path.dirname(__file__), # when running straight from a source tree

    "/cephome/mol/projects/LOFAR/RTCP/Run/src/packetanalysis", # fallback: Jan David's version
  ] )
  
  location = None
  for l in locations:
    if os.path.exists( l ):
      location = l
      break

  if location is None:
    return "ERROR: Could not find `packetanalysis' binary"

  mainAnalysis = backquote( "ssh -tq %s %s %s" % (ip,location,port), 5)

  if not mainAnalysis or " 0.00 pps" in mainAnalysis:
    # something went wrong -- don't run tcpdump
    return mainAnalysis

  # do a tcpdump analysis to obtain source mac address
  """ tcpdump: The following information will be received from stations:

08:30:23.175116 10:fa:00:01:01:01 > 00:14:5e:7d:19:71, ethertype IPv4 (0x0800), length 6974: 10.159.1.2.4347 > 10.170.0.1.4347: UDP, length 6928
  """
  tcpdump = backquote("ssh -q %s /opt/lofar/bin/tcpdump -i eth0 -c 10 -e -n udp 2>/dev/null" % (ip,), 2).split("\n")
  macaddress = "UNKNOWN"
  for p in tcpdump:
    if not p: continue

    try:
      f = p.split()
      srcmac = f[1]
      dstip = f[-4]

      dstip,dstport = dstip[:-1].rsplit(".",1)
      if dstport != port:
        continue
    except ValueError:
      continue

    macaddress = srcmac

  if macaddress in [
    "00:12:f2:c3:3a:00", # Effelsberg
  ]:
    macline = " OK Source MAC address:  %s (known router)" % (macaddress,)
  elif macaddress == "UNKNOWN" or not macaddress.startswith("00:22:86:"):
    macline = "NOK Source MAC address:  %s (no LOFAR station)" % (macaddress,)
  else:
    rscs,nr,field = name[0:2],name[2:5],name[5:]
    nr1,nr2,nr3 = nr

    macnrs = macaddress.split(":")
    srcnr = "%s%s" % (macnrs[3],macnrs[4])

    if int(nr) == int(srcnr):
      macline = " OK Source MAC address:  %s" % (macaddress,)
    else:
      macline = "NOK Source MAC address:  %s (station %d?)" % (macaddress,int(srcnr))
    
  return "%s\n%s" % (macline,mainAnalysis) 

def allInputs( station ):
  """ Generates a list of name,ip,port tuples for all inputs for a certain station. """

  for name,input,ionode in sum( ([(s.name,i,s.ionode)] for s in station for i in s.inputs), [] ):
    # skip non-network inputs
    if input == "null:":
      continue

    if input.startswith( "file:" ):
      continue

    # strip tcp:
    if input.startswith( "tcp:" ):
      input = input[4:]

    # only process ip:port combinations
    if ":" in input:
      ip,port = input.split(":")
      if ip in ["0.0.0.0","0"]:
        ip = ionode
      yield (name,ip,port)

def stationInPartition( station, partition ):
  """ Returns a list of stations that are not received within the given partition.
  
      Returns (True,[]) if the station is received correctly.
      Returns (False,missingInputs) if some inputs are missing, where missingInputs is a list of (name,ip:port) pairs.
  """

  notfound = []

  for name,ip,port in allInputs( station ):  
    if ip not in PartitionPsets[partition]:
      notfound.append( (name,"%s:%s" % (ip,port)) )
    
  return (not notfound, notfound)	


class UnknownStationError(StandardError):
    pass

class Station(object):
  """
  Represents a real or virtual station.
  """
  def __init__(self, name, ionode, inputs):
    self.name   = name
    self.ionode	= ionode
    self.inputs	= inputs

  def getPsetIndex(self, partition):
    assert partition in PartitionPsets, "Unknown partition: %s" % (partition,)

    psets = PartitionPsets[partition]

    assert self.ionode in psets, "IONode %s not in partition %s" % (self.ionode,partition)

    return psets.index(self.ionode)

  def getName(self):
    return self.name

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
    "CS302LBA":    [Station('CS302LBA',  '10.170.0.165', ports( [4346,4347,4348,4350] ))],
    "CS302HBA":    [Station('CS302HBA',  '10.170.0.165', ports( [4346,4347,4348,4350] ))],
    "CS302HBA0":   [Station('CS302HBA0', '10.170.0.165', ports( [4346,4347,4348,4350] ))],
    "CS302HBA1":   [Station('CS302HBA1', '10.170.0.37',  ports( [4352,4353,4354,4355] ))],

    # RS106
    "RS106LBA":   [Station('RS106LBA',  '10.170.0.174', defaultPorts)],
    "RS106HBA":   [Station('RS106HBA',  '10.170.0.174', defaultPorts)],

    # RS208
    "RS208LBA":   [Station('RS208LBA',  '10.170.0.162', defaultPorts)],
    "RS208HBA":   [Station('RS208HBA',  '10.170.0.162', defaultPorts)],

    # RS307
    "RS307LBA":   [Station('RS307LBA',  '10.170.0.189', defaultPorts)],
    "RS307HBA":   [Station('RS307HBA',  '10.170.0.189', defaultPorts)],

    # RS503
    "RS503LBA":   [Station('RS503LBA',  '10.170.0.170', defaultPorts)],
    "RS503HBA":   [Station('RS503HBA',  '10.170.0.170', defaultPorts)],

    # CS030
    "CS030LBA":   [Station('CS030LBA',  '10.170.0.153', defaultPorts)],
    "CS030HBA":   [Station('CS030HBA',  '10.170.0.153', defaultPorts)],

    # DE601, a.k.a. Effelsberg
    "DE601LBA":   [Station('DE601LBA',  '10.170.0.178', ports( [4353,4359,4363,4364] ))],
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
     s["T%s"   % (suffix,)] = [Station("S%s" % (suffix,), ip, inputs)]

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
  } )

# Construct everything

Stations = Stations()
defineStations( Stations )
Stations.parseAll()

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import sys

  # parse the command line
  parser = OptionParser( "usage: %prog [options] station" )
  parser.add_option( "-i", "--inputs",
  			dest = "inputs",
			action = "store_true",
			default = False,
  			help = "list the inputs for the station" )
  parser.add_option( "-c", "--check",
  			dest = "check",
			action = "store_true",
			default = False,
  			help = "check whether the station provide correct data" )

  # parse arguments
  (options, args) = parser.parse_args()
  errorOccurred = False

  if not args:
    parser.print_help()
    sys.exit(0)

  for stationName in args:
    # print the inputs of a single station
    if options.inputs:
      for s in Stations[stationName]:
        print "Station:     %s" % (s.name,)
        print "IONode:      %s" % (s.ionode,)
        print "Inputs:      %s" % (" ".join(s.inputs),)

    # check stations if requested so
    if options.check:
      if stationName not in Stations:
        # unknown station
        errorOccurred = True
        print "NOK Station name unknown: %s" % (stationName,)
        continue

      for name,ip,port in allInputs( Stations[stationName] ):
        print "---- Packet analysis for %s %s:%s" % (name,ip,port)
        print packetAnalysis( name, ip, port )

  sys.exit(int(errorOccurred))
