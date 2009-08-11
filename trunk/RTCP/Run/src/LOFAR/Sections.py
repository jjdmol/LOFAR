#!/usr/bin/env python

from util.Commands import SyncCommand,AsyncCommand,mpikill
from util.Aborter import runUntilSuccess
from Locations import Locations
import os
import BGcontrol
from Logger import debug,info,warning

class Section:
  """ A 'section' is a set of commands which together perform a certain function. """

  def __init__(self, parset):
    self.parset = parset
    self.commands = []

    self.logoutputs = []
    if Locations.nodes["logserver"]:
      self.logoutputs.append( "tcp:%s" % (Locations.nodes["logserver"],) )

  def __str__(self):
    return self.__class__.__name__

  def run(self):
    pass

  def postProcess(self):
    pass

  def killSequence(self,name,killfunc,timeout):
    killfuncs = [ lambda: killfunc(2), lambda: killfunc(9) ]

    success = runUntilSuccess( killfuncs, timeout )

    if not success:
      warning( "%s: Could not kill %s" % (self,name) )

    return success

  def killCommand(self,cmd,timeout):
    def kill( signal ):
      cmd.abort( signal )
      cmd.wait()

    return self.killSequence(str(cmd),kill,timeout)

  def abort(self,timeout):
    for c in self.commands:
      self.killCommand(c,timeout)

  def wait(self):
    for c in self.commands:
      c.wait()

  def check(self):
    pass

class SectionSet(list):
  def run(self):
    for s in self:
      info( "Starting %s." % (s,) )
      s.run()

  def postProcess(self):
    for s in self:
      info( "Post processing %s." % (s,) )
      s.postProcess()


  def abort(self, timeout=5):
    for s in self:
      info( "Killing %s." % (s,) )
      s.abort(timeout)

  def wait(self):
    for s in self:
      info( "Waiting for %s." % (s,) )
      s.wait()

  def check(self):
    for s in self:
      info( "Checking %s for validity." % (s,) )
      s.check()

class CNProcSection(Section):
  def run(self):
    logfiles = ["%s/run.CNProc.%s.log" % (Locations.files["logdir"],self.parset.partition)] + self.logoutputs

    # CNProc is started on the Blue Gene, which has BG/P mpirun 1.65
    # NOTE: This mpirun needs either stdin or stdout to be line buffered,
    # otherwise it will not provide output from CN_Processing (why?).
    mpiparams = [
     # where
      "-mode VN",
      "-partition %s" % (self.parset.partition,),

      # environment
      "-env DCMF_COLLECTIVES=0",
      "-env BG_MAPPING=XYZT",
      "-env LD_LIBRARY_PATH=/bgsys/drivers/ppcfloor/comm/lib:/bgsys/drivers/ppcfloor/runtime/SPI:/cephome/romein/lib.bgp",

      # working directory
      "-cwd %s" % (Locations.files["rundir"],),

      # executable
      "-exe %s" % (Locations.files["cnproc"],),

      # arguments
      "-args %s" % (Locations.files["parset"],),
    ]

    self.commands.append( AsyncCommand( "mpirun %s" % (" ".join(mpiparams),), logfiles, killcmd=mpikill ) )

  def check(self):
    # we have to own the partition
    owner = BGcontrol.owner( self.parset.partition )
    me = os.environ["USER"]

    assert owner is not None, "Partition %s is not allocated." % ( self.parset.partition, )
    assert owner == me, "Partition %s owned by %s, but you are %s." % ( self.parset.partition, owner, me )

    # no job needs to be running on the partition
    job = BGcontrol.runningJob( self.parset.partition )

    assert job is None, "Partition %s already running job %s (%s)." % ( self.parset.partition, job[0], job[1] )

class IONProcSection(Section):
  def run(self):
    self.pidfiles = {}

    ionodes = self.parset.psets

    for node in ionodes:
      nodenr = node.split(".")[-1]

      # the PID of the IONProcs
      self.pidfiles[node] = "%s/IONProc-%s-%s.pid" % (Locations.files["rundir"],self.parset.partition,nodenr)

      # change working directory to the rundir, remember the pid of the shell, and execute IONProc
      cmd = "cd %s; echo $$ > %s; exec %s %s" % (
         Locations.files["rundir"],
         self.pidfiles[node],
         Locations.files["ionproc"],Locations.files["parset"] )

      logfiles = ["%s/run.IONProc.%s.%s" % (Locations.files["logdir"],self.parset.partition,nodenr)] + self.logoutputs

      self.commands.append( AsyncCommand( "ssh %s %s" % (node,cmd,), logfiles ) )

  def abort( self, timeout ):
    for node,pidfile in self.pidfiles.iteritems():
      def kill( signal ):
        # kill remote process using the pid we stored
        SyncCommand( "ssh %s kill -%s `cat %s`" % (node,signal,pidfile) )

      self.killSequence( "IONProc process on %s" % (node,), kill, timeout )

    # fallback: kill local commands
    if not runUntilSuccess( [ self.wait ], timeout ):
      Section.abort( self )

  def check(self):
    # I/O nodes need to be reachable
    ionodes = self.parset.psets

    for node in ionodes:
      c = SyncCommand( "ping %s -c 1 -w 2 -q" % (node,), ["/dev/null"] )
      assert c.isSuccess(), "Cannot reach I/O node %s" % (node,)

class StorageSection(Section):
  def run(self):
    logfiles = ["%s/run.Storage.%s.log" % (Locations.files["logdir"],self.parset.partition)] + self.logoutputs

    # the PID of mpirun
    self.pidfile = "%s/Storage-%s.pid" % (Locations.files["rundir"],self.parset.partition)

    # unique identifier to locate the mpi processes
    self.universe = "OLAP-%s" % (self.parset.getObsID(),)

    # create the target directories
    for n in Locations.nodes["storage"]:
      self.commands.append( SyncCommand( "ssh %s mkdir %s" % (n,os.path.dirname(self.parset.parseMask()),), logfiles ) )

    # Storage is started on LIIFEN, which has mpirun (Open MPI) 1.1.1
    mpiparams = [
      # provide this run with an unique name to identify the corresponding
      # processes on the storage nodes
      "-universe %s" % (self.universe,),

      # where
      "-host %s" % (",".join(Locations.nodes["storage"]),),

      "-np %s" % (len(Locations.nodes["storage"]),),

      # environment

      # working directory
      "-wdir %s" % (Locations.files["rundir"],),

      # executable
      "%s" % (Locations.files["storage"],),

      # arguments
      "%s" % (Locations.files["parset"],),
    ]

    self.commands.append( AsyncCommand( "ssh %s echo $$ > %s;exec mpirun %s" % (Locations.nodes["storagemaster"],self.pidfile," ".join(mpiparams),), logfiles ) )

  def abort( self, timeout ):
    # kill mpirun using the pid we stored locally
    def kill( signal ):
      SyncCommand( "ssh %s kill -%s `cat %s`" % (Locations.nodes["storagemaster"],signal,self.pidfile) )

    self.killSequence( "mpirun process on %s" % (Locations.nodes["storagemaster"],), kill, timeout )

    # kill Storage and orted processes on storage nodes
    for node in Locations.nodes["storage"]:
      # We kill the process group rooted at the orted process
      # (kill -PID) belonging to our MPI universe. This takes Storage with it.
      def kill( signal ):
        SyncCommand( "ssh %s kill -%s -`ps --no-heading -o pid,cmd -ww -C orted | grep %s | awk '{ print $1; }'`" % (
          node, signal, self.universe) )

      self.killSequence( "orted process on %s" % (node,), kill, timeout )

    # fallback: kill local commands
    if not runUntilSuccess( [ self.wait ], timeout ):
      Section.abort( self )
