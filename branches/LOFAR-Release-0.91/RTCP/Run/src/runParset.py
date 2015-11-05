#!/usr/bin/env python

from LOFAR import Logger
from logging import debug,info,warning,error,critical
from LOFAR.Core import buildParset
from LOFAR.Parset import Parset
from util.Hosts import rsymlink
from LOFAR.Locations import Locations
from LOFAR.CommandClient import sendCommand
import sys
import socket

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
    nostorage         do not start storage processes

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
                        default = "%s/OLAP.parset" % (Locations.files["configdir"],),
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

  Logger.initLogger()  

  # read and convert parsets
  parsets = [buildParset( None, arg, Locations.resolvePath( options.olapparset ), options.partition ) for arg in args]
  for p in parsets:
    p.preWrite()
    p.check()

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
      except OSError,msg:
        warning( "Failed to create symlink %s -> %s" % (symlinkName,obsDir) )

      # save in separate location for IO nodes, to prevent contention for NFS drives with other processes
      parset.setFilename( Locations.resolvePath( Locations.files["parset-ion"], parset ) )
      info( "Saving parset to %s" % (parset.filename,) )
      parset.save()

      info( "Sending parset %s to the correlator on partition %s" % (parset.filename,parset.partition) )
      try:
        sendCommand( options.partition, "parset %s" % (parset.filename,) )
      except socket.error,msg:
        error( "Failed to connect to correlator on partition %s: %s" % (parset.partition,msg) )

  info( "========== Done ==========" )
