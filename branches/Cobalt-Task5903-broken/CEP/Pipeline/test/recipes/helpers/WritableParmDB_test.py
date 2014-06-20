from __future__ import with_statement
import os
import unittest
import tempfile
import sys
import shutil

from argparse import ArgumentTypeError

from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from lofarpipe.recipes.helpers.WritableParmDB import WritableParmDB          #@UnresolvedImport
from lofarpipe.recipes.helpers.WritableParmDB import list_stations
#import from fixtures:


class writableParmdbWrapper(WritableParmDB):
    """
    The test wrapper allows overwriting of function with muck functionality
    """
    def __init__(self, name):
        """
        """
        super(writableParmdbWrapper, self).__init__(name)


class writableParmdbTest(unittest.TestCase):
    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(writableParmdbTest, self).__init__(arg)

    def setUp(self):
        self.tempDir = tempfile.mkdtemp()
        self.tempParmDB = os.path.join(self.tempDir, "parmDB")
        self.writable_parmdb = writableParmdbWrapper(self.tempParmDB)

    def tearDown(self):
        shutil.rmtree(self.tempDir)
        # close the parmdb connection
        self.writable_parmdb = None

    def test_list_stations_get_names_from_path(self):
        list_of_names = list_stations("test")
        goal_set = ["name1", "name2", "name3", "name4", "station1"]
        self.assertTrue(list_of_names == goal_set, "{0} != {1}".format(
                    list_of_names, goal_set))

    def test_list_stations_get_names_from_writableParmdb(self):
        list_of_names = list_stations(self.writable_parmdb)
        goal_set = ["name1", "name2", "name3", "name4", "station1"]
        self.assertTrue(list_of_names == goal_set, "{0} != {1}".format(
                    list_of_names, goal_set))

    def test_list_stations_get_names_from_list(self):
        self.assertRaises(ArgumentTypeError,
            list_stations, ["List station called with list"])
        #python 2.6 does not allow testing of message

    def test_list_stations_get_names_with_pattern_filter(self):
        list_of_names = list_stations(self.writable_parmdb, pattern = "name2*")
        goal_set = ["name2"]
        self.assertTrue(list_of_names == goal_set, "{0} != {1}".format(
                    list_of_names, goal_set))

    def test_get_name_specific_filter(self):
        list_of_names = self.writable_parmdb.getNames(parmnamepattern = "Gain:*:*:*:*")
        goal_set = ["Gain:1:2:Real:station1"]
        self.assertTrue(list_of_names == goal_set, "{0} != {1}".format(
                    list_of_names, goal_set))



