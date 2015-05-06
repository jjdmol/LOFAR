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

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message
from qmf.console import Session as QMFSession

import lofar.messagebus.MCQLib as MCQLib
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)


if __name__ == "__main__":
    username = pwd.getpwuid(os.getuid()).pw_name
    topicName = "Test.NCQLib.{0}".format(username)
    broker = "127.0.0.1" 
    hostname = "localhost"
    logger = logging.getLogger("MCQLib")


    running_jobs = {}
    running_jobs_lock = threading.Lock()
    queuHandler = MCQLib.resultQueueHandler("testQueue",running_jobs,
                                     running_jobs_lock, logger)




    #fromTopic = msgbus.FromBus(topicName, 
    #          options = "create:always, node: { type: topic, durable: False}",
    #          broker = broker)

    #NCQLibObject = NCQLib.NCQLib("TempReturnQueueForTest",
    #      topicName,
    #      "NCQDaemon.klijn.parameters.b6008d4888d7437aa13d5c2e7237343c")

    #print dir(fromTopic.connection)
    #print fromTopic.connection.check_closed()
    #print str(fromTopic.connection.sessions)

    #qpidLoggerHandler = NCQLib.QPIDLoggerHandler(topicName)

    #logger = logging.getLogger("NCQDaemon")
    #logger.propagate = False
    ## remove the default handler
    #loghandlers = logger.handlers[:]
    #print loghandlers
    #for hdlr in loghandlers:  
    #    logger.removeHandler(hdlr) 

    #logger.addHandler(NCQLibObject.QPIDLoggerHandler)
    #logger.propagate = False
    #print "before logging"

    #logger.error("Send using qpid")

    #print "after logging"

    #msg = fromTopic.get(2)
    #if not msg == None:
    #    msg_content = eval(msg.content().payload)

    #    print msg_content
    #else:
    #    print 'queue failed'

    #msg = NCQLibObject._parameterQueue.get(0.1) 

    #print msg.content().payload
