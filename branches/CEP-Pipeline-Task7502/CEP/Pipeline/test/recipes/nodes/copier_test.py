from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile
import socket

#imports from fixture:
from logger import logger

from lofarpipe.support.lofarexceptions import PipelineException
from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from lofarpipe.recipes.nodes.copier import copier as copier_node       #@UnresolvedImport


class copierWrapper_node(copier_node):
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
        pass
        #self.imager_create_dbs = copierWrapper()
        #create_directory(self.test_path)

    def tearDown(self):
        pass
        #shutil.rmtree(self.test_path)

#    # refactor of copier means this test is not needed anymore. Comment out for 
#    # now
#    def test_copy_to_unowned_dir(self):
#        """
#        Test for bug reported in Task  #3375: Copier can not copy to unowned files
#        If trying to copy to dir without write throw usefull IOError, in stead of 
#        rsync return value
#        
#        """
#        temp_dir = tempfile.mkdtemp()
#        path_to_unowned_dir = "/home/klijntest/testdir"  #
#        file_to_copy = open(os.path.join(temp_dir, "test.txt"), 'w')
#        file_to_copy.close()
#
#        source_map_file = open(os.path.join(temp_dir, "source.map"), 'w')
#        source_map_file.write(repr(
#                              [(socket.gethostname(), os.path.join(temp_dir, "test.txt"))]
#                              ))
#        source_map_file.close()
#
#        target_map_file = open(os.path.join(temp_dir, "target.map"), 'w')
#        target_map_file.write(repr(
#                              [(socket.gethostname(), os.path.join(path_to_unowned_dir,
#                                                           "test.txt"))]
#                              ))
#        target_map_file.close()
#
#        sut = copierWrapper_node()
#        # error msg =  Failed to (rsync) copy file: /tmp/tmpdWhBbM/test.txt on node nodename
#        self.assertRaises(IOError, sut.run,
#        os.path.join(temp_dir, "source.map"), os.path.join(temp_dir, "target.map"))



if __name__ == "__main__":
    unittest.main()
