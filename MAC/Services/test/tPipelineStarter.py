#!/usr/bin/env python

# Be able to find service python file
import sys


import lofar.mac.PipelineStarter as module
from lofar.sas.otdb.OTDBBusListener import OTDBBusListener
from lofar.messaging import ToBus, Service, EventMessagea

from methodtrigger import MethodTrigger

import subprocess
import unittest
import uuid
import datetime

import logging
logging.basicConfig(stream=sys.stdout, level=logging.INFO)

try:
  from mock import patch
except ImportError:
  print "Cannot run test without python MagicMock"
  print "Call 'pip install mock' / 'apt-get install python-mock'"
  exit(3)

def setUpModule():
  pass

def tearDownModule():
  pass

class TestRunCommand(unittest.TestCase):
  def test_basic(self):
    """ Test whether we can run a trivial command. """
    module.runCommand("true")

  def test_invalid_command(self):
    """ Test whether an invalid command produces an error. """
    with self.assertRaises(subprocess.CalledProcessError):
      output = module.runCommand(".")

  def test_shell(self):
    """ Test whether the command is parsed by a shell. """
    module.runCommand("true --version")

  def test_output(self):
    """ Test whether we catch the command output correctly. """
    output = module.runCommand("echo yes")
    self.assertEqual(output, "yes")

  def test_input(self):
    """ Test whether we can provide input. """
    output = module.runCommand("cat -", "yes")
    self.assertEqual(output, "yes")

class TestSlurmJobInfo(unittest.TestCase):
  def test_no_jobs(self):
    """ Test 'scontrol show job' output if there are no jobs. """
    with patch('lofar.mac.PipelineStarter.runSlurmCommand') as MockRunSlurmCommand:
      MockRunSlurmCommand.return_value = """No jobs in the system"""

      self.assertEqual(module.getSlurmJobInfo(), {})

  def test_one_job(self):
    """ Test 'scontrol show job' output for a single job. """
    with patch('lofar.mac.PipelineStarter.runSlurmCommand') as MockRunSlurmCommand:
      MockRunSlurmCommand.return_value = """JobId=119 JobName=foo UserId=mol(7261) GroupId=mol(7261) Priority=4294901736 Nice=0 Account=(null) QOS=(null) JobState=RUNNING Reason=None Dependency=(null) Requeue=1 Restarts=0 BatchFlag=0 Reboot=0 ExitCode=0:0 RunTime=00:00:07 TimeLimit=UNLIMITED TimeMin=N/A SubmitTime=2016-03-04T12:05:52 EligibleTime=2016-03-04T12:05:52 StartTime=2016-03-04T12:05:52 EndTime=Unknown PreemptTime=None SuspendTime=None SecsPreSuspend=0 Partition=cpu AllocNode:Sid=thead01:7040 ReqNodeList=(null) ExcNodeList=(null) NodeList=tcpu[01-02] BatchHost=tcpu01 NumNodes=2 NumCPUs=2 CPUs/Task=1 ReqB:S:C:T=0:0:*:* TRES=cpu=2,mem=6000,node=2 Socks/Node=* NtasksPerN:B:S:C=0:0:*:* CoreSpec=* MinCPUsNode=1 MinMemoryNode=3000M MinTmpDiskNode=0 Features=(null) Gres=(null) Reservation=(null) Shared=OK Contiguous=0 Licenses=(null) Network=(null) Command=(null) WorkDir=/home/mol Power= SICP=0"""

      jobs = module.getSlurmJobInfo()
      self.assertEqual(jobs["foo"]["JobName"], "foo")
      self.assertEqual(jobs["foo"]["JobId"], "119")

  def test_two_jobs(self):
    """ Test 'scontrol show job' output for multiple jobs. """
    with patch('lofar.mac.PipelineStarter.runSlurmCommand') as MockRunSlurmCommand:
      MockRunSlurmCommand.return_value = """JobId=120 JobName=foo UserId=mol(7261) GroupId=mol(7261) Priority=4294901735 Nice=0 Account=(null) QOS=(null) JobState=RUNNING Reason=None Dependency=(null) Requeue=1 Restarts=0 BatchFlag=0 Reboot=0 ExitCode=0:0 RunTime=00:00:17 TimeLimit=UNLIMITED TimeMin=N/A SubmitTime=2016-03-04T12:09:53 EligibleTime=2016-03-04T12:09:53 StartTime=2016-03-04T12:09:53 EndTime=Unknown PreemptTime=None SuspendTime=None SecsPreSuspend=0 Partition=cpu AllocNode:Sid=thead01:7250 ReqNodeList=(null) ExcNodeList=(null) NodeList=tcpu[01-02] BatchHost=tcpu01 NumNodes=2 NumCPUs=2 CPUs/Task=1 ReqB:S:C:T=0:0:*:* TRES=cpu=2,mem=6000,node=2 Socks/Node=* NtasksPerN:B:S:C=0:0:*:* CoreSpec=* MinCPUsNode=1 MinMemoryNode=3000M MinTmpDiskNode=0 Features=(null) Gres=(null) Reservation=(null) Shared=OK Contiguous=0 Licenses=(null) Network=(null) Command=(null) WorkDir=/home/mol Power= SICP=0
    JobId=121 JobName=bar UserId=mol(7261) GroupId=mol(7261) Priority=4294901734 Nice=0 Account=(null) QOS=(null) JobState=PENDING Reason=Resources Dependency=(null) Requeue=1 Restarts=0 BatchFlag=0 Reboot=0 ExitCode=0:0 RunTime=00:00:00 TimeLimit=UNLIMITED TimeMin=N/A SubmitTime=2016-03-04T12:09:59 EligibleTime=2016-03-04T12:09:59 StartTime=2017-03-04T12:09:53 EndTime=Unknown PreemptTime=None SuspendTime=None SecsPreSuspend=0 Partition=cpu AllocNode:Sid=thead01:7250 ReqNodeList=(null) ExcNodeList=(null) NodeList=(null) NumNodes=2-2 NumCPUs=2 CPUs/Task=1 ReqB:S:C:T=0:0:*:* TRES=cpu=2,node=2 Socks/Node=* NtasksPerN:B:S:C=0:0:*:* CoreSpec=* MinCPUsNode=1 MinMemoryNode=0 MinTmpDiskNode=0 Features=(null) Gres=(null) Reservation=(null) Shared=OK Contiguous=0 Licenses=(null) Network=(null) Command=(null) WorkDir=/home/mol Power= SICP=0"""

      jobs = module.getSlurmJobInfo()
      self.assertEqual(jobs["foo"]["JobName"], "foo")
      self.assertEqual(jobs["foo"]["JobId"], "120")

      self.assertEqual(jobs["bar"]["JobName"], "bar")
      self.assertEqual(jobs["bar"]["JobId"], "121")

class TestPipelineStarterClassMethods(unittest.TestCase):
  def test_shouldHandle(self):
    """ Test whether we filter the right OTDB trees. """

    trials = [ { "type": "Observation", "cluster": "CEP2", "shouldHandle": False },
               { "type": "Observation", "cluster": "CEP4", "shouldHandle": False },
               { "type": "Observation", "cluster": "foo",  "shouldHandle": False },
               { "type": "Observation", "cluster": "",     "shouldHandle": False },
               { "type": "Pipeline",    "cluster": "CEP2", "shouldHandle": False },
               { "type": "Pipeline",    "cluster": "CEP4", "shouldHandle": True },
               { "type": "Pipeline",    "cluster": "foo",  "shouldHandle": True },
               { "type": "Pipeline",    "cluster": "",     "shouldHandle": False },
             ]

    for t in trials:
      parset = { "ObsSW.Observation.processType": t["type"],
                 "ObsSW.Observation.Cluster.ProcessingCluster.clusterName": t["cluster"] }
      self.assertEqual(module.PipelineStarter._shouldHandle(parset), t["shouldHandle"])

class TestPipelineStarter(unittest.TestCase):
  def setUp(self):
    # Create a random bus
    self.busname = "%s-%s" % (sys.argv[0], str(uuid.uuid4())[:8])
    self.bus = ToBus(self.busname, { "create": "always", "delete": "always", "node": { "type": "topic" } })
    self.bus.open()
    self.addCleanup(self.bus.close)

    # Patch SLURM
    def _runSlurmCommand(cmdline, **kwargs):
      print "SLURM call: %s" % cmdline

      return ""

    patcher = patch('lofar.mac.PipelineStarter.runSlurmCommand')
    patcher.start().side_effect = _runSlurmCommand
    self.addCleanup(patcher.stop)

    patcher = patch('lofar.mac.PipelineStarter.getSlurmJobInfo')
    patcher.start().return_value = {
      "1": { "JobName": "1" },
      "2": { "JobName": "2" },
      "3": { "JobName": "3" },
    }
    self.addCleanup(patcher.stop)

    # Catch functions to prevent running executables
    patcher = patch('lofar.mac.PipelineStarter.Parset.dockerTag')
    patcher.start().return_value = "trunk"
    self.addCleanup(patcher.stop)

    # ================================
    # Setup mock parset service
    # ================================

    def TaskSpecificationService( OtdbID ):
      print "***** TaskSpecificationService(%s) *****" % (OtdbID,)

      if OtdbID == 1:
        predecessors = "[2,3]"
      elif OtdbID == 2:
        predecessors = "[3]"
      elif OtdbID == 3:
        predecessors = "[]"
      else:
        raise Exception("Invalid OtdbID: %s" % OtdbID)

      return {
        "Version.number":                                                           "1",
        module.PARSET_PREFIX + "Observation.ObsID":                                 str(OtdbID),
        module.PARSET_PREFIX + "Observation.Scheduler.predecessors":                predecessors,
        module.PARSET_PREFIX + "Observation.processType":                           "Pipeline",
        module.PARSET_PREFIX + "Observation.Cluster.ProcessingCluster.clusterName": "CEP4",
      }

    service = Service("TaskSpecification", TaskSpecificationService, busname=self.busname)
    service.start_listening()
    self.addCleanup(service.stop_listening)

    # ================================
    # Setup mock status update service
    # ================================

    def StatusUpdateCmd( OtdbID, NewStatus ):
      print "***** StatusUpdateCmd(%s,%s) *****" % (OtdbID, NewStatus)

      # Broadcast the state change
      content = { "treeID" : OtdbID, "state" : NewStatus, "time_of_change" : datetime.datetime.utcnow() }
      msg = EventMessage(context="otdb.treestatus", content=content)
      self.bus.send(msg)

    service = Service("StatusUpdateCmd", StatusUpdateCmd, busname=self.busname)
    service.start_listening()
    self.addCleanup(service.stop_listening)

    # ================================
    # Setup listener to catch result
    # of our service
    # ================================

    listener = OTDBBusListener(busname=self.busname)
    listener.start_listening()
    self.addCleanup(listener.stop_listening)

    self.trigger = MethodTrigger(listener, "onObservationQueued")

  def testNoPredecessors(self):
    """
      Request the resources for a simulated obsid 3, with the following predecessor tree:

        3 requires nothing
    """
    with module.PipelineStarter(otdb_busname=self.busname, setStatus_busname=self.busname) as ps:
      # Send fake status update
      ps._setStatus(3, "scheduled")

      # Wait for message to arrive
      self.assertTrue(self.trigger.wait())

      # Verify message
      self.assertEqual(self.trigger.args[0], 3) # treeId

  def testPredecessors(self):
    """
      Request the resources for a simulated obsid 3, with the following predecessor tree:

        1 requires 2, 3
    """
    with module.PipelineStarter(otdb_busname=self.busname, setStatus_busname=self.busname) as ps:
      # Send fake status update
      ps._setStatus(1, "scheduled")

      # Wait for message to arrive
      self.assertTrue(self.trigger.wait())

      # Verify message
      self.assertEqual(self.trigger.args[0], 1) # treeId

def main(argv):
  unittest.main(verbosity=2)

if __name__ == "__main__":
  # run all tests
  import sys
  main(sys.argv[1:])

