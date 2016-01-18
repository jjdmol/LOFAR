#!/usr/bin/env python

import unittest
import sys

from lofar.sas.resourceassignement.resourceassigner.ramodule1 import answerToEverything

def setUpModule():
    # here we can setup stuff we need once per module testing
    pass


def tearDownModule():
    # here we can tear down stuff we need once per module testing
    pass


class RATest1(unittest.TestCase):
    '''Test the logic in the ResourceAssignmentEditor web service'''

    def test1(self):
        self.assertEqual(42, answerToEverything())

def main(argv):
    unittest.main(verbosity=2)

# run all tests if main
if __name__ == '__main__':
    main(sys.argv[1:])
