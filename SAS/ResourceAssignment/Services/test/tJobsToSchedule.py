#!/usr/bin/env python

# Be able to find service python file
import sys, os
sys.path.insert(0, "{srcdir}/../src".format(**os.environ))
from JobsToSchedule import *
from lofar.parameterset import PyParameterSet

import unittest
from glob import glob

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
  def test_foo(self):
    parset = parset_as_dict("tJobsToSchedule.in_426942")

    print resourceIndicatorsFromParset(parset)


def main(argv):
  unittest.main(verbosity=2)

if __name__ == "__main__":
  # run all tests
  import sys
  main(sys.argv[1:])
