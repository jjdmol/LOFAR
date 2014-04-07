from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile


import pyrap.tables as tb                                                       #@UnresolvedImport
from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from nodes.imager_prepare import imager_prepare         #@UnresolvedImport
from logger import logger



class ImagerPrepareTestWrapper(imager_prepare):
    """
    Wrapper for the imager_prepare allows overwriting of class members
    """
    def __init__(self):
        """
        Overloaded __init__ function, hiding the original __init__ on 
        LOFARnodeTCP.
        """
        self.logger = logger()


class ImagerPrepareTest(unittest.TestCase):
    """
    Tests for imager_prepare class 
    """
    test_path = "/data/scratch/python_unittest"

    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(ImagerPrepareTest, self).__init__(arg)

    def setUp(self):
        self.imager_create_dbs = ImagerPrepareTestWrapper()
        create_directory(self.test_path)

    def tearDown(self):
        shutil.rmtree(self.test_path)

    def test__copy_input_files_multi_file(self):
        """
        Test the copy of a single file
        """
        if(False):
            #create a file
            test_file_path = os.path.join(self.test_path, "test.txt")
            file = open(test_file_path, "w")
            file.write("test")
            file.close()

            #create a file
            test_file_path_2 = os.path.join(self.test_path, "test2.txt")
            file = open(test_file_path_2, "w")
            file.write("test")
            file.close()

            #create a filepath to a not existing file
            test_file_path_3 = os.path.join(self.test_path, "not_existing_file.txt")

            #create target dir
            target_dir = os.path.join(self.test_path, "test")
            os.mkdir(target_dir)

            #input is on the test cluster == lce072
            input_map = [("lce072", test_file_path),
                         ("lce072", test_file_path_2),
                         ("lce072", test_file_path_3)]
            self.imager_create_dbs._copy_input_files(target_dir, input_map)

            #Validate that the file has been copied
            fileexists = os.path.exists(test_file_path)
            self.assertTrue(fileexists)
            fileexists2 = os.path.exists(test_file_path_2)
            self.assertTrue(fileexists2)

            #validate that a log entry has been entered for the missing file
            last_log_entry = self.imager_create_dbs.logger.last()
            target_log_entry = "Failed loading file: {0}".format(test_file_path_3)
            self.assertTrue(last_log_entry[0] == "info")
            self.assertTrue(last_log_entry[1] == target_log_entry,
                            "{0} != {1}".format(last_log_entry[1], target_log_entry))


if __name__ == "__main__":
    unittest.main()
