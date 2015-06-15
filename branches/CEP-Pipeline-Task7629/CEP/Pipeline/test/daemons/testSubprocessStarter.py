#!/usr/bin/python
# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$


#!/usr/bin/python
# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$
import logging

import CQDaemonTestFunctions as testFunctions
import lofarpipe.daemons.subprocessStarter as subprocessStarter

import unittest
import time

# Wraps the actual slave implementation, allows to catch calls to internal 
# function we need to validate.
class testForwardOfJobMsgToQueueuSlaveWrapper(
            PipelineSCQDaemonImp.PipelineSCQDaemonImp):
    def __init__(self, broker, busname, masterCommandQueueName,
                 deadLetterQueueName,
                 loop_interval, daemon):
        super(testForwardOfJobMsgToQueueuSlaveWrapper, self).__init__(
           broker, busname, 
           masterCommandQueueName, deadLetterQueueName,
           loop_interval, daemon)
        pass
        self._start_subprocess_called = False
        self._process_deadletter_run_job_called = False

    def _start_subprocess(self):
        """
        This function hides the actual run subprocess module
        """
        self._start_subprocess_called = True

    def _process_deadletter_run_job(self, unpacked_msg_content):
        """

        """
        self._process_deadletter_run_job_called = True



class testForwardOfJobMsgToQueueuSlave(
                unittest.TestCase):

    def __init__(self, arg):  
        super(testForwardOfJobMsgToQueueuSlave, self).__init__(arg)

    # For now leave the setup and tearDown empty: single test
    # when the number of test increased it is an idea to implement them
    def setUp(self):
        pass

    def tearDown(self):
        pass
  
    def test_run_job_results_in_start_subprocess_call(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        """
        # Create the daemon and get all the default queues
        pass




if __name__ == "__main__":
    unittest.main()




       

