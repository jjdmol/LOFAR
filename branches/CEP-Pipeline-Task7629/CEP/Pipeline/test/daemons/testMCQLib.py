# import lofar.messagebus.MCQDaemon as MCQ  # communicate using the lib

#import uuid
#import copy
#import os
#import logging
#import time
#import threading 
#import pwd
#import socket  # needed for username TODO: is misschien een betere manier os.environ['USER']
#import signal
import time
import unittest

import lofar.messagebus.msgbus as msgbus
import lofar.messagebus.message as message

import lofarpipe.daemons.MCQLib as MCQLib
import logging
logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s', level=logging.INFO)

def create_msg(payload):
    """
    TODO: should be moved into a shared code lib
    Creates a minimal valid msg with payload
    """
    msg = message.MessageContent(
                    from_="test",
                    forUser="",
                    summary="summary",
                    protocol="protocol",
                    protocolVersion="test", 
                    #momid="",
                    #sasid="", 
                    #qpidMsg=None
                          )
    msg.payload = payload
    return msg

class testMCQLib(unittest.TestCase):

    def __init__(self, arg):  
        super(testMCQLib, self).__init__(arg)

    # For now leave the setup and tearDown empty: single test
    # when the number of test increased it is an idea to implement them
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_init(self):
        busname = "testmcqdaemon"
        broker = "locus102"
        logger = logging.getLogger("testMCQLib")

        mcqlib = MCQLib.MCQLib(logger, broker, busname)

    def test_logtopic_handler(self):
        # TODO: Add test that the loglines are send to logging:
        #      test the _process_log_message member
        busname = "testmcqdaemon"
        broker = "locus102"
        logger = logging.getLogger("testMCQLib")
        logTopicName = busname + "/logtopichandler"
        
        #Wrapper to catch the call to the content handler
        class logTopicHandlerWrapper(MCQLib.logTopicHandler):
            def __init__(self, broker,logTopicName, logger=None,  poll_interval=1.0):  
                super(logTopicHandlerWrapper, self).__init__(
                           broker,logTopicName, logger,  poll_interval)

                self._process_log_message_called = False
                self._log_payload = None

            def _process_log_message(self, msg_content):
                self._process_log_message_called = True
                self._log_payload = msg_content

        
         # create wrapped handler
        logHandler = logTopicHandlerWrapper(broker, logTopicName, logger,1.0)
        
        logHandler.start()  # start the thread
        
        # create a to bus and send log msg        
        logToBus = msgbus.ToBus(logTopicName, broker = broker)

        payload = {'level': 'ERROR',
                   'sender': 'STATIC HOSTNAME',
                   'log_data': 'Send using qpid'}
        msg = create_msg(payload)
        logToBus.send(msg)

        # allow some time for the msg to arrive
        time.sleep(0.1)  
        self.assertEqual(logHandler._log_payload, payload)
        logHandler.setStopFlag()


    #def test_logtopic_handler(self):
    #    # TODO: Add test that the loglines are send to logging:
    #    #      test the _process_log_message member
    #    busname = "testmcqdaemon"
    #    broker = "locus102"
    #    logger = logging.getLogger("testMCQLib")
    #    logTopicName = busname + "/resultsqueuehandler"
        
    #    #Wrapper to catch the call to the handler functioinality
    #    class resultQueueHandlerrWrapper(MCQLib.logTopicHandler):
    #        def __init__(self,broker, resultQueueName, 
    #             running_jobs, 
    #             running_jobs_lock, 
    #             logger=None, 
    #             poll_interval=1.0):
    #            super(resultQueueHandlerrWrapper, self).__init__(
    #                      broker,resultQueueName,
    #                      running_jobs, running_jobs_lock,
    #                      logger,  poll_interval)

    #            self._process_queue_message_called = False
    #            self._msg_payload = None

    #        def _process_queue_message(self, msg_content):
    #            self._process_queue_message_called = True
    #            self._msg_payload = msg_content

        
    #     # create wrapped handler
    #    logHandler = logTopicHandlerWrapper(broker, logTopicName, logger,1.0)
        
    #    logHandler.start()  # start the thread
        
    #    # create a to bus and send log msg        
    #    logToBus = msgbus.ToBus(logTopicName, broker = broker)

    #    payload = {'level': 'ERROR',
    #               'sender': 'STATIC HOSTNAME',
    #               'log_data': 'Send using qpid'}
    #    msg = create_msg(payload)
    #    logToBus.send(msg)

    #    # allow some time for the msg to arrive
    #    time.sleep(0.1)  
    #    self.assertEqual(logHandler._log_payload, payload)
    #    logHandler.setStopFlag()


if __name__ == "__main__":
    unittest.main()  

    #username = pwd.getpwuid(os.getuid()).pw_name
    #topicName = "Test.NCQLib.{0}".format(username)
    #broker = "127.0.0.1" 
    #hostname = "localhost"
    #logger = logging.getLogger("MCQLib")


    #running_jobs = {}
    #running_jobs_lock = threading.Lock()
    #queuHandler = MCQLib.resultQueueHandler("testQueue",running_jobs,
    #                                 running_jobs_lock, logger)




    ##fromTopic = msgbus.FromBus(topicName, 
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
