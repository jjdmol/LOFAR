#!/usr/bin/python

from LOFAR import Logger
from logging import debug,info,warning,error,critical
from LOFAR.ParsetTester import ParsetTester
from LOFAR.Locations import Locations
import os
import sys

if __name__ == "__main__":
  from optparse import OptionParser,OptionGroup
  import os
  import time

  # parse the command line
  parser = OptionParser( usage = """usage: %prog -P partition [options]""" )

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

  hwgroup = OptionGroup(parser, "Hardware" )
  hwgroup.add_option( "-P", "--partition",
                        dest = "partition",
                        type = "string",
                        help = "name of the BlueGene partition [%default]" )
  parser.add_option_group( hwgroup )

  dirgroup = OptionGroup(parser, "Directory and file locations")
  dirgroup.add_option( "--basedir",
  			dest = "basedir",
			default = Locations.files["basedir"],
			help = "base directory [%default]" )
  dirgroup.add_option( "--logdir",
  			dest = "logdir",
			default = "%s/VALIDATION-${TIMESTAMP}" % (os.getcwd(),),
			help = "log directory (syntax: [host:]path) [%default]" )
  dirgroup.add_option( "--rundir",
  			dest = "rundir",
			default = "${LOGDIR}",
			help = "run directory [%default]" )
  dirgroup.add_option( "--cnproc",
  			dest = "cnproc",
			default = Locations.files["cnproc"],
			help = "CNProc executable [%default]" )
  dirgroup.add_option( "--ionproc",
  			dest = "ionproc",
			default = Locations.files["ionproc"],
			help = "IONProc executable [%default]" )
  parser.add_option_group( dirgroup )

  # parse arguments
  (options, args) = parser.parse_args()

  if not options.partition:
    parser.print_help()
    sys.exit(1)

  Logger.initLogger()  

  for opt in dirgroup.option_list:
    Locations.setFilename( opt.dest, getattr( options, opt.dest ) )

  Locations.resolveAllPaths()

  logdir = Locations.files["logdir"]

  for arg in args:
    testParset( "test", options.partition, arg )
