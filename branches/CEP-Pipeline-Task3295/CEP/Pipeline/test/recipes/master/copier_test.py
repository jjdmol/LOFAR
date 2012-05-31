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
from lofarpipe.recipes.master.copier import copier       #@UnresolvedImport



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
        self.test_path = temp_path = tempfile.mkdtemp()

    def tearDown(self):
        #shutil.rmtree(self.test_path)
        pass

    def test_validate_source_target_mapfile(self):
        source_map = [("node1", "path1"), ("node2", "path2"), ("node2", "path3")]
        target_map = [("node3", "path1"), ("node4", "path2"), ("node4", "path3")]

        sut = copierWrapper()
        self.assertTrue(sut._validate_source_target_mapfile(source_map, target_map))

    def test_create_target_node_keyed_dict(self):
        source_map = [("node1", "path1"), ("node2", "path2"), ("node2", "path3")]
        target_map = [("node3", "path1"), ("node4", "path2"), ("node4", "path3")]


        sut = copierWrapper()
        output = sut._create_target_node_keyed_dict(source_map, target_map)
        expected_output = {'node3': [(('node1', 'path1'), ('node3', 'path1'))],
                           'node4': [
                                     (('node2', 'path2'), ('node4', 'path2')),
                                     (('node2', 'path3'), ('node4', 'path3'))
                                    ]
                           }
        self.assertTrue(output == expected_output, "incorrect output")

    def test_construct_node_specific_mapfiles(self):
        temp_path = self.test_path

        source_target_dict = {
                           'node1': [
                                     [('node2', 'path2'), ('node1', 'path2')],
                                     [('node2', 'path3'), ('node1', 'path3')]
                                    ]
                           }
        mapfile1 = os.path.join(temp_path, "copier_source_node1.map")
        mapfile2 = os.path.join(temp_path, "copier_target_node1.map")
        sut = copierWrapper()
        mapfile_dict = sut._construct_node_specific_mapfiles(source_target_dict,
                                               temp_path)
#        except:
#            self.assertTrue(False, sut.logger.last())
        expected_output = {'node1':(mapfile1, mapfile2)}

        self.assertTrue(repr(expected_output) == repr(mapfile_dict),
                        "Output of function incorrect. dict with mapfile pairs"
                        "expected received-expected: {0} - {1}".format(
                                repr(mapfile_dict), repr(expected_output)))

        # validation
        #files exist
        self.assertTrue(os.path.exists(mapfile1),
                        "mapfile for first node not created properly")
        # content 
        fp = open(mapfile1)
        content = fp.read()
        fp.close()
        expected_content = "[('node2', 'path2'), ('node2', 'path3')]"
        self.assertTrue(content == expected_content, "source mapfile content incorrect")
        #now for the target mapfile
        self.assertTrue(os.path.exists(mapfile2),
                        "mapfile for second node not created properly")

        fp = open(mapfile2)
        content = fp.read()
        fp.close()
        expected_content = "[('node1', 'path2'), ('node1', 'path3')]"
        self.assertTrue(content == expected_content, "target mapfile content incorrect")

        # check if the writing of the log is performed
        log_message = "Wrote mapfile with node specific target"\
                              " paths: {0}"
        self.assertTrue(sut.logger._log[-2][1] == log_message.format(mapfile2),
                        "incorrect logging for first write action of"
                        " mapfile: {0}".format(sut.logger._log[-2]))
        log_message = "Wrote mapfile with node specific source"\
                              " paths: {0}"
        self.assertTrue(sut.logger._log[-1][1] == log_message.format(mapfile1),
                        "incorrect logging for second write action of "
                        "mapfile: {0}".format(sut.logger._log[-1]))



if __name__ == "__main__":
    unittest.main()
