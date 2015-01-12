from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile

#imports from fixture:
from logger import logger

from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from lofarpipe.recipes.master.imager_create_dbs import imager_create_dbs                              #@UnresolvedImport



class image_create_dbsWrapper(imager_create_dbs):
    """
    Wrapper for the imager_create_dbs allows overwriting of 
    """
    def __init__(self):
        """
        Overloaded __init__ function, hiding the original __init__ on 
        LOFARnodeTCP.
        """
        self.logger = logger()

class imager_create_dbsTest(unittest.TestCase):
    """
    Does not contain testable functionality.
    Do leave the unittest class
    """

    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(imager_create_dbsTest, self).__init__(arg)

    def setUp(self):
        self.test_path = temp_path = tempfile.mkdtemp()

    def tearDown(self):
        #shutil.rmtree(self.test_path)
        pass

    def test_validate_input_data(self):

        sut = image_create_dbsWrapper()
        self.assertTrue(True)

