from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile


import pyrap.tables as tb                                                       #@UnresolvedImport
from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from lofarpipe.recipes.nodes.imager_prepare import imager_prepare \
     as imager_prepare_node
from logger import logger

class ImagerPrepareTestWrapper(imager_prepare_node):
    """
    Wrapper for the imager_prepare allows overwriting of class members
    """
    def __init__(self):
        """
        Overloaded __init__ function, hiding the original __init__ on 
        LOFARnodeTCP.
        """
        self.logger = logger()
        self.dppp_call_vars = None
        self.environment = None

    def _dppp_call(self, working_dir, ndppp, cmd, environment):
        self.dppp_call_vars = (working_dir, ndppp, cmd, environment)


class ImagerPrepareTest(unittest.TestCase):
    """
    Tests for imager_prepare class 
    """

    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(ImagerPrepareTest, self).__init__(arg)

    def setUp(self):
        self.ImagerPrepareTestWrapper = ImagerPrepareTestWrapper()
        self.test_path = tempfile.mkdtemp()

    def tearDown(self):
        pass

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
            self.ImagerPrepareTestWrapper._copy_input_files(target_dir, input_map)

            #Validate that the file has been copied
            fileexists = os.path.exists(test_file_path)
            self.assertTrue(fileexists)
            fileexists2 = os.path.exists(test_file_path_2)
            self.assertTrue(fileexists2)

            #validate that a log entry has been entered for the missing file
            last_log_entry = self.ImagerPrepareTestWrapper.logger.last()
            target_log_entry = "Failed loading file: {0}".format(test_file_path_3)
            self.assertTrue(last_log_entry[0] == "info")
            self.assertTrue(last_log_entry[1] == target_log_entry,
                            "{0} != {1}".format(last_log_entry[1], target_log_entry))



    def test_run_dppp(self):
        """
        This unittest border a functional test:
        framework is mucked by using an muckable function
        """
        working_dir = ""

        time_slice_dir_path = tempfile.mkdtemp()
        slices_per_image = 2
        input_map = [("lce072", "test_file_path1"),
                         ("lce072", "test_file_path2"),
                         ("lce072", "test_file_path3"),
                         ("lce072", "test_file_path4")]
        subbands_per_image = 2
        collected_ms_dir_name = ""
        fp = open(os.path.join(self.test_path, "parset"), 'w')
        fp.write("key=value\n")
        fp.close()
        parset = os.path.join(self.test_path, "parset")
        ndppp = ""
        init_script = ""

        sut = ImagerPrepareTestWrapper()
        output = sut._run_dppp(working_dir, time_slice_dir_path, slices_per_image,
                  input_map, subbands_per_image, collected_ms_dir_name, parset,
                  ndppp)

        # The output should contain two timeslices ms prepended with the time_slice_dir_path
        expected_output = [os.path.join(time_slice_dir_path, "time_slice_0.dppp.ms"),
                           os.path.join(time_slice_dir_path, "time_slice_1.dppp.ms")]
        self.assertTrue(output == expected_output,
            "_run_dppp did not return timeslice ms: {0} !=  {1}".format(output,
                                 expected_output))

        # Two parset should be written in the time_slice_dir_path
        parset_1_content_expected = [('replace', 'uselogger', 'True'),
                 ('replace', 'msin', "['test_file_path1', 'test_file_path2']"),
                 ('replace', 'msout', '{0}'.format(
                    os.path.join(time_slice_dir_path, "time_slice_0.dppp.ms")))]

        parset_1_output = eval(open(os.path.join(time_slice_dir_path, \
                "time_slice_0.dppp.ms.ndppp.par")).read())
        self.assertTrue(parset_1_output == parset_1_content_expected,
                "\n{0} != \n{1}".format(parset_1_output, parset_1_content_expected))

        # Two parset should be written in the time_slice_dir_path
        parset_2_content_expected = [('replace', 'uselogger', 'True'),
                 ('replace', 'msin', "['test_file_path3', 'test_file_path4']"),
                 ('replace', 'msout', '{0}'.format(
                    os.path.join(time_slice_dir_path, "time_slice_1.dppp.ms")))]

        parset_2_output = eval(open(os.path.join(time_slice_dir_path, \
                "time_slice_1.dppp.ms.ndppp.par")).read())
        self.assertTrue(parset_2_output == parset_2_content_expected,
                "\n{0} != \n{1}".format(parset_2_output, parset_2_content_expected))


if __name__ == "__main__":
    unittest.main()
