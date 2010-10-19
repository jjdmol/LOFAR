#!/usr/bin/env python

from util.Commands import SyncCommand,AsyncCommand,mpikill,backquote,PIPE
from util.Aborter import runUntilSuccess,runFunc
from Locations import Locations,Hosts
import os
import Partitions
import ObservationID
from Logger import debug,info,warning
from threading import Thread,Lock

DEBUG=False
DRYRUN=False
VALGRIND_ION=False
VALGRIND_STORAGE=False

SSH="ssh -o StrictHostKeyChecking=no -q "

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

    self.preProcess()

  def __str__(self):
    return self.__class__.__name__

  def preProcess(self):
    pass

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
    lock = Lock()
    commands = self.commands

    # wait in a separate thread to allow python to capture KeyboardInterrupts in the main thread
    class WaitThread(Thread):
      def run(self):
        for c in commands:
          c.wait()

        lock.release()  

    lock.acquire()
    WaitThread().start()      

    # wait for lock to be released by waiting thread
    lock.acquire()

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
      "-env LD_LIBRARY_PATH=/bgsys/drivers/ppcfloor/comm/lib:/bgsys/drivers/ppcfloor/runtime/SPI:/globalhome/romein/lib.bgp",

      # working directory
      "-cwd %s" % (Locations.files["rundir"],),

      # executable
      "-exe %s" % (Locations.files["cnproc"],),

      # arguments
    ]

    self.commands.append( AsyncCommand( "mpirun %s" % (" ".join(mpiparams),), logfiles, killcmd=mpikill ) )

class IONProcSection(Section):
  def run(self):
    logfiles = ["%s/run.IONProc.%s.log" % (Locations.files["logdir"],self.partition)] + self.logoutputs

    if VALGRIND_ION:
      valgrind = "/globalhome/mol/root-ppc/bin/valgrind --suppressions=%s --leak-check=full --show-reachable=no" % (Locations.files["ionsuppfile"],)
    else:
      valgrind = ""

    mpiparams = [
      # where
      "-host %s" % (",".join(self.psets),),
      "--pernode",

      "--tag-output",

      # environment
      # "-x FOO=BAR",

      # working directory
      "-wd %s" % (Locations.files["rundir"],),

      # valgrind
      valgrind,

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

    # ssh & flatmem access

    # cat /dev/flatmem returns "Invalid argument" if the memory is available,
    # and "Cannot allocate memory" if not.
    successStr = "cat: /dev/flatmem: Invalid argument"
    sshcmds = [
      (node,AsyncCommand( SSH+"%s cat /dev/flatmem 2>&1 | grep -F '%s'" % (node,successStr),PIPE) )
      for node in self.psets
    ]

    def waitForSuccess():
      for (node,c) in sshcmds:
        c.wait()

        assert successStr in c.output(), "Cannot allocate flat memory on I/O node %s" % (node,)

    assert runFunc( waitForSuccess, 20 ), "Failed to reach one or more I/O nodes [ssh]"

class StorageSection(Section):
  def run(self):
    logfiles = ["%s/run.Storage.%s.log" % (Locations.files["logdir"],self.partition)] + self.logoutputs

    # create the target directories
    for p in self.parsets:
      for n in p.storagenodes:
        self.commands.append( SyncCommand( SSH+"-t %s mkdir -p %s" % (Hosts.resolve(n,"back"),os.path.dirname(p.parseMask()),), logfiles ) )

    if VALGRIND_STORAGE:
      valgrind = "/globalhome/mol/root-ppc/bin/valgrind --suppressions=%s --leak-check=full --show-reachable=yes" % (Locations.files["storagesuppfile"],)
    else:
      valgrind = ""

    # Storage is started on LIIFEN, which has mpirun (Open MPI) 1.1.1
    storagenodes = self.parsets[0].storagenodes

    mpiparams = [
      # where
      "-host %s" % (",".join(storagenodes),),

      "-np %s" % (len(storagenodes),),

      # environment

      # working directory
      "-wdir %s" % (Locations.files["rundir"],),

      # valgrind
      valgrind,

      # executable
      "%s" % (Locations.files["storage"],),

      # arguments
      "%s" % (" ".join([p.getFilename() for p in self.parsets]), ),
    ]

    self.commands.append( AsyncCommand( SSH+"%s mpirun %s" % (Locations.nodes["storagemaster"]," ".join(mpiparams),), logfiles ) )

  def abort( self, timeout ):
    storagenodes = [Hosts.resolve(s,"back") for s in self.parsets[0].storagenodes]
    
    # kill mpirun using the pid we stored locally
    def kill( signal ):
      SyncCommand( SSH+"-t %s ps --no-heading -o pid,ppid,cmd -ww -C mpirun | grep -F '%s' | awk '{ print $1; if($2>1) { print $2; } }' | sort | uniq | xargs -I foo kill -%s foo" % (Locations.nodes["storagemaster"], self.parsets[0].getFilename(), signal) )

    self.killSequence( "mpirun process on %s" % (Locations.nodes["storagemaster"],), kill, timeout )

    # kill Storage and orted processes on storage nodes
    for signal in [2,9]:
      for node in storagenodes:
        def kill(signal):
          # Kill all Storage processes which have our parset on the command line, and their parents
          # the parent of Storage is either orted, or init (1) if orted died
          SyncCommand( SSH+"-t %s ps --no-heading -o pid,ppid,cmd -ww -C Storage | grep -F '%s' | awk '{ print $1; if($2>1) { print $2; } }' | sort | uniq | xargs -I foo kill -%s foo" % (
            node, self.parsets[0].getFilename(), signal) )

        runFunc( lambda: kill(signal), 20 )     

    # fallback: kill local commands
    if not runUntilSuccess( [ self.wait ], timeout ):
      Section.abort( self )

  def check( self ):
    storagenodes = [Hosts.resolve(s,"back") for s in self.parsets[0].storagenodes]

    # Storage nodes need to be reachable -- check in parallel

    # ping

    pingcmds = [
      (node,AsyncCommand( "ping %s -c 1 -w 2 -q" % (node,), ["/dev/null"] ))
      for node in storagenodes
    ]

    for (node,c) in pingcmds:
      assert c.isSuccess(), "Cannot reach Storage node %s [ping]" % (node,)

    # ssh

    sshcmds = [
      (node,AsyncCommand( SSH+"%s /bin/true" % (node,),PIPE) )
      for node in storagenodes
    ]

    def waitForSuccess():
      for (node,c) in sshcmds:
        c.wait()

    assert runFunc( waitForSuccess, 20 ), "Failed to reach one or more Storage nodes [ssh]"

    # ports need to be open

    storagePorts = self.parsets[0].getStoragePorts()

    for node,neededPorts in storagePorts.iteritems():
      usedPorts = [int(p) for p in backquote( SSH+""" %s netstat -nta | awk 'NR>2 { n=split($4,f,":"); print f[n]; }'""" % (Hosts.resolve(node,"back"),) ).split() if p.isdigit()]

      cannotOpen = [p for p in neededPorts if p in usedPorts]

      assert cannotOpen == [], "Storage: Cannot open ports %s on node %s." % (",".join(sorted(map(str,cannotOpen))), node)

