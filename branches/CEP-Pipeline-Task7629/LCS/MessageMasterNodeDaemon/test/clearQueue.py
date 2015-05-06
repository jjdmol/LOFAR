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


if __name__ == "__main__":

    queueToClear = sys.argv[1]
    # create the connection the broker agent (on local host)
    broker = "127.0.0.1" 

    # connect a queue:
    fromQueue = msgbus.FromBus(queueToClear, 
            options = "create:always, node: { type: queue, durable: False}",
            broker = broker)

    while True:
        # get is blocking, always use timeout.
        msg = fromQueue.get(0.2)  
        if msg == None:
            break   # break the loop
               
        fromQueue.ack(msg)

