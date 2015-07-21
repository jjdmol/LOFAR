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
import os
import logging
import time


import unittest
from contextlib import nested   #>2.7 allows nesting out of the box

import lofar.messagebus.CQDaemon as CQDaemon
import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
import time



# **********************************
# MCQDaemon is a template class for testing we need to instantiate it
class subclassedMCQDaemonFalse(CQDaemon.CQDaemon):
    """
    Shallow wrapper around the MCQDaemon.

    MCQDaemon is an abstract class and should always ben subclassed.
    Only a two function MUST be implemented process_commands 
    """
    def __init__(self, *args, **kwargs):
        super(subclassedMCQDaemonFalse, self).__init__(*args, **kwargs)
    
    def process_command(self, msg, unpacked_msg_content, command):
        return False

class subclassedMCQDaemontestcommand(CQDaemon.CQDaemon):
    """
    Shallow wrapper around the MCQDaemon.

    if the received command is testcommand forward adapted msg to slave queue
    return true
    """
    def __init__(self, *args, **kwargs):
        super(subclassedMCQDaemontestcommand, self).__init__(*args, **kwargs)

        self._adapted_content = None
    
    def process_command(self, msg, unpacked_msg_content, command):

        if command == "testcommand":
            adapted_content = unpacked_msg_content
            adapted_content['testcommand']="received"
            # Just add as a member
            self._adapted_content = adapted_content
            return True
        return false


class testCQDaemon(unittest.TestCase):
    def __init__(self, arg):  
        super(testCQDaemon, self).__init__(arg)

    # For now leave the setup and tearDown empty: single test
    # when the number of test increased it is an idea to implement them
    def setUp(self):
        self.logfile = "/tmp/testCQDaemon.log"
        open( self.logfile , 'a').close()
        self.deadletterfile = "/tmp/testCQDaemonDeadletter.log"
        open( self.deadletterfile , 'a').close()

    def tearDown(self):
        os.remove(self.logfile)
        os.remove(self.deadletterfile)

    #def test_subclass_processing(self):
    #    """
    #    Test if the subclass process_commands is called  
    #    """
    #    daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
    #        prepare_test(subclassedMCQDaemontestcommand,
    #                         self.logfile, self.deadletterfile )

    #    with nested(daemon, commandQueueBus, 
    #                deadletterQueue, deadletterToQueue) as (
    #                  daemon, commandQueueBus, deadletterQueue, deadletterToQueue):
            
    #        # Test1: Create a test job payuoad
    #        send_payload =  {'type':'command',
    #                       'command':'testcommand',
    #                       'node':"locus102",
    #                       'job':{}}

    #        msg = create_test_msg(send_payload)
    #        commandQueueBus.send(msg)

    #        # start the daemon processing
    #        daemon._process_command_queue()
  
    #        # validate that a job is processed o
    #        expected_payload = { 'type':'command',
    #                      'testcommand':"received",
    #                       'command':'testcommand',
    #                       'node':"locus102",
    #                       'job':{}}
    #        # validate correct content
    #        self.assertEqual(expected_payload, daemon._adapted_content)


    #def test_not_implemented_exception_when_calling_superclass(self):
    #    """
    #    The daemon should always be subclasses. The process command should raise
    #    a not implemented exception when used
    #    """
    #    self.assertRaises(NotImplementedError,
    #                      CQDaemon.CQDaemon, 
    #                      *[None]*8   # Just pass a list with 8 none s as args
    #                      )
            
          
    def test_silent_eating_of_incorrect_commands(self):
        """
        The Command queue daemon should always continue working, even in the case 
        of unknown of incorrect commands
        Send a number of broken msg to the command queue
        """
        daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
            prepare_test(subclassedMCQDaemonFalse,
                             self.logfile, self.deadletterfile )

        with nested(daemon, commandQueueBus, 
                    deadletterQueue, deadletterToQueue) as (
                      daemon, commandQueueBus, deadletterQueue, deadletterToQueue):

            # Excercise the SUT 
            # ****************************************
            # Test 1: incorrect command
            payload = {'command':'incorrect',  
                           'node':'locus102',
                           'job':{}}
            msg = create_test_msg(payload)
            commandQueueBus.send(msg)

            # Exercise sut
            daemon._process_command_queue() # Start the processing on the sut

            # Deadletter content example:
            #START DEADLETTER
            #2015-07-16 09:57:29.334901 <class '__main__.subclassedMCQDaemonFalse'>
            #[67b103d3-5f4e-49a8-b8a1-071366aecc97] [sasid ] summary
            #{'node': 'locus102', 'job': {}, 'command': 'incorrect'}
            #END DEADLETTER

            # We expect a deadletter on the 
            content = None
            with open( self.deadletterfile , 'r') as file:
                content = file.readlines()

            self.assertEqual(content[0],"START DEADLETTER\n")
            self.assertTrue("subclassedMCQDaemonFalse" in content[1] )
            self.assertEqual(content[3],
                      "{'node': 'locus102', 'job': {}, 'command': 'incorrect'}\n")
            self.assertEqual(content[4],"END DEADLETTER\n")
            return 

# TODO: Add test of additional functionality

# ******************** helper functions ******************
def prepare_test(subclass, logfile, deadletterfile):
    """
    Hides boiler plate code

    return the deamon and needed
    """
        # config
    broker =  "locus102"
    busname = "testmcqdaemon"  # TODO: Use a different name
    #busname = "testbus"
    masterCommandQueueName = busname + "/" + "masterCommandQueueName"
    #masterCommandQueueName = "masterCommandQueueName"
    deadLetterQueueName = busname + ".deadletter"
    # create the sut
    daemon = subclass(broker, busname, masterCommandQueueName,
                      deadLetterQueueName, deadletterfile,logfile,
                      1, False)

    # connect to the queueus
    commandQueueBus =get_to_bus(masterCommandQueueName, broker)

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

def get_to_bus(masterCommandQueueName, broker):
    """
    Creates a command queue to bus
    """
    commandQueueBus = msgbus.ToBus(
                   masterCommandQueueName,
              broker = broker)

    return commandQueueBus

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


       


