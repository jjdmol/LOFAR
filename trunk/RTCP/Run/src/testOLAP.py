#!/usr/bin/python

from LOFAR import Logger
from logging import debug,info,warning,error,critical
from startOLAP import runCorrelator
from runParset import convertParsets
from LOFAR.Parset import Parset
from LOFAR.CommandClient import sendCommand
from LOFAR.Locations import Locations
from threading import Thread
from time import sleep
import os
import sys

class ParsetTester:
  """ Tests a parset by starting the correlator in a separate process and inserting the parset. 

      Usage example:

         pt = ParsetTester( partition, logdir, "name of test" )
         pt.readParset( "parset=RTCP.parset" )
         pt.runParset()
         pt.validate( [NoErrors()] )
  """       

  def __init__( self, partition, testname ):
    self.partition = partition
    self.testname = testname

    self.parset = Parset()
    self.olapparset_filename = "%s/OLAP.parset" % (os.path.dirname(__file__),)

    # configure the correlator before the parsets are added such that they will use the right paths
    testfilename = self.testname

    for c in " :":
      testfilename = testfilename.replace(c,"_")

    self.logdir = "%s/%s" % (Locations.files["logdir"],testfilename)

    self.results = {
      "started":    False,
      "terminated": False,
      "logdir":     self.logdir,
    }

  def readParset( self, argstr, override_keys = {} ):
    self.parset = convertParsets( [argstr], self.olapparset_filename, self.partition, override_keys )[0]

  def runParset( self, starttimeout = 30, runtimeout = 300, stoptimeout = 60 ):
    self.results["started"] = True

    class CorrelatorThread(Thread):
      def __init__(self,partition):
        Thread.__init__(self)
        self.partition = partition

      def run(self):
        runCorrelator( self.partition )

    info( "********** Starting test '%s' **********" % (self.testname,) )

    try:
      # start the correlator (ni a separate process to allow full control in runCorrelator
      info( "Starting correlator." )
      pid = os.fork()
      if pid == 0:
        # child process
        try:
          Locations.files["logdir"] = self.logdir
          info("Logdir = %s" % (self.logdir,))
          os.makedirs( self.logdir )

          runCorrelator( self.partition )
        except:
          error( "Correlator aborted." )
          os._exit(1)
        else:  
          info( "Correlator stopped." )
          os._exit(0)
      else:
        # parent process
        for i in xrange( starttimeout ):
          sleep( 1 )
          try:
            sendCommand( self.partition, "" )
          except:
            continue
          else:  
            break
        else:
          raise Exception("Correlator did not start.")

      # inject the parset
      self.parset.setFilename( Locations.resolvePath( "%s/RTCP-${MSNUMBER}.parset" % (self.logdir,), self.parset ) );
      info( "Sending parset '%s' to correlator." % (self.parset.filename,) )
      self.parset.save()
      sendCommand( self.partition, "parset %s" % (self.parset.filename,) )

      # quit immediately after processing the parset
      sendCommand( self.partition, "quit" )

      # wait for correlator to finish
      def isStopped():
        ret = os.waitpid( pid, os.WNOHANG )

        if ret[1] > 0:
          raise Exception("Correlator did not start.")

        return ret != (0,0)

      for i in xrange( runtimeout + stoptimeout ):
        sleep( 1 )

        if isStopped():
          stopped = True
          break
      else:  
        stopped = False

      # process outcome
      if stopped:
        info( "Correlator terminated succesfully." );
        self.results["terminated"] = True
      else:  
        error( "Correlator did not terminate." );
    except Exception,e:
      error( "Exception: %s" % (e,) )

  def validate(self,validators,continue_on_error = False):
     """ Run a set of validators on the logfiles produced by an earlier runParset run. """

     valid = True
     logfiles = ["run.CNProc.log","run.IONProc.log"]

     for v in validators:
       v.begin()

     for f in logfiles:
       fname = "%s/%s" % (self.results["logdir"],f)

       try:
         fd = file(fname)
       except IOError,e:
         error( "Could not open %s: %s" % (fname,e) )
         valid = False
       else:
         for linenr,l in enumerate(fd):
           try:
             v.parse(l)
           except ValidationError,e:
             error( "Validation error in %s:%s: %s" % (fname,linenr,e) )
             error( "Offending line: %s" % (l,) )
             valid = False

             if not continue_on_error:
               return

     for v in validators:
       try:
         v.end()
       except ValidationError,e:
         error( "Validation error in %s: %s" % (fname,e) )
         valid = False

         if not continue_on_error:
           return

     if valid and self.results["terminated"]:
       info( "********** Test '%s': OK **********" % (self.testname,) )
       return True
     else:  
       error( "********** Test '%s': Not OK **********" % (self.testname,) )
       return False

class ValidationError(Exception):
  pass

class LogValidator:
  """ Validates a log file. """

  def __init__(self):
    self.ok = True

  def begin(self):
    pass

  def parse(self,line):
    parts = line.split(" ",4)
    if len(parts) != 5:
      self.parseLine(line)
    else:
      self.parseLogLine(*parts)

  def parseLogLine(self,proc,date,time,level,msg):
    pass

  def parseLine(self,line):
    pass

  def end(self):
    pass

  def valid(self):
    return self.ok

class AlwaysValid(LogValidator): 
  """ Considers a log to be always valid. """
  pass

class NoErrors(LogValidator):
  """ Considers a log valid if there are no errors or exceptions. """

  def parseLogLine(self,proc,date,time,level,msg):
    if level in ["FATAL","ERROR","EXCEPTION"]:
      raise ValidationError( "Encountered an %s" % (level,) )

def testParset( testname, partition, argstr, override_keys = {}, validators = [], ignore_errors = False ):
  """ Test and validate a parset.

      testname:                 name of the test
      partition:                name of the BG/P partition
      argstr:                   string specifying the observation
      override_keys:            parset keys which override those in the parset
      validators:               set of validators to run on the logfiles
      ignore_errors:            if true, continue validating if an error is encountered """

  pt = ParsetTester( partition, testname )
  pt.readParset( argstr, override_keys )
  pt.runParset()

  return pt.validate( validators or [NoErrors()], ignore_errors )

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
