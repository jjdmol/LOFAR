from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile


from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from lofarpipe.recipes.nodes.imager_bbs import imager_bbs         #@UnresolvedImport



class imager_bbsWrapper(imager_bbs):
    """
    Wrapper for the imager_bbs
    """
    def __init__(self):
        """
        Overloaded __init__ function, hiding the original __init__ on 
        LOFARnodeTCP.
        """
        self.logger = logger()

class imager_bbsTest(unittest.TestCase):
    """
    Tests for imager_bbs class 
    """
    #self.test_path = "/data/scratch/python_unittest"

    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(imager_bbsTest, self).__init__(arg)

    def setUp(self):
        self.test_path = tempfile.mkdtemp()

    def tearDown(self):
        pass

    def test_constructor(self):
        """
        When the measurement set is from an antenna with a name NOT
        containing either LBA or HBA en Exception should be trown
        """
        sut = imager_bbsWrapper()
        self.assertTrue(True)

if __name__ == "__main__":
    unittest.main()
