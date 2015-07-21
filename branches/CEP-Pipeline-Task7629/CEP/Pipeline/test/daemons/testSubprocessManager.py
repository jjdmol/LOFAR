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
import lofarpipe.daemons.subprocessManager as subprocessManager

import unittest
import time

# Only the bigger functions are tested: Most implementation are shallow
# functions

import socket
HOST_NAME = socket.gethostname()

class BusMuck(object):
    def __init__(self, busname, broker):
        self._busname = busname
        self._broker = broker

        self._send_calls = {}

    def send(self, msg):
        nr_entries = len(self._send_calls)
        self._send_calls[nr_entries] = eval(msg.payload)  # msg is content here

        

# Define logging.  Until we have a python loging framework, we'll have
# to do any initialising here
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)


class TestSubprocessManager(unittest.TestCase):

    def __init__(self, arg):  
        super(TestSubprocessManager, self).__init__(arg)

    # For now leave the setup and tearDown empty: single test
    # when the number of test increased it is an idea to implement them
    def setUp(self):
        pass

    def tearDown(self):
        pass
  
    def test_construction(self):
        """
        basic test, create the object and expect no exception etc.

        """
        broker = HOST_NAME
        busname = "testmcqdaemon"
        toBus = None
        logger = logging.getLogger("subprocessManager")
        starter = subprocessManager.SubprocessManager(broker, busname, toBus,
                                                      logger)

    def test_start_job_from_msg_succes(self):
        """
        Main function of the class, creates the state and calls all the needed

        functionality
        """
        # small wrapper class mucking members
        class SubprocessManagerWrapper(
            subprocessManager.SubprocessManager):
            def __init__(self, broker, busname, toBus, logger):
                  super(SubprocessManagerWrapper, self).__init__(
                                                broker, busname, toBus, logger)
                  self._connect_called = None
                  self._session_uuid = None

            #def send_job_parameters(self, session_uuid, job_uuid, msg_content):

            #    self._session_uuid = session_uuid
            #    self._job_uuid = job_uuid
            #    self._msg_content = msg_content

            def _start_subprocess(self, command, working_dir, environment):
                self._start_subprocess_called = True
                return "Just a string", "Not an error"


        broker = HOST_NAME
        busname = "testmcqdaemon"
        toBus = BusMuck(broker, busname)
        logger = logging.getLogger("subprocessManager")
        starter = SubprocessManagerWrapper(broker, busname, toBus,
                                                      logger)

        # A fake msg content
        msg_content = {}
        msg_content['session_uuid'] = "123456"
        msg_content['job_uuid'] = "654321"
        msg_content['parameters'] = {}
        msg_content['parameters']['cdw'] = "/home"
        msg_content['parameters']['environment'] = {"ENV":"Value"}
        msg_content['parameters']['cmd'] = "ls"

        starter.start_job_from_msg(msg_content)

        # Should result in a parameter msg on the toBus
        self.assertEqual(toBus._send_calls, {0:msg_content})
        
        # subprocess called
        self.assertTrue(starter._start_subprocess_called)

    def test_start_job_from_msg_fail(self):
        """
        Main function of the class, creates the state and calls all the needed

        functionality
        """
        # small wrapper class mucking members
        class SubprocessManagerWrapper(
            subprocessManager.SubprocessManager):
            def __init__(self, broker, busname, toBus, logger):
                  super(SubprocessManagerWrapper, self).__init__(
                                                broker, busname, toBus, logger)
                  self._connect_called = None
                  self._start_subprocess_called = None
                  self._send_process_cout_cerr_called =None
                  self.__send_results_called = None

            def _start_subprocess(self, cmd_with_bus_details,
                                  working_dir, environment):
                self._start_subprocess_called = True
                return None, "ERROR"

            def _send_process_cout_cerr(self, session_uuid, job_uuid,
                                         std_str, error_st):
                self._send_process_cout_cerr_called = True

            def _send_results(self, session_uuid, job_uuid, exit_status):
                self._send_results_called =  exit_status
                


        broker = HOST_NAME
        busname = "testmcqdaemon"
        toBus = BusMuck(broker, busname)
        logger = logging.getLogger("subprocessManager")
        starter = SubprocessManagerWrapper(broker, busname, toBus,
                                                      logger)

        # A fake msg content
        msg_content = {}
        msg_content['session_uuid'] = "123456"
        msg_content['job_uuid'] = "654321"
        msg_content['parameters'] = {}
        msg_content['parameters']['cdw'] = "/home"
        msg_content['parameters']['environment'] = {"ENV":"Value"}
        msg_content['parameters']['cmd'] = "notavalidexecutablename"

        starter.start_job_from_msg(msg_content)

        # subprocess called
        self.assertTrue(starter._start_subprocess_called)

        # But the starting of the job failed: This should result in both a
        # logline and a results msg
        expect_msg_on_tobus = {0: {'sender': HOST_NAME, 
                                   'level': 'ERROR', 
                                   'log_data': 'ERROR', 
                                   'session_uuid': '123456', 
                                   'type': 'log',
                                   'job_uuid': '654321'}, 
                               1: {'info': '', 
                                   'type': 'exit_value', 
                                   'exit_value': '-1', 
                                   'job_uuid': '654321', 
                                   'session_uuid': '123456'}}         

        self.assertEqual(toBus._send_calls,expect_msg_on_tobus)

if __name__ == "__main__":
    unittest.main()




       

