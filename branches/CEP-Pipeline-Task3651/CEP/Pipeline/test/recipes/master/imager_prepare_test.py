from __future__ import with_statement
import os
import errno
import unittest
import shutil
import numpy
import tempfile

from lofarpipe.support.data_map import DataMap

#imports from fixture:
from logger import logger

from lofarpipe.support.utilities import create_directory                        #@UnresolvedImport
from master.imager_prepare import imager_prepare                              #@UnresolvedImport



class ImagerPrepareWrapper(imager_prepare):
    """
    Wrapper for the imager_prepare allows overwriting of 
    """
    def __init__(self):
        """
        Overloaded __init__ function, hiding the original __init__ on 
        LOFARnodeTCP.
        """
        self.logger = logger()

class imager_prepareTest(unittest.TestCase):

    def __init__(self, arg):
        super(imager_prepareTest, self).__init__(arg)

    def setUp(self):
        self.test_path = temp_path = tempfile.mkdtemp()

    def tearDown(self):
        #shutil.rmtree(self.test_path)
        pass

    def test_create_input_map_for_sbgroup_single_ms(self):

        slices_per_image = 1
        n_subband_groups = 1
        subbands_per_image = 1
        idx_sb_group = 0 # get the first sb group
        input_mapfile = [('host', "path")]

        sut = ImagerPrepareWrapper()
        output = sut._create_input_map_for_sbgroup(slices_per_image, n_subband_groups,
                       subbands_per_image, idx_sb_group, input_mapfile)
        target = DataMap(input_mapfile)

        self.assertTrue(output == target, "Actual output = {0}".format(output))

    def test_create_input_map_for_sbgroup_2slice(self):
        """
        Test correct collection of the subbands for the first subband group
        with two timeslices and one subband per image
        """
        slices_per_image = 2
        n_subband_groups = 1
        subbands_per_image = 1
        idx_sb_group = 0 # get the 1st
        input_mapfile = [('host', "path"), ('host2', "path2"), ('host3', "path3")]

        sut = ImagerPrepareWrapper()
        output = sut._create_input_map_for_sbgroup(slices_per_image, n_subband_groups,
                       subbands_per_image, idx_sb_group, input_mapfile)
        target = DataMap([('host', "path"), ('host2', "path2")])
        self.assertTrue(target == output,

                        "Actual output = {0}".format(output))

    def test_create_input_map_for_sbgroup_2slice_2ndgroup(self):

        slices_per_image = 2  # two time slice
        n_subband_groups = 2  # two sb goups (to be combined in single group)
        subbands_per_image = 2# two image per subband
        idx_sb_group = 1 # get the 2nd sb group (
        input_mapfile = [('host', "path"), ('host2', "path2"),
                         ('host3', "path3"), ('host4', "path4"),
                         ('host5', "path5"), ('host6', "path6"),
                         ('host7', "path7"), ('host8', "path8")]

        sut = ImagerPrepareWrapper()
        output = sut._create_input_map_for_sbgroup(slices_per_image, n_subband_groups,
                       subbands_per_image, idx_sb_group, input_mapfile)
        target = DataMap([('host3', "path3"), ('host4', "path4"),
                                   ('host7', "path7"), ('host8', "path8")])
        self.assertTrue(output == target, "Actual output = {0}".format(output))

    def test_validate_input_map_succes(self):
        input_map = [(1), (1), (1), (1)]
        output_map = [(1)]
        slices_per_image = 2
        subbands_per_image = 2

        sut = ImagerPrepareWrapper()
        output = sut._validate_input_map(input_map, output_map,
                                    slices_per_image, subbands_per_image)

        self.assertTrue(output == True, "validating input map failed: incorrect output")

    def test_validate_input_map_incorrect(self):
        input_map = [(1), (1), (1)]
        output_map = [(1)]
        slices_per_image = 2
        subbands_per_image = 2

        sut = ImagerPrepareWrapper()
        output = sut._validate_input_map(input_map, output_map,
                                    slices_per_image, subbands_per_image)

        self.assertTrue(output == False,
                     "Actual output = {0}".format(output))
        self.assertTrue(sut.logger.last() == ('error', 'Incorrect number of input ms for supplied parameters:\n\tlen(input_map) = 3\n\tlen(output_map) * slices_per_image * subbands_per_image = 1 * 2 * 2 = 4'),
                                "incorrect logger message retrieved")

if __name__ == "__main__":
    unittest.main()
