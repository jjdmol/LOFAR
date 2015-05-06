# import lofar.messagebus.MCQDaemon as MCQ  # communicate using the lib

import uuid
import copy
import os
import logging
import time
import threading 
import pwd
import socket  # needed for username TODO: is misschien een betere manier os.environ['USER']
import signal
import sys

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
from qmf.console import Session as QMFSession

import lofar.messagebus.MCQLib as MCQLib

# programmatically interact with the qpid broker
from qpid.messaging import Connection	
from qpidtoollibs import BrokerAgent


# Define logging. Until we have a python loging framework, we'll have
# to do any initialising here
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)
logger=logging.getLogger("testBrokerConsumercount")

if __name__ == "__main__":

    # create the connection the broker agent (on local host)
    broker = "127.0.0.1" 
    connection = Connection.establish(broker)
    brokerAgent = BrokerAgent(connection)

        # connect a queue:
    fromQueueName = "testBrokerConsumercount"
    fromQueue = msgbus.FromBus(fromQueueName, 
            options = "create:always, delete:always, node: { type: queue, durable: False}",
            broker = broker)

    print "after broker agent stuff"
    # get the registered queue objects
    queues = brokerAgent.getAllQueues()
    topics = brokerAgent.getAllExchanges()


    # attempt to read the number of consumers
    brokerAgentQueueObject = None
    for queue in queues:
        if queue.name == fromQueueName:
            brokerAgentQueueObject = queue

    # sanity check.
    if brokerAgentQueueObject == None:
        print "Coult not find queue object on the broker. aborting"
        sys.exit(1)

    # print the number of found consumers (should be 1)
    print "Number of consumer: {0}".format(
                     brokerAgentQueueObject.consumerCount)
    

