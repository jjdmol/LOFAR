import os
import shutil
import tempfile
import unittest
import time

from lofarpipe.support.subprocessgroup import SubProcessGroup

class SubProcessGroupTest(unittest.TestCase):
    """
    Test class for the SubProcessGroupTest class in lofarpipe.support.SubProcessGroupTest
    """
    def __init__(self, arg):
        super(SubProcessGroupTest, self).__init__(arg)


    def setUp(self):
        """
        Create scratch directory and create required input files in there.
        """

    def tearDown(self):
        """
        Cleanup all the files that were produced by this test
        """


    def test_limit_number_of_proc(self):
        process_group = SubProcessGroup(polling_interval=1)

        # wait for 2 seconds
        cmd = "sleep 2"
        start_time = time.time()
        # Quickly start a large number of commands, assur
        for idx in range(10):
            process_group.run(cmd)

        # if there is no  serialization the test would take about 5 seconds
        # with serialization i will take at a minimum 10 second, use 8 seconds
        # to have some safety from rounding errors

        process_group.wait_for_finish()
        end_time = time.time()
        self.assertTrue((end_time - start_time) > 3)
        

    def test_start_without_jobs(self):
        process_group = SubProcessGroup(polling_interval=1)

        # wait for 5 seconds
        start_time = time.time()

        process_group.wait_for_finish()
        end_time = time.time()
        
        # The wait should complete without a polling interfal
        self.assertTrue((end_time - start_time) < 1)

if __name__ == '__main__':
    import xmlrunner
    #unittest.main(testRunner=xmlrunner.XMLTestRunner(output='result.xml'))
    unittest.main()

