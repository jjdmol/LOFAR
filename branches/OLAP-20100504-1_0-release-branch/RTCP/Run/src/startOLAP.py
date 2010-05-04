#!/usr/bin/python

from LOFAR import Logger
from LOFAR.Logger import debug,info,warning,error,fatal
from LOFAR import Sections
from util import Commands
from LOFAR.Locations import Locations
from LOFAR.CommandClient import sendCommand
from util.Hosts import ropen,rmkdir,rexists,runlink,rsymlink
import sys
import signal
from threading import Lock
import thread

DRYRUN = False

aborted = False
lock = Lock()
lock.acquire() # lock can be released by anyone to signal the end of the run

# translate signals to KeyboardInterrupts to catch them in a try block
def sigHandler( sig, frame ):
  global aborted

  fatal( "Caught signal %s -- aborting" % (sig,) )
  aborted = True

  try:
    lock.release()
  except thread.error:
    pass

signal.signal( signal.SIGTERM, sigHandler )
signal.signal( signal.SIGQUIT, sigHandler )
signal.signal( signal.SIGINT, sigHandler )

def runCorrelator( partition, start_cnproc = True, start_ionproc = True ):
  """ Run an observation using the provided parsets. """

  # ----- Select the sections to start
  sections = Sections.SectionSet()

  if start_ionproc:
    sections += [Sections.IONProcSection( partition )]
  if start_cnproc:
    sections += [Sections.CNProcSection( partition )]

  # sanity check on sections
  if not DRYRUN:
    sections.check()

  # ----- Run all sections
  try:
    # start all sections
    sections.run()

    # wait for all sections to complete
    sections.wait( lock )

    if aborted:
      raise "aborted"

  except:
    try:
      # soft abort -- wait for all observations to stop
      sendCommand( partition, "quit" )
    except: 
      # hard abort -- kill all sections
      sections.abort()

  # let the sections clean up 
  sections.postProcess()

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import os
  import time

  # parse the command line
  parser = OptionParser( usage = """usage: %prog -P partition [options]""" )
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
  dirgroup.add_option( "--valgrind-ion-suppressions",
  			dest = "ionsuppfile",
			default = Locations.files["ionsuppfile"],
  			help = "Valgrind suppressions file for IONProc [%default]" )
  parser.add_option_group( dirgroup )

  # parse arguments
  (options, args) = parser.parse_args()

  if not options.partition:
    parser.print_help()
    sys.exit(1)

  # ========== Global options

  if options.verbose:
    Commands.debug = debug
    Logger.DEBUG = True
    Sections.DEBUG = True

  if options.valgrind_ion:
    Sections.VALGRIND_ION = True

  if not options.quiet:
    DEBUG = True
    Logger.VERBOSE = True

  if options.dryrun:
    DRYRUN = True
    Commands.DRYRUN = True

  # set log server
  Locations.nodes["logserver"] = options.logserver

  for opt in dirgroup.option_list:
    Locations.setFilename( opt.dest, getattr( options, opt.dest ) )

  Locations.resolveAllPaths()

  # create log and directory if it does not exist
  for d in ["logdir","rundir"]:
    if not rexists(Locations.files[d]):
      warning( "Creating %s directory %s" % ( d, Locations.files[d], ) )

      if not DRYRUN:
        rmkdir( Locations.files[d] )

  # create symlink to log directory in run directory
  log_symlink = {
    "source": "%s" % (Locations.files["logsymlink"],),
    "dest":   "%s" % (Locations.files["logdir"],),
  }

  if not DRYRUN:
    try:
      if rexists( log_symlink["source"] ):
        runlink( log_symlink["source"] )

      rsymlink( log_symlink["source"], log_symlink["dest"] )
    except OSError,e:
      warning( "Could not create symlink %s pointing to %s" % (log_symlink["source"],log_symlink["dest"]) )

  # ========== Run
  info( "========== Run ==========" )

  runCorrelator( options.partition, not options.nocnproc, not options.noionproc )

  info( "========== Done ==========" )
