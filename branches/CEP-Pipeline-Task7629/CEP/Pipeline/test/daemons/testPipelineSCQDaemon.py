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
import lofarpipe.daemons.pipelineSCQDaemonImp as PipelineSCQDaemonImp

import unittest
import time

# Wraps the actual slave implementation, allows to catch calls to internal 
# function we need to validate.
class testForwardOfJobMsgToQueueuSlaveWrapper(
            PipelineSCQDaemonImp.PipelineSCQDaemonImp):
    def __init__(self, broker, busname, masterCommandQueueName,
                 deadLetterQueueName,
                 subprocessStartedExec,
                 loop_interval, daemon):
        super(testForwardOfJobMsgToQueueuSlaveWrapper, self).__init__(
           broker, busname, 
           masterCommandQueueName, deadLetterQueueName,
           subprocessStartedExec,
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
        job_node = 'locus102'
        daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
            testFunctions.prepare_test( testForwardOfJobMsgToQueueuSlaveWrapper)

        # Test1: Create a test job payload
        send_payload =  {'command':'run_job',
                         'session_uuid':"123456321654",
                         'node':job_node,
                         'job':{}}
        msg = testFunctions.create_test_msg(send_payload)
        commandQueueBus.send(msg)

        # Run the process loop, The job will be send to a bus adress that does
        # not exist, it should end up in the deadletter queue
        daemon._process_commands()

        # read from the deadletter queue
        msg = testFunctions.try_get_msg(deadletterQueue, 10) 
        if msg == None:
            raise Exception(
                 "Did not receive the expect msg on the deadletter queue")
        deadletterQueue.ack(msg) 

        # check that the correct msg is receive in the deadletter queue        
        unpacked_msg_data = eval(msg.content().payload)
        self.assertEqual(unpacked_msg_data, send_payload)
        
        
        ## Cleanup sut
        commandQueueBus.close()
        deadletterQueue.close()
        daemon.close()

    def test_deadletterQueue_startjob_processing(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        """
        # Create the daemon and get all the default queues
        job_node = 'locus102'
        daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
            testFunctions.prepare_test( testForwardOfJobMsgToQueueuSlaveWrapper)

        # Test1: Create a test job payload
        send_payload =  {'command':'run_job',
                         'session_uuid':"123456321654",
                         'node':job_node,
                         'job':{}}
        msg = testFunctions.create_test_msg(send_payload)
        commandQueueBus.send(msg)

        # Run the process loop, The job will be send to a bus adress that does
        # not exist, it should end up in the deadletter queue
        daemon._process_commands()

        # Run the deadletter processer
        daemon._process_deadletter_queue()

        self.assertTrue(daemon._process_deadletter_run_job_called)
        
        
        ## Cleanup sut
        commandQueueBus.close()
        deadletterQueue.close()
        daemon.close()



if __name__ == "__main__":
    unittest.main()




       

