#!/usr/bin/env python

from LOFAR import Logger
from LOFAR.ObservationID import ObservationID
from LOFAR.Logger import debug,info,warning,error,fatal
from LOFAR.Parset import Parset
from util.Hosts import rsymlink
from LOFAR.Stations import Stations
from LOFAR.Locations import Locations
from LOFAR.CommandClient import sendCommand
from util.dateutil import format
import sys
import socket

DRYRUN = False

def print_exception( str ):
  import traceback

  print >>sys.stderr, str

  traceback.print_exc()

def convertParsets( args, olapparset, partition = None ):
  # valid observation parameters
  validObsParams = [
    "parset", "output", "stations", "tcp", "null",
    "start", "stop", "run",
    "clock", "integration"
  ]

  def isValidObsParam( key ):
    return key in validObsParams or "." in key

  # default observation parameters
  defaultObsParams = {
    "parset": "RTCP.parset",
    "start": "+15",
    "output": Locations.files["parset"],
  }
  defaultRunTime = "00:01:00"

  # define no default run or stop time in defaultObsParams,
  # otherwise it will always overrule the parset
  assert "stop" not in defaultObsParams
  assert "run" not in defaultObsParams

  # ========== Observations
  parsets = []
  for obsIndex,obs in enumerate(args):
    info( "===== Parsing observation %s: %s =====" % (obsIndex,obs,) )

    # parse and check the observation parameters
    def splitparam( s ):
      """ Convert a parameter which is either 'key=value' or 'key' into a
          key,value tuple. """

      if "=" in s:
        return s.split("=",1)

      return (s,None)

    obsparams = defaultObsParams.copy()
    obsparams.update( dict( map( splitparam, obs.split(",") ) ) )

    for p in obsparams:
      if not isValidObsParam( p ):
        fatal("Unknown observation parameter '%s'" % (p,))

    if "parset" not in obsparams:
      fatal("Observation '%s' does not define a parset file." % (obs,))

    parset = Parset()
    parsets.append( parset )

    # read the parset file
    def addParset( f ):
      info( "Reading parset %s..." % (f,) )
      parset.readFile( Locations.resolvePath( f ) )

    try:
      addParset( olapparset )
      addParset( obsparams["parset"] )  
    except IOError,e:
      fatal("ERROR: Cannot read parset file: %s" % (e,))

    # parse specific parset values from the command line (all options containing ".")
    # apply them so that we will parse them instead of the parset values
    for k,v in obsparams.iteritems():
      if "." not in k:
        continue

      parset.parse( "%s=%s" % (k,v) )

    # reserve an observation id
    parset.postRead()

    if parset.getObsID():
      info( "Distilled observation ID %s from parset." % (parset.getObsID(),) )
    else:  
      try:
        obsid = ObservationID().generateID()
      except IOError,e:
        print_exception("Could not generate observation ID: %s" % (e,))
        sys.exit(1)  

      parset.setObsID( obsid )

      info( "Generated observation ID %s" % (obsid,) )

    # override parset with command-line values
    if partition:
      parset.setPartition( partition )
    else:
      parset.setPartition( parset.distillPartition() )

    # set stations
    if "stations" in obsparams:
      stationList = Stations.parse( obsparams["stations"] )

      parset.forceStations( stationList )
    else:
      stationList = parset.stations

    if "tcp" in obsparams:
      # turn all inputs into tcp:*
      def tcpify( input ):
        if input.startswith("tcp:") or input.startswith("file:"):
          return input
        elif input.startswith("udp:"):
          return "tcp:"+input[4:]
        else:
          return "tcp:"+input

      for s in stationList:
        s.inputs = map( tcpify, s.inputs )

    if "null" in obsparams:
      # turn all inputs into null:
      for s in stationList:
        s.inputs = ["null:"] * len(s.inputs)

    parset.setStations( stationList )

    # set runtime
    if "start" in obsparams and "stop" in obsparams:
      parset.setStartStopTime( obsparams["start"], obsparams["stop"] )
    elif "start" in obsparams and "run" in obsparams:
      parset.setStartRunTime( obsparams["start"], obsparams["run"] )
    elif "Observation.startTime" in parset and "Observation.stopTime" in parset:
      parset.setStartStopTime( parset["Observation.startTime"], parset["Observation.stopTime"] )
    else:  
      parset.setStartRunTime( obsparams["start"], defaultRunTime )

    info( "Running from %s to %s." % (parset["Observation.startTime"], parset["Observation.stopTime"] ) )
    info( "Partition     = %s" % (parset.partition,) )
    info( "Storage Nodes = %s" % (", ".join(parset.storagenodes),) )
    info( "Stations      = %s" % (", ".join([s.name for s in parset.stations]),) )

    # set a few other options
    configmap = {
      "clock": parset.setClock,
      "integration": parset.setIntegrationTime,
    }

    for k,v in configmap.iteritems():
      if k in obsparams:
        v( obsparams[k] )

    # reapply them in case they got overwritten
    for k,v in obsparams.iteritems():
      if "." not in k:
        continue

      parset.parse( "%s=%s" % (k,v) )

  # finalise and save parsets
  usedStoragePorts = []

  for obsIndex,parset in enumerate(parsets):
    parset.disableStoragePorts( usedStoragePorts )

    # parse final settings (derive some extra keys)
    parset.preWrite()

    # finalise() allocates the ports that will be used, so don't use them for other observations
    for ports in parset.getStoragePorts().itervalues():
      usedStoragePorts.extend( ports )

    # sanity check on parset
    parset.check()

    parset.setFilename( Locations.resolvePath( obsparams["output"], parset ) )

  return parsets

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import os
  import time

  # parse the command line
  parser = OptionParser( usage = """usage: %prog [options] observation [observation] ...

    'observation' is a comma-separated list of the following options:

    parset=name       (mandatory) the filename of the parset to read
    output=name       the filename of the parset to write (default: RCTP-(obsid).parset in latest logdir)
    stations=xxx+yyy  use stations xxx and yyy
    tcp               station input arrives over TCP
    null              station input is generated from null:

    start=xxx         start/stop time. allowed syntax:
    stop=xxx            [YYYY-MM-DD] HH:MM[:SS]
                        timestamp
                        +seconds
                        +HH:MM[:SS]
    run=xxx           run time. allowed syntax:
                        HH:MM[:SS]
                        seconds

    clock=xxx         use a station clock of xxx MHz
    integration=xxx   use xxx seconds of integration time on the IO node

    foo.bar=value     override parset key 'foo.bar' with 'value'.
    """ )

  opgroup = OptionGroup(parser, "Output" )
  opgroup.add_option( "-v", "--verbose",
  			dest = "verbose",
			action = "store_true",
			default = False,
  			help = "be verbose [%default]" )
  opgroup.add_option( "-q", "--quiet",
  			dest = "quiet",
			action = "store_true",
			default = False,
  			help = "be quiet [%default]" )
  parser.add_option_group( opgroup )

  hwgroup = OptionGroup(parser, "Settings" )
  hwgroup.add_option( "-O", "--olap-parset",
  			dest = "olapparset",
			type = "string",
                        default = "%s/OLAP.parset" % (os.path.dirname(__file__),),
  			help = "the parset containing station definitions [%default]" )
  hwgroup.add_option( "-P", "--partition",
  			dest = "partition",
			type = "string",
  			help = "name of the BlueGene partition [%default]" )
  hwgroup.add_option( "-n", "--norun",
  			dest = "norun",
			action = "store_true",
			default = False,
  			help = "do not send the parset to the correlator" )
  parser.add_option_group( hwgroup )

  # parse arguments
  (options, args) = parser.parse_args()

  # ========== Global options

  if not args:
    parser.print_help()
    sys.exit(1)

  if options.verbose:
    Logger.DEBUG = True

  if not options.quiet:
    DEBUG = True
    Logger.VERBOSE = True

  # read and convert parsets
  parsets = convertParsets( args, options.olapparset, options.partition )  

  # output them to stdout or file
  info( "========== Saving parsets ==========" )
  for parset in parsets:
    info( "Saving parset to %s" % (parset.filename,) )
    parset.save()

    if not options.norun:
      try:
        obsDir = os.path.dirname( os.path.realpath( parset.filename ) )
        symlinkName = Locations.resolvePath( Locations.files["obssymlink"], parset )
        rsymlink( symlinkName, obsDir )

        info( "Created symlink %s -> %s" % (symlinkName, obsDir) )
      except IOError,msg:
        warning( "Failed to create symlink %s -> %s" % (symlinkName,obsDir) )

      info( "Sending parset %s to the correlator on partition %s" % (parset.filename,parset.partition) )
      try:
        sendCommand( options.partition, "parset %s" % (parset.filename,) )
      except socket.error,msg:
        error( "Failed to connect to correlator on partition %s: %s" % (parset.partition,msg) )

  info( "========== Done ==========" )
