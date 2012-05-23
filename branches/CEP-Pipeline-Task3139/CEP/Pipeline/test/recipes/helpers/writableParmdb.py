from __future__ import with_statement
import os
import unittest
import tempfile

from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from lofarpipe.recipes.helpers.writableParmdb import writableParmdb          #@UnresolvedImport

#import from fixtures:


class writableParmdbWrapper(writableParmdb):
    """
    The test wrapper allows overwriting of function with muck functionality
    """
    def __init__(self, name):
        """
        """
        super(writableParmdbWrapper, self).__init__(name)


class writableParmdbTest(unittest.TestCase):


    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(ImagerCreateDBsTest, self).__init__(arg)

    def setUp(self):
        self.tempDir = tempfile.mkdtemp()
        self.tempParmDB = os.path.join(self.tempDir, "parmDB")
        self.imager_create_dbs = writableParmdbWrapper(self.tempParmDB)

    def tearDown(self):
        shutil.rmtree(self.tempParmDB)
        # close the parmdb connection
        self.imager_create_dbs = None
