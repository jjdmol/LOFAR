#from __future__ import with_statement
#import os
#import errno
#import unittest
#import shutil
#import numpy
#import tempfile
#
#
#import pyrap.tables as tb                                                       #@UnresolvedImport
#from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
#from lofarpipe.recipes.nodes.imager_prepare import imager_prepare         #@UnresolvedImport
#from logger import logger
#
#
#class ImagerPrepareTestWrapper(imager_prepare):
#    """
#    Wrapper for the imager_prepare allows overwriting of class members
#    """
#    def __init__(self):
#        """
#        Overloaded __init__ function, hiding the original __init__ on 
#        LOFARnodeTCP.
#        """
#        self.logger = logger()
#
#class ImagerPrepareTest(unittest.TestCase):
#    """
#    Tests for imager_prepare class 
#    """
#    test_path = "/data/scratch/python_unittest"
#
#    def __init__(self, arg):  #todo deze moet toch in de setUp
#        super(ImagerPrepareTest, self).__init__(arg)
#
#    def setUp(self):
#        self.imager_create_dbs = ImagerPrepareTestWrapper()
#        create_directory(self.test_path)
#
#    def tearDown(self):
#        shutil.rmtree(self.test_path)
#        
#    def test_fail(self):
#        """
#        Test the calcultaion of the FOV for lowest freq on a hba core station 
#        """
#        self.assertTrue(False)
#        
#        
#        
#
#
#if __name__ == "__main__":
#    unittest.main()