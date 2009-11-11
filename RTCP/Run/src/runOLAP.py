#!/usr/bin/env python

from LOFAR import Logger
from LOFAR.ObservationID import ObservationID
from LOFAR.Logger import debug,info,warning,error,fatal
from LOFAR import Sections
from LOFAR.Parset import Parset
from LOFAR.Stations import Stations
from util import Commands
from util.dateutil import format
from LOFAR.Locations import Locations,isDevelopment
from util.Hosts import ropen,rmkdir,rexists,runlink,rsymlink
import sys

DRYRUN = False

def print_exception( str ):
  import traceback

  print >>sys.stderr, str

  traceback.print_exc()

def extend_basedir( basedir, path ):
  """ Resolve a path relative to a certain base directory. """

  if path[:1] == "/":
    return path
  else:
    return "%s/%s" % (basedir,path)

def runObservations( parsets, partition, start_cnproc = True, start_ionproc = True, start_storage = True ):
  """ Run an observation using the provided parsets. """

  # ----- Select the sections to start
  sections = Sections.SectionSet()

  if start_ionproc:
    sections += [Sections.IONProcSection( parsets, partition )]
  if start_cnproc:
    sections += [Sections.CNProcSection( parsets, partition )]
  if start_storage:
    sections += [Sections.StorageSection( parsets, partition )]

  # sanity check on sections
  sections.check()

  # ----- Run all sections
  try:
    # start all sections
    sections.run()

    # wait for all sections to complete
    sections.wait()
  except KeyboardInterrupt:
    # abort all sections
    sections.abort()

  # let the sections clean up 
  sections.postProcess()

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import os
  import time

  # default observation parameters
  defaultObsParams = {
    "parset": "RTCP.parset",
    "start": "+15",
  }
  defaultRunTime = "00:01:00"

  # define no default run or stop time in defaultObsParams,
  # otherwise it will always overrule the parset
  assert "stop" not in defaultObsParams
  assert "run" not in defaultObsParams

  # parse the command line
  parser = OptionParser( usage = """usage: %prog [options] observation [observation] ...

    'observation' is a comma-separated list of the following options:

    parset=name       (mandatory) the filename of the parset to use
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
  parser.add_option( "-d", "--dry-run",
  			dest = "dryrun",
			action = "store_true",
			default = False,
  			help = "do not actually execute anything [%default]" )
  parser.add_option( "--valgrind-ion",
  			dest = "valgrind_ion",
			action = "store_true",
			default = False,
  			help = "run IONProc under valgrind [%default]" )
  parser.add_option( "--valgrind-storage",
  			dest = "valgrind_storage",
			action = "store_true",
			default = False,
  			help = "run Storage under valgrind [%default]" )

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
  opgroup.add_option( "-l", "--log-server",
  			dest = "logserver",
			type = "string",
			default = Locations.nodes["logserver"],
  			help = "TCP address (IP:port) to send logging to [%default]" )
  parser.add_option_group( opgroup )

  hwgroup = OptionGroup(parser, "Hardware" )
  hwgroup.add_option( "-O", "--olap-parset",
  			dest = "olapparset",
			type = "string",
                        default = "%s/OLAP.parset" % (os.path.dirname(__file__),),
  			help = "the parset containing station definitions [%default]" )
  hwgroup.add_option( "-P", "--partition",
  			dest = "partition",
			type = "string",
  			help = "name of the BlueGene partition [%default]" )
  hwgroup.add_option( "--storageprocs",
  			dest = "storageprocs",
			type = "int",
                        default = 1,
  			help = "number of processes per storage node [%default]" )
  hwgroup.add_option( "-M", "--storage-master",
  			dest = "storagemaster",
			type = "string",
			default = Locations.nodes["storagemaster"],
  			help = "Front-end for storage nodes [%default]" )
  parser.add_option_group( hwgroup )

  secgroup = OptionGroup(parser, "Sections" )
  secgroup.add_option( "--nocnproc",
  			dest = "nocnproc",
			action = "store_true",
			default = False,
			help = "disable the CNProc section [%default]" )
  secgroup.add_option( "--noionproc",
  			dest = "noionproc",
			action = "store_true",
			default = False,
			help = "disable the IONProc section [%default]" )
  secgroup.add_option( "--nostorage",
  			dest = "nostorage",
			action = "store_true",
			default = False,
			help = "disable the storage section [%default]" )
  parser.add_option_group( secgroup )

  dirgroup = OptionGroup(parser, "Directory and file locations")
  dirgroup.add_option( "--basedir",
  			dest = "basedir",
			default = Locations.files["basedir"],
			help = "base directory [%default]" )
  dirgroup.add_option( "--logdir",
  			dest = "logdir",
			default = Locations.files["logdir"],
			help = "log directory (syntax: [host:]path) [%default]" )
  dirgroup.add_option( "--rundir",
  			dest = "rundir",
			default = Locations.files["rundir"],
			help = "run directory [%default]" )
  dirgroup.add_option( "--cnproc",
  			dest = "cnproc",
			default = Locations.files["cnproc"],
			help = "CNProc executable [%default]" )
  dirgroup.add_option( "--ionproc",
  			dest = "ionproc",
			default = Locations.files["ionproc"],
			help = "IONProc executable [%default]" )
  dirgroup.add_option( "--storage",
  			dest = "storage",
			default = Locations.files["storage"],
			help = "Storage executable [%default]" )
  dirgroup.add_option( "--valgrind-ion-suppressions",
  			dest = "ionsuppfile",
			default = Locations.files["ionsuppfile"],
  			help = "Valgrind suppressions file for IONProc [%default]" )
  dirgroup.add_option( "--valgrind-storage-suppressions",
  			dest = "storagesuppfile",
			default = Locations.files["storagesuppfile"],
  			help = "Valgrind suppressions file for Storage [%default]" )

  parser.add_option_group( dirgroup )

  # parse arguments
  (options, args) = parser.parse_args()

  # ========== Global options

  if not args:
    parser.print_help()
    sys.exit(1)

  if options.verbose:
    Commands.debug = debug
    Logger.DEBUG = True
    Sections.DEBUG = True

  if options.valgrind_ion:
    Sections.VALGRIND_ION = True

  if options.valgrind_storage:
    Sections.VALGRIND_STORAGE = True

  if not options.quiet:
    DEBUG = True
    Sections.DEBUG = True
    Logger.VERBOSE = True

  if options.dryrun:
    DRYRUN = True
    Commands.DRYRUN = True
    ObservationID.DRYRUN = True

  Locations.nodes["storagemaster"] = options.storagemaster

  # set log server
  Locations.nodes["logserver"] = options.logserver

  # ========== Observations
  parsets = []
  for obsIndex,obs in enumerate(args):
    info( "===== Parsing observation %s: %s =====" % (obsIndex,obs,) )

    def splitparam( s ):
      """ Convert a parameter which is either 'key=value' or 'key' into a
          key,value tuple. """

      if "=" in s:
        return s.split("=",1)

      return (s,None)

    obsparams = defaultObsParams.copy()
    obsparams.update( dict( map( splitparam, obs.split(",") ) ) )

    if "parset" not in obsparams:
      fatal("Observation '%s' does not define a parset file." % (obs,))

    parset = Parset()
    parsets.append( parset )

    # read the parset file
    def addParset( f ):
      info( "Reading parset %s..." % (f,) )
      parset.readFile( Locations.resolvePath( f ) )

    try:
      addParset( options.olapparset )
      addParset( obsparams["parset"] )  
    except IOError,e:
      fatal("ERROR: Cannot read parset file: %s" % (e,))

    # reserve an observation id
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
    if options.partition is None:
      options.partition = parset.distillPartition()

      if options.partition:
        info( "Distilled partition %s from parset." % (options.partition,) )
      else:
        fatal( "No BlueGene partition selected on command line or in first parset." )
    elif parset.distillPartition() not in ["",options.partition]:
      # make sure all parsets use the same partition
      fatal( "Parset selects partition %s, but partition %s was already selected." % (parset.distillPartition(),options.partition) )
    parset.setPartition( options.partition )

    # define storage nodes
    if not options.nostorage:
      storagenodes = parset.distillStorageNodes()

      if storagenodes:
        info( "Distilled storage nodes %s from parset." % (storagenodes,) )
      else:
        fatal( "No storage nodes selected in parset." )

      parset.setStorageNodes( storagenodes * options.storageprocs )

    # set stations
    if "stations" in obsparams:
      stationStr = obsparams["stations"]
    else:
      stationStr = parset.distillStations()

      if stationStr:
        info( "Distilled stations %s from parset." % (stationStr,) )
      else:
        fatal( "No stations or inputs selected on command line or in parset. " )

    stationList = Stations.parse( stationStr )

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

    # set a few other options
    configmap = {
      "clock": parset.setClock,
      "integration": parset.setIntegrationTime,
    }

    for k,v in configmap.iteritems():
      if k in obsparams:
        v( obsparams[k] )

    if options.valgrind_ion:
      # force settings required to run under valgrind
      parset["OLAP.OLAP_Conn.IONProc_CNProc_Transport"] = "TCP" # FCNP uses BG/P instructions
      parset["OLAP.nrSecondsOfBuffer"] = 1                      # reduce memory footprint

      # default to valgrind builds
      options.ionproc = "${BASEDIR}/installed/gnubgp_ion-valgrind/bin/IONProc"
      options.cnproc  = "${BASEDIR}/installed/gnubgp_cn-valgrind/bin/CN_Processing"

    # parse specific parset values from the command line (all options containing ".")
    for k,v in obsparams.iteritems():
      if "." not in k:
        continue

      parset[k] = v

    # disable sections we won't start
    if options.nocnproc:
      parset.disableCNProc() 
    if options.noionproc:
      parset.disableIONProc()
    if options.nostorage:
      parset.disableStorage()


  # resolve all paths now that parsets are set up and the observation ID is known
  for opt in dirgroup.option_list:
    Locations.setFilename( opt.dest, getattr( options, opt.dest ) )

  Locations.resolveAllPaths( parsets[0] ) 

  # create log and directory if it does not exist
  for d in ["logdir","rundir"]:
    if not rexists(Locations.files[d]):
      warning( "Creating %s directory %s" % ( d, Locations.files[d], ) )

      if not DRYRUN:
        rmkdir( Locations.files[d] )

  # create symlink to log directory in run directory
  log_symlink = {
    "source": "%s/log" % (Locations.files["rundir"],),
    "dest":   "%s"     % (Locations.files["logdir"],),
  }

  if not DRYRUN:
    try:
      if rexists( log_symlink["source"] ):
        runlink( log_symlink["source"] )

      rsymlink( log_symlink["source"], log_symlink["dest"] )
    except OSError,e:
      warning( "Could not create symlink %s pointing to %s" % (log_symlink["source"],log_symlink["dest"]) )

  # finalise and save parsets
  usedStoragePorts = []

  for obsIndex,parset in enumerate(parsets):
    if not options.nostorage:
      parset.disableStoragePorts( usedStoragePorts )

    # parse final settings (derive some extra keys)
    parset.finalise()

    # finalise() allocates the ports that will be used, so don't use them for other observations
    if not options.nostorage:
      for ports in parset.getStoragePorts().itervalues():
        usedStoragePorts.extend( ports )

    # sanity check on parset
    parset.check()

    # write parset to disk (both rundir and logdir)
    parset.setFilename( "%s.%s" % (Locations.files["parset"], obsIndex) )

    if not DRYRUN:
      parset.save()
      parset.writeFile( "%s/RTCP.parset.%s" % (Locations.files["logdir"],obsIndex) )

  # ========== Run
  info( "========== Run ==========" )

  runObservations( parsets, options.partition, not options.nocnproc, not options.noionproc, not options.nostorage )

  # ========== Clean up
  if not DRYRUN:
    for parset in parsets:
      try:
        runlink( parset.getFilename() )
      except OSError,e:
        warning( "Could not delete %s: %s" % (parset.getFilename(),e) )

  info( "========== Done ==========" )


