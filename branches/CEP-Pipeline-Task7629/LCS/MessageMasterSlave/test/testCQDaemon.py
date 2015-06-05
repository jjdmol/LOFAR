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

import logging
import time

import lofar.messagebus.CQDaemon as CQDaemon
import CQDaemonTestFunctions as testFunctions

# Define logging. Until we have a python loging framework, we'll have
# to do any initialising here
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("testMCQDaemon")


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
    
    def process_commands(self, command, unpacked_msg_content, msg):
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
    
    def process_commands(self, command, unpacked_msg_content, msg):

        if command == "testcommand":
            adapted_content = unpacked_msg_content
            adapted_content['testcommand']="received"
            # Just add as a member
            self._adapted_content = adapted_content
            return True
        return false

# **********************************************************
# from here the test function
def test_subclass_processing():
    """
    Test if the subclass process_commands is called  
    """
    daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
        testFunctions.prepare_test( subclassedMCQDaemontestcommand)

    # Test1: Create a test job payuoad
    send_payload =  {'command':'testcommand',
                   'node':"locus102",
                   'job':{}}

    msg = testFunctions.create_test_msg(send_payload)
    commandQueueBus.send(msg)

    # start the daemon processing
    daemon._process_commands()
  

    # validate that a job is processed on the subclass
    expected_payload = {'testcommand':"received",
                   'command':'testcommand',
                   'node':"locus102",
                   'job':{}}
    # validate correct content
    if expected_payload != daemon._adapted_content:
        raise Exception("Send data not the same as received data")

    # Cleanup sut
    commandQueueBus.close()
    deadletterQueue.close()
    daemon.close()

def test_not_implemented_exception_when_calling_superclass():
    """
    The daemon should always be subclasses. The process command should raise
    a not implemented exception when used
    """
    try:
        daemon, commandQueueBus, deadletterQueue,deadletterToQueue = \
            testFunctions.prepare_test( CQDaemon.CQDaemon)
    except NotImplementedError, Exception:
        pass # correctly thrown NotImplemented exception
    else:
        raise Exception(
          "CQDaemon should raise an exception when initiated from baseclass")


          
def test_silent_eating_of_incorrect_commands():
    """
    The Command queue daemon should always continue working, even in the case 
    of unknown of incorrect commands
    Send a number of broken msg to the command queue
    """
    daemon, commandQueueBus, deadletterQueue, deadletterToQueue = \
        testFunctions.prepare_test( subclassedMCQDaemonFalse)

    # Excercise the SUT 
    # ****************************************
    # Test 1: incorrect command
    payload = {'command':'incorrect',  
                   'node':'locus102',
                   'job':{}}
    msg = testFunctions.create_test_msg(payload)
    commandQueueBus.send(msg)

    # Exercise sut
    daemon._process_commands() # Start the processing on the sut

    # We expect a deadletter on the 
    idx = 0
    msg_received = testFunctions.try_get_msg(deadletterQueue)
    
    received_payload = eval(msg_received.content().payload)
    if payload != received_payload:
        raise Exception("Did not receive the correct msg on the deadletterq")
    
    # ****************************************
    # test 2: No payload
    send_payload = "Some text"
    msg = testFunctions.create_test_msg(send_payload)
    commandQueueBus.send(msg)

    # Exercise sut
    daemon._process_commands()

    # check the deadletter queue
    msg_received = testFunctions.try_get_msg(deadletterQueue)

    # validate the content
    received_payload = msg_received.content().payload
    if send_payload != received_payload:
         raise Exception("Did not receive the correct msg on the deadletterq")

    # clear the queueus
    commandQueueBus.close()
    deadletterQueue.close()
    daemon.close()


def test_default_process_deadletter_queue():
    """
    The Command queue daemon should always continue working, even in the case 
    of unknown of incorrect commands
    Send a number of broken msg to the command queue
    """
    broker =  "locus102"
    busname = "testmcqdaemon"

    masterCommandQueueName = busname + "/" + "masterCommandQueueName"
    deadLetterQueueName = busname + ".proxy.deadletter"
    # create the sut
    daemon = subclassedMCQDaemonFalse(broker, busname, masterCommandQueueName,
                                deadLetterQueueName, 1, False)

    # connect to the queueus
    commandQueueBus = testFunctions.get_to_bus(masterCommandQueueName, broker)

    deadletterToQueue = testFunctions.get_to_bus(deadLetterQueueName,
                                                 broker)


    # Excercise the SUT 
    # ****************************************
    # Test 1: incorrect command
    payload = {'command':'incorrect',  
                   'node':'locus102',
                   'job':{}}


    msg = testFunctions.create_test_msg(payload)
    deadletterToQueue.send(msg)

    # Exercise sut: In this instance we want the daemon to process the deadletter
    # queue
    daemon._process_deadletter_queue() 

    # check the deadletter queue, this should now be empty!!!
    #Connect to the deadletter queue to check for unread msg
    deadletterQueue = testFunctions.get_from_bus(deadLetterQueueName,
                                                 broker)

    try:
        msg_received = testFunctions.try_get_msg(deadletterQueue, wait_period=2)
        print eval(msg_received.content().payload)
    except IOError:  # we expect an IO exception!!
        pass
    else:
        raise Exception("received an unexpected msg on the deadletter queue")

    # clear the queueus
    commandQueueBus.close()
    deadletterQueue.close()
    deadletterToQueue.close()
    daemon.close()




if __name__ == "__main__":
    print "test_not_implemented_exception_when_calling_superclass"
    test_not_implemented_exception_when_calling_superclass()
    print "test_silent_eating_of_incorrect_commands()"
    test_silent_eating_of_incorrect_commands()
    print "test_subclass_processing()"
    test_subclass_processing()
    print "test_default_process_deadletter_queue()"
    test_default_process_deadletter_queue()



       

