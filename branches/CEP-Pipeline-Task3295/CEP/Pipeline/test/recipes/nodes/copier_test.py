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
from lofarpipe.recipes.nodes.copier import copier       #@UnresolvedImport



class copierWrapper(copier):
    """
    Wrapper for the imager_create_dbs allows overwriting of 
    """
    def __init__(self):
        """
        Overloaded __init__ function, hiding the original __init__ on 
        LOFARnodeTCP.
        """
        self.logger = logger()

class copierTest(unittest.TestCase):

    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(copierTest, self).__init__(arg)

    def setUp(self):
        self.imager_create_dbs = copierWrapper()
        #create_directory(self.test_path)

    def tearDown(self):
        pass
        #shutil.rmtree(self.test_path)



if __name__ == "__main__":
    unittest.main()
