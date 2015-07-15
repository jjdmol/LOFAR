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
import CQDaemonTestFunctions as testFunctions

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

    def test_subclass_processing(self):
        """
        Test if the subclass process_commands is called  
        """
        daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
            testFunctions.prepare_test(subclassedMCQDaemontestcommand,
                             self.logfile, self.deadletterfile )

        with nested(daemon, commandQueueBus, 
                    deadletterQueue, deadletterToQueue) as (
                      daemon, commandQueueBus, deadletterQueue, deadletterToQueue):
            # Test1: Create a test job payuoad
            send_payload =  {'type':'command',
                           'command':'testcommand',
                           'node':"locus102",
                           'job':{}}

            msg = testFunctions.create_test_msg(send_payload)
            commandQueueBus.send(msg)

            # start the daemon processing
            daemon._process_command_queue()
  

            # validate that a job is processed on the subclass
            expected_payload = { 'type':'command',
                        'testcommand':"received",
                           'command':'testcommand',
                           'node':"locus102",
                           'job':{}}
            # validate correct content
            self.assertEqual(expected_payload, daemon._adapted_content)


    def test_not_implemented_exception_when_calling_superclass(self):
        """
        The daemon should always be subclasses. The process command should raise
        a not implemented exception when used
        """
        self.assertRaises(NotImplementedError,
                          CQDaemon.CQDaemon, 
                          *[None]*8   # Just pass a list with 8 none s as args
                          )
            
        


          
    #def test_silent_eating_of_incorrect_commands(self):
    #    """
    #    The Command queue daemon should always continue working, even in the case 
    #    of unknown of incorrect commands
    #    Send a number of broken msg to the command queue
    #    """
    #    daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
    #        testFunctions.prepare_test( subclassedMCQDaemonFalse,
    #                         self.logfile, self.deadletterfile )

    #    # Excercise the SUT 
    #    # ****************************************
    #    # Test 1: incorrect command
    #    payload = {'command':'incorrect',  
    #                   'node':'locus102',
    #                   'job':{}}
    #    msg = testFunctions.create_test_msg(payload)
    #    commandQueueBus.send(msg)

    #    # Exercise sut
    #    daemon._process_commands() # Start the processing on the sut

    #    # We expect a deadletter on the 
    #    idx = 0
    #    msg_received = testFunctions.try_get_msg(deadletterQueue)
    
    #    received_payload = eval(msg_received.content().payload)
    #    if payload != received_payload:
    #        raise Exception("Did not receive the correct msg on the deadletterq")
    
    #    # ****************************************
    #    # test 2: No payload
    #    send_payload = "Some text"
    #    msg = testFunctions.create_test_msg(send_payload)
    #    commandQueueBus.send(msg)

    #    # Exercise sut
    #    daemon._process_commands()

    #    # check the deadletter queue
    #    msg_received = testFunctions.try_get_msg(deadletterQueue)

    #    # validate the content
    #    received_payload = msg_received.content().payload
    #    if send_payload != received_payload:
    #         raise Exception("Did not receive the correct msg on the deadletterq")

    #    # clear the queueus
    #    commandQueueBus.close()
    #    deadletterQueue.close()
    #    daemon.close()


    #def test_default_process_deadletter_queue(self):
    #    """
    #    The Command queue daemon should always continue working, even in the case 
    #    of unknown of incorrect commands
    #    Send a number of broken msg to the command queue
    #    """
    #    broker =  "locus102"
    #    busname = "testmcqdaemon"

    #    masterCommandQueueName = busname + "/" + "masterCommandQueueName"
    #    deadLetterQueueName = busname + ".proxy.deadletter"

    #    # create the sut
    #    daemon = subclassedMCQDaemonFalse(broker, busname, masterCommandQueueName,
    #                     deadLetterQueueName, self.logfile,self.deadletterfile, 
    #                                1, False)

    #    # connect to the queueus
    #    commandQueueBus = testFunctions.get_to_bus(masterCommandQueueName, broker)

    #    deadletterToQueue = testFunctions.get_to_bus(deadLetterQueueName,
    #                                                 broker)


    #    # Excercise the SUT 
    #    # ****************************************
    #    # Test 1: incorrect command
    #    payload = {'command':'incorrect',  
    #                   'node':'locus102',
    #                   'job':{}}


    #    msg = testFunctions.create_test_msg(payload)
    #    deadletterToQueue.send(msg)

    #    # Exercise sut: In this instance we want the daemon to process the deadletter
    #    # queue
    #    daemon._process_deadletter_queue() 

    #    # check the deadletter queue, this should now be empty!!!
    #    #Connect to the deadletter queue to check for unread msg
    #    deadletterQueue = testFunctions.get_from_bus(deadLetterQueueName,
    #                                                 broker)

    #    try:
    #        msg_received = testFunctions.try_get_msg(deadletterQueue, wait_period=2)
    #        print eval(msg_received.content().payload)
    #    except IOError:  # we expect an IO exception!!
    #        pass
    #    else:
    #        raise Exception("received an unexpected msg on the deadletter queue")

    #    # clear the queueus
    #    commandQueueBus.close()
    #    deadletterQueue.close()
    #    deadletterToQueue.close()
    #    daemon.close()




if __name__ == "__main__":
    unittest.main()


       

