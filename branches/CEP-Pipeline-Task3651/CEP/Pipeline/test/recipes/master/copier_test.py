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
from lofarpipe.support.data_map import DataMap
from lofarpipe.recipes.master.copier import copier                              #@UnresolvedImport



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

    def test_validate_mapfiles_norename(self):
        sut = copierWrapper()
        sut.source_map = DataMap(
            [("node1", "path1"), ("node2", "path2"), ("node2", "path3")]
        )
        sut.target_map = DataMap(
            [("node3", "path1"), ("node4", "path2"), ("node4", "path3")]
        )
        self.assertTrue(sut._validate_mapfiles())
        self.assertTrue(sut._validate_mapfiles(allow_rename=True))
        
    def test_validate_mapfiles_rename(self):
        sut = copierWrapper()
        sut.source_map = DataMap(
            [("node1", "path1"), ("node2", "path2"), ("node2", "path3")]
        )
        sut.target_map = DataMap(
            [("node3", "path4"), ("node4", "path5"), ("node4", "path6")]
        )
        self.assertFalse(sut._validate_mapfiles())
        self.assertTrue(sut._validate_mapfiles(allow_rename=True))
        
    
#    def test_validate_source_target_mapfile(self):
#        source_map = [("node1", "path1"), ("node2", "path2"), ("node2", "path3")]
#        target_map = [("node3", "path1"), ("node4", "path2"), ("node4", "path3")]

#        sut = copierWrapper()
#        self.assertTrue(sut._validate_source_target_mapfile(source_map, target_map))

#    def test_construct_node_specific_mapfiles(self):
#        temp_path = self.test_path
#
#        source_map = [('node1', 'path1'), ('node2', 'path2')]
#        target_map = [('node3', 'path3'), ('node4', 'path4')]
#
#        # Targets on node 3 and 4: mapfiles named after them
#        mapfile1 = os.path.join(temp_path, "copier_source_node3.map")
#        mapfile2 = os.path.join(temp_path, "copier_target_node3.map")
#        mapfile3 = os.path.join(temp_path, "copier_source_node4.map")
#        mapfile4 = os.path.join(temp_path, "copier_target_node4.map")
#        sut = copierWrapper()
#        mapfile_dict = sut._construct_node_specific_mapfiles(source_map,
#                                               target_map, temp_path)
#
#        expected_output = {'node3':(mapfile1, mapfile2),
#                           'node4':(mapfile3, mapfile4)}
#
#        self.assertTrue(repr(expected_output) == repr(mapfile_dict),
#                        "Output of function incorrect. dict with mapfile pairs"
#                        "output: \n{0} \n expected: \n{1}".format(
#                                repr(mapfile_dict), repr(expected_output)))
#
#        # validation
#        #files exist
#        self.assertTrue(os.path.exists(mapfile1),
#                        "mapfile for first node not created properly")
#        # content 
#        fp = open(mapfile1)
#        content = fp.read()
#        fp.close()
#        expected_content = "[('node1', 'path1')]"
#        self.assertTrue(content == expected_content, "source mapfile content incorrect")
#        #now for the target mapfile
#        self.assertTrue(os.path.exists(mapfile2),
#                        "mapfile for second node not created properly")
#
#        fp = open(mapfile2)
#        content = fp.read()
#        fp.close()
#        expected_content = "[('node3', 'path3')]"
#        self.assertTrue(content == expected_content,
#            "target mapfile content incorrect, expected, output \n{0}\n{1}".format(
#                expected_content, content))


#    def test_copier_create_correct_mapfile(self):
#        sut = copierWrapper()
#
#        instr = [('node1', '/path1/1'), ('node1', '/path1/2')]
#        data = [('node2', '/path2/3'), ('node2', '/path2/4')]
#
#
#        expected_result = [('node2', '/path2/1'), ('node2', '/path2/2')]
#        target_map = sut._create_target_map_for_instruments(instr, data)
#
#        self.assertTrue(expected_result == target_map, target_map)


from logger import logger
from lofarpipe.recipes.master.copier import MasterNodeInterface                              #@UnresolvedImport
from lofarpipe.support.remotecommand import ComputeJob

class MasterNodeInterfaceWrapper(MasterNodeInterface):
    """
    Wrapper for the imager_create_dbs an actual implementation of abstract class
    """
    def __init__(self, command):
        """
        """
        super(MasterNodeInterfaceWrapper, self).__init__(command)

        self.logger = logger()

        self._function_calls = []

        class Error():
            def __init__(self):
                self._return_value = 0
            def isSet(self):
                return self._return_value

        self.error = Error()
        
        class Job():
            def __init__(self):
                self.results = {'returncode': 0}
            
        self._jobs = [Job(), Job()]

    def _schedule_jobs(self, *args):
        self._function_calls.append(('_schedule_jobs', args))
        if self._command == "failure":
            self.error._return_value = -1
            for job in self._jobs:
                job.results['returncode'] = 1
        elif self._command == "error":
            self.error._return_value = 1
            self._jobs[0].results['returncode'] = 1
        elif self._command == "succes":
            self.error._return_value = 0

    def on_failure(self, *args):
        self._function_calls.append(('on_failure', args))

    def on_error(self, *args):
        self._function_calls.append(('on_error', args))

    def on_succes(self, *args):
        self._function_calls.append(('on_succes', args))

class MasterNodeInterfaceTest(unittest.TestCase):

    def __init__(self, arg):  #todo deze moet toch in de setUp
        super(MasterNodeInterfaceTest, self).__init__(arg)

    def setUp(self):
        self.test_path = temp_path = tempfile.mkdtemp()

    def tearDown(self):
        #shutil.rmtree(self.test_path)
        pass

    def test__init__raise_exception(self):
        """
        Test if MasterNodeInterface constructor raises a TypeError 
        if called without an string (ideally containing the command to run
        on the node
        """
        self.assertRaises(TypeError, MasterNodeInterface)


    def test__init__raise_called_with_command(self):
        command = "a string"
        sut = MasterNodeInterface(command)

        self.assertTrue(sut._command == command,
            "The constructor did not assign the command to the local variabale")

        self.assertTrue(hasattr(sut, '_jobs') and isinstance(sut._jobs, list),
            "The constructor did create a list data member called _list")


    def test_on_failure_return_value(self):
        command = "a string"
        sut = MasterNodeInterface(command)
        # on_failure by default returns 1, check return value.
        self.assertEqual(-1, sut.on_failure())


    def test_on_error_return_value(self):
        command = "a string"
        sut = MasterNodeInterface(command)
        # on_error by default returns 0, check return value.
        self.assertEqual(1, sut.on_error())


    def test_on_succes_raise_exception(self):
        command = "a string"
        sut = MasterNodeInterface(command)
        # on_error by default returns 0, check return value.
        self.assertEqual(0, sut.on_succes())


    def test_append_job(self):
        command = "a string"
        sut = MasterNodeInterface(command)

        sut.append_job("test", ["arg1", "arg2"])

        self.assertTrue(len(sut._jobs) == 1 , "append_job did not add a job"
                        " To the job list")

        self.assertTrue(isinstance(sut._jobs[0], ComputeJob) ,
                 "append_job did not add an object with the type ComputeJob to"
                 " the job list")

    def test_run_jobs_fail(self):
        command = "failure"
        sut = MasterNodeInterfaceWrapper(command)
        # command failure will result in all calls to the run_jobs to 'fail'
        # error.isSet will return True (used internaly) resulting in a call to
        # on_fail
        sut.run_jobs()

        self.assertTrue(len(sut._function_calls) == 2,
             "run_jobs in a fail state should return in two function calls")

        self.assertTrue(sut._function_calls[0][0] == '_schedule_jobs' ,
             "the name of the first called function in a fail state should be _schedule_jobs")

        self.assertTrue(sut._function_calls[1][0] == 'on_failure' ,
             "the name of the second called function in a fail state should be on_error")


    def test_run_jobs_error(self):
        command = "error"
        sut = MasterNodeInterfaceWrapper(command)
        # command error will result in any calls to the run_jobs to 'fail'
        # error.isSet will return True (used internaly) resulting in a call to
        # on_error
        sut.run_jobs()

        self.assertTrue(len(sut._function_calls) == 2,
             "run_jobs in a error state should return in two function calls")

        self.assertTrue(sut._function_calls[0][0] == '_schedule_jobs' ,
             "the name of the first called function in an error state should be _schedule_jobs")

        self.assertTrue(sut._function_calls[1][0] == 'on_error' ,
             sut._function_calls#"the name of the second called function in an error state should be on_error"
             )


    def test_run_jobs_succes(self):
        command = "succes"
        sut = MasterNodeInterfaceWrapper(command)
        # command success will result in all calls to the run_jobs to 'succeed'
        # error.isSet will return False (used internaly) resulting in a call to
        # on_success
        sut.run_jobs()

        self.assertTrue(len(sut._function_calls) == 2,
             "run_jobs in a fail state should return in two function calls")

        self.assertTrue(sut._function_calls[0][0] == '_schedule_jobs' ,
             "the name of the first called function in a succes state should be _schedule_jobs")

        self.assertTrue(sut._function_calls[1][0] == 'on_succes' ,
             "the name of the second called function in a succes state should be on_succes")

        #Test the adding of an log string (info level)
        self.assertTrue(sut.logger.last()[0] == "info")
        self.assertTrue(sut.logger.last()[1].startswith("Start scheduling jobs with"))


if __name__ == "__main__":
    unittest.main()
