from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile


import pyrap.tables as tb                                                       #@UnresolvedImport
from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from lofarpipe.recipes.nodes.imager_create_dbs import imager_create_dbs         #@UnresolvedImport
from logger import logger


class ImagerPrepareTestWrapper(imager_create_dbs):
    """
    Wrapper for the imager_create_dbs allows overwriting of 
    """
    def __init__(self):
        """
        Overloaded __init__ function, hiding the original __init__ on 
        LOFARnodeTCP.
        """
        self.logger = logger()

class ImagerPrepareTest(unittest.TestCase):
    """
    Tests for ImagerCreateDBs class 
    """
    test_path = "/data/scratch/python_unittest"

    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(ImagerCreateDBsTest, self).__init__(arg)

    def setUp(self):
        self.imager_create_dbs = ImagerCreateDBsTestWrapper()
        create_directory(self.test_path)

    def tearDown(self):
        shutil.rmtree(self.test_path)

if __name__ == "__main__":
    unittest.main()