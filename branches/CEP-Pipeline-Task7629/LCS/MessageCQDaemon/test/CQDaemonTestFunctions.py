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


import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
import time

# ******************** helper function ******************
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
 