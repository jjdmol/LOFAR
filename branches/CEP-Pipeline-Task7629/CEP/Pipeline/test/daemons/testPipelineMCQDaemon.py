#!usr/bin/python
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

import unittest
import os
from contextlib import nested   #>2.7 allows nesting out of the box
import socket

import CQDaemonTestFunctions as testFunctions
import lofarpipe.daemons.pipelineMCQDaemonImp as pipelineMCQDaemonImp
import lofar.messagebus.msgbus as msgbus



class testForwardOfJobMsgToQueueu(unittest.TestCase):

    def __init__(self, arg):  
        super(testForwardOfJobMsgToQueueu, self).__init__(arg)

    def setUp(self):
        self.logfile = "/tmp/testPipelineMCQDaemon.log"
        open( self.logfile , 'a').close()
        self.deadletterfile = "/tmp/testPipelineMCQDaemonDeadletter.log"
        open( self.deadletterfile , 'a').close()

    def tearDown(self):
        os.remove(self.logfile)
        os.remove(self.deadletterfile)

    def test_forwarding_of_job_msg_to_queue(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        """

        slaveCommandQueueNameTemplate = "slaveCommandQueue_{0}"
        daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
            testFunctions.prepare_test_MCQ(pipelineMCQDaemonImp.pipelineMCQDaemonImp,
                             self.logfile, self.deadletterfile, slaveCommandQueueNameTemplate )

        with nested(daemon, commandQueueBus, 
                    deadletterQueue, deadletterToQueue) as (
                      daemon, commandQueueBus, deadletterQueue, deadletterToQueue):



            job_node = socket.gethostname()
            slaveCommandQueue_topic_name = slaveCommandQueueNameTemplate.format(job_node)
            slaveCommandQueueBusName = "testmcqdaemon" + "/" + \
                                      slaveCommandQueue_topic_name
            slaveCommandQueueBus = testFunctions.get_from_bus( 
                    slaveCommandQueueBusName, "locus102")

            # Test1: Create a test job payuoad
            send_payload =  {'command':'run_job',
                             'type':'command',
                             'node':job_node,
                             'parameters':{                             
                             'job':{}},
                             'subject':slaveCommandQueue_topic_name
                             }

            msg = testFunctions.create_test_msg(send_payload)
            commandQueueBus.send(msg)

            # start the daemon processing
            daemon._process_command_queue()
  

            # validate that a job is received on the slave queue
            # wait on the slave command queue
            msg_received = testFunctions.try_get_msg(slaveCommandQueueBus)

            # unpack received data
            received_payload = eval(msg_received.content().payload)
            expected_payload = send_payload
            # not a deepcopy so send is also change, mhe
            expected_payload['subject'] = slaveCommandQueue_topic_name 
            # validate correct content
            self.assertEqual(received_payload, send_payload)

    def test_forwarding_of_quit_msg_to_queue(self):
        """
        A msg with the command run_job should be forwarded to jobnode
        """

        slaveCommandQueueNameTemplate = "slaveCommandQueue_{0}"
        daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
            testFunctions.prepare_test_MCQ(pipelineMCQDaemonImp.pipelineMCQDaemonImp,
                             self.logfile, self.deadletterfile, slaveCommandQueueNameTemplate )

        with nested(daemon, commandQueueBus, 
                    deadletterQueue, deadletterToQueue) as (
                      daemon, commandQueueBus, deadletterQueue, deadletterToQueue):



            job_node = socket.gethostname()
            slaveCommandQueue_topic_name = slaveCommandQueueNameTemplate.format(job_node)
            slaveCommandQueueBusName = "testmcqdaemon" + "/" + \
                                      slaveCommandQueue_topic_name
            slaveCommandQueueBus = testFunctions.get_from_bus( 
                    slaveCommandQueueBusName, "locus102")

            # Test1: Create a test job payuoad
            send_payload =  {'command':'stop_session',
                             'type':'command',
                             'node':job_node}

            msg = testFunctions.create_test_msg(send_payload)
            commandQueueBus.send(msg)

            # start the daemon processing
            daemon._process_command_queue()
  

            # validate that a job is received on the slave queue
            # wait on the slave command queue
            msg_received = testFunctions.try_get_msg(slaveCommandQueueBus)

            # unpack received data
            received_payload = eval(msg_received.content().payload)
            expected_payload = send_payload
            
            # validate correct content
            self.assertEqual(received_payload, send_payload)



if __name__ == "__main__":
    unittest.main()




       

