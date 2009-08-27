#!/usr/bin/env python

from util.Commands import SyncCommand,AsyncCommand,mpikill,backquote
from util.Aborter import runUntilSuccess,runFunc
from Locations import Locations
import os
import BGcontrol
import Partitions
import ObservationID
from Logger import debug,info,warning

DEBUG=False

class Section:
  """ A 'section' is a set of commands which together perform a certain function. """

  def __init__(self, parsets, partition):
    self.parsets = parsets
    self.commands = []


    self.logoutputs = []
    if Locations.nodes["logserver"]:
      self.logoutputs.append( "%s" % (Locations.nodes["logserver"],) )

    self.partition = partition
    self.psets     = Partitions.PartitionPsets[self.partition]
    self.obsid     = ObservationID.ObservationID.generateID()

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
    logfiles = ["%s/run.CNProc.%s.log" % (Locations.files["logdir"],self.partition)] + self.logoutputs

    # CNProc is started on the Blue Gene, which has BG/P mpirun 1.65
    # NOTE: This mpirun needs either stdin or stdout to be line buffered,
    # otherwise it will not provide output from CN_Processing (why?).
    mpiparams = [
      # where
      "-mode VN",
      "-partition %s" % (self.partition,),

      # environment
      "-env DCMF_COLLECTIVES=0",
      "-env BG_MAPPING=XYZT",
      "-env LD_LIBRARY_PATH=/bgsys/drivers/ppcfloor/comm/lib:/bgsys/drivers/ppcfloor/runtime/SPI:/cephome/romein/lib.bgp",

      # working directory
      "-cwd %s" % (Locations.files["rundir"],),

      # executable
      "-exe %s" % (Locations.files["cnproc"],),

      # arguments
      "-args %s" % (" ".join([p.getFilename() for p in self.parsets]), ),
    ]

    self.commands.append( AsyncCommand( "mpirun %s" % (" ".join(mpiparams),), logfiles, killcmd=mpikill ) )

  def check(self):
    # we have to own the partition
    owner = BGcontrol.owner( self.partition )
    me = os.environ["USER"]

    assert owner is not None, "Partition %s is not allocated." % ( self.partition, )
    assert owner == me, "Partition %s owned by %s, but you are %s." % ( self.partition, owner, me )

    # no job needs to be running on the partition
    job = BGcontrol.runningJob( self.partition )

    assert job is None, "Partition %s already running job %s (%s)." % ( self.partition, job[0], job[1] )

class IONProcSection(Section):
  def run(self):
    logfiles = ["%s/run.IONProc.%s.log" % (Locations.files["logdir"],self.partition)] + self.logoutputs

    mpiparams = [
      # where
      "-host %s" % (",".join(self.psets),),
      "--pernode",

      "--tag-output",

      # environment

      # working directory
      "-wd %s" % (Locations.files["rundir"],),

      # executable
      "%s" % (Locations.files["ionproc"],),

      # arguments
      "%s" % (" ".join([p.getFilename() for p in self.parsets]), ),
    ]

    if DEBUG:
      mpiparams = [
        "-v",
      ] + mpiparams

    self.commands.append( AsyncCommand( "/bgsys/LOFAR/openmpi-ion/bin/mpirun %s" % (" ".join(mpiparams),), logfiles ) )

  def check(self):
    # I/O nodes need to be reachable -- check in parallel

    # ping
    pingcmds = [
      (node,AsyncCommand( "ping %s -c 1 -w 2 -q" % (node,), ["/dev/null"] ))
      for node in self.psets
    ]

    for (node,c) in pingcmds:
      assert c.isSuccess(), "Cannot reach I/O node %s [ping]" % (node,)

    # ssh
    sshcmds = [
      (node,AsyncCommand( "ssh %s /bin/true" % (node,), ["/dev/null"] ))
      for node in self.psets
    ]

    def waitForSuccess():
      for (node,c) in sshcmds:
        assert c.isSuccess(), "Cannot reach I/O node %s [ssh]" % (node,)

    if not runFunc( waitForSuccess, 10 ):
      for (node,c) in sshcmds:
        assert c.isDone(), "Cannot reach I/O node %s [ssh]" % (node,)

    # test flatmem access
    for node in self.psets:
      assert "FAIL" not in backquote("ssh %s cat /dev/flatmem 2>&1 | grep -q 'Cannot allocate memory' && echo FAIL" % (node,)), "Cannot allocate flat memory on I/O node %s" % (node,)

class StorageSection(Section):
  def run(self):
    logfiles = ["%s/run.Storage.%s.log" % (Locations.files["logdir"],self.partition)] + self.logoutputs

    # the PID of mpirun
    self.pidfile = "%s/Storage-%s.pid" % (Locations.files["rundir"],self.partition)

    # unique identifier to locate the mpi processes
    self.universe = "OLAP-%s" % (self.obsid,)

    # create the target directories
    for n in Locations.nodes["storage"]:
      # use the first parset for the mask
      self.commands.append( SyncCommand( "ssh -q %s mkdir %s" % (n,os.path.dirname(self.parsets[0].parseMask()),), logfiles ) )

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
      "%s" % (" ".join([p.getFilename() for p in self.parsets]), ),
    ]

    self.commands.append( AsyncCommand( "ssh -q %s echo $$ > %s;exec mpirun %s" % (Locations.nodes["storagemaster"],self.pidfile," ".join(mpiparams),), logfiles ) )

  def abort( self, timeout ):
    # kill mpirun using the pid we stored locally
    def kill( signal ):
      SyncCommand( "ssh -q %s kill -%s `cat %s`" % (Locations.nodes["storagemaster"],signal,self.pidfile) )

    self.killSequence( "mpirun process on %s" % (Locations.nodes["storagemaster"],), kill, timeout )

    # kill Storage and orted processes on storage nodes
    for node in Locations.nodes["storage"]:
      def kill( signal ):
        # We kill the process group rooted at the orted process
        # (kill -PID) belonging to our MPI universe. This should take Storage with it.
        SyncCommand( "ssh -q %s ps --no-heading -o pid,cmd -ww -C orted | grep %s | awk '{ print $1; }' | xargs -I foo kill -%s -foo" % (
          node, self.universe, signal) )

        # Sometimes it does not though, so send Storage (identified by parset file on command line, which is unique) the same signal
        SyncCommand( "ssh -q %s ps --no-heading -o pid,cmd -ww -C Storage | grep '%s' | awk '{ print $1; }' | xargs -I foo kill -%s -foo" % (
          node, Locations.files["parset"], signal) )

      self.killSequence( "orted/Storage processes on %s" % (node,), kill, timeout )

    # fallback: kill local commands
    if not runUntilSuccess( [ self.wait ], timeout ):
      Section.abort( self )

  def check( self ):
    storagePorts = self.parsets[0].getStoragePorts()

    for node,neededPorts in storagePorts.iteritems():
      usedPorts = map( int, backquote( """ssh %s netstat -nta | awk 'NR>2 { n=split($4,f,":"); print f[n]; }'""" % (node,) ).split() )

      cannotOpen = [p for p in neededPorts if p in usedPorts]

      assert cannotOpen == [], "Storage: Cannot open ports %s on node %s." % (",".join(sorted(map(str,cannotOpen))), node)
