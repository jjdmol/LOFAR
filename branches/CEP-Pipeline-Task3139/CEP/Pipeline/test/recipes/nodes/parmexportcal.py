from __future__ import with_statement
import os
import unittest
import tempfile
import sys
import shutil

from argparse import ArgumentTypeError

from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from lofarpipe.recipes.nodes.parmexportcal import ParmExportCal
#import from fixtures:
from logger import logger

class ParmExportCalWrapper(ParmExportCal):
    """
    The test wrapper allows overwriting of function with muck functionality
    """
    def __init__(self, name):
        """
        """
        super(ParmExportCalWrapper, self).__init__(name)
        self.logger = logger()

class ParmExportCalTest(unittest.TestCase):
    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(ParmExportCalTest, self).__init__(arg)

    def setUp(self):
        self.tempDir = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.tempDir)

    def test_convert_data_to_ComplexArray_real_imag(self):
        data = [[0], [1]]
        type_pair = ["Image", "Real"]  # Order is alphabetical
        parmExportCal = ParmExportCalWrapper()
        complex_array = parmExportCal._convert_data_to_ComplexArray(data, type_pair)

        list_of_names = list_stations("test")
        goal_set = ["name1", "name2", "name3", "name4", "station1"]
        self.assertTrue(list_of_names == goal_set, "{0} != {1}".format(
                    list_of_names, goal_set))
