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
HOST_NAME = socket.gethostname()

import lofarpipe.daemons.pipelineMCQDaemonImp as pipelineMCQDaemonImp
import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message





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
            prepare_test_MCQ(pipelineMCQDaemonImp.pipelineMCQDaemonImp,
                             self.logfile, self.deadletterfile, slaveCommandQueueNameTemplate )

        with nested(daemon, commandQueueBus, 
                    deadletterQueue, deadletterToQueue) as (
                      daemon, commandQueueBus, deadletterQueue, deadletterToQueue):



            job_node = socket.gethostname()
            slaveCommandQueue_topic_name = slaveCommandQueueNameTemplate.format(job_node)
            slaveCommandQueueBusName = "testPipelineMCQDaemon" + "/" + \
                                      slaveCommandQueue_topic_name
            slaveCommandQueueBus = get_from_bus( 
                    slaveCommandQueueBusName, HOST_NAME)

            # Test1: Create a test job payuoad
            send_payload =  {'command':'run_job',
                             'type':'command',
                             'node':job_node,
                             'parameters':{                             
                             'job':{}},
                             'subject':slaveCommandQueue_topic_name
                             }

            msg = create_test_msg(send_payload)
            commandQueueBus.send(msg)

            # start the daemon processing
            daemon._process_command_queue()
  

            # validate that a job is received on the slave queue
            # wait on the slave command queue
            msg_received = try_get_msg(slaveCommandQueueBus)

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
            prepare_test_MCQ(pipelineMCQDaemonImp.pipelineMCQDaemonImp,
                             self.logfile, self.deadletterfile, slaveCommandQueueNameTemplate )

        with nested(daemon, commandQueueBus, 
                    deadletterQueue, deadletterToQueue) as (
                      daemon, commandQueueBus, deadletterQueue, deadletterToQueue):



            job_node = socket.gethostname()
            slaveCommandQueue_topic_name = slaveCommandQueueNameTemplate.format(job_node)
            slaveCommandQueueBusName = "testPipelineMCQDaemon" + "/" + \
                                      slaveCommandQueue_topic_name
            slaveCommandQueueBus = get_from_bus( 
                    slaveCommandQueueBusName, HOST_NAME)

            # Test1: Create a test job payuoad
            send_payload =  {'command':'stop_session',
                             'type':'command',
                             'node':job_node}

            msg = create_test_msg(send_payload)
            commandQueueBus.send(msg)

            # start the daemon processing
            daemon._process_command_queue()
  

            # validate that a job is received on the slave queue
            # wait on the slave command queue
            msg_received = try_get_msg(slaveCommandQueueBus)

            # unpack received data
            received_payload = eval(msg_received.content().payload)
            expected_payload = send_payload
            
            # validate correct content
            self.assertEqual(received_payload, send_payload)


# ******************** helper function ******************
def prepare_test_MCQ(subclass, logfile, deadletterfile, slaveCommandQueueNameTemplate):
    """
    Hides boiler plate code

    return the deamon and needed
    """
        # config
    broker =  HOST_NAME
    busname = "testPipelineMCQDaemon"  # TODO: Use a different name
    #busname = "testbus"
    commandQueueName = busname + "/" + "masterCommandQueueName"
    #masterCommandQueueName = "masterCommandQueueName"
    deadLetterQueueName = busname + ".deadletter"
    # create the sut
    daemon = subclass(broker, 
                      busname, 
                      commandQueueName,
                      deadLetterQueueName, 
                      deadletterfile,
                      logfile,
                      slaveCommandQueueNameTemplate,
                      1, 
                      False)

    # connect to the queueus
    commandQueueBus =get_to_bus(commandQueueName, broker)

    deadletterQueue = get_from_bus(deadLetterQueueName,
                                                 broker)

    deadletterToQueue = get_to_bus(deadLetterQueueName,
                                                 broker)


    return daemon, commandQueueBus,  deadletterQueue, deadletterToQueue

def create_test_msg(payload):
    """
    Creates a minimal valid msg with payload
    """
    msg = message.MessageContent(
                from_="test",
                forUser="MCQDaemon",
                summary="summary",
                protocol="protocol",
                protocolVersion="test", 
                #momid="",
                #sasid="", 
                #qpidMsg=None
                      )
    msg.payload = payload
    return msg

def get_from_bus(queueName, broker):
    """
    Helper function, creates validated frombus connected on the expected
    slave bus name
    """

    slaveCommandQueueBus = None
    try:
        slaveCommandQueueBus = msgbus.FromBus(queueName,
                                              broker = broker)

    except Exception, ex:
        logger.error("Exception thrown by FromBus, this is probably caused"
                     " by the msgbus routing not been set up correctly.")
        raise ex

    return slaveCommandQueueBus

def get_to_bus(masterCommandQueueName, broker):
    """
    Creates a command queue to bus
    """
    commandQueueBus = msgbus.ToBus(
                   masterCommandQueueName,
              broker = broker)

    return commandQueueBus


def try_get_msg(queue, wait_period=10):
    """
    Helper function, try to get msg from queue. raise exception if not gotten
    after 10 sec. return msg if received

    """
    # We expect a deadletter on the 
    idx = 0
    msg_received = None
    while (True):
        print "Waiting for msg"
        if idx >= wait_period:
            print "Did not receive a msg after {0} seconds!!".format(
                      wait_period)

            raise IOError("Did not receive a msg after 10 seconds!!")

        msg_received = queue.get(1)
        if msg_received is None:
            print "Did not receive a msg on  queue"
            idx += 1
            time.sleep(1)
            continue

        queue.ack(msg_received)
        break

    return msg_received
 

if __name__ == "__main__":
    unittest.main()




       

