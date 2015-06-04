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

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
import lofar.messagebus.MCQDaemon as MCQDaemon


# Define logging. Until we have a python loging framework, we'll have
# to do any initialising here
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("testMCQDaemon")


# **********************************
# MCQDaemon is a template class for testing we need to instantiate it
class subclassedMCQDaemonFalse(MCQDaemon.MCQDaemon):
    """
    Shallow wrapper around the MCQDaemon.

    MCQDaemon is an abstract class and should always ben subclassed.
    Only a two function MUST be implemented process_commands 
    """
    def __init__(self, *args, **kwargs):
        super(subclassedMCQDaemonFalse, self).__init__(*args, **kwargs)
    
    def process_commands(self, command, unpacked_msg_content, msg):
        return False

class subclassedMCQDaemontestcommand(MCQDaemon.MCQDaemon):
    """
    Shallow wrapper around the MCQDaemon.

    if the received command is testcommand forward adapted msg to slave queue
    return true
    """
    def __init__(self, *args, **kwargs):
        super(subclassedMCQDaemontestcommand, self).__init__(*args, **kwargs)
    
    def process_commands(self, command, unpacked_msg_content, msg):

        if command == "testcommand":
            adapted_content = unpacked_msg_content
            adapted_content['testcommand']="received"

            self._process_run_job(adapted_content)
            return True
        return false

# **********************************************************
# from here the test function
def test_subclass_processing():
    """
    Test if the subclass process_commands is called  
    """
    job_node = 'locus102'
    daemon, commandQueueBus, slaveCommandQueueBus,deadletterQueue, deadletterToQueue = \
        prepare_test(job_node, subclassedMCQDaemontestcommand)

    # Test1: Create a test job payuoad
    send_payload =  {'command':'testcommand',
                   'node':job_node,
                   'job':{}}

    msg = create_test_msg(send_payload)
    commandQueueBus.send(msg)

    # start the daemon processing
    daemon._process_commands()
  

    # validate that a job is received on the slave queue

    # wait on the slave command queue
    msg_received = try_get_msg(slaveCommandQueueBus)

    # unpack received data
    received_payload = eval(msg_received.content().payload)

    expected_payload = {'testcommand':"received",
                   'command':'testcommand',
                   'node':job_node,
                   'job':{}}
    # validate correct content
    if expected_payload != received_payload:
        raise Exception("Send data not the same as received data")

    # Cleanup sut
    commandQueueBus.close()
    slaveCommandQueueBus.close()
    deadletterQueue.close()
    daemon.close()

def test_not_implemented_exception_when_calling_superclass():
    """
    The daemon should always be subclasses. The process command should raise
    a not implemented exception when used
    """
    job_node = 'locus102'
    daemon, commandQueueBus, slaveCommandQueueBus,deadletterQueue,deadletterToQueue = \
        prepare_test(job_node, MCQDaemon.MCQDaemon)


    # Test1: Create a test job payuoad
    send_payload =  {'command':'run_job',
                   'node':job_node,
                   'job':{}}

    msg = create_test_msg(send_payload)
    commandQueueBus.send(msg)

    # start the daemon processing
    try:
        daemon._process_commands()
    except NotImplementedError, Exception:
        pass
    else:
        raise Exception("The daemon should throw an exception when not subclassed")
  
    # Cleanup sut
    commandQueueBus.close()
    slaveCommandQueueBus.close()
    deadletterQueue.close()
    daemon.close()
           
def test_silent_eating_of_incorrect_commands():
    """
    The Command queue daemon should always continue working, even in the case 
    of unknown of incorrect commands
    Send a number of broken msg to the command queue
    """
    job_node = 'locus102'
    daemon, commandQueueBus, slaveCommandQueueBus,deadletterQueue, deadletterToQueue = \
        prepare_test(job_node, subclassedMCQDaemonFalse)

    # Excercise the SUT 
    # ****************************************
    # Test 1: incorrect command
    payload = {'command':'incorrect',  
                   'node':'locus102',
                   'job':{}}
    msg = create_test_msg(payload)
    commandQueueBus.send(msg)

    # Exercise sut
    daemon._process_commands() # Start the processing on the sut

    # We expect a deadletter on the 
    idx = 0
    msg_received = try_get_msg(deadletterQueue)
    
    received_payload = eval(msg_received.content().payload)
    if payload != received_payload:
        raise Exception("Did not receive the correct msg on the deadletterq")
    
    # ****************************************
    # test 2: No payload
    send_payload = "Some text"
    msg = create_test_msg(send_payload)
    commandQueueBus.send(msg)

    # Exercise sut
    daemon._process_commands()

    # check the deadletter queue
    msg_received = try_get_msg(deadletterQueue)

    # validate the content
    received_payload = msg_received.content().payload
    if send_payload != received_payload:
         raise Exception("Did not receive the correct msg on the deadletterq")

    # clear the queueus
    commandQueueBus.close()
    deadletterQueue.close()
    daemon.close()

def test_forwarding_of_job_msg_to_queue():
    """
    A msg with the command run_job should be forwarded to jobnode
    """
    job_node = 'locus102'
    daemon, commandQueueBus, slaveCommandQueueBus,deadletterQueue, deadletterToQueue = \
        prepare_test(job_node, subclassedMCQDaemonFalse)


    # Test1: Create a test job payuoad
    send_payload =  {'command':'run_job',
                   'node':job_node,
                   'job':{}}

    msg = create_test_msg(send_payload)
    commandQueueBus.send(msg)

    # start the daemon processing
    daemon._process_commands()
  

    # validate that a job is received on the slave queue
    # wait on the slave command queue
    msg_received = try_get_msg(slaveCommandQueueBus)

    # unpack received data
    received_payload = eval(msg_received.content().payload)

    # validate correct content
    if received_payload != send_payload:
        raise Exception("Send data not the same as received data")

    # Cleanup sut
    commandQueueBus.close()
    slaveCommandQueueBus.close()
    deadletterQueue.close()
    daemon.close()


def test_default_process_deadletter_queue():
    """
    The Command queue daemon should always continue working, even in the case 
    of unknown of incorrect commands
    Send a number of broken msg to the command queue
    """
    job_node = 'locus102'
    broker =  "locus102"
    busname = "testmcqdaemon"

    masterCommandQueueName = busname + "/" + "masterCommandQueueName"
    slaveCommandQueueName = busname + "/" + job_node 
    deadLetterQueueName = busname + ".proxy.deadletter"
    # create the sut
    daemon = subclassedMCQDaemonFalse(broker, busname, masterCommandQueueName,
                                deadLetterQueueName, 1, False)

    # connect to the queueus
    commandQueueBus =get_to_bus(masterCommandQueueName, broker)
    slaveCommandQueueBus = get_from_bus(slaveCommandQueueName,
                                                 broker)

    deadletterToQueue = get_to_bus(deadLetterQueueName,
                                                 broker)


    # Excercise the SUT 
    # ****************************************
    # Test 1: incorrect command
    payload = {'command':'incorrect',  
                   'node':'locus102',
                   'job':{}}


    msg = create_test_msg(payload)
    deadletterToQueue.send(msg)

    # Exercise sut
    # first close the deadletterqueue otherwise it is received also

    daemon._process_deadletter_queue() # Start the processing on the sut

    # check the deadletter queue
    #Connect to the deadletter queue to check for unread msg
    deadletterQueue = get_from_bus(deadLetterQueueName,
                                                 broker)

    try:
        msg_received = try_get_msg(deadletterQueue, wait_period=2)
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

# ******************** helper function ******************
def prepare_test(job_node, subclass):
    """
    Hides boiler plate code

    return the deamon and needed
    """
        # config
    broker =  "locus102"
    busname = "testmcqdaemon"
    #busname = "testbus"
    masterCommandQueueName = busname + "/" + "masterCommandQueueName"
    #masterCommandQueueName = "masterCommandQueueName"
    slaveCommandQueueName = busname + "/" + job_node 
    deadLetterQueueName = busname + ".proxy.deadletter"
    # create the sut
    daemon = subclass(broker, busname, masterCommandQueueName,
                                deadLetterQueueName, 1, False)

    # connect to the queueus
    commandQueueBus =get_to_bus(masterCommandQueueName, broker)
    slaveCommandQueueBus = get_from_bus(slaveCommandQueueName,
                                                 broker)
    deadletterQueue = get_from_bus(deadLetterQueueName,
                                                 broker)

    deadletterToQueue = get_to_bus(deadLetterQueueName,
                                                 broker)


    return daemon, commandQueueBus, slaveCommandQueueBus, deadletterQueue, deadletterToQueue



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

def get_from_bus(slaveCommandQueueName, broker):
    """
    Helper function, creates validated frombus connected on the expected
    slave bus name
    """

    slaveCommandQueueBus = None
    try:
        slaveCommandQueueBus = msgbus.FromBus(slaveCommandQueueName,
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
    print "test_not_implemented_exception_when_calling_superclass"
    test_not_implemented_exception_when_calling_superclass()
    print "test_silent_eating_of_incorrect_commands()"
    test_silent_eating_of_incorrect_commands()
    print "test_forwarding_of_job_msg_to_queue()"    
    test_forwarding_of_job_msg_to_queue()
    print "test_subclass_processing()"
    test_subclass_processing()
    print "test_default_process_deadletter_queue()"
    test_default_process_deadletter_queue()



       

