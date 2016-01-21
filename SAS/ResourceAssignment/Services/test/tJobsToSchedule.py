#!/usr/bin/env python

# Be able to find service python file
import sys, os
sys.path.insert(0, "{srcdir}/../src".format(**os.environ))

from JobsToSchedule import *
from lofar.parameterset import PyParameterSet
from lofar.messaging import EventMessage, Service

import unittest
from glob import glob
import uuid

import logging
logging.basicConfig(stream=sys.stdout, level=logging.INFO)

def setUpModule():
  pass

def tearDownModule():
  pass


class TestGetPredecessors(unittest.TestCase):
  def test_0_predecessors(self):
    parset = { PARSET_PREFIX + "Observation.Scheduler.predecessors": "[]" }

    self.assertEqual(predecessors(parset), [])

  def test_1_predecessor(self):
    parset = { PARSET_PREFIX + "Observation.Scheduler.predecessors": "[L426528]" }

    self.assertEqual(predecessors(parset), [426528])

  def test_2_predecessors(self):
    parset = { PARSET_PREFIX + "Observation.Scheduler.predecessors": "[L426528,L1]" }

    self.assertEqual(sorted(predecessors(parset)), [1,426528])


def parset_as_dict(filename):
    parset = PyParameterSet(filename, False)
    d = {}
    for k in parset.keywords():
      d[k] = parset.getString(k)

    return d


class TestResourceIndicators(unittest.TestCase):
  """
    The spec for the resource indicators is a draft at this point,
    and the output is quite extensive (many parset keys), so
    verification of the output is pending.
  """

  def test_preprocessing_pipeline(self):
    parset = parset_as_dict("tJobsToSchedule.in_preprocessing")
    r = resourceIndicatorsFromParset(parset)

  def test_correlator_observation(self):
    parset = parset_as_dict("tJobsToSchedule.in_correlator")
    r = resourceIndicatorsFromParset(parset)


class TestService(unittest.TestCase):
  def setUp(self):
    # Create a random bus
    self.busname = "%s-%s" % (sys.argv[0], str(uuid.uuid4())[:8])
    self.bus = ToBus(self.busname + '; { create: "always", delete: "always", node: { type: "topic" }}')
    self.bus.open()

    # Define the services we use
    self.status_service = "%s/TaskStatus" % (self.busname,)
    self.parset_service = "%s/TaskSpecification" % (self.busname,)
    self.jts_service    = "%s/TaskSpecified" % (self.busname,)

    # setup mock parset service
    def TaskSpecificationService( input_dict ):
      obsid = input_dict["OtdbID"]

      if obsid == 1:
        predecessors = "[2,3]"
      elif obsid == 2:
        predecessors = "[3]"
      elif obsid == 3:
        predecessors = "[]"
      else:
        raise Exception("Invalid obsID")

      return {
        "Version.number":                                     "1",
        PARSET_PREFIX + "Observation.ObsID":                  str(obsid),
        PARSET_PREFIX + "Observation.Scheduler.predecessors": predecessors,
      }


    # Setup our fake TaskSpecification server, and start our JobsToSchedule service to test
    self.parset_service = Service("TaskSpecification", TaskSpecificationService, busname=self.busname)
    self.parset_service.start_listening()

  def tearDown(self):
    self.parset_service.stop_listening()
    self.bus.close()

  def testNoPredecessors(self):
    """
      Request the resources for a simulated obsid 3, with the following predecessor tree:

        3 requires nothing
    """

    with JobsToSchedule("TaskSpecified", otdb_busname=self.busname, my_busname=self.busname) as jts:
      # Start listening for answer before we trigger it
      with FromBus(self.jts_service) as fb:

        # Send fake status update
        with ToBus(self.status_service) as tb:
          msg = EventMessage(content={
            "treeID": 3,
            "state": "prescheduled",
            "time_of_change": "2016-01-01 00:00:00.00",
          })
          tb.send(msg)

        # Wait for answer from service
        result = fb.receive(1.0)
        self.assertIsNotNone(result)

        # Verify result
        self.assertIn("sasID", result.content)
        self.assertIn("resource_indicators", result.content)

        self.assertEqual(result.content["sasID"], 3)
        self.assertNotIn("1", result.content["resource_indicators"])
        self.assertNotIn("2", result.content["resource_indicators"])
        self.assertIn("3", result.content["resource_indicators"])

  def testPredecessors(self):
    """
      Request the resources for a simulated obsid 1, with the following predecessor tree:

        1 requires 2, 3
        2 requires 3
        3 requires nothing
    """

    with JobsToSchedule("TaskSpecified", otdb_busname=self.busname, my_busname=self.busname) as jts:
      # Start listening for answer before we trigger it
      with FromBus(self.jts_service) as fb:

        # Send fake status update
        with ToBus(self.status_service) as tb:
          msg = EventMessage(content={
            "treeID": 1,
            "state": "prescheduled",
            "time_of_change": "2016-01-01 00:00:00.00",
          })
          tb.send(msg)

        # Wait for answer from service
        result = fb.receive(1.0)
        self.assertIsNotNone(result)

        # Verify result
        self.assertIn("sasID", result.content)
        self.assertIn("resource_indicators", result.content)

        self.assertEqual(result.content["sasID"], 1)
        self.assertIn("1", result.content["resource_indicators"])
        self.assertIn("2", result.content["resource_indicators"])
        self.assertIn("3", result.content["resource_indicators"])

def main(argv):
  unittest.main(verbosity=2)

if __name__ == "__main__":
  # run all tests
  import sys
  main(sys.argv[1:])
