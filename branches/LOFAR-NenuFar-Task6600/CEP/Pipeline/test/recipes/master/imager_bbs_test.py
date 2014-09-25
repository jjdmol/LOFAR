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
from lofarpipe.recipes.master.imager_bbs import imager_bbs                              #@UnresolvedImport



class imager_bbsWrapper(imager_bbs):
    """
    Wrapper for the imager_create_dbs allows overwriting of 
    """
    def __init__(self):
        """
        Overloaded __init__ function, hiding the original __init__ on 
        LOFARnodeTCP.
        """
        self.logger = logger()

class imager_bbsTest(unittest.TestCase):
    """
    Does not contain testable functionality.
    Do leave the unittest class
    """

    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(imager_bbsTest, self).__init__(arg)

    def setUp(self):
        self.test_path = temp_path = tempfile.mkdtemp()

    def tearDown(self):
        #shutil.rmtree(self.test_path)
        pass

    def test_constructor(self):

        sut = imager_bbsWrapper()
        self.assertTrue(True)

