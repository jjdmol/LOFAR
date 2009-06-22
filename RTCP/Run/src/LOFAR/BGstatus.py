#!/usr/bin/env python

import os
from Partitions import PartitionPsets
from Stations import Stations

# allow ../util to be found, a bit of a hack
sys.path += [os.path.dirname(__file__)+"/.."]

from util.Commands import SyncCommand

__all__ = ["owner","runningJob","stationInfo","allStationInfo","receivingStation","stationInPartition","killJobs","freePartition","allocatePartition"]

"""
  Reports about the status of the BlueGene. An interface to the bg* scripts, and to check
  what data is being received on the BlueGene from the stations.
"""

# Default partition to query
PARTITION = "R00-M0-N00-256"

# Locations of the bg* scripts
BGBUSY = "/usr/local/bin/bgbusy"
BGJOBS = "/usr/local/bin/bgjobs"

def owner( partition ):
  """ Returns the name of the owner of the partition, or None if the partition is not allocated. """

  cmd = "%s" % (BGBUSY,)

  for l in os.popen( cmd, "r" ).readlines():
    try:
      part,nodes,cores,state,owner,net = l.split()
    except ValueError:
      continue

    if part == partition:  
      # partition found     
      return owner
  
  # partition is not allocated
  return None

def runningJob( partition ):
  """ Returns a (jobId,name) tuple of the job running on the partition, or None if no job is running. """

  cmd = "%s" % (BGJOBS,)

  for l in os.popen( cmd, "r" ).readlines():
    try:
      job,part,mode,executable,user,state,queue,limit,wall = l.split()
    except ValueError:
      continue

    if part == partition: 
      # job found 
      return (job,executable)
  
  # partition is not allocated or has no job running
  return None

def stationInfo( ip ):
  """ Returns information about station data received on a pset (which is an IP address).
  
      Returns a list of tuples (name, boardnr, ip:port, #beamlets, #samples). """ 

  """ The following information will be received from stations:

08:30:23.175116 10:fa:00:01:01:01 > 00:14:5e:7d:19:71, ethertype IPv4 (0x0800), length 6974: 10.159.1.2.4347 > 10.170.0.1.4347: UDP, length 6928
	0x0000:  4600 1b30 0000 4000 8011 c871 0a9f 0102  F..0..@....q....
	0x0010:  0aaa 0001 0000 0000 10fb 10fb 1b18 0000  ................
	0x0020:  0201 aabb 2100 3610 9f1c 114a 4468 0000  ....!.6....JDh..
	                        ^^^^ #beamlets, #times
  """
  cmd = "ssh %s /opt/lofar/bin/tcpdump -i eth0 -c 10 -X -s 62 -e -n 2>/dev/null" % (ip,)
  tcpdump = os.popen( cmd, "r" ).readlines()

  # split into packets, lines starting with a tab belong to the previous header
  packets = []
  for l in tcpdump:
    if packets and l.startswith("\t"):
      # content line
      packets[-1].append( l )
    else:
      # header
      packets.append( [l] ) 

  # analyse packets
  stations = set()
  for p in packets:
    # analyse header
    try:
      _,srcmac,_,dstmac,_,_,_,_,_,srcip,_,dstip,prot,_,length = p[0].split()
    except ValueError:
      continue

    fromstation = srcmac.startswith("10:fa:00:")
    _,_,_,stationnr,boardnr,_ = map( lambda x: int(x,16), srcmac.split(":") )
    dstip,dstport = dstip[:-1].rsplit(".",1)

    if not fromstation:
      continue

    # analyse content
    thirdhexline = p[3].split()[1:9]
    beamlets     = int(thirdhexline[3][0:2],16)
    nrsamples    = int(thirdhexline[3][2:4],16)

    station      = ("CS%03d" % (stationnr,), boardnr, "%s:%s" % (dstip, dstport), beamlets, nrsamples)

    stations.add( station )

  # return all stations found
  return list(stations)

def allStationInfo( partition ):
  """ Returns information about all stations sending data to this partition. """
 
  stations = sum( (stationInfo(pset) for pset in PartitionPsets[partition]), [] )

  return sorted(stations)

def allInputs( station ):
  """ Generates a list of name,ip,port tuples for all inputs for a certain station. """

  for name,input in sum( ([(s.name,i)] for s in station for i in s.inputs), [] ):
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
      yield (name,ip,port)

def receivingStation( station ):
  """ Returns whether we are receiving a station correctly.
  
      Returns (True,[]) if the station is received correctly.
      Returns (False,missingInputs) if some inputs are missing, where missingInputs is a list of (name,ip:port) pairs.
  """

  # cache info per pset
  infocache = {}

  # accumulate inputs we cannot find
  notfound = []

  for name,ip,port in allInputs( station ):
    if ip not in infocache:
      infocache[ip] = stationInfo( ip )
    info = infocache[ip]  

    input = "%s:%s" % (ip,port)

    for _,_,dest,_,_ in info:
      if dest == input:
        break
    else:
      notfound.append( (name,input) )

  return (not notfound, notfound)	

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

def killJobs( partition ):
  # kill anything running on the partition
  return SyncCommand( "%s | /usr/bin/grep %s | /usr/bin/awk '{ print $1; }' | /usr/bin/xargs -r bgkilljob" % (BGBUSY,(partition,)) ).isSuccess()


def freePartition( partition ):
  # free the given partition
  return SyncCommand( "mpirun -partition %s -free wait" % (partition,) ).isSuccess()

def allocatePartition( partition ):
  # allocate the given partition by running Hello World
  return SyncCommand( "mpirun -partition %s -nofree -exe /bgsys/tools/hello" % (partition,), "/dev/null" ).isSuccess()

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import sys

  def okstr( q ):
    return ["NOK","OK"][int(bool(q))]

  # parse the command line
  parser = OptionParser()
  parser.add_option( "-s", "--status",
  			dest = "status",
			action = "store_true",
			default = False,
  			help = "print a status overview" )
  parser.add_option( "-c", "--check",
  			dest = "checkStation",
			action = "append",
			type = "string",
			default = [],
  			help = "check whether a certain station is being received" )
  parser.add_option( "-k", "--kill",
  			dest = "kill",
			action = "append",
			type = "string",
			default = [],
  			help = "kill all jobs running on the partition" )
  parser.add_option( "-a", "--allocate",
  			dest = "allocate",
			action = "append",
			type = "string",
			default = [],
  			help = "allocate the partition" )
  parser.add_option( "-f", "--free",
  			dest = "free",
			action = "append",
			type = "string",
			default = [],
  			help = "free the partition" )

  hwgroup = OptionGroup(parser, "Hardware" )
  hwgroup.add_option( "-S", "--stations",
  			dest = "stations",
			type = "string",
  			help = "the station(s) to use [%default]" )
  hwgroup.add_option( "-P", "--partition",
  			dest = "partition",
			type = "string",
			default = "R00-M0-N00-256",
  			help = "name of the BlueGene partition [%default]" )
  parser.add_option_group( hwgroup )

  # parse arguments
  (options, args) = parser.parse_args()
  errorOccurred = False

  assert options.partition in PartitionPsets

  if not options.status and not options.checkStation and not options.kill and not options.allocate and not options.free:
    parser.print_help()
    sys.exit(0)

  if options.kill:
    errorOccured = errorOccurred or killJobs( options.partition )

  if options.free:
    errorOccured = errorOccurred or freePartition( options.partition )

  if options.allocate:
    errorOccured = errorOccurred or allocatePartition( options.partition )

  if options.status:
    expected_owner = os.environ["USER"]
    real_owner = owner( options.partition )

    print "Partition Owner : %-40s %s" % (real_owner,okstr(real_owner == expected_owner))

    expected_job = None
    real_job = runningJob( options.partition )

    print "Running Job     : %-40s %s" % (real_job,okstr(real_job == expected_job))

    print "Receiving boards:"
    allinfo = allStationInfo( options.partition )
    for s in allinfo:
      station, boardnr, dest, beamlets, nrsamples = s
      print "\t%s, board %s -> %s (%s beamlets)" % (station,boardnr,dest,beamlets)

    print "Receiving stations:"
    allinputs = map( lambda x: x[2], allinfo )
    foundstations = []
    for name,inputs in Stations.iteritems():
      # flatten inputs
      stationinputs = sum( map( lambda x: x.inputs, inputs ), [] )

      numfound = len( [i for i in stationinputs if i in allinputs] )
      if numfound == len(stationinputs):
        foundstations.append( name )
    print "\t%s" % (" ".join(sorted(foundstations)))


  # check stations if requested so
  expected_owner = os.environ["USER"]
  real_owner = owner( options.partition )

  for stationName in options.checkStation:
    if stationName not in Stations:
      # unknown station
      errorOccurred = True
      print "%-25s NOK Station name unknown" % (stationName,)
      continue

    status,missingInputs = receivingStation( Stations[stationName] )

    if not status:
      # missing an input
      errorOccurred = True
      print missingInputs
      print "%-25s NOK Missing input(s) %s" % (stationName,", ".join( ("%s @ %s" % m for m in missingInputs) ))
      continue
      
    status,missingInputs = stationInPartition( Stations[stationName], options.partition )

    if not status:
      # input outside partition
      errorOccurred = True
      print missingInputs
      print "%-25s NOK Input(s) outside partition %s: %s" % (stationName,options.partition,", ".join( ("%s @ %s" % m for m in missingInputs) ))
      continue

    if expected_owner != real_owner:
      # not our partition so not our data to receive
      errorOccurred = True
      print "%-25s NOK Partition %s is assigned to %s, not to you" % (stationName,options.partition,real_owner)
      continue

    print "%-25s OK" % (stationName,)
   
  sys.exit(int(errorOccurred))
